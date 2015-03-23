/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/*
 * File version info: $Rev$
 *
 * PKOv3 helper file
 */
//#define	__SUPPORT_PFC_ON_XAUI

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-pko3-resources.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-pko.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#else
#include "cvmx.h"
#include "cvmx-ilk.h"
#include "cvmx-fpa3.h"
#include "cvmx-pko3.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-helper.h"
#include "cvmx-helper-pko.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-helper-bgx.h"
#include "cvmx-helper-cfg.h"
#endif

/* bootloader has limited memory, not much traffic */
#if defined(__U_BOOT__)
#define CVMX_PKO3_POOL_BUFFERS (1024)
#endif

/*
 * PKO3 requires 4 buffers for each active Descriptor Queue,
 * and because it is not known how many DQs will in fact be used
 * when the PKO pool is populated, it is allocated the maximum
 * number it may required.
 * The additional 1K buffers are provisioned to acomodate
 * jump buffers for long descriptor commands, that should
 * be rarely used.
 */
#ifndef CVMX_PKO3_POOL_BUFFERS
/* Calculate buffer count based on maximum queue depth if defined */
#ifdef CVMX_PKO3_DQ_MAX_DEPTH
/* Number of command words per buffer */
#define _NUMW	(4*1024/8)
/* Assume 8 concurrently active, fully loaded queues */
#define _NUMQ	1024
/* Assume worst case of 16 words per command per DQ */
#define _DQ_SZ	16
/* Combine the above guesstimates into the total buffer count */
#define CVMX_PKO3_POOL_BUFFERS (CVMX_PKO3_DQ_MAX_DEPTH*_DQ_SZ/_NUMW*_NUMQ+1024)
#else
#define CVMX_PKO3_POOL_BUFFERS (1024*4+1024)
#endif
#endif

/* Simulator has limited memory, use fewer buffers */
#ifndef	CVMX_PKO3_POOL_BUFS_SIM
#define CVMX_PKO3_POOL_BUFS_SIM (1024*4+1024)
#endif

/* channels are present at L2 queue level by default */
static const int cvmx_pko_default_channel_level = 0;

static const int debug = 0;

/* These global variables are relevant for boot CPU only */
static CVMX_SHARED cvmx_fpa3_gaura_t __cvmx_pko3_aura[CVMX_MAX_NODES];

/* This constant can not be modified, defined here for clarity only */
#define CVMX_PKO3_POOL_BUFFER_SIZE 4096 /* 78XX PKO requires 4KB */

/**
 * @INTERNAL
 *
 * Build an owner tag based on interface/port
 */
