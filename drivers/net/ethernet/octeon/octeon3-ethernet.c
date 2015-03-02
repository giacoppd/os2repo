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
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/rculist.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-helper-pki.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-app-config.h>

#include <asm/octeon/cvmx-fpa-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>

#include "octeon-bgx.h"


/*

 First buffer:

                              +---SKB---------+
                              |               |
                              |               |
                           +--+--*data        |
                           |  |               |
                           |  |               |
                           |  +---------------+
                           |       /|\
                           |        |
                           |        |
                          \|/       |
  WQE - 128 --+-----> +-------------+-------+     -+-
              |       |    *skb ----+       |      |
              |       |                     |      |
              |       |                     |      |
    WQE_SKIP = 128    |                     |      |
              |       |                     |      |
              |       |                     |      |
              |       |                     |      |
              |       |                     |      First Skip
  WQE    -----+-----> +---------------------+      |
                      |   word 0            |      |
                      |   word 1            |      |
                      |   word 2            |      |
                      |   word 3            |      |
                      |   word 4            |      |
                      +---------------------+     -+-
                 +----+- packet link        |
                 |    |  packet data        |
                 |    |                     |
                 |    |                     |
                 |    |         .           |
                 |    |         .           |
                 |    |         .           |
                 |    +---------------------+
                 |
                 |
 Later buffers:  |
                 |
                 |
                 |
                 |
                 |
                 |            +---SKB---------+
                 |            |               |
                 |            |               |
                 |         +--+--*data        |
                 |         |  |               |
                 |         |  |               |
                 |         |  +---------------+
                 |         |       /|\
                 |         |        |
                 |         |        |
                 |        \|/       |
  WQE - 128 -----+--> +-------------+-------+     -+-
                 |    |    *skb ----+       |      |
                 |    |                     |      |
                 |    |                     |      |
                 |    |                     |      |
                 |    |                     |      LATER_SKIP = 128
                 |    |                     |      |
                 |    |                     |      |
                 |    |                     |      |
                 |    +---------------------+     -+-
                 |    |  packet link        |
                 +--> |  packet data        |
                      |                     |
                      |                     |
                      |         .           |
                      |         .           |
                      |         .           |
                      +---------------------+
 */

#define MAX_TX_QUEUE_DEPTH 512
#define SSO_INTSN_EXE 0x61
#define OCTEON3_ETH_MAX_NUMA_NODES 2
#define MAX_RX_CONTEXTS 32

#define SKB_PTR_OFFSET		0
#define SKB_AURA_OFFSET		1
#define SKB_AURA_MAGIC		0xbadc0ffee4dad000UL

#define USE_ASYNC_IOBDMA	1
#define CVMX_SCR_SCRATCH	0

/*  octeon3_napi_wrapper:	Structure containing the napi structure. This
 *				structure is added to receive contexts to
 *				increase the number of threads (napis) receiving
 *				packets.
 *
 *  napi:			Used by the kernel napi core.
 *  available:			0 = This napi instance is in use.
 *				1 = This napi instance is available.
 *  idx:			Napi index per context.
 *  cpu:			CPU this napi should run on.
 *  ctx:			Receive context this napi belongs to.
 */
struct octeon3_napi_wrapper {
	struct napi_struct	napi;
	int			available;
	int			idx;
	int			cpu;
	struct octeon3_rx	*cxt;
} ____cacheline_aligned_in_smp;

static struct octeon3_napi_wrapper
napi_wrapper[OCTEON3_ETH_MAX_NUMA_NODES][CVMX_MAX_CORES]
__cacheline_aligned_in_smp;

struct octeon3_ethernet;

struct octeon3_rx {
	struct octeon3_napi_wrapper *napiw;
	DECLARE_BITMAP(napi_idx_bitmap, CVMX_MAX_CORES);
	spinlock_t napi_idx_lock;
	struct octeon3_ethernet *parent;
	int rx_grp;
	int rx_irq;
	cpumask_t rx_affinity_hint;
};

struct octeon3_ethernet {
	struct bgx_port_netdev_priv bgx_priv; /* Must be first element. */
	struct list_head list;
	struct net_device *netdev;
	struct octeon3_rx rx_cxt[MAX_RX_CONTEXTS];
	int num_rx_cxt;
	int pki_laura;
	int pki_pkind;
	int pko_queue;
	int numa_node;
	int xiface;
	int port_index;
	int rx_buf_count;
	int tx_complete_grp;
	spinlock_t stat_lock;
	u64 last_packets;
	u64 last_octets;
	u64 last_dropped;
	atomic64_t rx_packets;
	atomic64_t rx_octets;
	atomic64_t rx_dropped;
	atomic64_t rx_errors;
	atomic64_t rx_length_errors;
	atomic64_t rx_crc_errors;
	atomic64_t tx_packets;
	atomic64_t tx_octets;
	atomic64_t tx_dropped;
	/*
	 * The following two fields need to be on a different cache line as
	 * they are updated by pko which invalidates the cache every time it
	 * updates them. The idea is to prevent other fields from being
	 * invalidated unnecessarily.
	 */
	char cacheline_pad1[CVMX_CACHE_LINE_SIZE];
	atomic64_t buffers_needed;
	atomic64_t tx_backlog;
	char cacheline_pad2[CVMX_CACHE_LINE_SIZE];
};

static DEFINE_MUTEX(octeon3_eth_init_mutex);

struct octeon3_ethernet_node;

struct octeon3_ethernet_worker {
	wait_queue_head_t queue;
	struct task_struct *task;
	struct octeon3_ethernet_node *oen;
	atomic_t kick;;
	int order;
};

struct octeon3_ethernet_node {
	bool init_done;
	bool napi_init_done;
	int next_cpu_irq_affinity;
	int numa_node;
	cvmx_fpa3_pool_t  pki_packet_pool;
	cvmx_fpa3_pool_t sso_pool;
	cvmx_fpa3_pool_t pko_pool;
	cvmx_fpa3_gaura_t sso_aura;
	cvmx_fpa3_gaura_t pko_aura;
	int tx_complete_grp;
	int tx_irq;
	cpumask_t tx_affinity_hint;
	struct octeon3_ethernet_worker workers[8];
	struct mutex device_list_lock;
	struct list_head device_list;
	DECLARE_BITMAP(napi_cpu_bitmap, CVMX_MAX_CORES);
	int napi_max_cpus;
	spinlock_t napi_alloc_lock;
};

static int recycle_skbs = 1;
module_param(recycle_skbs, int, 0644);
MODULE_PARM_DESC(recycle_skbs, "Allow recycling skbs back to fpa auras.");

static int use_tx_queues;
module_param(use_tx_queues, int, 0644);
MODULE_PARM_DESC(use_tx_queues, "Use network layer transmit queues.");

static int wait_pko_response;
module_param(wait_pko_response, int, 0644);
MODULE_PARM_DESC(use_tx_queues, "Wait for response after each pko command.");

static int num_packet_buffers = 4096;
module_param(num_packet_buffers, int, S_IRUGO);
MODULE_PARM_DESC(num_packet_buffers, "Number of packet buffers to allocate per port.");

static int packet_buffer_size = 2048;
module_param(packet_buffer_size, int, S_IRUGO);
MODULE_PARM_DESC(packet_buffer_size, "Size of each RX packet buffer.");

static int rx_contexts = 1;
module_param(rx_contexts, int, S_IRUGO);
MODULE_PARM_DESC(rx_contexts, "Number of RX threads per port.");

static struct octeon3_ethernet_node octeon3_eth_node[OCTEON3_ETH_MAX_NUMA_NODES];
static struct kmem_cache *octeon3_eth_sso_pko_cache;

/* octeon3_eth_sso_pass1_limit:	Near full TAQ can cause hang. When the TAQ
 *				(Transitory Admission Queue) is near-full, it is
 *				possible for SSO to hang.
 *				Workaround: Ensure that the sum of
 *				SSO_GRP(0..255)_TAQ_THR[MAX_THR] of all used
 *				groups is <= 1264. This may reduce single-group
 *				performance when many groups are used.
 *
 *  node:			Node to update.
 *  grp:			SSO group to update.
 */
static void octeon3_eth_sso_pass1_limit(int node, int	grp)
{
	cvmx_sso_grpx_taq_thr_t	taq_thr;
	cvmx_sso_taq_add_t	taq_add;
	int			max_thr;
	int			rsvd_thr;

	/* Ideally, we would like to divide the maximum number of TAQ buffers
	 * (1264) among the sso groups in use. However, since we don't know how
	 * many sso groups are used by code outside this driver we take the
	 * worst case approach and assume all 256 sso groups must be supported.
	 */
	max_thr = 1264 / CVMX_SSO_NUM_XGRP;
	if (max_thr < 4)
		max_thr = 4;
	rsvd_thr = max_thr - 1;

	/* Changes to CVMX_SSO_GRPX_TAQ_THR[rsvd_thr] must also update
	 * SSO_TAQ_ADD[RSVD_FREE].
	 */
	taq_thr.u64 = cvmx_read_csr_node(node, CVMX_SSO_GRPX_TAQ_THR(grp));
	taq_add.u64 = 0;
	taq_add.s.rsvd_free = rsvd_thr - taq_thr.s.rsvd_thr;

	taq_thr.s.max_thr = max_thr;
	taq_thr.s.rsvd_thr = rsvd_thr;

	cvmx_write_csr_node(node, CVMX_SSO_GRPX_TAQ_THR(grp), taq_thr.u64);
	cvmx_write_csr_node(node, CVMX_SSO_TAQ_ADD, taq_add.u64);
}

