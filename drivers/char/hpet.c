/*
 * Intel & MS High Precision Event Timer Implementation.
 *
 * Copyright (C) 2003 Intel Corporation
 *	Venki Pallipadi
 * (c) Copyright 2004 Hewlett-Packard Development Company, L.P.
 *	Bob Picco <robert.picco@hp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/major.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/sysctl.h>
#include <linux/wait.h>
#include <linux/bcd.h>
#include <linux/seq_file.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/clocksource.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/acpi.h>
#include <linux/hpet.h>
#include <asm/current.h>
#include <asm/irq.h>
#include <asm/div64.h>

/*
 * The High Precision Event Timer driver.
 * This driver is closely modelled after the rtc.c driver.
 * http://www.intel.com/hardwaredesign/hpetspec_1.pdf
 */
#define	HPET_USER_FREQ	(64)
#define	HPET_DRIFT	(500)

#define HPET_RANGE_SIZE		1024	/* from HPET spec */


/* WARNING -- don't get confused.  These macros are never used
 * to write the (single) counter, and rarely to read it.
 * They're badly named; to fix, someday.
 */
#if BITS_PER_LONG == 64
#define	write_counter(V, MC)	writeq(V, MC)
#define	read_counter(MC)	readq(MC)
#else
#define	write_counter(V, MC)	writel(V, MC)
#define	read_counter(MC)	readl(MC)
#endif

static u32 hpet_nhpet, hpet_max_freq = HPET_USER_FREQ;

/* This clocksource driver currently only works on ia64 */
#ifdef CONFIG_IA64
static void __iomem *hpet_mctr;

static cycle_t read_hpet(struct clocksource *cs)
{
	return (cycle_t)read_counter((void __iomem *)hpet_mctr);
}

static struct clocksource clocksource_hpet = {
	.name		= "hpet",
	.rating		= 250,
	.read		= read_hpet,
	.mask		= CLOCKSOURCE_MASK(64),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};
static struct clocksource *hpet_clocksource;
#endif

/* A lock for concurrent access by app and isr hpet activity. */
static DEFINE_SPINLOCK(hpet_lock);

static bool hpet_available;

struct hpet_dev {
	struct hpets *hd_hpets;
	struct hpet __iomem *hd_hpet;
	struct hpet_timer __iomem *hd_timer;
	unsigned long hd_ireqfreq;
	unsigned long hd_irqdata;
	wait_queue_head_t hd_waitqueue;
	struct fasync_struct *hd_async_queue;
	unsigned int hd_flags;
	unsigned int hd_irq;
#define HPET_DEV_NAME_SIZE	16
	char hd_name[HPET_DEV_NAME_SIZE];
};

struct hpets {
	struct hpets *hp_next;
	struct hpet __iomem *hp_hpet;
	unsigned long hp_hpet_phys;
	struct clocksource *hp_clocksource;
	unsigned long long hp_tick_freq;
	unsigned long hp_delta;
	unsigned int hp_ntimer;
	unsigned int hp_which;
	struct hpet_dev hp_dev[1];
};

static struct hpets *hpets;

#define	HPET_OPEN		0x0001
#define	HPET_IE			0x0002	/* interrupt enabled */
#define	HPET_PERIODIC		0x0004
#define	HPET_SHARED_IRQ		0x0008


#ifndef readq
static inline unsigned long long readq(void __iomem *addr)
{
	return readl(addr) | (((unsigned long long)readl(addr + 4)) << 32LL);
}
#endif

#ifndef writeq
static inline void writeq(unsigned long long v, void __iomem *addr)
{
	writel(v & 0xffffffff, addr);
	writel(v >> 32, addr + 4);
}
#endif