static int __cvmx_helper_pko3_res_owner(int ipd_port)
{
	int res_owner;
	const int res_owner_pfix = 0x19d0 << 14;

	ipd_port &= 0x3fff;	/* 12-bit for local CHAN_E value + node */

	res_owner = res_owner_pfix | ipd_port;

	return res_owner;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Configure an AURA/POOL designated for PKO internal use.
 *
 * This pool is used for (a) memory buffers that store PKO descriptor queues,
 * (b) buffers for use with PKO_SEND_JUMP_S sub-header.
 *
 * The buffers of type (a) are never accessed by software, and their number
 * should be at least equal to 4 times the number of descriptor queues
 * in use.
 *
 * Type (b) buffers are consumed by PKO3 command-composition code,
 * and are released by the hardware upon completion of transmission.
 *
 * @returns -1 if the pool could not be established or 12-bit AURA
 * that includes the node number for use in PKO3 intialization call.
 *
 * NOTE: Linux kernel should pass its own aura to PKO3 initialization
 * function so that the buffers can be mapped into kernel space
 * for when software needs to adccess their contents.
 *
 */
static int __cvmx_pko3_config_memory(unsigned node)
{
	cvmx_fpa3_gaura_t aura;
	int aura_num;
	unsigned buf_count;

	/* Simulator has limited memory */
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		buf_count = CVMX_PKO3_POOL_BUFS_SIM;
	else
		buf_count = CVMX_PKO3_POOL_BUFFERS;

	cvmx_dprintf("%s: creating AURA with %u buffers\n",
		__func__, buf_count);

	aura = cvmx_fpa3_setup_aura_and_pool(node, -1,
		"PKO3 AURA", NULL,
		CVMX_PKO3_POOL_BUFFER_SIZE, buf_count);

	if (!__cvmx_fpa3_aura_valid(aura)) {
		cvmx_printf("ERROR: %s AURA create failed\n", __func__);
		return -1;
	}

	aura_num = aura.node << 10 | aura.laura;

	/* Store handle for destruction */
	__cvmx_pko3_aura[node] = aura;

	return aura_num;
}


#endif

/** Initialize a single ILK link
 *
 * Each ILK link is one interface, the port portion of IPD
 * represents a logical channel.
 * The number of channels for each interface is derived from the ILK
 * module configuration.
 */
static int __cvmx_pko3_config_ilk_interface(int xiface,
	unsigned num_dq, bool prioritized)
{
	int l1_q_num;
	int l2_q_num;
	int res;
	int res_owner;
	int pko_mac_num;
	unsigned num_chans, i;
	uint16_t ipd_port;
	int prio;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (prioritized && num_dq > 1)
		prio = num_dq;
	else
		prio = -1;

	num_chans = __cvmx_helper_ilk_enumerate(xiface);

	if(debug)
		cvmx_dprintf("%s: configuring ILK xiface %u:%u with "
				"%u chans %u queues each\n",
				__FUNCTION__, xi.node, xi.interface,
				num_chans, num_dq);

	/* ILK channels all go to the same mac */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, 0);
	if (pko_mac_num < 0) {
                cvmx_dprintf ("%s: ERROR Invalid interface\n", __FUNCTION__);
		return -1;
	}

	/* Resources of all channels on this link have common owner */
	ipd_port = cvmx_helper_get_ipd_port(xiface, 0);

	/* Build an identifiable owner */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = pko_mac_num;
        l1_q_num = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_PORT_QUEUES,
				res_owner, l1_q_num, 1);

	if (l1_q_num != pko_mac_num) {
                cvmx_dprintf ("%s: ERROR Reserving L1 PQ\n", __FUNCTION__);
		return -1;
	}


        /* allocate level 2 queues, one per channel */
        l2_q_num = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L2_QUEUES, res_owner,
					 -1, num_chans);
        if (l2_q_num < 0) {
                cvmx_dprintf ("%s: ERROR allocation L2 SQ\n", __FUNCTION__);
                return -1;
        }


	/* Configre <num_chans> children for MAC, with Fair-RR scheduling */
	res = cvmx_pko3_pq_config_children( xi.node,
			pko_mac_num, l2_q_num, num_chans, -1);

	if (res < 0) {
		cvmx_dprintf("%s: ERROR: Could not setup ILK Channel queues\n",
			__FUNCTION__);
		return -1;
	}

	/* Configure children with one DQ per channel */
	for (i = 0; i < num_chans; i++) {
		int l3_q, l4_q, l5_q, dq, res;

		l3_q = l4_q = l5_q = dq = -1;
		ipd_port = cvmx_helper_get_ipd_port(xiface, i);

		/* map channels to l2 queues */
		cvmx_pko3_map_channel(xi.node, l1_q_num, l2_q_num+i, ipd_port);

		l3_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L3_QUEUES,
			res_owner, -1, 1);
		if(l3_q < 0) goto _fail;

		res = cvmx_pko3_sq_config_children(xi.node, 2, l2_q_num+i, l3_q, 1, 1);
		if(res < 0) goto _fail;

		l4_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L4_QUEUES,
			res_owner, -1, 1);
		if(l4_q < 0) goto _fail;
		res = cvmx_pko3_sq_config_children(xi.node, 3, l3_q, l4_q, 1, 1);
		if(res < 0) goto _fail;

		l5_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L5_QUEUES,
			res_owner, -1, 1);
		if(l5_q < 0) goto _fail;
		res = cvmx_pko3_sq_config_children(xi.node, 4, l4_q, l5_q, 1, 1);
		if(res < 0) goto _fail;

		dq = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_DESCR_QUEUES,
			res_owner, -1, num_dq);
		if(dq < 0) goto _fail;

		res = cvmx_pko3_sq_config_children(xi.node, 5, l5_q,
			dq, num_dq, prio);
		if(res < 0) goto _fail;

		/* register DQ range with the translation table */
		res = __cvmx_pko3_ipd_dq_register(xiface, i, dq, num_dq);
		if(res < 0) goto _fail;
	}

	return 0;
  _fail:
	cvmx_dprintf("ERROR: %s: configuring queues for xiface %u:%u chan %u\n",
				__FUNCTION__, xi.node, xi.interface, i);
	return -1;
}


/** Initialize a channelized port
 * This is intended for LOOP and NPI interfaces which have one MAC
 * per interface and need a channel per subinterface (e.g. ring).
 * This function is somewhat similar to __cvmx_pko3_config_ilk_interface()
 * but are kept separate for easier maintenance.
 */