/*
 * Map auras to the field priv->buffers_needed. Used to speed up packet
 * transmission.
 */
static void *aura2buffers_needed[1024];

static void octeon3_eth_gen_affinity(int node, cpumask_t *mask)
{
	int cpu;

	do {
		cpu = cpumask_next(octeon3_eth_node[node].next_cpu_irq_affinity,
				   cpu_online_mask);
		octeon3_eth_node[node].next_cpu_irq_affinity++;
		if (cpu >= nr_cpu_ids) {
			octeon3_eth_node[node].next_cpu_irq_affinity = -1;
			continue;
		}
	} while (false);
	cpumask_clear(mask);
	cpumask_set_cpu(cpu, mask);
}

static int octeon3_eth_fpa_pool_init(cvmx_fpa3_pool_t pool, int num_ptrs)
{
	void *pool_stack;
	u64 pool_stack_start, pool_stack_end;
	union cvmx_fpa_poolx_end_addr limit_addr;
	union cvmx_fpa_poolx_cfg cfg;
	int stack_size = (DIV_ROUND_UP(num_ptrs, 29) + 1) * 128;

	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool), 0);
	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_START_ADDR(pool.lpool), 128);
	limit_addr.u64 = 0;
	limit_addr.cn78xx.addr = ~0ll;
	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_END_ADDR(pool.lpool), limit_addr.u64);

	pool_stack = kmalloc_node(stack_size, GFP_KERNEL, pool.node);
	if (!pool_stack)
		return -ENOMEM;

	pool_stack_start = virt_to_phys(pool_stack);
	pool_stack_end = round_down(pool_stack_start + stack_size, 128);
	pool_stack_start = round_up(pool_stack_start, 128);

	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_STACK_BASE(pool.lpool), pool_stack_start);
	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_STACK_ADDR(pool.lpool), pool_stack_start);
	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_STACK_END(pool.lpool), pool_stack_end);

	cfg.u64 = 0;
	cfg.s.l_type = 2; /* Load with DWB */
	cfg.s.ena = 1;
	cvmx_write_csr_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool), cfg.u64);
	return 0;
}

static int octeon3_eth_fpa_aura_init(cvmx_fpa3_pool_t pool, cvmx_fpa3_gaura_t aura, unsigned int limit)
{
	int shift;
	union cvmx_fpa_aurax_cnt_levels cnt_levels;
	unsigned int drop, pass;

	BUG_ON(aura.node != pool.node);

	limit *= 2; /* allow twice the limit before saturation at zero. */

	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_CFG(aura.laura), 0);
	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_CNT_LIMIT(aura.laura), limit);
	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_CNT(aura.laura), limit);
	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_POOL(aura.laura), pool.lpool);
	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_POOL_LEVELS(aura.laura), 0); /* No per-pool RED/Drop */

	shift = 0;
	while ((limit >> shift) > 255)
		shift++;

	drop = (limit - num_packet_buffers / 20) >> shift;	  /* 95% */
	pass = (limit - (num_packet_buffers * 3) / 20) >> shift;  /* 85% */

	cnt_levels.u64 = 0;
	cnt_levels.s.shift = shift;
	cnt_levels.s.red_ena = 1;
	cnt_levels.s.drop = drop;
	cnt_levels.s.pass = pass;
	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_CNT_LEVELS(aura.laura), cnt_levels.u64);

	return 0;
}

static int octeon3_eth_sso_init(unsigned int node, int aura)
{
	union cvmx_sso_aw_cfg cfg;
	union cvmx_sso_xaq_aura xaq_aura;
	union cvmx_sso_err0 err0;
	union cvmx_sso_grpx_pri grp_pri;
	int i;
	int rv = 0;

	grp_pri.u64 = 0;
	grp_pri.s.weight = 0x3f;

	cfg.u64 = 0;
	cfg.s.stt = 1;
	cfg.s.ldt = 1;
	cfg.s.ldwb = 1;
	cvmx_write_csr_node(node, CVMX_SSO_AW_CFG, cfg.u64);

	xaq_aura.u64 = 0;
	xaq_aura.s.laura = aura;
	xaq_aura.s.node = node;
	cvmx_write_csr_node(node, CVMX_SSO_XAQ_AURA, xaq_aura.u64);

	for (i = 0; i < 256; i++) {
		u64 phys;
		void *mem = cvmx_fpa3_alloc(__cvmx_fpa3_gaura(node, aura));
		if (!mem) {
			rv = -ENOMEM;
			goto err;
		}
		phys = virt_to_phys(mem);
		cvmx_write_csr_node(node, CVMX_SSO_XAQX_HEAD_PTR(i), phys);
		cvmx_write_csr_node(node, CVMX_SSO_XAQX_TAIL_PTR(i), phys);
		/* SSO-18678 */
		cvmx_write_csr_node(node, CVMX_SSO_GRPX_PRI(i), grp_pri.u64);
	}
	err0.u64 = 0;
	err0.s.fpe = 1;
	cvmx_write_csr_node(node, CVMX_SSO_ERR0, err0.u64);

	cfg.s.rwen = 1;
	cvmx_write_csr_node(node, CVMX_SSO_AW_CFG, cfg.u64);

err:
	return rv;
}

static void octeon3_eth_sso_irq_set_armed(int node, int grp, bool v)
{
	union cvmx_sso_grpx_int_thr grp_int_thr;
	union cvmx_sso_grpx_int grp_int;

	/* Disarm/Rearm the irq. */
	grp_int_thr.u64 = 0;
	grp_int_thr.s.iaq_thr = v ? 1 : 0;
	cvmx_write_csr_node(node, CVMX_SSO_GRPX_INT_THR(grp), grp_int_thr.u64);

	grp_int.u64 = 0;
	grp_int.s.exe_int = 1;
	cvmx_write_csr_node(node, CVMX_SSO_GRPX_INT(grp), grp_int.u64);
}

struct wr_ret {
	void *work;
	u16 grp;
};

static inline struct wr_ret octeon3_eth_work_request_grp_sync(unsigned int lgrp, cvmx_pow_wait_t wait)
{
	cvmx_pow_load_addr_t ptr;
	cvmx_pow_tag_load_resp_t result;
	struct wr_ret r;
	unsigned int node = cvmx_get_node_num() & 3;

	ptr.u64 = 0;

	ptr.swork_78xx.mem_region = CVMX_IO_SEG;
	ptr.swork_78xx.is_io = 1;
	ptr.swork_78xx.did = CVMX_OCT_DID_TAG_SWTAG;
	ptr.swork_78xx.node = node;
	ptr.swork_78xx.rtngrp = 1;
	ptr.swork_78xx.grouped = 1;
	ptr.swork_78xx.index = (lgrp & 0xff) | node << 8;
	ptr.swork_78xx.wait = wait;

	result.u64 = cvmx_read_csr(ptr.u64);
	r.grp = result.s_work.grp;
	if (result.s_work.no_work)
		r.work = NULL;
	else
		r.work = cvmx_phys_to_ptr(result.s_work.addr);
	return r;
}

/* octeon3_eth_get_work_async:	Request work via a iobdma command. Doesn't wait
 *				for the response.
 *
 *  lgrp:			Group to request work for.
 */
static inline void octeon3_eth_get_work_async(unsigned int lgrp)
{
	cvmx_pow_iobdma_store_t	data;
	int			node = cvmx_get_node_num();

	data.u64 = 0;
	data.s_cn78xx.scraddr = CVMX_SCR_SCRATCH;
	data.s_cn78xx.len = 1;
	data.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
	data.s_cn78xx.node = node;
	data.s_cn78xx.grouped = 1;
	data.s_cn78xx.rtngrp = 1;
	data.s_cn78xx.index_grp_mask = (lgrp & 0xff) | node << 8;
	data.s_cn78xx.wait = CVMX_POW_NO_WAIT;

	cvmx_send_single(data.u64);
}

/* octeon3_eth_get_work_response_async:	Read the request work response. Must
 *					be called after calling
 *					octeon3_eth_get_work_async().
 *
 *  Returns:				Work queue entry.
 */
static inline struct wr_ret octeon3_eth_get_work_response_async(void)
{
	cvmx_pow_tag_load_resp_t	result;
	struct wr_ret			r;

	CVMX_SYNCIOBDMA;
	result.u64 = cvmx_scratch_read64(CVMX_SCR_SCRATCH);

	r.grp = result.s_work.grp;
	if (result.s_work.no_work)
		r.work = NULL;
	else
		r.work = cvmx_phys_to_ptr(result.s_work.addr);
	return r;
}

static void octeon3_eth_replenish_rx(struct octeon3_ethernet *priv, int count)
{
	struct sk_buff *skb;
	int i;

	for (i = 0; i < count; i++) {
		void **buf;
		skb = __alloc_skb(packet_buffer_size, GFP_ATOMIC, 0, priv->numa_node);
		if (!skb) {
			pr_err("WARNING: octeon3_eth_replenish_rx out of memory\n");
			break;
		}
		buf = (void **)PTR_ALIGN(skb->head, 128);
		buf[SKB_PTR_OFFSET] = skb;
		cvmx_fpa3_free(buf,
			__cvmx_fpa3_gaura(priv->numa_node, priv->pki_laura),
			0);
	}
}