static irqreturn_t hpet_interrupt(int irq, void *data)
{
	struct hpet_dev *devp;
	unsigned long isr;

	devp = data;
	isr = 1 << (devp - devp->hd_hpets->hp_dev);

	spin_lock(&hpet_lock);

	if (devp->hd_flags & HPET_SHARED_IRQ) {

		if (!(readl(&devp->hd_hpet->hpet_isr) & isr)) {
			spin_unlock(&hpet_lock);
			return IRQ_NONE;
		}

		writel(isr, &devp->hd_hpet->hpet_isr);
	}

	/* we could race against disable ... */
	if ((readq(&devp->hd_timer->hpet_config) & Tn_INT_ENB_CNF_MASK) == 0) {
		spin_unlock(&hpet_lock);
		return IRQ_HANDLED;
	}

	devp->hd_irqdata++;

	/*
	 * For non-periodic timers, increment the accumulator.
	 * This has the effect of treating non-periodic like periodic.
	 */
	if ((devp->hd_flags & (HPET_IE | HPET_PERIODIC)) == HPET_IE) {
		unsigned long m, t, mc, base, k;
		struct hpet __iomem *hpet = devp->hd_hpet;
		struct hpets *hpetp = devp->hd_hpets;

		t = devp->hd_ireqfreq;
		m = read_counter(&devp->hd_timer->hpet_compare);
		mc = read_counter(&hpet->hpet_mc);
		/* The time for the next interrupt would logically be t + m,
		 * however, if we are very unlucky and the interrupt is delayed
		 * for longer than t then we will completely miss the next
		 * interrupt if we set t + m and an application will hang.
		 * Therefore we need to make a more complex computation assuming
		 * that there exists a k for which the following is true:
		 * k * t + base < mc + delta
		 * (k + 1) * t + base > mc + delta
		 * where t is the interval in hpet ticks for the given freq,
		 * base is the theoretical start value 0 < base < t,
		 * mc is the main counter value at the time of the interrupt,
		 * delta is the time it takes to write the a value to the
		 * comparator.
		 * k may then be computed as (mc - base + delta) / t .
		 */
		base = mc % t;
		k = (mc - base + hpetp->hp_delta) / t;
		write_counter(t * (k + 1) + base,
			      &devp->hd_timer->hpet_compare);
	}

	spin_unlock(&hpet_lock);

	wake_up_interruptible(&devp->hd_waitqueue);

	kill_fasync(&devp->hd_async_queue, SIGIO, POLL_IN);

	return IRQ_HANDLED;
}

/* called under lock, temporary releases it */
static int hpet_try_open(struct hpet_dev *devp)
{
	struct hpet_timer __iomem *timer;
	struct hpet __iomem *hpet;
	struct hpets *hpetp;
	unsigned long long v;
	unsigned long irq_flags;
	int ret;

	timer = devp->hd_timer;
	hpet = devp->hd_hpet;
	hpetp = devp->hd_hpets;

	/* Just for case: before requesting interrupt,
	 * - force timer into non-periodic mode,
	 * - ensure no match in near future,
	 * - disable interrupt generation and clear any pending interrupt */

	v = readq(&timer->hpet_config);
	v &= ~(Tn_TYPE_CNF_MASK | Tn_INT_ENB_CNF_MASK);
	writeq(v, &timer->hpet_config);

	write_counter(read_counter(&hpet->hpet_mc) - 1, &timer->hpet_compare);

	if (v & Tn_INT_TYPE_CNF_MASK) {
		devp->hd_flags |= HPET_SHARED_IRQ;
		irq_flags = IRQF_SHARED;
	} else
		irq_flags = 0;

	if (devp->hd_flags & HPET_SHARED_IRQ)
		writel(1 << (devp - hpetp->hp_dev), &hpet->hpet_isr);

	devp->hd_irqdata = 0;

	spin_unlock_irq(&hpet_lock);
	ret = request_irq(devp->hd_irq, hpet_interrupt, irq_flags,
							devp->hd_name, devp);
	if (ret < 0) {
		pr_warn("hpet%d: failed to request irq %d for comparator %d\n",
			hpetp->hp_which, devp->hd_irq,
			(int)(devp - hpetp->hp_dev));
	}
	spin_lock_irq(&hpet_lock);

	return ret;
}

static int hpet_open(struct inode *inode, struct file *file)
{
	struct hpet_dev *devp;
	struct hpets *hpetp;
	int i, ret;

	if (file->f_mode & FMODE_WRITE)
		return -EINVAL;

	if (!hpet_available)
		return -ENXIO;

	spin_lock_irq(&hpet_lock);

	ret = -EBUSY;

	for (devp = NULL, hpetp = hpets; hpetp; hpetp = hpetp->hp_next) {
		for (i = 0; i < hpetp->hp_ntimer; i++) {
			devp = &hpetp->hp_dev[i];
			if (devp->hd_flags & HPET_OPEN)
				continue;
			devp->hd_flags |= HPET_OPEN;
			ret = hpet_try_open(&hpetp->hp_dev[i]);
			if (!ret)
				goto out;
			devp->hd_flags &= ~HPET_OPEN;
		}
	}

out:
	if (!ret)
		file->private_data = devp;

	spin_unlock_irq(&hpet_lock);

	return ret;
}

