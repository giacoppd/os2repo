/*
 *   Octeon POW Ethernet Driver
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005-2012 Cavium, Inc.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/string.h>
#include <linux/delay.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-fpa1.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-wqe.h>
#include <asm/octeon/cvmx-pow-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>

#define VIRTUAL_PORT    63	/* Value to put in work->ipprt */
#define CN78XX_SSO_INTSN_EXE 0x61

#define DEBUGPRINT(format, ...) do {					\
		if (printk_ratelimit())					\
			printk(format, ##__VA_ARGS__);			\
	} while (0)

#define DEV_NAME "octeon-pow-ethernet"

static int receive_group = -1;
module_param(receive_group, int, 0444);
MODULE_PARM_DESC(receive_group,
		 " 0-16 POW group to receive packets from. This must be unique in\n"
		 "\t\tthe system. If you don't specify a value, the core ID will\n"
		 "\t\tbe used.");

static int broadcast_groups;
module_param(broadcast_groups, int, 0644);
MODULE_PARM_DESC(broadcast_groups,
		 " Bitmask of groups to send broadcasts to. This MUST be specified.\n"
		 "\t\tWARNING: Be careful to not send broadcasts to groups that aren't\n"
		 "\t\tread otherwise you may fill the POW and stop receiving packets.\n");



static int ptp_rx_group = -1;
module_param(ptp_rx_group, int, 0444);
MODULE_PARM_DESC(ptp_rx_group,
		 "For the PTP POW device, 0-64 POW group to receive packets from.\n"
		 "\t\tIf you don't specify a value, the 'pow0' device will not be created\n.");

static int ptp_tx_group = -1;
module_param(ptp_tx_group, int, 0444);
MODULE_PARM_DESC(ptp_tx_group,
		 "For the PTP POW device, 0-64 POW group to transmit packets to.\n"
		 "\t\tIf you don't specify a value, the 'pow0' device will not be created\n.");

static int pki_packet_pool = 0;
module_param(pki_packet_pool, int, 0644);
MODULE_PARM_DESC(pki_packet_pool,
		 "Pool to use for transmit/receive buffer alloc/frees.\n");

static int reverse_endian;
module_param(reverse_endian, int, 0444);
MODULE_PARM_DESC(reverse_endian,
		 "Link partner is running with different endianness (set on only one end of the link).\n");

static void *memcpy_re_to(void *d, const void *s, size_t n)
{
	u8 *dst = d;
	const u8 *src = s;
	while (n) {
		u8 *pd = (u8 *)((unsigned long)dst ^ 7);
		*pd = *src;
		n--;
		dst++;
		src++;
	}
	return d;
}
static void *memcpy_re_from(void *d, const void *s, size_t n)
{
	u8 *dst = d;
	const u8 *src = s;
	while (n) {
		u8 *ps = (u8 *)((unsigned long)src ^ 7);
		*dst = *ps;
		n--;
		dst++;
		src++;
	}
	return d;
}
static void * (*octeon_pow_copy_to)(void *d, const void *s, size_t n);
static void * (*octeon_pow_copy_from)(void *d, const void *s, size_t n);

/*
 * This is the definition of the Ethernet driver's private
 * driver state.
 */
struct octeon_pow {
	u64 tx_mask;
	int rx_group;
	bool is_ptp;
	int rx_irq;
	int numa_node;
};

static int fpa_wqe_pool = 1;	/* HW FPA pool to use for work queue entries */
static int fpa_packet_pool;	/* HW FPA pool to use for packet buffers */
static int fpa_packet_pool_size = 2048;	/* Size of the packet buffers */
static struct net_device *octeon_pow_oct_dev;
static struct net_device *octeon_pow_ptp_dev;
static int octeon_pow_num_groups;

void *work_to_skb(void *work)
{
	return work - (octeon_has_feature(OCTEON_FEATURE_PKI) ? 0x80 : 0);
}

/*
 * Given a packet data address, return a pointer to the
 * beginning of the packet buffer.
 */
static void *get_buffer_ptr(union cvmx_buf_ptr packet_ptr)
{
	return phys_to_virt(((packet_ptr.s.addr >> 7) -
			     packet_ptr.s.back) << 7);
}