static bool octeon3_eth_tx_complete_runnable(struct octeon3_ethernet_worker *worker)
{
	return atomic_read(&worker->kick) != 0;
}

static int octeon3_eth_replenish_all(struct octeon3_ethernet_node *oen)
{
	int pending = 0;
	int batch_size = 32;
	struct octeon3_ethernet *priv;

	rcu_read_lock();
	list_for_each_entry_rcu(priv, &oen->device_list, list) {
		int amount = atomic64_sub_if_positive(batch_size,
						      &priv->buffers_needed);
		if (amount >= 0) {
			octeon3_eth_replenish_rx(priv, batch_size);
			pending += amount;
		}
	}
	rcu_read_unlock();
	return pending;
}

static int octeon3_eth_tx_complete_worker(void *data)
{
	union cvmx_sso_grpx_aq_cnt aq_cnt;
	struct octeon3_ethernet_worker *worker = data;
	struct octeon3_ethernet_node *oen = worker->oen;
	int backlog;
	int loops;
	const int tx_complete_batch = 100;
	int backlog_stop_thresh = worker->order == 0 ? 0 : (worker->order - 1) * 80;
	int i;
#ifndef CONFIG_PREEMPT
	unsigned long last_jiffies = jiffies;
#endif
	for (;;) {
		/*
		 * replaced by wait_event to avoid warnings like
		 * "task oct3_eth/0:2:1250 blocked for more than 120 seconds."
		 */
		wait_event_interruptible(worker->queue, octeon3_eth_tx_complete_runnable(worker));
		atomic_dec_if_positive(&worker->kick); /* clear the flag */
		loops = 0;
		do {
		re_enter:
			loops++;
			backlog = octeon3_eth_replenish_all(oen);
			if (loops > 3 && backlog > 100 * worker->order &&
			    worker->order < ARRAY_SIZE(oen->workers) - 1) {
				atomic_set(&oen->workers[worker->order + 1].kick, 1);
				wake_up(&oen->workers[worker->order + 1].queue);
			}
			for (i = 0; i < tx_complete_batch; i++) {
				void **work;
				struct net_device *tx_netdev;
				struct octeon3_ethernet *tx_priv;
				struct sk_buff *skb;
				struct wr_ret r;

#ifndef CONFIG_PREEMPT
				if (jiffies != last_jiffies) {
					schedule();
					last_jiffies = jiffies;
				}
#endif
				r = octeon3_eth_work_request_grp_sync(oen->tx_complete_grp, CVMX_POW_NO_WAIT);
				work = r.work;
				if (work == NULL)
					break;
				tx_netdev = work[0];
				tx_priv = netdev_priv(tx_netdev);
				if (unlikely(netif_queue_stopped(tx_netdev)) &&
				    atomic64_read(&tx_priv->tx_backlog) < MAX_TX_QUEUE_DEPTH)
					netif_wake_queue(tx_netdev);
				skb = container_of((void *)work, struct sk_buff, cb);
				dev_kfree_skb(skb);
			}
			aq_cnt.u64 = cvmx_read_csr_node(oen->numa_node, CVMX_SSO_GRPX_AQ_CNT(oen->tx_complete_grp));
		} while (backlog > backlog_stop_thresh   && aq_cnt.s.aq_cnt > worker->order * tx_complete_batch);
		if (!octeon3_eth_tx_complete_runnable(worker))
			octeon3_eth_sso_irq_set_armed(oen->numa_node, oen->tx_complete_grp, true);
		aq_cnt.u64 = cvmx_read_csr_node(oen->numa_node, CVMX_SSO_GRPX_AQ_CNT(oen->tx_complete_grp));
		if (aq_cnt.s.aq_cnt > worker->order * tx_complete_batch)
			goto re_enter;
	}
	return 0;
}

static irqreturn_t octeon3_eth_tx_handler(int irq, void *info)
{
	struct octeon3_ethernet_node *oen = info;
	/* Disarm the irq. */
	octeon3_eth_sso_irq_set_armed(oen->numa_node, oen->tx_complete_grp, false);
	atomic_set(&oen->workers[0].kick, 1);
	wake_up(&oen->workers[0].queue);
	return IRQ_HANDLED;
}

static int octeon3_eth_global_init(unsigned int node)
{
	int i;
	int rv = 0;
	struct irq_domain *d;
	unsigned int sso_intsn;
	struct octeon3_ethernet_node *oen;
	union cvmx_fpa_gen_cfg fpa_cfg;
	mutex_lock(&octeon3_eth_init_mutex);

	oen = octeon3_eth_node + node;

	if (oen->init_done)
		goto done;

	__cvmx_export_app_config_to_named_block(CVMX_APP_CONFIG);

	INIT_LIST_HEAD(&oen->device_list);
	mutex_init(&oen->device_list_lock);
	spin_lock_init(&oen->napi_alloc_lock);

	oen->numa_node = node;
	for (i = 0; i < 1024; i++) {
		cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT(i), 0x100000000ull);
		cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT_LIMIT(i), 0xfffffffffull);
		cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT_THRESHOLD(i), 0xffffffffeull);
	}

	fpa_cfg.u64 = cvmx_read_csr_node(node, CVMX_FPA_GEN_CFG);
	fpa_cfg.s.lvl_dly = 3;
	cvmx_write_csr_node(node, CVMX_FPA_GEN_CFG, fpa_cfg.u64);

	oen->sso_pool = cvmx_fpa3_reserve_pool(node, -1);
	if (!__cvmx_fpa3_pool_valid(oen->sso_pool)) {
		rv = -ENODEV;
		goto done;
	}
	oen->pko_pool = cvmx_fpa3_reserve_pool(node, -1);
	if (!__cvmx_fpa3_pool_valid(oen->pko_pool)) {
		rv = -ENODEV;
		goto done;
	}
	oen->pki_packet_pool = cvmx_fpa3_reserve_pool(node, -1);
	if (!__cvmx_fpa3_pool_valid(oen->pki_packet_pool)) {
		rv = -ENODEV;
		goto done;
	}
	oen->sso_aura = cvmx_fpa3_reserve_aura(node, -1);
	oen->pko_aura = cvmx_fpa3_reserve_aura(node, -1);

	pr_err("octeon3_eth_global_init  SSO:%d:%d, PKO:%d:%d\n",
	       oen->sso_pool.lpool, oen->sso_aura.laura,
	       oen->pko_pool.lpool, oen->pko_aura.laura);

	octeon3_eth_fpa_pool_init(oen->sso_pool, 40960);
	octeon3_eth_fpa_pool_init(oen->pko_pool, 40960);
	octeon3_eth_fpa_pool_init(oen->pki_packet_pool, 64 * num_packet_buffers);
	octeon3_eth_fpa_aura_init(oen->sso_pool, oen->sso_aura, 20480);
	octeon3_eth_fpa_aura_init(oen->pko_pool, oen->pko_aura, 20480);

	if (!octeon3_eth_sso_pko_cache) {
		octeon3_eth_sso_pko_cache = kmem_cache_create("sso_pko", 4096, 128, 0, NULL);
		if (!octeon3_eth_sso_pko_cache) {
			rv = -ENOMEM;
			goto done;
		}
	}
	for (i = 0; i < 1024; i++) {
		void *mem;
		mem = kmem_cache_alloc_node(octeon3_eth_sso_pko_cache, GFP_KERNEL, node);
		if (!mem) {
			rv = -ENOMEM;
			goto done;
		}
		cvmx_fpa3_free(mem, oen->sso_aura, 0);
		mem = kmem_cache_alloc_node(octeon3_eth_sso_pko_cache, GFP_KERNEL, node);
		if (!mem) {
			rv = -ENOMEM;
			goto done;
		}
		cvmx_fpa3_free(mem, oen->pko_aura, 0);
	}

	BUG_ON(node != oen->sso_aura.node);
	rv = octeon3_eth_sso_init(node, oen->sso_aura.laura);
	if (rv)
		goto done;

	oen->tx_complete_grp = cvmx_sso_allocate_group(node);
	d = octeon_irq_get_block_domain(oen->numa_node, SSO_INTSN_EXE);
	sso_intsn = SSO_INTSN_EXE << 12 | oen->tx_complete_grp;
	oen->tx_irq = irq_create_mapping(d, sso_intsn);
	if (!oen->tx_irq) {
		rv = -ENODEV;
		goto done;
	}

	__cvmx_helper_init_port_config_data();
	rv = __cvmx_helper_pko3_init_global(node, oen->pko_aura.laura | (node << 10));
	if (rv) {
		pr_err("cvmx_helper_pko3_init_global failed\n");
		rv = -ENODEV;
		goto done;
	}

	__cvmx_helper_pki_install_dflt_vlan(node);
	cvmx_pki_setup_clusters(node);
	__cvmx_helper_pki_set_dflt_ltype_map(node);
	cvmx_pki_enable_backpressure(node);
	cvmx_pki_parse_enable(node, 0);
	cvmx_pki_enable(node);
	/* Statistics per pkind */
	cvmx_write_csr_node(node, CVMX_PKI_STAT_CTL, 0);

	for (i = 0; i < ARRAY_SIZE(oen->workers); i++) {
		oen->workers[i].oen = oen;
		init_waitqueue_head(&oen->workers[i].queue);
		oen->workers[i].order = i;
	}
	for (i = 0; i < ARRAY_SIZE(oen->workers); i++) {
		oen->workers[i].task = kthread_create_on_node(octeon3_eth_tx_complete_worker,
							      oen->workers + i, node,
							      "oct3_eth/%d:%d", node, i);
		if (IS_ERR(oen->workers[i].task)) {
			rv = PTR_ERR(oen->workers[i].task);
			goto done;
		} else {
			wake_up_process(oen->workers[i].task);
		}
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		octeon3_eth_sso_pass1_limit(node, oen->tx_complete_grp);

	rv = request_irq(oen->tx_irq, octeon3_eth_tx_handler, 0, "oct3_eth_tx_done", oen);
	if (rv)
		goto done;
	octeon3_eth_gen_affinity(node, &oen->tx_affinity_hint);
	irq_set_affinity_hint(oen->tx_irq, &oen->tx_affinity_hint);

	octeon3_eth_sso_irq_set_armed(node, oen->tx_complete_grp, true);

	__cvmx_export_config();

	oen->init_done = true;
done:
	mutex_unlock(&octeon3_eth_init_mutex);
	return rv;
}