static int __cvmx_pko3_config_chan_interface( int xiface, unsigned num_chans,
	uint8_t num_queues, bool prioritized)
{
	int l1_q_num;
	int l2_q_num;
	int res;
	int pko_mac_num;
	uint16_t ipd_port;
	int res_owner, prio;
	unsigned i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (prioritized && num_queues > 1)
		prio = num_queues;
	else
		prio = -1;

	if(debug)
		cvmx_dprintf("%s: configuring xiface %u:%u with "
				"%u chans %u queues each\n",
				__FUNCTION__, xi.node, xi.interface,
				num_chans, num_queues);

	/* all channels all go to the same mac */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, 0);
	if (pko_mac_num < 0) {
                cvmx_dprintf ("%s: ERROR Invalid interface\n", __FUNCTION__);
		return -1;
	}

	/* Resources of all channels on this port have common owner */
	ipd_port = cvmx_helper_get_ipd_port(xiface, 0);

	/* Build an identifiable owner */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = pko_mac_num;
        l1_q_num = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_PORT_QUEUES,
				res_owner, l1_q_num, 1);

	if (l1_q_num != pko_mac_num) {
                cvmx_dprintf ("%s: ERROR Reserving L1 PQ\n", __FUNCTION__);
		return -1;
	}


        /* allocate level 2 queues, one per channel */
        l2_q_num = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L2_QUEUES, res_owner,
					 -1, num_chans);
        if (l2_q_num < 0) {
                cvmx_dprintf ("%s: ERROR allocation L2 SQ\n", __FUNCTION__);
                return -1;
        }


	/* Configre <num_chans> children for MAC, non-prioritized */
	res = cvmx_pko3_pq_config_children( xi.node,
			l1_q_num, l2_q_num, num_chans, -1);

	if (res < 0) {
		cvmx_dprintf("%s: ERROR: Failed channel queues\n",
			__FUNCTION__);
		return -1;
	}

	/* Configure children with one DQ per channel */
	for (i = 0; i < num_chans; i++) {
		int l3_q, l4_q, l5_q, dq, res;
		unsigned chan = i;

		l3_q = l4_q = l5_q = dq = -1;

		ipd_port = cvmx_helper_get_ipd_port(xiface, chan);

		/* map channels to l2 queues */
		cvmx_pko3_map_channel(xi.node, l1_q_num, l2_q_num+chan,
			ipd_port);

		l3_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L3_QUEUES,
			res_owner, -1, 1);
		if(l3_q < 0) goto _fail;

		res = cvmx_pko3_sq_config_children(xi.node, 2, l2_q_num+chan,
			l3_q, 1, 1);
		if(res < 0) goto _fail;

		l4_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L4_QUEUES,
			res_owner, -1, 1);
		if(l4_q < 0) goto _fail;

		res = cvmx_pko3_sq_config_children(xi.node, 3,
			l3_q, l4_q, 1, 1);
		if(res < 0) goto _fail;

		l5_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L5_QUEUES,
			res_owner, -1, 1);
		if(l5_q < 0) goto _fail;
		res = cvmx_pko3_sq_config_children(xi.node, 4,
			l4_q, l5_q, 1, 1);
		if(res < 0) goto _fail;

		dq = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_DESCR_QUEUES,
			res_owner, -1, num_queues);
		if(dq < 0) goto _fail;

		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0) && (dq & 7))
			cvmx_dprintf("WARNING: %s: DQ# %u not integral of 8\n",
				__func__, dq);

		res = cvmx_pko3_sq_config_children(xi.node, 5, l5_q,
			dq, num_queues, prio);
		if(res < 0) goto _fail;

		/* register DQ range with the translation table */
		res = __cvmx_pko3_ipd_dq_register(xiface, chan, dq, num_queues);
		if(res < 0) goto _fail;
	}

	return 0;
  _fail:
	cvmx_dprintf("ERROR: %s: configuring queues for xiface %u:%u chan %u\n",
				__FUNCTION__, xi.node, xi.interface, i);
	return -1;
}

/** Initialize a single Ethernet port with PFC-style channels
 *
 * One interface can contain multiple ports, this function is per-port
 * Here, a physical port is allocated 8 logical channel, one per VLAN
 * tag priority, one DQ is assigned to each channel, and all 8 DQs
 * are registered for that IPD port.
 * Note that the DQs are arrange such that the Ethernet QoS/PCP field
 * can be used as an offset to the value returned by cvmx_pko_base_queue_get().
 *
 * For HighGig2 mode, 16 channels may be desired, instead of 8,
 * but this function does not support that.
 */
static int __cvmx_pko3_config_pfc_interface(int xiface, unsigned port)
{
	int l1_q_num;
	int l2_q_num;
	int res;
	int pko_mac_num;
	int l3_q, l4_q, l5_q, dq;
	const unsigned num_chans = 8;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned node = xi.node;
	uint16_t ipd_port;
	int res_owner;
	unsigned i;

	if(debug)
		cvmx_dprintf("%s: configuring xiface %u:%u port %u with %u PFC channels\n",
			__FUNCTION__, node, xi.interface, port, num_chans);

	/* Get MAC number for the iface/port */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, port);
	if (pko_mac_num < 0) {
		cvmx_dprintf ("%s: ERROR Invalid interface\n", __FUNCTION__);
		return -1;
	}

	ipd_port = cvmx_helper_get_ipd_port(xiface, port);

	/* Build an identifiable owner identifier */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	/* Allocate port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, CVMX_PKO_PORT_QUEUES,
				res_owner, pko_mac_num, 1);

	if (l1_q_num != pko_mac_num) {
		cvmx_dprintf ("%s: ERROR allocation L1 SQ\n", __FUNCTION__);
		return -1;
	}


	/* allocate or reserve level 2 queues */
	l2_q_num = cvmx_pko_alloc_queues(node, CVMX_PKO_L2_QUEUES, res_owner,
					 -1, num_chans);
	if (l2_q_num < 0) {
		cvmx_dprintf ("%s: ERROR allocation L2 SQ\n", __FUNCTION__);
		return -1;
	}


	/* Configre <num_chans> children for MAC, with static priority */
	res = cvmx_pko3_pq_config_children( node,
			pko_mac_num, l2_q_num, num_chans, num_chans);

	if (res < 0) {
		cvmx_dprintf("Error: Could not setup PFC Channel queues\n");
		return -1;
	}

	/* Allocate all SQ levels at once to assure contigous range */
	l3_q = cvmx_pko_alloc_queues(node, CVMX_PKO_L3_QUEUES,
			res_owner, -1, num_chans);
	l4_q = cvmx_pko_alloc_queues(node, CVMX_PKO_L4_QUEUES,
			res_owner, -1, num_chans);
	l5_q = cvmx_pko_alloc_queues(node, CVMX_PKO_L5_QUEUES,
			res_owner, -1, num_chans);
	dq = cvmx_pko_alloc_queues(node, CVMX_PKO_DESCR_QUEUES,
			res_owner, -1, num_chans);
	if (l3_q < 0 || l4_q < 0 || l5_q < 0 ||dq < 0) {
		cvmx_dprintf("%s: ERROR:could not allocate queues, "
			"xiface %u:%u port %u\n",
			__FUNCTION__, xi.node, xi.interface, port);
		return -1;
	}

	/* Configure children with one DQ per channel */
	for (i = 0; i < num_chans; i++) {
		uint16_t chan, dq_num;
		/* <i> moves in priority order, 0=highest, 7=lowest */

		/* Get CHAN_E value for this PFC channel, PCP in low 3 bits */
		chan = ipd_port | cvmx_helper_prio2qos(i);

		/* map channels to L2 queues */
		cvmx_pko3_map_channel(node, l1_q_num, l2_q_num+i, chan);

		cvmx_pko3_sq_config_children(node, 2, l2_q_num+i, l3_q+i, 1, 1);

		cvmx_pko3_sq_config_children(node, 3, l3_q+i, l4_q+i, 1, 1);

		cvmx_pko3_sq_config_children(node, 4, l4_q+i, l5_q+i, 1, 1);

		/* Configure DQs in QoS order, so that QoS/PCP can be index */
		dq_num = dq + cvmx_helper_prio2qos(i);
		cvmx_pko3_sq_config_children(node, 5, l5_q+i, dq_num, 1, 1);
	}

	/* register entire DQ range with the IPD translation table */
	__cvmx_pko3_ipd_dq_register(xiface, port, dq, num_chans);

	return 0;
}