uint64_t oct_get_packet_ptr(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		return cvmx_wqe_get_pki_pkt_ptr(work).u64;
	} else {
		if ((work->word2.s.bufs > 0) || (work->word2.s.software))
		    return work->packet_ptr.s.addr;
		return (cvmx_ptr_to_phys(work) + 32);
	}
}

static int octeon_pow_free_work(cvmx_wqe_t *work)
{
	int segments = cvmx_wqe_get_bufs(work);

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* FIXME */
		int segments = cvmx_wqe_get_bufs(work);
		if (segments > 1) pr_warn(DEV_NAME " WARNING: segments > 1 not yet supported.\n");

		cvmx_fpa_free(work_to_skb(work), cvmx_wqe_get_aura(work), 0);
	} else {
		while (segments--) {
			union cvmx_buf_ptr segment_ptr = work->packet_ptr;
			union cvmx_buf_ptr next_ptr =
				*(union cvmx_buf_ptr *)phys_to_virt(segment_ptr.s.addr - 8);
			if (unlikely(!segment_ptr.s.i))
				cvmx_fpa_free(get_buffer_ptr(segment_ptr),
					     segment_ptr.s.pool, 0);
			segment_ptr = next_ptr;
		}
		cvmx_fpa_free(work, fpa_wqe_pool, 0);
	}

	return 0;
}

static int octeon_pow_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct octeon_pow *priv;
	cvmx_wqe_t *work = NULL;
	void *packet_buffer = NULL;
	void *copy_location;
	u64 send_group_mask;
	int send_group;

	priv = netdev_priv(dev);

	/* Any unknown MAC address goes to all groups in the module
	 * param broadcast_groups. Known MAC addresses use the low
	 * order dest mac byte as the group number.
	 */
	if (!priv->is_ptp && ((*(uint64_t *) (skb->data) >> 16) < 0x01ff))
		send_group_mask = 1ull << (skb->data[5] & (octeon_pow_num_groups - 1));
	else
		send_group_mask = priv->tx_mask;

	/* Remove self from group mask */
	send_group_mask &= ~(1 << priv->rx_group);

	/* It is ugly, but we need to send multiple times for
	 * broadcast packets. The hardware doesn't support submitting
	 * work to multiple groups
	 */
	for (send_group = 0; send_group < octeon_pow_num_groups; send_group++) {
		/* Don't transmit to groups not in our send_group_mask */
		if (likely((send_group_mask & (1ULL << send_group)) == 0))
			continue;

		/* Get a work queue entry */
		work = cvmx_fpa_alloc(fpa_wqe_pool) +
			(octeon_has_feature(OCTEON_FEATURE_FPA3) ? 0x80 : 0);
		if (unlikely(work == NULL)) {
			DEBUGPRINT("%s: Failed to allocate a work queue entry\n",
				   dev->name);
			goto fail;
		}

		/* Get a packet buffer */
		if (octeon_has_feature(OCTEON_FEATURE_PKI))
			/* octeon3-ethernet uses a different fpa/packet system */
			packet_buffer = ((void*)work) + 0x80;
		else
			packet_buffer = cvmx_fpa_alloc(fpa_packet_pool);
		if (unlikely(packet_buffer == NULL)) {
			DEBUGPRINT("%s: Failed to allocate a packet buffer\n",
				   dev->name);
			goto fail;
		}

		/* Calculate where we need to copy the data to. We
		 * need to leave 8 bytes for a next pointer
		 * (unused). Then we need to align the IP packet src
		 * and dest into the same 64bit word.
		 */
		copy_location = packet_buffer + sizeof(uint64_t) + 6;

		/* Fail if the packet won't fit in a single buffer */
		if (unlikely
		    (copy_location + skb->len >
		     packet_buffer + fpa_packet_pool_size)) {
			DEBUGPRINT("%s: Packet too large for FPA buffer\n",
				   dev->name);
			goto fail;
		}

		octeon_pow_copy_to(copy_location, skb->data, skb->len);

		/* Fill in some of the work queue fields. We may need
		 * to add more if the software at the other end needs
		 * them.
		 */
		work->word0.u64 = 0;
		work->word2.u64 = 0;	/* Default to zero. Sets of zero later
					   are commented out */
#if 0
		work->hw_chksum = skb->csum;
#endif

		cvmx_wqe_set_len(work, skb->len);
		cvmx_wqe_set_port(work, VIRTUAL_PORT);
		cvmx_wqe_set_qos(work, 0);
		cvmx_wqe_set_grp(work, send_group);
		cvmx_wqe_set_tt(work, 2);
		cvmx_wqe_set_tag(work, 0);

		cvmx_wqe_set_bufs(work, 1);
		cvmx_wqe_set_aura(work, fpa_wqe_pool);

		if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
			/* We use a 78xx format wqe where necessary */
			union cvmx_buf_ptr_pki pki_ptr;
			cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t*) work;
			pki_ptr.u64 = 0;
			pki_ptr.addr = virt_to_phys(copy_location);
			pki_ptr.packet_outside_wqe = 0;
			pki_ptr.size = skb->len;
			wqe->packet_ptr.u64 = pki_ptr.u64;
			/* Mark errata as handled to prevent additional byteswap */
			wqe->pki_errata20776 = 1;
		} else {
			work->packet_ptr.u64 = 0;
			work->packet_ptr.s.addr = virt_to_phys(copy_location);
			work->packet_ptr.s.pool = fpa_packet_pool;
			work->packet_ptr.s.size = fpa_packet_pool_size;
			work->packet_ptr.s.back = (copy_location - packet_buffer) >> 7;
		}
		if (skb->protocol == htons(ETH_P_IP)) {
			cvmx_wqe_set_l3_offset(work, 14);
			cvmx_wqe_set_l4_udp(work, ip_hdr(skb)->protocol == IPPROTO_UDP);
			if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE) ||
				(ip_hdr(skb)->protocol == IPPROTO_TCP))
				cvmx_wqe_set_l4_tcp(work, ip_hdr(skb)->protocol == IPPROTO_TCP);
			cvmx_wqe_set_l3_frag(work, !((ip_hdr(skb)->frag_off == 0)
						     || (ip_hdr(skb)->frag_off ==
							 1 << 14)));
			cvmx_wqe_set_l2_bcast(work, skb->pkt_type == PACKET_BROADCAST);
			cvmx_wqe_set_l2_mcast(work, skb->pkt_type == PACKET_MULTICAST);