static struct sk_buff *octeon3_eth_work_to_skb(void *w)
{
	struct sk_buff *skb;
	void **f = w;
	skb = f[-16];
	return skb;
}

/* octeon3_napi_alloc:		Allocate a napi.
 *
 *  cxt:			Receive context the napi will be added to.
 *  idx:			Napi index within the receive context.
 *  cpu:			Cpu to bind the napi to:
 *					<  0: use any cpu.
 *					>= 0: use requested cpu.
 *
 *  Returns:			Pointer to napi wrapper or NULL on error.
 */
static struct octeon3_napi_wrapper *octeon3_napi_alloc(struct octeon3_rx *cxt,
						       int		  idx,
						       int		  cpu)
{
	struct octeon3_ethernet_node	*oen;
	struct octeon3_ethernet		*priv = cxt->parent;
	int				node = priv->numa_node;
	int				i;

	oen = octeon3_eth_node + node;
	spin_lock(&oen->napi_alloc_lock);

	/* Find a free napi wrapper */
	for (i = 0; i < CVMX_MAX_CORES; i++) {
		if (napi_wrapper[node][i].available) {
			/* Assign a cpu to use (a free cpu if possible) */
			if (cpu < 0) {
				cpu = find_first_zero_bit(oen->napi_cpu_bitmap,
							  oen->napi_max_cpus);
				if (cpu < oen->napi_max_cpus) {
					bitmap_set(oen->napi_cpu_bitmap,
						   cpu, 1);
				} else {
					/* Choose a random cpu */
					get_random_bytes(&cpu, sizeof(int));
					cpu = cpu % oen->napi_max_cpus;
				}
			} else
				bitmap_set(oen->napi_cpu_bitmap, cpu, 1);

			napi_wrapper[node][i].available = 0;
			napi_wrapper[node][i].idx = idx;
			napi_wrapper[node][i].cpu = cpu;
			napi_wrapper[node][i].cxt = cxt;
			spin_unlock(&oen->napi_alloc_lock);
			return &napi_wrapper[node][i];
		}
	}

	spin_unlock(&oen->napi_alloc_lock);
	return NULL;
}

/* octeon_cpu_napi_sched:	Schedule a napi for execution. The napi will
 *				start executing on the cpu calling this
 *				function.
 *
 *  info:			Pointer to the napi to schedule for execution.
 */
static void octeon_cpu_napi_sched(void	*info)
{
	struct napi_struct		*napi = info;
	napi_schedule(napi);
}

/* octeon3_rm_napi_from_cxt:	Remove a napi from a receive context.
 *
 *  node:			Node napi belongs to.
 *  napiw:			Pointer to napi to remove.
 *
 *  returns:			0 on success, otherwise a negative error code.
 */
static int octeon3_rm_napi_from_cxt(int				node,
				    struct octeon3_napi_wrapper	*napiw)
{
	struct octeon3_ethernet_node	*oen;
	struct octeon3_rx		*cxt;
	int				idx;

	oen = octeon3_eth_node + node;
	cxt = napiw->cxt;
	idx = napiw->idx;

	/* Free the napi block */
	spin_lock(&oen->napi_alloc_lock);
	bitmap_clear(oen->napi_cpu_bitmap, napiw->cpu, 1);
	napiw->available = 1;
	napiw->idx = -1;
	napiw->cpu = -1;
	napiw->cxt = NULL;
	spin_unlock(&oen->napi_alloc_lock);

	/* Free the napi idx */
	spin_lock(&cxt->napi_idx_lock);
	bitmap_clear(cxt->napi_idx_bitmap, idx, 1);
	spin_unlock(&cxt->napi_idx_lock);

	return 0;
}

/* octeon3_add_napi_to_cxt:	Add a napi to a receive context.
 *
 *  cxt:			Pointer to receive context.
 *
 *  returns:			0 on success, otherwise a negative error code.
 */
static int octeon3_add_napi_to_cxt(struct octeon3_rx *cxt)
{
	struct octeon3_napi_wrapper	*napiw;
	struct octeon3_ethernet		*priv = cxt->parent;
	int				idx;
	int				rc;

	/* Get a free napi idx */
	spin_lock(&cxt->napi_idx_lock);
	idx = find_first_zero_bit(cxt->napi_idx_bitmap, CVMX_MAX_CORES);
	if (unlikely(idx >= CVMX_MAX_CORES)) {
		spin_unlock(&cxt->napi_idx_lock);
		return -ENOMEM;
	}
	bitmap_set(cxt->napi_idx_bitmap, idx, 1);
	spin_unlock(&cxt->napi_idx_lock);

	/* Get a free napi block */
	napiw = octeon3_napi_alloc(cxt, idx, -1);
	if (unlikely(napiw == NULL)) {
		spin_lock(&cxt->napi_idx_lock);
		bitmap_clear(cxt->napi_idx_bitmap, idx, 1);
		spin_unlock(&cxt->napi_idx_lock);
		return -ENOMEM;
	}

	rc = smp_call_function_single(napiw->cpu, octeon_cpu_napi_sched,
				      &napiw->napi, 0);
	if (unlikely(rc))
		octeon3_rm_napi_from_cxt(priv->numa_node, napiw);

	return rc;
}

/* Receive one packet.
 * returns the number of RX buffers consumed.
 */
static int octeon3_eth_rx_one(struct octeon3_rx *rx, bool is_async,
			      bool req_next)
{
	int segments;
	int ret;
	unsigned int packet_len;
	cvmx_wqe_78xx_t *work;
	u8 *data;
	int len_remaining;
	struct sk_buff *skb;
	union cvmx_buf_ptr_pki packet_ptr;
	struct wr_ret r;
	struct octeon3_ethernet *priv = rx->parent;
	void **buf;

	if (is_async == true)
		r = octeon3_eth_get_work_response_async();
	else
		r = octeon3_eth_work_request_grp_sync(rx->rx_grp,
						      CVMX_POW_NO_WAIT);
	work = r.work;
	if (!work)
		return 0;

	/* Request the next work so it'll be ready when we need it */
	if (is_async == true && req_next)
		octeon3_eth_get_work_async(rx->rx_grp);

	skb = octeon3_eth_work_to_skb(work);

	/* Save the aura this skb came from to allow the pko to free the skb
	 * back to the correct aura.
	 */
	buf = (void **)PTR_ALIGN(skb->head, 128);
	buf[SKB_AURA_OFFSET] = (void *)(SKB_AURA_MAGIC | priv->pki_laura);

	segments = work->word0.bufs;
	ret = segments;
	packet_ptr = work->packet_ptr;
//	pr_err("RX %016lx.%016lx.%016lx\n",
//	       (unsigned long)work->word0.u64, (unsigned long)work->word1.u64, (unsigned long)work->word2.u64);
	if (unlikely(work->word2.err_level <= CVMX_PKI_ERRLEV_E_LA &&
		     work->word2.err_code != CVMX_PKI_OPCODE_RE_NONE)) {
		atomic64_inc(&priv->rx_errors);
		switch (work->word2.err_code) {
		case CVMX_PKI_OPCODE_RE_JABBER:
			atomic64_inc(&priv->rx_length_errors);
			break;
		case CVMX_PKI_OPCODE_RE_FCS:
			atomic64_inc(&priv->rx_crc_errors);
			break;
		}
		data = phys_to_virt(packet_ptr.addr);
		for (;;) {
			dev_kfree_skb_any(skb);
			segments--;
			if (segments <= 0)
				break;
			packet_ptr.u64 = *(u64 *)(data - 8);
			/* PKI_BUFLINK_S's are endian-swapped */
			packet_ptr.u64 = swab64(packet_ptr.u64);
			data = phys_to_virt(packet_ptr.addr);
			skb = octeon3_eth_work_to_skb((void *)round_down((unsigned long)data, 128ul));
		}
		goto out;
	}

	packet_len = work->word1.len;
	data = phys_to_virt(packet_ptr.addr);
	skb->data = data;
	skb->len = packet_len;
	len_remaining = packet_len;
	if (segments == 1) {
		skb_set_tail_pointer(skb, skb->len);
	} else {
		bool first_frag = true;
		struct sk_buff *current_skb = skb;
		struct sk_buff *next_skb = NULL;
		unsigned int segment_size;

		skb_frag_list_init(skb);
		for (;;) {
			segment_size = (segments == 1) ? len_remaining : packet_ptr.size;
			len_remaining -= segment_size;
			if (!first_frag) {
				current_skb->len = segment_size;
				skb->data_len += segment_size;
				skb->truesize += current_skb->truesize;
			}
			skb_set_tail_pointer(current_skb, segment_size);
			segments--;
			if (segments == 0)
				break;
			packet_ptr.u64 = *(u64 *)(data - 8);
			/* PKI_BUFLINK_S's are endian-swapped */
			packet_ptr.u64 = swab64(packet_ptr.u64);

			data = phys_to_virt(packet_ptr.addr);
			next_skb = octeon3_eth_work_to_skb((void *)round_down((unsigned long)data, 128ul));
			if (first_frag) {
				skb_frag_add_head(current_skb, next_skb);
			} else {
				current_skb->next = next_skb;
				next_skb->next = NULL;
			}
			current_skb = next_skb;
			first_frag = false;
			current_skb->data = data;
		}
	}
	if (likely(priv->netdev->flags & IFF_UP)) {
		skb_checksum_none_assert(skb);
		skb->protocol = eth_type_trans(skb, priv->netdev);
		skb->dev = priv->netdev;
		if (priv->netdev->features & NETIF_F_RXCSUM) {
			if ((work->word2.lc_hdr_type == CVMX_PKI_LTYPE_E_IP4 ||
			     work->word2.lc_hdr_type == CVMX_PKI_LTYPE_E_IP6) &&
			    (work->word2.lf_hdr_type == CVMX_PKI_LTYPE_E_TCP ||
			     work->word2.lf_hdr_type == CVMX_PKI_LTYPE_E_UDP ||
			     work->word2.lf_hdr_type == CVMX_PKI_LTYPE_E_SCTP))
				if (work->word2.err_code == 0)
					skb->ip_summed = CHECKSUM_UNNECESSARY;


		}
		netif_receive_skb(skb);
	} else {
		/* Drop any packet received for a device that isn't up */
		atomic64_inc(&priv->rx_dropped);
		dev_kfree_skb_any(skb);
	}
out:
	return ret;
}