/** 
 * Initialize a simple interface with a a given number of
 * fair or prioritized queues.
 * This function will assign one channel per sub-interface.
 */
int __cvmx_pko3_config_gen_interface(int xiface, uint8_t subif,
	uint8_t num_queues, bool prioritized)
{
	int l1_q_num;
	int l2_q_num;
	int res, res_owner;
	int pko_mac_num;
	int l3_q, l4_q, l5_q, dq;
	uint16_t ipd_port;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	int static_pri;

#if defined(__U_BOOT__)
	num_queues = 1;
#endif

	if (num_queues == 0) {
		num_queues = 1;
		cvmx_dprintf("%s: WARNING xiface %#x misconfigured\n",
			__func__, xiface);
	}

	if (debug)
		cvmx_dprintf("%s: configuring xiface %u:%u/%u nq=%u %s\n",
			     __FUNCTION__, xi.node, xi.interface, subif,
			    num_queues, (prioritized)?"qos":"fair");

	/* Get MAC number for the iface/port */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, subif);
	if (pko_mac_num < 0) {
		cvmx_dprintf ("%s: ERROR Invalid interface %u:%u\n",
			__FUNCTION__, xi.node, xi.interface);
		return -1;
	}

	ipd_port = cvmx_helper_get_ipd_port(xiface, subif);

	if(debug)
		cvmx_dprintf("%s: xiface %u:%u/%u ipd_port=%#03x\n",
			     __FUNCTION__, xi.node, xi.interface, subif,
				ipd_port);

	/* Build an identifiable owner identifier */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_PORT_QUEUES,
					 res_owner, pko_mac_num, 1);

	if (l1_q_num != pko_mac_num) {
		cvmx_dprintf("%s: ERROR xiface %u:%u/%u"
			" failed allocation L1 SQ\n",
			__FUNCTION__, xi.node, xi.interface, subif);
		return -1;
	}

	/* allocate or reserve level 2 queues */
	l2_q_num = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L2_QUEUES, res_owner,
				-1, 1);
	if (l2_q_num < 0) {
		cvmx_dprintf("%s: ERROR xiface %u:%u/%u"
			"  failed allocation L2 SQ\n",
			__FUNCTION__, xi.node, xi.interface, subif);
		return -1;
	}


	/* Configre L2 SQ */
	res = cvmx_pko3_pq_config_children(xi.node, pko_mac_num,
				l2_q_num, 1, 1);

	if (res < 0) {
		cvmx_dprintf("%s: ERROR xiface %u:%u/%u"
			" failed configuring PQ\n",
			__FUNCTION__, xi.node, xi.interface, subif);
		return -1;
	}

	l3_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L3_QUEUES,
				res_owner, -1, 1);
	l4_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L4_QUEUES,
				res_owner, -1, 1);
	l5_q = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_L5_QUEUES,
				res_owner, -1, 1);
	dq = cvmx_pko_alloc_queues(xi.node, CVMX_PKO_DESCR_QUEUES,
				res_owner, -1, num_queues);
	if (dq < 0) {
		cvmx_dprintf("%s: ERROR xiface %u:%u/%u"
			" failed configuring DQs\n",
			__FUNCTION__, xi.node, xi.interface, subif);
		return -1;
	}

	/* Configure hierarchy */
	cvmx_pko3_sq_config_children(xi.node, 2, l2_q_num, l3_q, 1, 1);
	cvmx_pko3_sq_config_children(xi.node, 3, l3_q, l4_q, 1, 1);
	cvmx_pko3_sq_config_children(xi.node, 4, l4_q, l5_q, 1, 1);

	/* Configure DQs relative priority (a.k.a. scheduling) */
	if (prioritized) {
		/* With 8 queues or fewer, use static priority, else WRR */
		static_pri = (num_queues < 9)? num_queues: 0;
	} else {
		/* Set equal-RR scheduling among queues */
		static_pri = -1;
	}

	cvmx_pko3_sq_config_children(xi.node, 5, l5_q, dq,
		num_queues, static_pri);

	/* map IPD/channel to L2 queues */
	cvmx_pko3_map_channel(xi.node, l1_q_num, l2_q_num, ipd_port);

	/* register DQ/IPD translation */
	__cvmx_pko3_ipd_dq_register(xiface, subif, dq, num_queues);

	if(debug)
		cvmx_dprintf("%s: xiface %u:%u/%u qs %u-%u\n",
			     __FUNCTION__, xi.node, xi.interface, subif,
				dq, dq+num_queues-1);
	return 0;
}
EXPORT_SYMBOL(__cvmx_pko3_config_gen_interface);