static ssize_t
hpet_read(struct file *file, char __user *buf, size_t count, loff_t * ppos)
{
	DECLARE_WAITQUEUE(wait, current);
	unsigned long data;
	ssize_t retval;
	struct hpet_dev *devp;

	devp = file->private_data;
	if (!devp->hd_ireqfreq)
		return -EIO;

	if (count < sizeof(unsigned long))
		return -EINVAL;

	add_wait_queue(&devp->hd_waitqueue, &wait);

	for ( ; ; ) {
		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_irq(&hpet_lock);
		data = devp->hd_irqdata;
		devp->hd_irqdata = 0;
		spin_unlock_irq(&hpet_lock);

		if (data)
			break;
		else if (file->f_flags & O_NONBLOCK) {
			retval = -EAGAIN;
			goto out;
		} else if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			goto out;
		}
		schedule();
	}

	retval = put_user(data, (unsigned long __user *)buf);
	if (!retval)
		retval = sizeof(unsigned long);
out:
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(&devp->hd_waitqueue, &wait);

	return retval;
}

static unsigned int hpet_poll(struct file *file, poll_table * wait)
{
	unsigned long v;
	struct hpet_dev *devp;

	devp = file->private_data;

	if (!devp->hd_ireqfreq)
		return 0;

	poll_wait(file, &devp->hd_waitqueue, wait);

	spin_lock_irq(&hpet_lock);
	v = devp->hd_irqdata;
	spin_unlock_irq(&hpet_lock);

	if (v != 0)
		return POLLIN | POLLRDNORM;

	return 0;
}

#ifdef CONFIG_HPET_MMAP
#ifdef CONFIG_HPET_MMAP_DEFAULT
static int hpet_mmap_enabled = 1;
#else
static int hpet_mmap_enabled = 0;
#endif

static __init int hpet_mmap_enable(char *str)
{
	get_option(&str, &hpet_mmap_enabled);
	pr_info("HPET mmap %s\n", hpet_mmap_enabled ? "enabled" : "disabled");
	return 1;
}
__setup("hpet_mmap", hpet_mmap_enable);

static int hpet_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct hpet_dev *devp;
	unsigned long addr;

	if (!hpet_mmap_enabled)
		return -EACCES;

	devp = file->private_data;
	addr = devp->hd_hpets->hp_hpet_phys;

	if (addr & (PAGE_SIZE - 1))
		return -ENOSYS;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	return vm_iomap_memory(vma, addr, PAGE_SIZE);
}
#else
static int hpet_mmap(struct file *file, struct vm_area_struct *vma)
{
	return -ENOSYS;
}
#endif

static int hpet_fasync(int fd, struct file *file, int on)
{
	struct hpet_dev *devp;

	devp = file->private_data;

	if (fasync_helper(fd, file, on, &devp->hd_async_queue) >= 0)
		return 0;
	else
		return -EIO;
}

static int hpet_release(struct inode *inode, struct file *file)
{
	struct hpet_dev *devp;
	struct hpet_timer __iomem *timer;
	unsigned long long v;

	devp = file->private_data;
	timer = devp->hd_timer;

	spin_lock_irq(&hpet_lock);

	v = readq(&timer->hpet_config);
	v &= ~(Tn_INT_ENB_CNF_MASK | Tn_TYPE_CNF_MASK);
	writeq(v, &timer->hpet_config);

	devp->hd_flags &= ~(HPET_OPEN | HPET_IE | HPET_PERIODIC);
	spin_unlock_irq(&hpet_lock);

	free_irq(devp->hd_irq, devp);

	file->private_data = NULL;
	return 0;
}