static int octeon3_eth_napi(struct napi_struct *napi, int budget)
{
	int rx_count = 0;
	struct octeon3_napi_wrapper *napiw;
	struct octeon3_rx *cxt;
	struct octeon3_ethernet *priv;
	union cvmx_sso_grpx_aq_cnt aq_cnt;
	int idx;
	int napis_inuse;
	int n = 0;
	int n_bufs = 0;
	u64 old_scratch;

	napiw = container_of(napi, struct octeon3_napi_wrapper, napi);
	cxt = napiw->cxt;
	priv = cxt->parent;

	/* Get the amount of work pending */
	aq_cnt.u64 = cvmx_read_csr_node(priv->numa_node,
					CVMX_SSO_GRPX_AQ_CNT(cxt->rx_grp));

	/* Allow the last thread to add/remove threads if the work
	 * incremented/decremented by more than what the current number
	 * of threads can support.
	 */
	idx = find_last_bit(cxt->napi_idx_bitmap, CVMX_MAX_CORES);
	napis_inuse = bitmap_weight(cxt->napi_idx_bitmap, CVMX_MAX_CORES);

	if (napiw->idx == idx) {
		if (aq_cnt.s.aq_cnt > napis_inuse * 128)
			octeon3_add_napi_to_cxt(cxt);
		else if (napiw->idx > 0 &&
			 aq_cnt.s.aq_cnt < (napis_inuse - 1) * 128) {
			napi_complete(napi);
			octeon3_rm_napi_from_cxt(priv->numa_node, napiw);
			return 0;
		}
	}

	if (likely(USE_ASYNC_IOBDMA)) {
		/* Save scratch in case userspace is using it */
		CVMX_SYNCIOBDMA;
		old_scratch = cvmx_scratch_read64(CVMX_SCR_SCRATCH);

		octeon3_eth_get_work_async(cxt->rx_grp);
	}

	while (rx_count < budget) {
		n = 0;

		if (likely(USE_ASYNC_IOBDMA)) {
			bool req_next = rx_count < (budget - 1) ? true : false;

			n = octeon3_eth_rx_one(cxt, true, req_next);
		} else
			n = octeon3_eth_rx_one(cxt, false, false);

		if (n == 0) {
			break;
		}

		n_bufs += n;
		rx_count++;
	}

	/* Wake up worker threads */
	n_bufs = atomic64_add_return(n_bufs, &priv->buffers_needed);
	if (n_bufs >= 32) {
		struct octeon3_ethernet_node *oen;

		oen = octeon3_eth_node + priv->numa_node;
		atomic_set(&oen->workers[0].kick, 1);
		wake_up(&oen->workers[0].queue);
	}

	/* Stop the thread when no work is pending */
	if (rx_count < budget) {
		napi_complete(napi);

		if (napiw->idx > 0) {
			octeon3_rm_napi_from_cxt(priv->numa_node, napiw);
		} else {
			octeon3_eth_sso_irq_set_armed(cxt->parent->numa_node,
						      cxt->rx_grp, true);
		}
	}

	if (likely(USE_ASYNC_IOBDMA)) {
		/* Restore the scratch area */
		cvmx_scratch_write64(CVMX_SCR_SCRATCH, old_scratch);
	}

	return rx_count;
}

/* octeon3_napi_init_node:	Initialize the node napis.
 *
 *  node:			Node napis belong to.
 *  netdev:			Default network device used to initialize the
 *				napis.
 *
 *  returns:			0 on success, otherwise a negative error code.
 */
static int octeon3_napi_init_node(int node, struct net_device *netdev)
{
	struct octeon3_ethernet_node	*oen;
	int				i;

	oen = octeon3_eth_node + node;
	spin_lock(&oen->napi_alloc_lock);

	if (oen->napi_init_done)
		goto done;

	bitmap_zero(oen->napi_cpu_bitmap, CVMX_MAX_CORES);
	oen->napi_max_cpus = nr_cpus_node(node);

	for (i = 0; i < CVMX_MAX_CORES; i++) {
		netif_napi_add(netdev, &napi_wrapper[node][i].napi,
			       octeon3_eth_napi, 32);
		napi_enable(&napi_wrapper[node][i].napi);
		napi_wrapper[node][i].available = 1;
		napi_wrapper[node][i].idx = 0;
		napi_wrapper[node][i].cpu = -1;
		napi_wrapper[node][i].cxt = NULL;
	}

	oen->napi_init_done = true;
 done:
	spin_unlock(&oen->napi_alloc_lock);
	return 0;

}

//#define BROKEN_SIMULATOR_CSUM 1

static void ethtool_get_drvinfo(struct net_device *netdev,
				struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "octeon3-ethernet");
	strcpy(info->version, "1.0");
	strcpy(info->bus_info, "Builtin");
}

static const struct ethtool_ops octeon3_ethtool_ops = {
	.get_drvinfo = ethtool_get_drvinfo,
	.get_settings = bgx_port_ethtool_get_settings,
	.set_settings = bgx_port_ethtool_set_settings,
	.nway_reset = bgx_port_ethtool_nway_reset,
	.get_link = ethtool_op_get_link,
};