/** Initialize the NULL interface
 *
 * A NULL interface is a special case in that it is not
 * one of the enumerated interfaces in the system, and does
 * not apply to input either. Still, it can be very handy
 * for dealing with packets that should be discarded in
 * a generic, streamlined way.
 *
 * The Descriptor Queue 0 will be reserved for the NULL interface
 * and the normalized (i.e. IPD) port number has the all-ones value.
 */
static int __cvmx_pko3_config_null_interface(unsigned int node)
{
	int l1_q_num;
	int l2_q_num;
	int l3_q, l4_q, l5_q;
	int i, res, res_owner;
	int xiface, ipd_port;
	int num_dq = 1;	/* # of DQs for NULL */
	const int dq = 0;	/* Reserve DQ#0 for NULL */
	const int pko_mac_num = 0x1C; /* MAC# 28 virtual MAC for NULL */

	if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
		num_dq = 8;

	if(debug)
		cvmx_dprintf("%s: null iface dq=%u-%u\n",
			__FUNCTION__, dq, dq+num_dq-1);

	ipd_port = cvmx_helper_node_to_ipd_port(node, CVMX_PKO3_IPD_PORT_NULL);

	/* Build an identifiable owner identifier by MAC# for easy release */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);
	if (res_owner < 0) {
		cvmx_dprintf ("%s: ERROR Invalid interface\n", __FUNCTION__);
		return -1;
	}

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, CVMX_PKO_PORT_QUEUES,
				res_owner, pko_mac_num, 1);

	if (l1_q_num != pko_mac_num) {
		cvmx_dprintf ("%s: ERROR reserving L1 SQ\n", __FUNCTION__);
		return -1;
	}

	/* allocate or reserve level 2 queues */
	l2_q_num = cvmx_pko_alloc_queues(node, CVMX_PKO_L2_QUEUES, res_owner,
				-1, 1);
	if (l2_q_num < 0) {
		cvmx_dprintf ("%s: ERROR allocating L2 SQ\n", __FUNCTION__);
		return -1;
	}


	/* Configre L2 SQ */
	res = cvmx_pko3_pq_config_children(node, pko_mac_num, l2_q_num, 1, 1);

	if (res < 0) {
		cvmx_dprintf("%s: ERROR: L2 queue\n", __FUNCTION__);
		return -1;
	}

	l3_q = cvmx_pko_alloc_queues(node, CVMX_PKO_L3_QUEUES, res_owner,-1, 1);
	l4_q = cvmx_pko_alloc_queues(node, CVMX_PKO_L4_QUEUES, res_owner,-1, 1);
	l5_q = cvmx_pko_alloc_queues(node, CVMX_PKO_L5_QUEUES, res_owner,-1, 1);

	/* Reserve DQ at 0 by convention */
	res = cvmx_pko_alloc_queues(node, CVMX_PKO_DESCR_QUEUES, res_owner,
		dq, num_dq);
	if (dq != res) {
		cvmx_dprintf("%s: ERROR: could not reserve DQs\n",
			__FUNCTION__);
		return -1;
	}

	/* Configure hierarchy */
	cvmx_pko3_sq_config_children(node, 2, l2_q_num, l3_q, 1, 1);
	cvmx_pko3_sq_config_children(node, 3, l3_q, l4_q, 1, 1);
	cvmx_pko3_sq_config_children(node, 4, l4_q, l5_q, 1, 1);
	cvmx_pko3_sq_config_children(node, 5, l5_q, dq, num_dq, num_dq);

	/* NULL interface does not need to map to a CHAN_E */

	/* register DQ/IPD translation */
	xiface = cvmx_helper_node_interface_to_xiface(node, __CVMX_XIFACE_NULL);
	__cvmx_pko3_ipd_dq_register(xiface, 0, dq, num_dq);

	/* open the null DQs here */
	for(i = 0; i < num_dq; i++)
		cvmx_pko_dq_open(node, dq + i);

	return 0;
}

/** Open all descriptor queues belonging to an interface/port
 * @INTERNAL
 */
int __cvmx_pko3_helper_dqs_activate(int xiface, int index, bool min_pad)
{
	int  ipd_port,dq_base, dq_count, i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	/* Get local IPD port for the interface */
	ipd_port = cvmx_helper_get_ipd_port(xiface, index);
	if(ipd_port < 0) {
		cvmx_dprintf("%s: ERROR: No IPD port for interface %d port %d\n",
			     __FUNCTION__, xiface, index);
		return -1;
	}

	/* Get DQ# range for the IPD port */
	dq_base = cvmx_pko3_get_queue_base(ipd_port);
	dq_count = cvmx_pko3_get_queue_num(ipd_port);
	if( dq_base < 0 || dq_count <= 0) {
		cvmx_dprintf("%s: ERROR: No descriptor queues for interface %d port %d\n",
			     __FUNCTION__, xiface, index);
		return -1;
	}

	/* Mask out node from global DQ# */
	dq_base &= (1<<10)-1;

	for(i = 0; i < dq_count; i++) {
		cvmx_pko_dq_open(xi.node, dq_base + i);
		cvmx_pko3_dq_options(xi.node, dq_base + i, min_pad);
	}


	return i;
}
EXPORT_SYMBOL(__cvmx_pko3_helper_dqs_activate);

/** Configure and initialize PKO3 for an interface
 *
 * @param xiface is the interface number to configure
 * @return 0 on success.
 */