static int hpet_ioctl_ieon(struct hpet_dev *devp)
{
	struct hpet_timer __iomem *timer;
	struct hpet __iomem *hpet;
	struct hpets *hpetp;
	unsigned long g, v, t, m;
	unsigned long flags, isr;

	timer = devp->hd_timer;
	hpet = devp->hd_hpet;
	hpetp = devp->hd_hpets;

	if (!devp->hd_ireqfreq)
		return -EIO;

	spin_lock_irq(&hpet_lock);

	if (devp->hd_flags & HPET_IE) {
		spin_unlock_irq(&hpet_lock);
		return -EBUSY;
	}
	devp->hd_flags |= HPET_IE;

	t = devp->hd_ireqfreq;
	v = readq(&timer->hpet_config);

	/* 64-bit comparators are not yet supported through the ioctls,
	 * so force this into 32-bit mode if it supports both modes
	 */
	g = v | Tn_32MODE_CNF_MASK | Tn_INT_ENB_CNF_MASK;

	if (devp->hd_flags & HPET_PERIODIC) {
		g |= Tn_TYPE_CNF_MASK;
		v |= Tn_TYPE_CNF_MASK | Tn_VAL_SET_CNF_MASK;
		writeq(v, &timer->hpet_config);
		local_irq_save(flags);

		/*
		 * NOTE: First we modify the hidden accumulator
		 * register supported by periodic-capable comparators.
		 * We never want to modify the (single) counter; that
		 * would affect all the comparators. The value written
		 * is the counter value when the first interrupt is due.
		 */
		m = read_counter(&hpet->hpet_mc);
		write_counter(t + m + hpetp->hp_delta, &timer->hpet_compare);
		/*
		 * Then we modify the comparator, indicating the period
		 * for subsequent interrupt.
		 */
		write_counter(t, &timer->hpet_compare);
	} else {
		local_irq_save(flags);
		m = read_counter(&hpet->hpet_mc);
		write_counter(t + m + hpetp->hp_delta, &timer->hpet_compare);
	}

	if (devp->hd_flags & HPET_SHARED_IRQ) {
		isr = 1 << (devp - devp->hd_hpets->hp_dev);
		writel(isr, &hpet->hpet_isr);
	}
	writeq(g, &timer->hpet_config);
	local_irq_restore(flags);

	spin_unlock_irq(&hpet_lock);

	return 0;
}

/* converts Hz to number of timer ticks */
static inline unsigned long hpet_time_div(struct hpets *hpets,
					  unsigned long dis)
{
	unsigned long long m;

	m = hpets->hp_tick_freq + (dis >> 1);
	do_div(m, dis);
	return (unsigned long)m;
}

static int
hpet_ioctl_common(struct hpet_dev *devp, int cmd, unsigned long arg,
		  struct hpet_info *info)
{
	struct hpet_timer __iomem *timer;
	struct hpet __iomem *hpet;
	struct hpets *hpetp;
	int err;
	unsigned long v;

	switch (cmd) {
	case HPET_IE_OFF:
	case HPET_INFO:
	case HPET_EPI:
	case HPET_DPI:
	case HPET_IRQFREQ:
		timer = devp->hd_timer;
		hpet = devp->hd_hpet;
		hpetp = devp->hd_hpets;
		break;
	case HPET_IE_ON:
		return hpet_ioctl_ieon(devp);
	default:
		return -EINVAL;
	}

	err = 0;

