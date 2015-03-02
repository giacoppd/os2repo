/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2014 Cavium, Inc.
 */
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/device.h>
#include <linux/percpu.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-boot-vector.h>
#include <asm/octeon/octeon-boot-info.h>
#include <asm/octeon/cvmx-app-hotplug.h>
#include <asm/octeon/cvmx-spinlock.h>

static struct cvmx_boot_vector_element *octeon_bootvector;
static void *octeon_replug_ll_raw;
asmlinkage void octeon_replug_ll(void);

static struct cvmx_app_hotplug_global *hgp;

DECLARE_PER_CPU(struct cpu, cpu_devices);

/* Need __ref to be able to call register_cpu().  This is OK as this
 * file is only compiled for HOTPLUG_CPU so the resulting call to a
 * __cpuinit function will always be valid.
 */
static ssize_t __ref plug_cpu_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int cpu, r, coreid, node;
	unsigned long flags;
	bool made_present = false;
	bool is_available = false;

	r = sscanf(buf, "%d", &cpu);

	if (r != 1 || cpu < 0 || cpu >= NR_CPUS)
		return -EINVAL;


	cpu_maps_update_begin();

	if (!cpu_present(cpu) && cpu_possible(cpu)) {
		coreid = cpu_logical_map(cpu);

		local_irq_save(flags);
		cvmx_spinlock_lock(&hgp->hotplug_global_lock);
		if (cvmx_coremask_is_core_set(&hgp->avail_coremask, coreid)) {
			is_available = true;
			cvmx_coremask_clear_core(&hgp->avail_coremask, coreid);
		}
		cvmx_spinlock_unlock(&hgp->hotplug_global_lock);
		local_irq_restore(flags);
		if (!is_available) {
			pr_notice("CPU %d is not available for plugging\n", cpu);
			goto not_available_out;
		}

		octeon_bootvector[coreid].target_ptr = (uint64_t)octeon_replug_ll_raw;
		mb();
		node = cvmx_coremask_core_to_node(coreid);
		coreid = cvmx_coremask_core_on_node(coreid);
		if (octeon_has_feature(OCTEON_FEATURE_CIU3))
			cvmx_write_csr_node(node, CVMX_CIU3_NMI, (1ull << coreid));
		else
			cvmx_write_csr(CVMX_CIU_NMI, (1 << coreid));

		set_cpu_present(cpu, true);
		made_present = true;
		pr_info("CPU %d now present\n", cpu);
	}
not_available_out:
	cpu_maps_update_done();

	if (made_present) {
		struct cpu *c = &per_cpu(cpu_devices, cpu);
		memset(c, 0, sizeof(struct cpu));
		c->hotpluggable = 1;
		r = register_cpu(c, cpu);
		if (r)
			pr_warn("unplug_cpu: register_cpu %d failed (%d)\n.", cpu, r);
	}

	return count;
}

static ssize_t unplug_cpu_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int cpu, r;
	bool made_not_present = false;
	unsigned long flags;

	r = sscanf(buf, "%d", &cpu);

	if (r != 1 || cpu < 0 || cpu >= NR_CPUS)
		return -EINVAL;

	cpu_maps_update_begin();

	if (!cpu_online(cpu) && cpu_present(cpu)) {
		pr_info("CPU %d now not present\n", cpu);
		set_cpu_present(cpu, false);
		made_not_present = true;
	}

	cpu_maps_update_done();

	if (made_not_present) {
		int coreid = cpu_logical_map(cpu);
		struct cpu *c = &per_cpu(cpu_devices, cpu);
		unregister_cpu(c);

		local_irq_save(flags);
		cvmx_spinlock_lock(&hgp->hotplug_global_lock);
		cvmx_coremask_set_core(&hgp->avail_coremask, coreid);
		cvmx_spinlock_unlock(&hgp->hotplug_global_lock);
		local_irq_restore(flags);
	}

	return count;
}

static ssize_t unplug_cpu_print(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "hello\n");
}

DEVICE_ATTR(octeon_plug, 0644, unplug_cpu_print, plug_cpu_store);
DEVICE_ATTR(octeon_unplug, 0644, unplug_cpu_print, unplug_cpu_store);

static void __init octeon_hotplug_global_init(void *arg)
{
	struct linux_app_boot_info *labi;
	cvmx_app_hotplug_global_t *hgp = arg;
	memset(hgp, 0, CVMX_APP_HOTPLUG_INFO_REGION_SIZE);

	hgp->magic_version = CVMX_HOTPLUG_MAGIC_VERSION;

	cvmx_spinlock_init(&hgp->hotplug_global_lock);

	/* Get legacy LABI data structure for initial parameters */
	labi = phys_to_virt(LABI_ADDR_IN_BOOTLOADER);

	/* Valicate signature */
	if (labi->labi_signature != LABI_SIGNATURE)
		return;

	/* Initialize available cores from LABI */
	cvmx_coremask_set64(&hgp->avail_coremask,
		(uint64_t) labi->avail_coremask);
}

static int __init unplug_cpu_init(void)
{
	unsigned long t;

	octeon_bootvector = cvmx_boot_vector_get();
	if (!octeon_bootvector) {
		pr_err("Error: Cannot allocate boot vector.\n");
		return -ENOMEM;
	}
	t = __pa_symbol(octeon_replug_ll);
	octeon_replug_ll_raw = phys_to_virt(t);

	hgp = cvmx_bootmem_alloc_named_range_once(
		CVMX_APP_HOTPLUG_INFO_REGION_SIZE,
		0x0, 1ull << 29, 0,
		CVMX_APP_HOTPLUG_INFO_REGION_NAME,
		octeon_hotplug_global_init);

	if (!hgp) {
		pr_err("Error: cvmx_bootmem_alloc_named_range_once(%s)\n",
		       CVMX_APP_HOTPLUG_INFO_REGION_NAME);
		return -ENOMEM;
	}
	return 0;
}
module_init(unplug_cpu_init);