#if 0
			work->word2.s.vlan_valid = 0;	/* FIXME */
			work->word2.s.vlan_cfi = 0;	/* FIXME */
			work->word2.s.vlan_id = 0;	/* FIXME */
			work->word2.s.dec_ipcomp = 0;	/* FIXME */
			if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
				work->word2.s.IP_exc = 0; /* Assume Linux is sending
							     a good packet */
			work->word2.s.IP_exc = 0; /* Assume Linux is sending
						     a good packet */
			work->word2.s.not_IP = 0; /* This is an IP packet */
			work->word2.s.rcv_error = 0; /* No error, packet is
							internal */
			work->word2.s.err_code = 0;  /* No error, packet is
							internal */
#endif

			/* When copying the data, include 4 bytes of the
			   ethernet header to align the same way hardware does */
			octeon_pow_copy_to(work->packet_data, skb->data + 10,
			       sizeof(work->packet_data));
		} else {
#if 0
			work->word2.snoip.is_rarp =
				skb->protocol == htons(ETH_P_RARP);
			work->word2.snoip.is_arp =
				skb->protocol == htons(ETH_P_ARP);
			work->word2.snoip.is_bcast =
				(skb->pkt_type == PACKET_BROADCAST);
			work->word2.snoip.is_mcast =
				(skb->pkt_type == PACKET_MULTICAST);
#endif
			cvmx_wqe_set_l3_ipv4(work, 0);
			octeon_pow_copy_to(work->packet_data, skb->data,
			       sizeof(work->packet_data));
		}

		/*
		 * Submit the packet to the POW
		 * tag: 0
		 * tag_type: 2
		 * qos: 0
		 * grp: send_group
		 */
		if (octeon_has_feature(OCTEON_FEATURE_PKI))
			cvmx_pow_work_submit_node(work, 0, 2, send_group, priv->numa_node);
		else
			cvmx_pow_work_submit(work, 0, 2, 0, send_group);
		work = NULL;
		packet_buffer = NULL;
	}

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;