	switch (cmd) {
	case HPET_IE_OFF:
		spin_lock_irq(&hpet_lock);
		if ((devp->hd_flags & HPET_IE)) {
			v = readq(&timer->hpet_config);
			v &= ~Tn_INT_ENB_CNF_MASK;
			writeq(v, &timer->hpet_config);
			devp->hd_flags ^= HPET_IE;
		}
		spin_unlock_irq(&hpet_lock);
		break;
	case HPET_INFO:
		memset(info, 0, sizeof(*info));
		spin_lock_irq(&hpet_lock);
		if (devp->hd_ireqfreq)
			info->hi_ireqfreq =
				hpet_time_div(hpetp, devp->hd_ireqfreq);
		info->hi_flags =
			    readq(&timer->hpet_config) & Tn_PER_INT_CAP_MASK;
		info->hi_hpet = hpetp->hp_which;
		info->hi_timer = devp - hpetp->hp_dev;
		spin_unlock_irq(&hpet_lock);
		break;
	case HPET_EPI:
		spin_lock_irq(&hpet_lock);
		v = readq(&timer->hpet_config);
		if ((v & Tn_PER_INT_CAP_MASK) == 0)
			err = -ENXIO;
		else
			devp->hd_flags |= HPET_PERIODIC;
		spin_unlock_irq(&hpet_lock);
		break;
	case HPET_DPI:
		spin_lock_irq(&hpet_lock);
		v = readq(&timer->hpet_config);
		if ((v & Tn_PER_INT_CAP_MASK) == 0)
			err = -ENXIO;
		else if (devp->hd_flags & HPET_PERIODIC) {
			if (v & Tn_TYPE_CNF_MASK) {
				v ^= Tn_TYPE_CNF_MASK;
				writeq(v, &timer->hpet_config);
			}
			devp->hd_flags &= ~HPET_PERIODIC;
		}
		spin_unlock_irq(&hpet_lock);
		break;
	case HPET_IRQFREQ:
		if ((arg > hpet_max_freq) &&
		    !capable(CAP_SYS_RESOURCE)) {
			err = -EACCES;
			break;
		}

		if (!arg) {
			err = -EINVAL;
			break;
		}

		spin_lock_irq(&hpet_lock);
		devp->hd_ireqfreq = hpet_time_div(hpetp, arg);
		spin_unlock_irq(&hpet_lock);
	}

	return err;
}

static long
hpet_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hpet_info info;
	int err;

	err = hpet_ioctl_common(file->private_data, cmd, arg, &info);

	if ((cmd == HPET_INFO) && !err &&
	    (copy_to_user((void __user *)arg, &info, sizeof(info))))
		err = -EFAULT;

	return err;
}

#ifdef CONFIG_COMPAT
struct compat_hpet_info {
	compat_ulong_t hi_ireqfreq;	/* Hz */
	compat_ulong_t hi_flags;	/* information */
	unsigned short hi_hpet;
	unsigned short hi_timer;
};

static long
hpet_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hpet_info info;
	int err;

	err = hpet_ioctl_common(file->private_data, cmd, arg, &info);

	if ((cmd == HPET_INFO) && !err) {
		struct compat_hpet_info __user *u = compat_ptr(arg);
		if (put_user(info.hi_ireqfreq, &u->hi_ireqfreq) ||
		    put_user(info.hi_flags, &u->hi_flags) ||
		    put_user(info.hi_hpet, &u->hi_hpet) ||
		    put_user(info.hi_timer, &u->hi_timer))
			err = -EFAULT;
	}

	return err;
}
#endif

static const struct file_operations hpet_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = hpet_read,
	.poll = hpet_poll,
	.unlocked_ioctl = hpet_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = hpet_compat_ioctl,
#endif
	.open = hpet_open,
	.release = hpet_release,
	.fasync = hpet_fasync,
	.mmap = hpet_mmap,
};

static int hpet_is_known(struct hpet_data *hdp)
{
	struct hpets *hpetp;

	for (hpetp = hpets; hpetp; hpetp = hpetp->hp_next)
		if (hpetp->hp_hpet_phys == hdp->hd_phys_address)
			return 1;

	return 0;
}

static struct ctl_table hpet_table[] = {
	{
	 .procname = "max-user-freq",
	 .data = &hpet_max_freq,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = proc_dointvec,
	 },
	{}
};

static struct ctl_table hpet_root[] = {
	{
	 .procname = "hpet",
	 .maxlen = 0,
	 .mode = 0555,
	 .child = hpet_table,
	 },
	{}
};

static struct ctl_table dev_root[] = {
	{
	 .procname = "dev",
	 .maxlen = 0,
	 .mode = 0555,
	 .child = hpet_root,
	 },
	{}
};

static struct ctl_table_header *sysctl_header;

/*
 * Adjustment for when arming the timer with
 * initial conditions.  That is, main counter
 * ticks expired before interrupts are enabled.
 */
#define	TICK_CALIBRATE	(1000UL)