static int octeon3_eth_ndo_init(struct net_device *netdev)
{
	struct octeon3_ethernet *priv = netdev_priv(netdev);
	struct octeon3_ethernet_node *oen = octeon3_eth_node + priv->numa_node;
	struct cvmx_pki_port_config pki_prt_cfg;
	struct cvmx_pki_prt_schd *prt_schd = NULL;
	union cvmx_pki_aurax_cfg pki_aura_cfg;
	union cvmx_pki_qpg_tblx qpg_tbl;
	int ipd_port, node_dq;
	int base_rx_grp;
	int first_skip, later_skip;
	struct cvmx_xport xdq;
	int r, i;
	const u8 *mac;
	cvmx_fpa3_gaura_t aura;

	netif_carrier_off(netdev);

	netdev->features |=
		NETIF_F_SG |
		NETIF_F_FRAGLIST |
		NETIF_F_RXCSUM |
		NETIF_F_LLTX
#ifndef BROKEN_SIMULATOR_CSUM
		|
		NETIF_F_IP_CSUM |
		NETIF_F_IPV6_CSUM
#endif
		;

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		netdev->features |= NETIF_F_SCTP_CSUM;

	priv->rx_buf_count = num_packet_buffers;

	ipd_port = cvmx_helper_get_ipd_port(priv->xiface, priv->port_index);

	r =  __cvmx_pko3_config_gen_interface(priv->xiface, priv->port_index, 1, false);
	if (r)
		return -ENODEV;

	r = __cvmx_pko3_helper_dqs_activate(priv->xiface, priv->port_index, false);
	if (r < 0)
		return -ENODEV;

	/* Padding and FCS are done in BGX */
	r = cvmx_pko3_interface_options(priv->xiface, priv->port_index, false, false, 0);
	if (r)
		return -ENODEV;

	node_dq = cvmx_pko3_get_queue_base(ipd_port);
	xdq = cvmx_helper_ipd_port_to_xport(node_dq);

	priv->pko_queue = xdq.port;
	aura = cvmx_fpa3_reserve_aura(priv->numa_node, -1);
	priv->pki_laura = aura.laura;
	octeon3_eth_fpa_aura_init(oen->pki_packet_pool, aura,
				  num_packet_buffers * 2);
	aura2buffers_needed[priv->pki_laura] = &priv->buffers_needed;

	base_rx_grp = -1;
	r = cvmx_sso_allocate_group_range(priv->numa_node, &base_rx_grp, rx_contexts);
	if (r) {
		dev_err(netdev->dev.parent, "Failed to allocated SSO group\n");
		return -ENODEV;
	}
	for (i = 0; i < rx_contexts; i++) {
		priv->rx_cxt[i].rx_grp = base_rx_grp + i;
		priv->rx_cxt[i].parent = priv;

		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			octeon3_eth_sso_pass1_limit(priv->numa_node,
						    priv->rx_cxt[i].rx_grp);
		}
	}
	priv->num_rx_cxt = rx_contexts;

	priv->tx_complete_grp = oen->tx_complete_grp;
	priv->pki_pkind = cvmx_helper_get_pknd(priv->xiface, priv->port_index);
	dev_err(netdev->dev.parent, "rx sso grp:%d..%d aura:%d pknd:%d pko_queue:%d\n",
		base_rx_grp, base_rx_grp + priv->num_rx_cxt, priv->pki_laura, priv->pki_pkind, priv->pko_queue);

	prt_schd = kzalloc(sizeof(*prt_schd), GFP_KERNEL);
	if (!prt_schd) {
		r = -ENOMEM;
		goto err;
	}

	prt_schd->style = -1; /* Allocate net style per port */
	prt_schd->qpg_base = -1;
	prt_schd->aura_per_prt = true;
	prt_schd->aura_num = priv->pki_laura;
	prt_schd->sso_grp_per_prt = true;
	prt_schd->sso_grp = base_rx_grp;
	prt_schd->qpg_qos = CVMX_PKI_QPG_QOS_NONE;

	cvmx_helper_pki_init_port(ipd_port, prt_schd);
	cvmx_pki_get_port_config(ipd_port, &pki_prt_cfg);

	pki_prt_cfg.style_cfg.parm_cfg.ip6_udp_opt = false;
	pki_prt_cfg.style_cfg.parm_cfg.lenerr_en = true;
	pki_prt_cfg.style_cfg.parm_cfg.maxerr_en = false;
	pki_prt_cfg.style_cfg.parm_cfg.minerr_en = false;
	pki_prt_cfg.style_cfg.parm_cfg.ip6_udp_opt = false;
	pki_prt_cfg.style_cfg.parm_cfg.ip6_udp_opt = false;
	pki_prt_cfg.style_cfg.parm_cfg.wqe_skip = 1 * 128;
	first_skip = 8 * 21;
	later_skip = 8 * 16;
	pki_prt_cfg.style_cfg.parm_cfg.first_skip = first_skip;
	pki_prt_cfg.style_cfg.parm_cfg.later_skip = later_skip;
#ifdef __LITTLE_ENDIAN
	pki_prt_cfg.style_cfg.parm_cfg.pkt_lend = true;
#else
	pki_prt_cfg.style_cfg.parm_cfg.pkt_lend = false;
#endif
	pki_prt_cfg.style_cfg.parm_cfg.tag_type = CVMX_SSO_TAG_TYPE_UNTAGGED;
	pki_prt_cfg.style_cfg.parm_cfg.qpg_dis_grptag = false;
	pki_prt_cfg.style_cfg.parm_cfg.dis_wq_dat = false;

	pki_prt_cfg.style_cfg.parm_cfg.csum_lb = true;
	pki_prt_cfg.style_cfg.parm_cfg.csum_lc = true;
	pki_prt_cfg.style_cfg.parm_cfg.csum_ld = true;
	pki_prt_cfg.style_cfg.parm_cfg.csum_le = true;
	pki_prt_cfg.style_cfg.parm_cfg.csum_lf = true;
	pki_prt_cfg.style_cfg.parm_cfg.csum_lg = false;

	pki_prt_cfg.style_cfg.parm_cfg.len_lb = false;
	pki_prt_cfg.style_cfg.parm_cfg.len_lc = true;
	pki_prt_cfg.style_cfg.parm_cfg.len_ld = false;
	pki_prt_cfg.style_cfg.parm_cfg.len_le = false;
	pki_prt_cfg.style_cfg.parm_cfg.len_lf = true;
	pki_prt_cfg.style_cfg.parm_cfg.len_lg = false;

	pki_prt_cfg.style_cfg.parm_cfg.mbuff_size = (packet_buffer_size - 128) & ~0xf;

	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.tag_vni = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.tag_gtp = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.tag_spi = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.tag_sync = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.ip_prot_nexthdr = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.second_vlan = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.first_vlan = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.mpls_label = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.input_port = 0;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_b_dst = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_c_dst = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_d_dst = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_e_dst = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_f_dst = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_g_dst = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_b_src = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_c_src = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_d_src = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_e_src = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_f_src = 1;
	pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_g_src = 1;

	cvmx_pki_set_port_config(ipd_port, &pki_prt_cfg);

	i = 0;
	while ((priv->num_rx_cxt & (1 << i)) == 0)
		i++;

	qpg_tbl.u64 = cvmx_read_csr_node(priv->numa_node, CVMX_PKI_QPG_TBLX(pki_prt_cfg.style_cfg.parm_cfg.qpg_base));
	qpg_tbl.s.grptag_ok = i;
	qpg_tbl.s.grptag_bad = i;
	cvmx_write_csr_node(priv->numa_node,
			    CVMX_PKI_QPG_TBLX(pki_prt_cfg.style_cfg.parm_cfg.qpg_base),
			    qpg_tbl.u64);
	pki_aura_cfg.u64 = 0;
	pki_aura_cfg.s.ena_red = 1;
	cvmx_write_csr_node(priv->numa_node, CVMX_PKI_AURAX_CFG(priv->pki_laura), pki_aura_cfg.u64);

	cvmx_write_csr_node(priv->numa_node, CVMX_PKI_STATX_STAT0(priv->pki_pkind), 0);
	priv->last_packets = 0;

	cvmx_write_csr_node(priv->numa_node, CVMX_PKI_STATX_STAT1(priv->pki_pkind), 0);
	priv->last_octets = 0;

	cvmx_write_csr_node(priv->numa_node, CVMX_PKI_STATX_STAT3(priv->pki_pkind), 0);
	priv->last_dropped = 0;

	mac = bgx_port_get_mac(netdev);
	if (mac && is_valid_ether_addr(mac)) {
		memcpy(netdev->dev_addr, mac, ETH_ALEN);
		netdev->addr_assign_type &= ~NET_ADDR_RANDOM;
	} else {
		eth_hw_addr_random(netdev);
	}
	bgx_port_set_rx_filtering(netdev);
	bgx_port_change_mtu(netdev, netdev->mtu);

	octeon3_napi_init_node(priv->numa_node, netdev);

	/* Register ethtool methods */
	SET_ETHTOOL_OPS(netdev, &octeon3_ethtool_ops);

	__cvmx_export_config();

	return 0;
err:
	kfree(prt_schd);
	return r;
}

static void octeon3_eth_ndo_uninit(struct net_device *netdev)
{
	return;
}

static irqreturn_t octeon3_eth_rx_handler(int irq, void *info)
{
	struct octeon3_rx *rx = info;

	/* Disarm the irq. */
	octeon3_eth_sso_irq_set_armed(rx->parent->numa_node, rx->rx_grp, false);

	napi_schedule(&rx->napiw->napi);
	return IRQ_HANDLED;
}

static int octeon3_eth_ndo_open(struct net_device *netdev)
{
	struct octeon3_ethernet *priv = netdev_priv(netdev);
	struct irq_domain *d = octeon_irq_get_block_domain(priv->numa_node, SSO_INTSN_EXE);
	struct octeon3_rx *rx;
	int idx;
	int cpu;
	int i;
	int r;

	for (i = 0; i < priv->num_rx_cxt; i++) {
		unsigned int sso_intsn;

		rx = priv->rx_cxt + i;
		sso_intsn = SSO_INTSN_EXE << 12 | rx->rx_grp;

		spin_lock_init(&rx->napi_idx_lock);

		rx->rx_irq = irq_create_mapping(d, sso_intsn);
		if (!rx->rx_irq) {
			netdev_err(netdev, "ERROR: Couldn't map hwirq: %x\n",
				   sso_intsn);
			r = -EINVAL;
			goto err1;
		}
		r = request_irq(rx->rx_irq, octeon3_eth_rx_handler, 0,
				netdev_name(netdev), rx);
		if (r) {
			netdev_err(netdev, "ERROR: Couldn't request irq: %d\n",
				   rx->rx_irq);
			r = -ENOMEM;
			goto err2;
		}

		octeon3_eth_gen_affinity(priv->numa_node, &rx->rx_affinity_hint);
		irq_set_affinity_hint(rx->rx_irq, &rx->rx_affinity_hint);

		/* Allocate a napi index for this receive context */
		bitmap_zero(priv->rx_cxt[i].napi_idx_bitmap, CVMX_MAX_CORES);
		idx = find_first_zero_bit(priv->rx_cxt[i].napi_idx_bitmap,
					 CVMX_MAX_CORES);
		if (idx >= CVMX_MAX_CORES) {
			netdev_err(netdev, "ERROR: Couldn't get napi index\n");
			r = -ENOMEM;
			goto err3;
		}
		bitmap_set(priv->rx_cxt[i].napi_idx_bitmap, idx, 1);

		cpu = cpumask_first(&rx->rx_affinity_hint);
		priv->rx_cxt[i].napiw = octeon3_napi_alloc(&priv->rx_cxt[i],
							   idx, cpu);
		if (priv->rx_cxt[i].napiw == NULL) {
			r = -ENOMEM;
			goto err4;
		}

		/* Arm the irq. */
		octeon3_eth_sso_irq_set_armed(priv->numa_node, rx->rx_grp,
					      true);
	}
	octeon3_eth_replenish_rx(priv, priv->rx_buf_count);

	r = bgx_port_enable(netdev);
	__cvmx_export_config();

	return r;

 err4:
	bitmap_clear(priv->rx_cxt[i].napi_idx_bitmap, idx, 1);
 err3:
	free_irq(rx->rx_irq, rx);
 err2:
	irq_dispose_mapping(rx->rx_irq);
 err1:
	for (i--; i >= 0; i--) {
		rx = priv->rx_cxt + i;
		irq_dispose_mapping(rx->rx_irq);
		free_irq(rx->rx_irq, rx);
		octeon3_rm_napi_from_cxt(priv->numa_node,
					 priv->rx_cxt[i].napiw);
		priv->rx_cxt[i].napiw = NULL;
	}

	return r;
}