fail:
	if (work)
		cvmx_fpa_free(work_to_skb(work), fpa_wqe_pool, 0);
	if (packet_buffer && !octeon_has_feature(OCTEON_FEATURE_PKI))
		cvmx_fpa_free(packet_buffer, fpa_packet_pool, 0);
	dev->stats.tx_dropped++;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}


/**
 * Interrupt handler. The interrupt occurs whenever the POW
 * transitions from 0->1 packets in our group.
 *
 * @param cpl
 * @param dev_id
 * @param regs
 * @return
 */
static irqreturn_t octeon_pow_interrupt(int cpl, void *dev_id)
{
	const uint64_t coreid = cvmx_get_core_num();
	struct net_device *dev = (struct net_device *) dev_id;
	struct octeon_pow *priv;
	uint64_t old_group_mask = 0;
	cvmx_wqe_t *work;
	struct sk_buff *skb;

	priv = netdev_priv(dev);

	/* Make sure any userspace operations are complete */
	asm volatile ("synciobdma" : : : "memory");

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* Can get-work from group explicitly here */
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Only allow work for our group */
		old_group_mask = cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
		cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(coreid),
			       1ull << priv->rx_group);
		/* Read it back so it takes effect before we request work */
		cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
	} else {
		/* Only allow work for our group */
		old_group_mask = cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(coreid));
		cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid), 1 << priv->rx_group);
	}
	while (1) {
		if (octeon_has_feature(OCTEON_FEATURE_PKI))
			work = cvmx_sso_work_request_grp_sync_nocheck(priv->rx_group,
				CVMX_POW_NO_WAIT);
		else
			work = cvmx_pow_work_request_sync(0);
		if (work == NULL)
			break;

		/* Silently drop packets if we aren't up */
		if ((dev->flags & IFF_UP) == 0) {
			octeon_pow_free_work(work);
			continue;
		}

		/* Throw away all packets with receive errors */
		if (unlikely(cvmx_wqe_get_rcv_err(work))) {
			DEBUGPRINT("%s: Receive error code %d, packet dropped\n",
				   dev->name, cvmx_wqe_get_rcv_err(work));
			octeon_pow_free_work(work);
			dev->stats.rx_errors++;
			continue;
		}

		/* We have to copy the packet. First allocate an skbuff for it */
		skb = dev_alloc_skb(cvmx_wqe_get_len(work));
		if (!skb) {
			DEBUGPRINT("%s: Failed to allocate skbuff, packet dropped\n",
				   dev->name);
			octeon_pow_free_work(work);
			dev->stats.rx_dropped++;
			continue;
		}

		/* Check if we've received a packet that was entirely
		 * stored the work entry. This is untested
		 */
		if (unlikely(cvmx_wqe_get_bufs(work) == 0)) {
			int len = cvmx_wqe_get_len(work);
			DEBUGPRINT("%s: Received a work with work->word2.s.bufs=0, untested\n",
				   dev->name);
			octeon_pow_copy_from(skb_put(skb, len),
					     phys_to_virt(oct_get_packet_ptr(work)), len);
		} else {
			int segments = cvmx_wqe_get_bufs(work);
			uint64_t buf_desc = oct_get_packet_ptr(work);
			int len = cvmx_wqe_get_len(work);
			while (segments--) {
				int segment_size;
				uint64_t pkt_ptr;
				if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
					cvmx_buf_ptr_pki_t pki_ptr;
					pki_ptr.u64 = buf_desc;
					segment_size = pki_ptr.size;
					pkt_ptr = pki_ptr.addr;
					buf_desc = *((uint64_t*)phys_to_virt(pki_ptr.addr - 8));
				} else {
					cvmx_buf_ptr_t buf_ptr;
					union cvmx_buf_ptr next_ptr;
					buf_ptr.u64 = buf_desc;
					next_ptr = *(union cvmx_buf_ptr *)
						phys_to_virt(buf_ptr.s.addr - 8);
					/* Octeon Errata PKI-100: The segment size is
					   wrong. Until it is fixed, calculate the
					   segment size based on the packet pool buffer
					   size. When it is fixed, the following line
					   should be replaced with this one: int
					   segment_size = segment_ptr.s.size; */
					segment_size =
						fpa_packet_pool_size -
						(buf_ptr.s.addr -
						 (((buf_ptr.s.addr >> 7) -
						   buf_ptr.s.back) << 7));
					/* Don't copy more than what is left in the
					   packet */
					pkt_ptr = buf_ptr.s.addr;
					buf_desc = next_ptr.u64;
				}
				if (segment_size > len)
					segment_size = len;
				/* Copy the data into the packet */
				octeon_pow_copy_from(skb_put(skb, segment_size),
						     phys_to_virt(pkt_ptr),
						     segment_size);
				/* Reduce the amount of bytes left to copy */
				len -= segment_size;
			}
		}
		octeon_pow_free_work(work);
		skb->protocol = eth_type_trans(skb, dev);
		skb->dev = dev;
		skb->ip_summed = CHECKSUM_NONE;
		dev->stats.rx_bytes += skb->len;
		dev->stats.rx_packets++;
		netif_rx(skb);
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* Clear interrupt */
		cvmx_write_csr_node(priv->numa_node, CVMX_SSO_GRPX_INT(priv->rx_group), 2);
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Restore the original POW group mask */
		cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(coreid), old_group_mask);
		/* Read it back so it takes effect before ?? */
		cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));

		/* Acknowledge the interrupt */
		cvmx_write_csr(CVMX_SSO_WQ_INT, 1ull << priv->rx_group);
	} else {
		/* Restore the original POW group mask */
		cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid), old_group_mask);

		/* Acknowledge the interrupt */
		cvmx_write_csr(CVMX_POW_WQ_INT, 1ull << priv->rx_group);
	}
	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER

static void octeon_pow_poll(struct net_device *dev)
{
	octeon_pow_interrupt(0, dev);
}
#endif

static int octeon_pow_open(struct net_device *dev)
{
	int r;
	struct octeon_pow *priv = netdev_priv(dev);

	/* Clear the statistics whenever the interface is brought up */
	memset(&dev->stats, 0, sizeof(dev->stats));

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		int sso_intsn = (CN78XX_SSO_INTSN_EXE << 12) | priv->rx_group;
		struct irq_domain *d = octeon_irq_get_block_domain(priv->numa_node,
								   sso_intsn);
		priv->rx_irq = irq_create_mapping(d, sso_intsn);
		if (!priv->rx_irq) {
			netdev_err(dev, "ERROR: Couldn't map hwirq: %x\n",
				   sso_intsn);
			return -EINVAL;
		}
	} else
		priv->rx_irq = OCTEON_IRQ_WORKQ0 + priv->rx_group;
	/* Register an IRQ hander for to receive POW interrupts */
	r = request_irq(priv->rx_irq, octeon_pow_interrupt, 0, dev->name, dev);
	if (r)
		return r;

	/* Enable POW interrupt when our port has at least one packet */
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		union cvmx_sso_grpx_int_thr thr;
		union cvmx_sso_grpx_int grp_int;
		thr.u64 = 0;
		thr.cn78xx.ds_thr = 1;
		thr.cn78xx.iaq_thr = 1;
		cvmx_write_csr_node(priv->numa_node, CVMX_SSO_GRPX_INT_THR(priv->rx_group),
				    thr.u64);
		grp_int.u64 = 0;
		grp_int.s.exe_int = 1;
		cvmx_write_csr_node(priv->numa_node, CVMX_SSO_GRPX_INT(priv->rx_group), grp_int.u64);
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		union cvmx_sso_wq_int_thrx thr;
		thr.u64 = 0;
		thr.s.iq_thr = 1;
		thr.s.ds_thr = 1;
		cvmx_write_csr(CVMX_SSO_WQ_INT_THRX(priv->rx_group), thr.u64);
	} else {
		union cvmx_pow_wq_int_thrx thr;
		thr.u64 = 0;
		thr.s.iq_thr = 1;
		thr.s.ds_thr = 1;
		cvmx_write_csr(CVMX_POW_WQ_INT_THRX(priv->rx_group), thr.u64);
	}

	return 0;
}