static unsigned long __hpet_calibrate(struct hpets *hpetp)
{
	struct hpet_timer __iomem *timer = NULL;
	unsigned long t, m, count, i, flags, start;
	struct hpet_dev *devp;
	int j;
	struct hpet __iomem *hpet;

	for (j = 0, devp = hpetp->hp_dev; j < hpetp->hp_ntimer; j++, devp++)
		if ((devp->hd_flags & HPET_OPEN) == 0) {
			timer = devp->hd_timer;
			break;
		}

	if (!timer)
		return 0;

	hpet = hpetp->hp_hpet;
	t = read_counter(&timer->hpet_compare);

	i = 0;
	count = hpet_time_div(hpetp, TICK_CALIBRATE);

	local_irq_save(flags);

	start = read_counter(&hpet->hpet_mc);

	do {
		m = read_counter(&hpet->hpet_mc);
		write_counter(t + m + hpetp->hp_delta, &timer->hpet_compare);
	} while (i++, (m - start) < count);

	local_irq_restore(flags);

	return (m - start) / i;
}

static unsigned long hpet_calibrate(struct hpets *hpetp)
{
	unsigned long ret = ~0UL;
	unsigned long tmp;

	/*
	 * Try to calibrate until return value becomes stable small value.
	 * If SMI interruption occurs in calibration loop, the return value
	 * will be big. This avoids its impact.
	 */
	for ( ; ; ) {
		tmp = __hpet_calibrate(hpetp);
		if (ret <= tmp)
			break;
		ret = tmp;
	}

	return ret;
}