int cvmx_helper_pko3_init_interface(int xiface)
{
	cvmx_helper_interface_mode_t mode;
	int subif, num_ports;
	bool fcs_enable, pad_enable, pad_enable_pko;
	uint8_t fcs_sof_off = 0;
	uint8_t num_queues = 1;
	bool qos = false, pfc = false;
	int res = -1;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	mode = cvmx_helper_interface_get_mode(xiface);
	num_ports = cvmx_helper_interface_enumerate(xiface);
	subif = 0;

	if ((unsigned) xi.interface <
		NUM_ELEMENTS(__cvmx_pko_queue_static_config
			.pknd.pko_cfg_iface)) {
		pfc = __cvmx_pko_queue_static_config
			.pknd.pko_cfg_iface[xi.interface]
			.pfc_enable;
		num_queues =
			__cvmx_pko_queue_static_config.
			pknd.pko_cfg_iface[xi.interface]
			.queues_per_port;
		qos =
			__cvmx_pko_queue_static_config.
			pknd.pko_cfg_iface[xi.interface]
			.qos_enable;
	}

	/* Force 8 DQs per port for pass 1.0 to circumvent limitations */
	if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
		num_queues = 8;
		qos = true;
	}

	/* For ILK there is one IPD port per channel */
	if ((mode == CVMX_HELPER_INTERFACE_MODE_ILK))
		num_ports =  __cvmx_helper_ilk_enumerate(xiface);

	/* Skip non-existent interfaces */
	if(num_ports < 1) {
		cvmx_dprintf("ERROR: %s: invalid iface %u:%u\n",
			     __FUNCTION__, xi.node, xi.interface);
		return -1;
	}

	if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
		num_queues =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_loop.queues_per_port;
		qos =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_loop.qos_enable;

		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_ports,
				num_queues, qos);
		if (res < 0) {
			goto __cfg_error;
		}
	}

	else if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
		num_queues =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_npi.queues_per_port;
		qos =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_npi.qos_enable;

		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_ports,
				num_queues, qos);
		if (res < 0) {
			goto __cfg_error;
		}
	}

	/* ILK-specific queue configuration */
	else if (mode == CVMX_HELPER_INTERFACE_MODE_ILK) {
		num_queues = 8; qos = true; pfc = false;
		res = __cvmx_pko3_config_ilk_interface(xiface, num_queues, qos);
	}

	/* Setup all ethernet configured for PFC */
	else if (pfc) {
		/* PFC interfaces have 8 prioritized queues */
		for (subif = 0; subif < num_ports; subif++) {
			res = __cvmx_pko3_config_pfc_interface(
				xiface, subif);
			if (res < 0)
				goto __cfg_error;

			/* Enable PFC/CBFC on BGX */
			__cvmx_helper_bgx_xaui_config_pfc(xi.node,
				xi.interface, subif, true);
		}
	}

	/* All other interfaces follow static configuration */
	else {

		for (subif = 0; subif < num_ports; subif++) {
			res = __cvmx_pko3_config_gen_interface(xiface, subif,
				num_queues, qos);
			if (res < 0) {
				goto __cfg_error;
			}
		}
	}

	fcs_enable = __cvmx_helper_get_has_fcs(xiface);
	pad_enable = __cvmx_helper_get_pko_padding(xiface);

	if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		pad_enable_pko = false;
	else
		pad_enable_pko = pad_enable;

	if(debug)
		cvmx_dprintf("%s: iface %u:%u FCS=%d pad=%d pko=%d\n",
			__func__, xi.node, xi.interface,
			fcs_enable, pad_enable, pad_enable_pko);

	/* Setup interface options */
	for (subif = 0; subif < num_ports; subif++) {

		/* Open interface/port DQs to allow transmission to begin */
		res = __cvmx_pko3_helper_dqs_activate(xiface,
			subif, pad_enable_pko);

		if (res < 0)
			goto __cfg_error;

		/* ILK has only one MAC, subif == logical-channel */
		if (mode == CVMX_HELPER_INTERFACE_MODE_ILK && subif > 0)
			continue;

		/* LOOP has only one MAC, subif == logical-channel */
		if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP && subif > 0)
			continue;

		/* NPI has only one MAC, subif == 'ring' */
		if (mode == CVMX_HELPER_INTERFACE_MODE_NPI && subif > 0)
			continue;

		if (xi.interface > 5) {
			/* Non-BGX interface, use PKO for FCS/PAD */
			res = cvmx_pko3_interface_options(xiface, subif,
				fcs_enable, pad_enable_pko, fcs_sof_off);
		} else if (pad_enable == pad_enable_pko) {
			/* BGX interface: FCS/PAD done by PKO */
			res = cvmx_pko3_interface_options(xiface, subif,
				  fcs_enable, pad_enable, fcs_sof_off);
			cvmx_helper_bgx_tx_options(xi.node, xi.interface, subif,
				false, false);
		} else {
			/* BGX interface: FCS/PAD done by BGX */
			res = cvmx_pko3_interface_options(xiface, subif,
				  false, false, fcs_sof_off);
			cvmx_helper_bgx_tx_options(xi.node, xi.interface, subif,
				fcs_enable, pad_enable);
		}

		if(res < 0)
			cvmx_dprintf("WARNING: %s: "
				"option set failed on iface %u:%u/%u\n",
				__FUNCTION__, xi.node, xi.interface, subif);
		if (debug)
			cvmx_dprintf("%s: face %u:%u/%u fifo size %d\n",
				__func__, xi.node, xi.interface, subif,
				cvmx_pko3_port_fifo_size(xiface, subif));
	}
	return 0;

  __cfg_error:
	cvmx_dprintf("ERROR: %s: failed on iface %u:%u/%u\n",
		__FUNCTION__, xi.node, xi.interface, subif);
	return -1;
}