static int octeon_pow_stop(struct net_device *dev)
{
	struct octeon_pow *priv = netdev_priv(dev);

	/* Disable POW interrupt */
	if (octeon_has_feature(OCTEON_FEATURE_PKI))
		cvmx_write_csr_node(priv->numa_node, CVMX_SSO_GRPX_INT_THR(priv->rx_group), 0);
	else if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		cvmx_write_csr(CVMX_SSO_WQ_INT_THRX(priv->rx_group), 0);
	else
		cvmx_write_csr(CVMX_POW_WQ_INT_THRX(priv->rx_group), 0);

	/* Free the interrupt handler */
	free_irq(priv->rx_irq, dev);
	return 0;
}

/**
 * Per network device initialization
 *
 * @param dev    Device to initialize
 * @return Zero on success
 */
static int octeon_pow_init(struct net_device *dev)
{
	struct octeon_pow *priv = netdev_priv(dev);

	dev->features |= NETIF_F_LLTX;	/* We do our own locking, Linux doesn't
					   need to */
	dev->dev_addr[0] = 0;
	dev->dev_addr[1] = 0;
	dev->dev_addr[2] = 0;
	dev->dev_addr[3] = 0;
	dev->dev_addr[4] = priv->is_ptp ? 3 : 1;
	dev->dev_addr[5] = priv->rx_group;
	priv->numa_node = cvmx_get_node_num();
	return 0;
}

static const struct net_device_ops octeon_pow_netdev_ops = {
	.ndo_init = octeon_pow_init,
	.ndo_open = octeon_pow_open,
	.ndo_stop = octeon_pow_stop,
	.ndo_start_xmit = octeon_pow_xmit,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller =  octeon_pow_poll,
#endif
};


/*
 * Module/ driver initialization. Creates the linux network
 * devices.
 *
 * @return Zero on success
 */