int hpet_alloc(struct hpet_data *hdp)
{
	u64 cap, conf;
	unsigned long mask, mask2, used_mask;
	struct hpet_dev *devp;
	int i, ntimer, irq, gsi;
	struct hpets *hpetp;
	size_t siz;
	struct hpet __iomem *hpet;
	struct hpet_timer __iomem *timer;
	static struct hpets *last;
	unsigned long period;
	unsigned long long temp;
	u32 remainder;

	/*
	 * hpet_alloc can be called by platform dependent code.
	 * If platform dependent code has allocated the hpet that
	 * ACPI has also reported, then we catch it here.
	 */
	if (hpet_is_known(hdp)) {
		printk(KERN_DEBUG "%s: duplicate HPET ignored\n",
			__func__);
		return 0;
	}

	hpet = hdp->hd_address;
	cap = readq(&hpet->hpet_cap);
	ntimer = ((cap & HPET_NUM_TIM_CAP_MASK) >> HPET_NUM_TIM_CAP_SHIFT) + 1;

	siz = sizeof(struct hpets) + ((ntimer - 1) * sizeof(struct hpet_dev));
	hpetp = kzalloc(siz, GFP_KERNEL);
	if (!hpetp)
		return -ENOMEM;

	hpetp->hp_which = hpet_nhpet++;
	hpetp->hp_hpet = hdp->hd_address;
	hpetp->hp_hpet_phys = hdp->hd_phys_address;
	hpetp->hp_ntimer = ntimer;

	if (last)
		last->hp_next = hpetp;
	else
		hpets = hpetp;
	last = hpetp;


	period = (cap & HPET_COUNTER_CLK_PERIOD_MASK) >>
		HPET_COUNTER_CLK_PERIOD_SHIFT; /* fs, 10^-15 */
	temp = 1000000000000000uLL; /* 10^15 femtoseconds per second */
	temp += period >> 1; /* round */
	do_div(temp, period);
	hpetp->hp_tick_freq = temp; /* ticks per second */

	temp = hpetp->hp_tick_freq;
	remainder = do_div(temp, 1000000);

	pr_info("hpet%d: at MMIO 0x%lx, %u comparators, "
		"%d-bit %u.%06u MHz counter\n",
		hpetp->hp_which, hdp->hd_phys_address, ntimer,
		cap & HPET_COUNTER_SIZE_MASK ? 64 : 32,
		(unsigned) temp, remainder);


	/* If HPET module is not used by platform code (which means, has no
	 * platform-reserved timers), disable legacy replacement IRQ routing
	 * for it */
	if (!hdp->hd_state) {
		conf = readq(&hpet->hpet_config);
		if (conf & HPET_LEG_RT_CNF_MASK) {
			pr_info("hpet%d: disabling legacy replacement "
				"IRQ routing\n", hpetp->hp_which);
			conf &= ~HPET_LEG_RT_CNF_MASK;
			writeq(conf, &hpet->hpet_config);
		}
	}

	used_mask = 0;

	for (i = 0; i < ntimer; i++) {

		devp = &hpetp->hp_dev[i];
		timer = &hpet->hpet_timers[i];

		devp->hd_hpets = hpetp;
		devp->hd_hpet = hpet;
		devp->hd_timer = timer;

		snprintf(devp->hd_name, HPET_DEV_NAME_SIZE,
				"hpet%d.%d", hpetp->hp_which, i);

		if (hdp->hd_state & (1 << i)) {
			/* This driver does not manage platform-reserved
			 * timers */
			pr_info("hpet%d: comparator %d reserved for "
				"system needs\n", hpetp->hp_which, i);
			devp->hd_flags = HPET_OPEN;
			continue;
		}

		/* Find out mask of IRQs applicable for this timer */

		conf = readq(&timer->hpet_config);
		mask = (conf & Tn_INT_ROUTE_CAP_MASK) >> Tn_INT_ROUTE_CAP_SHIFT;
		if (nr_irqs < 32)
			mask &= ((1 << nr_irqs) - 1);
		/* In PIC mode, blacklist IRQ0-4, IRQ6-9, IRQ12-15.
		 * In IO APIC mode, blacklist all legasy IRQs */
		if (acpi_irq_model == ACPI_IRQ_MODEL_PIC)
			mask &= ~0xf3df;
		else
			mask &= ~0xffff;

		if (i < hdp->hd_nirqs && hdp->hd_irq[i]) {
			/* ACPI provided an IRQ for this timer.
			 *
			 * Need to rule out situation when this IRQ selection
			 * is not friendly to the rest of the system (e.g.
			 * it is legacy RTC irq). To make different cases more
			 * similar, compare against the same mask as is used
			 * when we choose IRQ below.
			 *
			 * There is a sort of namespace violation here:
			 * hdp->hd_irq[i] is a result of acpi_register_gsi()
			 * call, while mask is in terms of argument to
			 * that call. However looks like in practice these are
			 * the same... FIXME.
			 */
			if ((1 << hdp->hd_irq[i]) & mask) {
				irq = gsi = hdp->hd_irq[i];
				goto setup_irq;
			}
		}

		/* First try IRQs not yet used by other timers */
		mask2 = mask & (~used_mask);
		for_each_set_bit(irq, &mask2, HPET_MAX_IRQ) {
			gsi = acpi_register_gsi(NULL, irq,
				ACPI_LEVEL_SENSITIVE, ACPI_ACTIVE_LOW);
			if (gsi > 0) {
				/* level-sensitive */
				conf |= Tn_INT_TYPE_CNF_MASK;
				goto setup_irq;
			}
		}

		/* Then try any possible IRQ */
		for_each_set_bit(irq, &mask, HPET_MAX_IRQ) {
			gsi = acpi_register_gsi(NULL, irq,
				ACPI_LEVEL_SENSITIVE, ACPI_ACTIVE_LOW);
			if (gsi > 0) {
				/* level-sensitive */
				conf |= Tn_INT_TYPE_CNF_MASK;
				goto setup_irq;
			}
		}

		/* No useable IRQ */
		pr_warn("hpet%d: no useable IRQ for "
			"comparator %d\n", hpetp->hp_which, i);
		devp->hd_flags = HPET_OPEN;
		continue;

setup_irq:
		devp->hd_irq = gsi;
		used_mask |= (1 << irq);

		conf &= ~Tn_INT_ROUTE_CNF_MASK;
		conf |= (irq << Tn_INT_ROUTE_CNF_SHIFT);
		writeq(conf, &timer->hpet_config);

		pr_info("hpet%d: comparator %d available, "
			"using irq %d\n", hpetp->hp_which, i, gsi);

		hpet_available = true;
		init_waitqueue_head(&devp->hd_waitqueue);
	}

	conf = readq(&hpet->hpet_config);
	if ((conf & HPET_ENABLE_CNF_MASK) == 0) {
		write_counter(0L, &hpet->hpet_mc);
		conf |= HPET_ENABLE_CNF_MASK;
		writeq(conf, &hpet->hpet_config);
	}

	hpetp->hp_delta = hpet_calibrate(hpetp);

/* This clocksource driver currently only works on ia64 */
#ifdef CONFIG_IA64
	if (!hpet_clocksource) {
		hpet_mctr = (void __iomem *)&hpetp->hp_hpet->hpet_mc;
		clocksource_hpet.archdata.fsys_mmio = hpet_mctr;
		clocksource_register_hz(&clocksource_hpet, hpetp->hp_tick_freq);
		hpetp->hp_clocksource = &clocksource_hpet;
		hpet_clocksource = &clocksource_hpet;
	}
#endif

	return 0;
}