/**
 * Global initialization for PKO3
 *
 * Should only be called once on each node
 *
 * TBD: Resolve the kernel case.
 * When Linux eats up the entire memory, bootmem will be unable to
 * satisfy our request, and the memory needs to come from Linux free pages.
 */
int __cvmx_helper_pko3_init_global(unsigned int node, uint16_t gaura)
{
	int res;

	res = cvmx_pko3_hw_init_global(node, gaura);
	if(res < 0) {
		cvmx_dprintf("ERROR: %s:failed block initialization\n",
			__FUNCTION__);
		return res;
	}

	/* configure channel level */
	cvmx_pko_setup_channel_credit_level(node,
		cvmx_pko_default_channel_level);

	/* add NULL MAC/DQ setup */
	res = __cvmx_pko3_config_null_interface(node);
	if (res < 0)
		cvmx_dprintf("ERROR: %s: creating NULL interface\n",
			__FUNCTION__);

	return res;
}
EXPORT_SYMBOL(__cvmx_helper_pko3_init_global);

/*#define	__PKO_HW_DEBUG*/
#ifdef	__PKO_HW_DEBUG
#define	CVMX_DUMP_REGX(reg) cvmx_dprintf("%s=%#llx\n",#reg,(long long)cvmx_read_csr(reg))
#define	CVMX_DUMP_REGD(reg) cvmx_dprintf("%s=%lld.\n",#reg,(long long)cvmx_read_csr(reg))
/*
 * function for debugging PKO reconfiguration
 */