static int octeon3_eth_ndo_stop(struct net_device *netdev)
{
	struct octeon3_ethernet *priv = netdev_priv(netdev);
	void **w;
	struct sk_buff *skb;
	struct octeon3_rx *rx;
	int i;
	int r;

	r = bgx_port_disable(netdev);
	if (r)
		goto err;

	msleep(20);

	for (i = 0; i < priv->num_rx_cxt; i++) {
		rx = priv->rx_cxt + i;
		/* Wait for SSO to drain */
		while (cvmx_read_csr_node(priv->numa_node, CVMX_SSO_GRPX_AQ_CNT(rx->rx_grp)))
			msleep(20);
	}

	for (i = 0; i < priv->num_rx_cxt; i++) {
		rx = priv->rx_cxt + i;
		octeon3_eth_sso_irq_set_armed(priv->numa_node, rx->rx_grp, false);

		irq_set_affinity_hint(rx->rx_irq, NULL);
		free_irq(rx->rx_irq, rx);
		rx->rx_irq = 0;
	}
	msleep(20);

	/* Free the packet buffers */
	for (;;) {
		w = cvmx_fpa3_alloc(__cvmx_fpa3_gaura(priv->numa_node, priv->pki_laura));
		if (!w)
			break;
		skb = w[0];
		dev_kfree_skb(skb);
	}

	/* Free the napis */
	for (i = 0; i < priv->num_rx_cxt; i++) {
		octeon3_rm_napi_from_cxt(priv->numa_node,
					 priv->rx_cxt[i].napiw);
		priv->rx_cxt[i].napiw = NULL;
	}

err:
	return r;
}

/* octeon3_prepare_skb_to_recycle:	Reset all the skb fields to default
 *					values so that the skb can be reused.
 *					Note: the data buffer is not touched.
 *
 *  skb:				skb to reset.
 */
static inline void octeon3_prepare_skb_to_recycle(struct sk_buff *skb)
{
	struct skb_shared_info	*shinfo;

	/* Prepare the skb to be recycled */
	skb->data_len = 0;
	skb_frag_list_init(skb);
	skb_release_head_state(skb);

	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->data = skb->head + NET_SKB_PAD;
	skb_reset_tail_pointer(skb);
	skb->truesize = sizeof(*skb) + skb_end_pointer(skb) - skb->head;
}

static int octeon3_eth_ndo_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct sk_buff *skb_tmp;
	struct octeon3_ethernet *priv = netdev_priv(netdev);
	unsigned int scr_off = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
	unsigned int ret_off = scr_off;
	union cvmx_pko_send_hdr send_hdr;
	union cvmx_pko_buf_ptr buf_ptr;
	union cvmx_pko_send_work send_work;
	union cvmx_pko_send_mem send_mem;
	union cvmx_pko_lmtdma_data lmtdma_data;
	union cvmx_pko_query_rtn query_rtn;
	u8 l4_hdr = 0;
	unsigned int checksum_alg;
	long backlog;
	int frag_count;
	int head_len, i;
	u64 dma_addr;
	void **work;
	bool can_recycle_skb = false;
	int aura = 0;
	void *buffers_needed = NULL;
	void **buf;

	frag_count = 0;
	if (skb_has_frag_list(skb))
		skb_walk_frags(skb, skb_tmp)
			frag_count++;

	/* Check if the skb can be recycled (freed back to the fpa) */
	if (likely(recycle_skbs) &&
	    likely(skb_shinfo(skb)->nr_frags == 0) &&
	    likely(skb_shared(skb) == 0) &&
	    likely(skb_cloned(skb) == 0) &&
	    likely(frag_count == 0) &&
	    likely(skb->fclone == SKB_FCLONE_UNAVAILABLE)) {
		uint64_t	magic;

		buf = (void **)PTR_ALIGN(skb->head, 128);
		magic = (uint64_t)buf[SKB_AURA_OFFSET];
		if (likely(buf[SKB_PTR_OFFSET] == skb) &&
		    likely((magic & 0xfffffffffffff000) == SKB_AURA_MAGIC)) {
			can_recycle_skb = true;
			aura = magic & 0xfff;
			buffers_needed = aura2buffers_needed[aura];
		}
	}

	/* We have space for 13 segment pointers, If there will be
	 * more than that, we must linearize.  The count is: 1 (base
	 * SKB) + frag_count + nr_frags.
	 */
	if (unlikely(skb_shinfo(skb)->nr_frags + frag_count > 12)) {
		if (unlikely(__skb_linearize(skb)))
			goto skip_xmit;
		frag_count = 0;
	}

	work = (void **)skb->cb;
	work[0] = netdev;
	work[1] = NULL;

	backlog = atomic64_inc_return(&priv->tx_backlog);
	if (unlikely(backlog > MAX_TX_QUEUE_DEPTH)) {
		if (use_tx_queues) {
			netif_stop_queue(netdev);
		} else {
			atomic64_dec(&priv->tx_backlog);
			goto skip_xmit;
		}
	}

	/* Adjust the port statistics. */
	atomic64_inc(&priv->tx_packets);
	atomic64_add(skb->len, &priv->tx_octets);

	/* Make sure packet data writes are committed before
	 * submitting the command below
	 */
	wmb();
	send_hdr.u64 = 0;
#ifdef __LITTLE_ENDIAN
	send_hdr.s.le = 1;
