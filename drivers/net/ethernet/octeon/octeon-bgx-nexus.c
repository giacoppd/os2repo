/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2014 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-bgxx-defs.h>

#include "octeon-bgx.h"

static atomic_t bgx_nexus_once;
static atomic_t request_ethernet3_once;

static int bgx_probe(struct platform_device *pdev)
{
	struct bgx_platform_data platform_data;
	const __be32 *reg;
	u32 port;
	u64 addr;
	struct device_node *child;
	struct platform_device *new_dev;
	struct platform_device *pki_dev;
	static int pki_id;
	int numa_node, interface;
	int i;
	int r = 0;
	char id[64];

	/* One time initialization */
	if (atomic_cmpxchg(&bgx_nexus_once, 0, 1) == 0)
		__cvmx_helper_init_port_config_data();

	reg = of_get_property(pdev->dev.of_node, "reg", NULL);
	addr = of_translate_address(pdev->dev.of_node, reg);
	interface = (addr >> 24) & 0xf;
	numa_node = (addr >> 36) & 0x7;

	__cvmx_helper_bgx_probe(cvmx_helper_node_interface_to_xiface(numa_node, interface));

	/* Assign 8 CAM entries per LMAC */
	for (i = 0; i < 32; i++) {
		union cvmx_bgxx_cmr_rx_adrx_cam adr_cam;
		adr_cam.u64 = 0;
		adr_cam.s.id = i >> 3;
		cvmx_write_csr_node(numa_node, CVMX_BGXX_CMR_RX_ADRX_CAM(i, interface), adr_cam.u64);
	}

	for_each_available_child_of_node(pdev->dev.of_node, child) {
		union cvmx_bgxx_cmrx_config cmr_config;
		if (!of_device_is_compatible(child, "cavium,octeon-7890-bgx-port"))
			continue;
		r = of_property_read_u32(child, "reg", &port);
		if (r)
			return -ENODEV;

		/* Connect to PKI/PKO */
		cmr_config.u64 = cvmx_read_csr_node(numa_node, CVMX_BGXX_CMRX_CONFIG(port, interface));
		cmr_config.s.mix_en = 0;
		cvmx_write_csr_node(numa_node, CVMX_BGXX_CMRX_CONFIG(port, interface), cmr_config.u64);

		snprintf(id, sizeof(id), "%llx.%u.ethernet-mac", (unsigned long long)addr, port);
		new_dev = of_platform_device_create(child, id, &pdev->dev);
		if (!new_dev) {
			dev_err(&pdev->dev, "Error creating %s\n", id);
			continue;
		}
		platform_data.numa_node = numa_node;
		platform_data.interface = interface;
		platform_data.port = port;

		pki_dev = platform_device_register_data(&new_dev->dev, "ethernet-mac-pki", pki_id++,
							&platform_data, sizeof(platform_data));
		dev_info(&pdev->dev, "Created PKI %u: %p\n", pki_dev->id, pki_dev);
#ifdef CONFIG_NUMA
		new_dev->dev.numa_node = pdev->dev.numa_node;
		pki_dev->dev.numa_node = pdev->dev.numa_node;
#endif
		/* One time request driver module */
		if (atomic_cmpxchg(&request_ethernet3_once, 0, 1) == 0)
			request_module_nowait("octeon3-ethernet");
	}

	dev_info(&pdev->dev, "Probed\n");
	return 0;
}

static int bgx_remove(struct platform_device *pdev)
{
	return 0;
}

static void bgx_shutdown(struct platform_device *pdev)
{
	return;
}

static struct of_device_id bgx_match[] = {
	{
		.compatible = "cavium,octeon-7890-bgx",
	},
	{},
};
MODULE_DEVICE_TABLE(of, bgx_match);

static struct platform_driver bgx_driver = {
	.probe		= bgx_probe,
	.remove		= bgx_remove,
	.shutdown       = bgx_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= KBUILD_MODNAME,
		.of_match_table = bgx_match,
	},
};

/* Allow bgx_port driver to force this driver to load */
void bgx_nexus_load(void)
{
}
EXPORT_SYMBOL(bgx_nexus_load);

module_platform_driver(bgx_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Networks <support@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks BGX MAC Nexus driver.");