void cvmx_fpa3_aura_dump_regs(unsigned node, uint16_t aura)
	{
	int pool_num =
		cvmx_read_csr_node(node,CVMX_FPA_AURAX_POOL(aura));

	CVMX_DUMP_REGX(CVMX_FPA_AURAX_POOL(aura));
	CVMX_DUMP_REGX(CVMX_FPA_POOLX_CFG(pool_num));
	CVMX_DUMP_REGX(CVMX_FPA_POOLX_OP_PC(pool_num));
	CVMX_DUMP_REGX(CVMX_FPA_POOLX_INT(pool_num));
	CVMX_DUMP_REGD(CVMX_FPA_POOLX_AVAILABLE(pool_num));
	CVMX_DUMP_REGD(CVMX_FPA_POOLX_THRESHOLD(pool_num));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_CFG(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_INT(aura));
	CVMX_DUMP_REGD(CVMX_FPA_AURAX_CNT(aura));
	CVMX_DUMP_REGD(CVMX_FPA_AURAX_CNT_LIMIT(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_CNT_THRESHOLD(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_CNT_LEVELS(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_POOL_LEVELS(aura));

	}

void cvmx_pko3_dump_regs(unsigned node)
{
	(void) node;
	CVMX_DUMP_REGX( CVMX_PKO_NCB_INT );
	CVMX_DUMP_REGX( CVMX_PKO_PEB_ERR_INT );
	CVMX_DUMP_REGX( CVMX_PKO_PDM_ECC_DBE_STS_CMB0 );
	CVMX_DUMP_REGX( CVMX_PKO_PDM_ECC_SBE_STS_CMB0 );
	CVMX_DUMP_REGX( CVMX_PKO_PEB_ECC_DBE_STS0 );
	CVMX_DUMP_REGX( CVMX_PKO_PEB_ECC_DBE_STS_CMB0 );
}
#endif	/* __PKO_HW_DEBUG */

/**
 * Global initialization for PKO3
 *
 * Should only be called once on each node
 *
 * When Linux eats up the entire memory, bootmem will be unable to
 * satisfy our request, and the memory needs to come from Linux free pages.
 */
int cvmx_helper_pko3_init_global(unsigned int node)
{
	void *ptr;
	int res = -1;
	unsigned aura_num = ~0;
	cvmx_fpa3_gaura_t aura;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Allocate memory required by PKO3 */
	res = __cvmx_pko3_config_memory(node);
#endif
	if(res < 0) {
		cvmx_dprintf("ERROR: %s: PKO3 memory allocation error\n",
			__FUNCTION__);
		return res;
	}

	aura_num = res;
	aura = __cvmx_pko3_aura[node];

	/* Exercise the FPA to make sure the AURA is functional */
	ptr = cvmx_fpa3_alloc(aura);

	if (ptr == NULL )
		res = -1;
	else {
		cvmx_fpa3_free_nosync(ptr, aura, 0);
		res = 0;
	}

	if (res < 0) {
		cvmx_dprintf("ERROR: %s: FPA failure AURA=%u:%d\n",
			__func__, aura.node, aura.laura);
		return -1;
	}

	res = __cvmx_helper_pko3_init_global(node, aura_num);

	if (res < 0)
		cvmx_dprintf("ERROR: %s: failed to start PPKO\n",__func__);

	return res;
}

/**
 * Uninitialize PKO3 interface
 *
 * Release all resources held by PKO for an interface.
 * The shutdown code is the same for all supported interfaces.
 *
 * NOTE: The NULL virtual interface is identified by interface
 * number -1, which translates into IPD port 0xfff, MAC#28. [Kludge]
 */
int cvmx_helper_pko3_shut_interface(int xiface)
{
	int index, num_ports;
	int dq_base, dq_count;
	uint16_t ipd_port;
	int i, res_owner, res;
	uint64_t cycles;
	const unsigned timeout = 10;	/* milliseconds */
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	if(__cvmx_helper_xiface_is_null(xiface)) {
		/* Special case for NULL interface */
		num_ports = 1;
	} else {
		cvmx_helper_interface_mode_t mode;
		mode = cvmx_helper_interface_get_mode(xiface);
		num_ports = cvmx_helper_interface_enumerate(xiface);
		(void) mode;
	}

	/* Skip non-existent interfaces silently */
	if(num_ports < 1) {
		return -1;
	}

	if (debug)
		cvmx_dprintf("%s: xiface %u:%d ports %d\n",
			__func__, xi.node, xi.interface , num_ports);

	for (index = 0; index < num_ports; index ++) {

		if (__cvmx_helper_xiface_is_null(xiface))
                        ipd_port = cvmx_helper_node_to_ipd_port(xi.node,
				CVMX_PKO3_IPD_PORT_NULL);
		else
			ipd_port = cvmx_helper_get_ipd_port(xiface, index);

		/* Retreive DQ range for the index */
                dq_base = cvmx_pko3_get_queue_base(ipd_port);
                dq_count = cvmx_pko3_get_queue_num(ipd_port);

                if( dq_base < 0 || dq_count < 0) {
                        cvmx_dprintf("ERROR: %s: No DQs for iface %u:%d/%u\n",
                                __FUNCTION__, xi.node, xi.interface, index);
			continue;
		}

		/* Get rid of node-number in DQ# */
		dq_base &= (1 << 10)-1;

		if (debug)
			cvmx_dprintf("%s: xiface %u:%d/%d dq %u-%u\n",
			__func__, xi.node, xi.interface, index,
			dq_base, dq_base + dq_count -1);

		/* Unregister the DQs for the port, should stop traffic */
		res = __cvmx_pko3_ipd_dq_unregister(xiface, index);
		if(res < 0) {
                        cvmx_dprintf("ERROR: %s: "
				"failed to unregister DQs iface %u/%d/%u\n",
                                __FUNCTION__, xi.node, xi.interface, index);
			continue;
		}

		/* Begin draining all queues */
		for(i = 0; i < dq_count; i++) {
			cvmx_pko3_dq_drain(xi.node, dq_base + i);
		}

		/* Wait for all queues to drain, and close them */
		for(i = 0; i < dq_count; i++) {
			/* Prepare timeout */
			cycles = cvmx_get_cycle();
			cycles += cvmx_clock_get_rate(CVMX_CLOCK_CORE)/1000 * timeout;

			/* Wait for queue to drain */
			do {
				res = cvmx_pko3_dq_query(xi.node, dq_base + i);
				if (cycles < cvmx_get_cycle())
					break;
			} while(res > 0);

			if (res != 0)
				cvmx_dprintf("ERROR: %s: querying queue %u\n",
					__FUNCTION__, dq_base + i);

			/* Close the queue, free internal buffers */
			res = cvmx_pko3_dq_close(xi.node, dq_base + i);

			if (res < 0)
				cvmx_dprintf("ERROR: %s: closing queue %u\n",
					__FUNCTION__, dq_base + i);

		}

		/* Release all global resources owned by this interface/port */

		res_owner = __cvmx_helper_pko3_res_owner(ipd_port);
		if (res_owner < 0) {
			cvmx_dprintf ("ERROR: %s: no resource owner ticket\n",
				__FUNCTION__);
			continue;
		}

		cvmx_pko_free_queues(xi.node, CVMX_PKO_DESCR_QUEUES, res_owner);
		cvmx_pko_free_queues(xi.node, CVMX_PKO_L5_QUEUES, res_owner);
		cvmx_pko_free_queues(xi.node, CVMX_PKO_L4_QUEUES, res_owner);
		cvmx_pko_free_queues(xi.node, CVMX_PKO_L3_QUEUES, res_owner);
		cvmx_pko_free_queues(xi.node, CVMX_PKO_L2_QUEUES, res_owner);
		cvmx_pko_free_queues(xi.node, CVMX_PKO_PORT_QUEUES, res_owner);

	} /* for port */

	return 0;
}

/**
 * Shutdown PKO3
 *
 * Should be called after all interfaces have been shut down on the PKO3.
 *
 * Disables the PKO, frees all its buffers.
 */
int cvmx_helper_pko3_shutdown(unsigned int node)
{
	unsigned dq;
	int res;

	 /* destroy NULL interface here, only PKO knows about it */
	cvmx_helper_pko3_shut_interface(cvmx_helper_node_interface_to_xiface(node, __CVMX_XIFACE_NULL));

#ifdef	__PKO_DQ_CLOSE_ERRATA_FIXED
	/* Check that all DQs are closed */
	/* this seems to cause issue on HW:
	 * the error code differs from expected
	 */
	for(dq =0; dq < (1<<10); dq++) {
		res = cvmx_pko3_dq_close(node, dq);
		if (res != 0) {
			cvmx_dprintf("ERROR: %s: PKO3 descriptor queue %u "
				"could not be closed\n",
				__FUNCTION__, dq);
			return -1;
		}
	}
#endif
	(void) dq;
	res = cvmx_pko3_hw_disable(node);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* shut down AURA/POOL we created, and free its resources */
	cvmx_fpa3_shutdown_aura_and_pool(__cvmx_pko3_aura[node]);
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */
	return res;
}