static int __init octeon_pow_mod_init(void)
{
	struct octeon_pow *priv;
	u64 allowed_group_mask;

	if (reverse_endian) {
		octeon_pow_copy_to = memcpy_re_to;
		octeon_pow_copy_from = memcpy_re_from;
	} else {
		octeon_pow_copy_to = memcpy;
		octeon_pow_copy_from = memcpy;
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* Actually 256 groups total, only 64 currently supported */
		octeon_pow_num_groups = 64;
		allowed_group_mask = 0xffffffffffffffffull;
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		octeon_pow_num_groups = 64;
		allowed_group_mask = 0xffffffffffffffffull;
	} else {
		octeon_pow_num_groups = 16;
		allowed_group_mask = 0xffffull;
	}

	/* If a receive group isn't specified, default to the core id */
	if (receive_group < 0)
		receive_group = cvmx_get_core_num();


	if ((receive_group > octeon_pow_num_groups)) {
		pr_err(DEV_NAME " ERROR: Invalid receive group. Must be 0-%d\n",
		       octeon_pow_num_groups - 1);
		return -1;
	}

	if (!broadcast_groups) {
		pr_err(DEV_NAME " ERROR: You must specify a broadcast group mask.\n");
		return -1;
	}

	if ((broadcast_groups & allowed_group_mask) != broadcast_groups) {
		pr_err(DEV_NAME " ERROR: Invalid broadcast group mask.\n");
		return -1;
	}

	if ((ptp_rx_group >= 0 && ptp_tx_group < 0) || (ptp_rx_group < 0 && ptp_tx_group >= 0)) {
		pr_err(DEV_NAME " ERROR: Both ptp_rx_group AND ptp_tx_group must be set.\n");
		return -1;
	}

	if (ptp_rx_group >= 0 && ptp_tx_group == ptp_rx_group) {
		pr_err(DEV_NAME " ERROR: ptp_rx_group and ptp_tx_group must differ.\n");
		return -1;
	}

	if (ptp_rx_group >= octeon_pow_num_groups || ptp_tx_group >= octeon_pow_num_groups) {
		pr_err(DEV_NAME " ERROR: ptp_rx_group and ptp_tx_group. Must be 0-%d\n",
		       octeon_pow_num_groups - 1);
		return -1;
	}

	if (receive_group == ptp_rx_group) {
		pr_err(DEV_NAME " ERROR: ptp_rx_group(%d) and  receive_group(%d) must differ.\n",
			ptp_rx_group, receive_group);
		return -1;
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKI) && (pki_packet_pool == 0)) {
		pr_err(DEV_NAME " ERROR: pki_packet_pool must be specified for CN78XX.\n");
	}

	pr_info("Octeon POW only ethernet driver\n");

	/* Setup is complete, create the virtual ethernet devices */
	octeon_pow_oct_dev = alloc_etherdev(sizeof(struct octeon_pow));
	if (octeon_pow_oct_dev == NULL) {
		pr_err(DEV_NAME " ERROR: Failed to allocate ethernet device\n");
		return -1;
	}

	octeon_pow_oct_dev->netdev_ops = &octeon_pow_netdev_ops;
	strcpy(octeon_pow_oct_dev->name, "oct%d");

	/* Initialize the device private structure. */
	priv = netdev_priv(octeon_pow_oct_dev);
	priv->rx_group = receive_group;
	priv->tx_mask = broadcast_groups;
	priv->numa_node = cvmx_get_node_num();


	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* Spin waiting for another core to setup all the hardware */
		printk("Waiting for another core to setup the PKI hardware...");
		while ((cvmx_read_csr_node(priv->numa_node, CVMX_PKI_BUF_CTL) & 1) == 0)
			mdelay(100);

		printk("Done\n");
		fpa_packet_pool_size = 2048;
		fpa_packet_pool = fpa_wqe_pool = pki_packet_pool;

		if (fpa_packet_pool < 0) {
			netdev_err(octeon_pow_oct_dev, "ERROR: Failed to initialize fpa pool\n");
			return -1;
		}
	} else {
		/* Spin waiting for another core to setup all the hardware */
		printk("Waiting for another core to setup the IPD hardware...");
		while ((cvmx_read_csr(CVMX_IPD_CTL_STATUS) & 1) == 0)
			mdelay(100);

		printk("Done\n");
		/* Read the configured size of the FPA packet buffers. This
		 * way we don't need changes if someone chooses to use a
		 * different buffer size
		 */
		fpa_packet_pool_size = (cvmx_read_csr(CVMX_IPD_PACKET_MBUFF_SIZE) & 0xfff) * 8;

		/* Read the work queue pool */
		fpa_packet_pool = fpa_wqe_pool = cvmx_read_csr(CVMX_IPD_WQE_FPA_QUEUE) & 7;
	}

	if (register_netdev(octeon_pow_oct_dev) < 0) {
		netdev_err(octeon_pow_oct_dev, "ERROR: Failed to register ethernet device\n");
		free_netdev(octeon_pow_oct_dev);
		return -1;
	}

	if (ptp_rx_group < 0)
		return 0;

	/* Else create a ptp device. */
	octeon_pow_ptp_dev = alloc_etherdev(sizeof(struct octeon_pow));
	if (octeon_pow_ptp_dev == NULL) {
		pr_err(DEV_NAME " ERROR: Failed to allocate ethernet device\n");
		return -1;
	}

	octeon_pow_ptp_dev->netdev_ops = &octeon_pow_netdev_ops;
	strcpy(octeon_pow_ptp_dev->name, "pow%d");

	/* Initialize the device private structure. */
	priv = netdev_priv(octeon_pow_ptp_dev);
	priv->rx_group = ptp_rx_group;
	priv->tx_mask = 1ull << ptp_tx_group;
	priv->is_ptp = true;

	if (register_netdev(octeon_pow_ptp_dev) < 0) {
		netdev_err(octeon_pow_ptp_dev, "ERROR: Failed to register ethernet device\n");
		free_netdev(octeon_pow_ptp_dev);
		return -1;
	}

	return 0;
}

/**
 * Module / driver shutdown
 *
 * @return Zero on success
 */
static void __exit octeon_pow_mod_exit(void)
{
	/* Free the ethernet devices */
	unregister_netdev(octeon_pow_oct_dev);
	free_netdev(octeon_pow_oct_dev);
	if (octeon_pow_ptp_dev) {
		unregister_netdev(octeon_pow_ptp_dev);
		free_netdev(octeon_pow_ptp_dev);
	}
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Inc. <support@cavium.com>");
MODULE_DESCRIPTION("Cavium Inc. OCTEON internal only POW ethernet driver.");
module_init(octeon_pow_mod_init);
module_exit(octeon_pow_mod_exit);