#endif
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		send_hdr.s.n2 = 1; /* Don't allocate to L2 */
	send_hdr.s.df = 1; /* Don't automatically free to FPA */
	send_hdr.s.total = skb->len;
	send_hdr.s.aura = aura;

	if (skb->ip_summed != CHECKSUM_NONE) {
#ifndef BROKEN_SIMULATOR_CSUM
		switch (skb->protocol) {
		case __constant_htons(ETH_P_IP):
			send_hdr.s.l3ptr = ETH_HLEN;
			send_hdr.s.ckl3 = 1;
			l4_hdr = ip_hdr(skb)->protocol;
			send_hdr.s.l4ptr = ETH_HLEN + (4 * ip_hdr(skb)->ihl);
			break;
		case __constant_htons(ETH_P_IPV6):
			l4_hdr = ipv6_hdr(skb)->nexthdr;
			break;
		default:
			break;
		}
#endif
		checksum_alg = 1; /* UDP == 1 */
		switch (l4_hdr) {
		case IPPROTO_SCTP:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				break;
			checksum_alg++; /* SCTP == 3 */
			/* Fall through */
		case IPPROTO_TCP: /* TCP == 2 */
			checksum_alg++;
			/* Fall through */
		case IPPROTO_UDP:
			if (skb_transport_header_was_set(skb)) {
				int l4ptr = skb_transport_header(skb) -
					skb->data;
				send_hdr.s.l4ptr = l4ptr;
				send_hdr.s.ckl4 = checksum_alg;
			}
			break;
		default:
			break;
		}
	}
	cvmx_scratch_write64(scr_off, send_hdr.u64);
	scr_off += sizeof(send_hdr);

	buf_ptr.u64 = 0;
	buf_ptr.s.subdc3 = CVMX_PKO_SENDSUBDC_GATHER;

	/* Add a Gather entry for each segment. */
	head_len = skb_headlen(skb);
	if (head_len > 0) {
		buf_ptr.s.size = head_len;
		buf_ptr.s.addr = virt_to_phys(skb->data);
		cvmx_scratch_write64(scr_off, buf_ptr.u64);
		scr_off += sizeof(buf_ptr);
	}
	for (i = 1; i <= skb_shinfo(skb)->nr_frags; i++) {
		struct skb_frag_struct *fs = skb_shinfo(skb)->frags + i - 1;
		buf_ptr.s.size = fs->size;
		buf_ptr.s.addr = virt_to_phys((u8 *)page_address(fs->page.p) + fs->page_offset);
		cvmx_scratch_write64(scr_off, buf_ptr.u64);
		scr_off += sizeof(buf_ptr);
	}
	skb_walk_frags(skb, skb_tmp) {
		buf_ptr.s.addr = virt_to_phys(skb_tmp->data);
		buf_ptr.s.size = skb_tmp->len;
		cvmx_scratch_write64(scr_off, buf_ptr.u64);
		scr_off += sizeof(buf_ptr);
	}

	/* Subtract 1 from the tx_backlog. */
	send_mem.u64 = 0;
	send_mem.s.subdc4 = CVMX_PKO_SENDSUBDC_MEM;
	send_mem.s.dsz = MEMDSZ_B64;
	send_mem.s.alg = MEMALG_SUB;
	send_mem.s.offset = 1;
	send_mem.s.addr = virt_to_phys(&priv->tx_backlog);
	cvmx_scratch_write64(scr_off, send_mem.u64);
	scr_off += sizeof(buf_ptr);

	if (likely(can_recycle_skb)) {
		cvmx_pko_send_free_t	send_free;

		/* Subtract 1 from buffers_needed. */
		send_mem.u64 = 0;
		send_mem.s.subdc4 = CVMX_PKO_SENDSUBDC_MEM;
		send_mem.s.dsz = MEMDSZ_B64;
		send_mem.s.alg = MEMALG_SUB;
		send_mem.s.offset = 1;
		send_mem.s.addr = virt_to_phys(buffers_needed);
		cvmx_scratch_write64(scr_off, send_mem.u64);
		scr_off += sizeof(buf_ptr);

		/* Free buffer when finished with the packet */
		send_free.u64 = 0;
		send_free.s.subdc4 = CVMX_PKO_SENDSUBDC_FREE;
		buf[SKB_PTR_OFFSET] = skb;
		send_free.s.addr = virt_to_phys(buf);
		cvmx_scratch_write64(scr_off, send_free.u64);
		scr_off += sizeof(buf_ptr);

		/* Reset skb before it's freed back to the fpa */
		octeon3_prepare_skb_to_recycle(skb);
	} else {
		/* Send work when finished with the packet. */
		send_work.u64 = 0;
		send_work.s.subdc4 = CVMX_PKO_SENDSUBDC_WORK;
		send_work.s.addr = virt_to_phys(work);
		send_work.s.tt = CVMX_POW_TAG_TYPE_NULL;
		send_work.s.grp = priv->tx_complete_grp;
		cvmx_scratch_write64(scr_off, send_work.u64);
		scr_off += sizeof(buf_ptr);
	}

	lmtdma_data.u64 = 0;
	lmtdma_data.s.scraddr = ret_off >> 3;
	lmtdma_data.s.rtnlen = wait_pko_response ? 1 : 0;
	lmtdma_data.s.did = 0x51;
	lmtdma_data.s.node = priv->numa_node;
	lmtdma_data.s.dq = priv->pko_queue;
	dma_addr = 0xffffffffffffa400ull | ((scr_off & 0x78) - 8);
	cvmx_write64_uint64(dma_addr, lmtdma_data.u64);

	if (wait_pko_response) {
		CVMX_SYNCIOBDMA;

		query_rtn.u64 = cvmx_scratch_read64(ret_off);
		if (unlikely(query_rtn.s.dqstatus != PKO_DQSTATUS_PASS)) {
			netdev_err(netdev, "PKO enqueue failed %llx\n",
				   (unsigned long long)query_rtn.u64);
			dev_kfree_skb_any(skb);
		}
	}

	return NETDEV_TX_OK;
skip_xmit:
	atomic64_inc(&priv->tx_dropped);
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

static u64 read_pki_stat(int numa_node, u64 csr)
{
	u64 v;

	/* PKI-20775, must read until not all ones. */
	do {
		v = cvmx_read_csr_node(numa_node, csr);
	} while (v == 0xffffffffffffffffull);
	return v;
}

static struct rtnl_link_stats64 *octeon3_eth_ndo_get_stats64(struct net_device *netdev,
							     struct rtnl_link_stats64 *s)
{
	struct octeon3_ethernet *priv = netdev_priv(netdev);
	u64 packets, octets, dropped;
	u64 delta_packets, delta_octets, delta_dropped;

	spin_lock(&priv->stat_lock);

	packets = read_pki_stat(priv->numa_node, CVMX_PKI_STATX_STAT0(priv->pki_pkind));
	octets = read_pki_stat(priv->numa_node, CVMX_PKI_STATX_STAT1(priv->pki_pkind));
	dropped = read_pki_stat(priv->numa_node, CVMX_PKI_STATX_STAT3(priv->pki_pkind));

	delta_packets = (packets - priv->last_packets) & ((1ull << 48) - 1);
	delta_octets = (octets - priv->last_octets) & ((1ull << 48) - 1);
	delta_dropped = (dropped - priv->last_dropped) & ((1ull << 48) - 1);

	priv->last_packets = packets;
	priv->last_octets = octets;
	priv->last_dropped = dropped;

	spin_unlock(&priv->stat_lock);

	atomic64_add(delta_packets, &priv->rx_packets);
	atomic64_add(delta_octets, &priv->rx_octets);
	atomic64_add(delta_dropped, &priv->rx_dropped);

	s->rx_packets = atomic64_read(&priv->rx_packets);
	s->rx_bytes = atomic64_read(&priv->rx_octets);
	s->rx_dropped = atomic64_read(&priv->rx_dropped);
	s->rx_errors = atomic64_read(&priv->rx_errors);
	s->rx_length_errors = atomic64_read(&priv->rx_length_errors);
	s->rx_crc_errors = atomic64_read(&priv->rx_crc_errors);

	s->tx_packets = atomic64_read(&priv->tx_packets);
	s->tx_bytes = atomic64_read(&priv->tx_octets);
	s->tx_dropped = atomic64_read(&priv->tx_dropped);
	return s;
}

static int octeon3_eth_set_mac_address(struct net_device *netdev, void *addr)
{
	int r = eth_mac_addr(netdev, addr);

	if (r)
		return r;

	bgx_port_set_rx_filtering(netdev);

	return 0;
}

static const struct net_device_ops octeon3_eth_netdev_ops = {
	.ndo_init		= octeon3_eth_ndo_init,
	.ndo_uninit		= octeon3_eth_ndo_uninit,
	.ndo_open		= octeon3_eth_ndo_open,
	.ndo_stop		= octeon3_eth_ndo_stop,
	.ndo_start_xmit		= octeon3_eth_ndo_start_xmit,
	.ndo_get_stats64	= octeon3_eth_ndo_get_stats64,
	.ndo_set_rx_mode	= bgx_port_set_rx_filtering,
	.ndo_set_mac_address	= octeon3_eth_set_mac_address,
	.ndo_change_mtu		= bgx_port_change_mtu
};

static int octeon3_eth_probe(struct platform_device *pdev)
{
	struct octeon3_ethernet *priv;
	struct net_device *netdev;
	int r;

	struct bgx_platform_data *pd = dev_get_platdata(&pdev->dev);

	r = octeon3_eth_global_init(pd->numa_node);
	if (r)
		return r;

	dev_err(&pdev->dev, "Probing %d-%d:%d\n", pd->numa_node, pd->interface, pd->port);
	netdev = alloc_etherdev(sizeof(struct octeon3_ethernet));
	if (!netdev) {
		dev_err(&pdev->dev, "Failed to allocated ethernet device\n");
		return -ENOMEM;
	}

	/* Using transmit queues degrades performance significantly */
	if (!use_tx_queues)
		netdev->tx_queue_len = 0;

	SET_NETDEV_DEV(netdev, &pdev->dev);
	dev_set_drvdata(&pdev->dev, netdev);

	bgx_port_set_netdev(pdev->dev.parent, netdev);
	priv = netdev_priv(netdev);
	priv->netdev = netdev;
	INIT_LIST_HEAD(&priv->list);
	priv->numa_node = pd->numa_node;

	mutex_lock(&octeon3_eth_node[priv->numa_node].device_list_lock);
	list_add_tail_rcu(&priv->list, &octeon3_eth_node[priv->numa_node].device_list);
	mutex_unlock(&octeon3_eth_node[priv->numa_node].device_list_lock);

	priv->xiface = cvmx_helper_node_interface_to_xiface(pd->numa_node, pd->interface);
	priv->port_index = pd->port;
	spin_lock_init(&priv->stat_lock);
	netdev->netdev_ops = &octeon3_eth_netdev_ops;

	if (register_netdev(netdev) < 0) {
		dev_err(&pdev->dev, "Failed to register ethernet device\n");
		list_del(&priv->list);
		free_netdev(netdev);
	}
	netdev_info(netdev, "Registered\n");
	return 0;
}

static int octeon3_eth_remove(struct platform_device *pdev)
{
//	struct net_device *netdev = dev_get_drvdata(&pdev->dev);
//	struct octeon3_ethernet *priv = netdev_priv(netdev);
	return 0;
}

static void octeon3_eth_shutdown(struct platform_device *pdev)
{
	return;
}


static struct platform_driver octeon3_eth_driver = {
	.probe		= octeon3_eth_probe,
	.remove		= octeon3_eth_remove,
	.shutdown       = octeon3_eth_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "ethernet-mac-pki",
	},
};



static int __init octeon3_eth_init(void)
{
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 0;

	if (rx_contexts <= 0)
		rx_contexts = 1;
	if (rx_contexts > MAX_RX_CONTEXTS)
		rx_contexts = MAX_RX_CONTEXTS;

	return platform_driver_register(&octeon3_eth_driver);
}
module_init(octeon3_eth_init);

static void __exit octeon3_eth_exit(void)
{
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX))
		return;

	platform_driver_unregister(&octeon3_eth_driver);
}
module_exit(octeon3_eth_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Networks <support@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks PKI/PKO Ethernet driver.");