static acpi_status hpet_resources(struct acpi_resource *res, void *data)
{
	struct hpet_data *hdp;
	acpi_status status;
	struct acpi_resource_address64 addr;

	hdp = data;

	status = acpi_resource_to_address64(res, &addr);

	if (ACPI_SUCCESS(status)) {
		hdp->hd_phys_address = addr.minimum;
		hdp->hd_address = ioremap(addr.minimum, addr.address_length);

		if (hpet_is_known(hdp)) {
			iounmap(hdp->hd_address);
			return AE_ALREADY_EXISTS;
		}
	} else if (res->type == ACPI_RESOURCE_TYPE_FIXED_MEMORY32) {
		struct acpi_resource_fixed_memory32 *fixmem32;

		fixmem32 = &res->data.fixed_memory32;

		hdp->hd_phys_address = fixmem32->address;
		hdp->hd_address = ioremap(fixmem32->address,
						HPET_RANGE_SIZE);

		if (hpet_is_known(hdp)) {
			iounmap(hdp->hd_address);
			return AE_ALREADY_EXISTS;
		}
	} else if (res->type == ACPI_RESOURCE_TYPE_EXTENDED_IRQ) {
		struct acpi_resource_extended_irq *irqp;
		int i, irq;

		irqp = &res->data.extended_irq;

		for (i = 0; i < irqp->interrupt_count; i++) {
			if (hdp->hd_nirqs >= HPET_MAX_TIMERS)
				break;

			irq = acpi_register_gsi(NULL, irqp->interrupts[i],
				      irqp->triggering, irqp->polarity);
			if (irq < 0)
				return AE_ERROR;

			hdp->hd_irq[hdp->hd_nirqs] = irq;
			hdp->hd_nirqs++;
		}
	}

	return AE_OK;
}

static int hpet_acpi_add(struct acpi_device *device)
{
	acpi_status result;
	struct hpet_data data;

	memset(&data, 0, sizeof(data));

	result = acpi_walk_resources(device->handle, METHOD_NAME__CRS,
				hpet_resources, &data);

	if (ACPI_FAILURE(result))
		return -ENODEV;

	if (!data.hd_address || !data.hd_nirqs) {
		if (data.hd_address)
			iounmap(data.hd_address);
		printk("%s: no address or irqs in _CRS\n", __func__);
		return -ENODEV;
	}

	return hpet_alloc(&data);
}

static int hpet_acpi_remove(struct acpi_device *device)
{
	/* XXX need to unregister clocksource, dealloc mem, etc */
	return -EINVAL;
}

static const struct acpi_device_id hpet_device_ids[] = {
	{"PNP0103", 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, hpet_device_ids);

static struct acpi_driver hpet_acpi_driver = {
	.name = "hpet",
	.ids = hpet_device_ids,
	.ops = {
		.add = hpet_acpi_add,
		.remove = hpet_acpi_remove,
		},
};

static struct miscdevice hpet_misc = { HPET_MINOR, "hpet", &hpet_fops };

static int __init hpet_init(void)
{
	int result;

	result = misc_register(&hpet_misc);
	if (result < 0)
		return -ENODEV;

	sysctl_header = register_sysctl_table(dev_root);

	result = acpi_bus_register_driver(&hpet_acpi_driver);
	if (result < 0) {
		if (sysctl_header)
			unregister_sysctl_table(sysctl_header);
		misc_deregister(&hpet_misc);
		return result;
	}

	return 0;
}

static void __exit hpet_exit(void)
{
	acpi_bus_unregister_driver(&hpet_acpi_driver);

	if (sysctl_header)
		unregister_sysctl_table(sysctl_header);
	misc_deregister(&hpet_misc);

	return;
}

module_init(hpet_init);
module_exit(hpet_exit);
MODULE_AUTHOR("Bob Picco <Robert.Picco@hp.com>");
MODULE_LICENSE("GPL");
