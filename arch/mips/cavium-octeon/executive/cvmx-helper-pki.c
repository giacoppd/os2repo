/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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

/**
 * @file
 *
 * PKI helper functions.
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki-defs.h>
#include <asm/octeon/cvmx-pki.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-pki-resources.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-global-resources.h>
#else
#include "cvmx.h"
#include "cvmx-version.h"
#include "cvmx-error.h"
#include "cvmx-pki-defs.h"
#include "cvmx-pki.h"
#include "cvmx-fpa3.h"
#include "cvmx-helper-pki.h"
#include "cvmx-pki-resources.h"
#include "cvmx-pow.h"
#include "cvmx-helper-util.h"
#include "cvmx-global-resources.h"
#endif

static CVMX_SHARED int pki_helper_debug;

CVMX_SHARED bool cvmx_pki_dflt_init[CVMX_MAX_NODES] = {[0 ... CVMX_MAX_NODES-1] = 1};

static CVMX_SHARED struct cvmx_pki_cluster_grp_config pki_dflt_clgrp[CVMX_MAX_NODES] = {
	{0, 0xf},
	{0, 0xf}
};

CVMX_SHARED struct cvmx_pki_pool_config pki_dflt_pool[CVMX_MAX_NODES] = { [0 ... CVMX_MAX_NODES-1] = {
	.pool_num = -1,
	.buffer_size = 2048,
	.buffer_count = 0} };

CVMX_SHARED struct cvmx_pki_aura_config pki_dflt_aura[CVMX_MAX_NODES] = { [0 ... CVMX_MAX_NODES-1] = {
	.aura_num = 0,
	.pool_num = -1,
	.buffer_count = 0} };

CVMX_SHARED struct cvmx_pki_style_config pki_dflt_style[CVMX_MAX_NODES] = { [0 ... CVMX_MAX_NODES-1] = {
	.parm_cfg = {.lenerr_en = 1, .maxerr_en = 1, .minerr_en = 1,
	.fcs_strip = 1, .fcs_chk = 1, .first_skip = 40, .mbuff_size = 2048} } };

CVMX_SHARED struct cvmx_pki_sso_grp_config pki_dflt_sso_grp[CVMX_MAX_NODES];
CVMX_SHARED struct cvmx_pki_qpg_config pki_dflt_qpg[CVMX_MAX_NODES];
CVMX_SHARED struct cvmx_pki_pkind_config pki_dflt_pkind[CVMX_MAX_NODES];
CVMX_SHARED uint64_t pkind_style_map[CVMX_MAX_NODES][CVMX_PKI_NUM_PKIND] = { [0 ... CVMX_MAX_NODES-1] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63} };

/* To store the qos watcher values before they are written to pcam when watcher is enabled
There is no cvmx-pip.c file exist so it ended up here */
CVMX_SHARED struct cvmx_pki_legacy_qos_watcher qos_watcher[8];


/** @INTERNAL
 * This function setsup default ltype map
 * @param node    node number
 */
void __cvmx_helper_pki_set_dflt_ltype_map(int node)
{
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_NONE, CVMX_PKI_BELTYPE_NONE);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_ENET, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_VLAN, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_SNAP_PAYLD, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		 CVMX_PKI_LTYPE_E_ARP, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_RARP, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IP4, CVMX_PKI_BELTYPE_IP4);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IP4_OPT, CVMX_PKI_BELTYPE_IP4);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IP6, CVMX_PKI_BELTYPE_IP6);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IP6_OPT, CVMX_PKI_BELTYPE_IP6);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IPSEC_ESP, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IPFRAG, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_IPCOMP, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_TCP, CVMX_PKI_BELTYPE_TCP);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_UDP, CVMX_PKI_BELTYPE_UDP);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_SCTP, CVMX_PKI_BELTYPE_SCTP);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_UDP_VXLAN, CVMX_PKI_BELTYPE_UDP);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_GRE, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_NVGRE, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_GTP, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_SW28, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_SW29, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_SW30, CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node,
		CVMX_PKI_LTYPE_E_SW31, CVMX_PKI_BELTYPE_MISC);
}
EXPORT_SYMBOL(__cvmx_helper_pki_set_dflt_ltype_map);

/** @INTERNAL
 * This function installs the default VLAN entries to identify
 * the VLAN and set WQE[vv], WQE[vs] if VLAN is found. In 78XX
 * hardware (PKI) is not hardwired to recognize any 802.1Q VLAN
 * Ethertypes
 *
 * @param node    node number
 */
int __cvmx_helper_pki_install_dflt_vlan(int node)
{
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;
	enum cvmx_pki_term field;
	int index;
	int bank;
	uint64_t cl_mask = CVMX_PKI_CLUSTER_ALL;

	memset(&pcam_input, 0, sizeof(pcam_input));
	memset(&pcam_action, 0, sizeof(pcam_action));

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		/* PKI-20858 */
		int i;
		for (i = 0; i < 4; i++) {
			union cvmx_pki_clx_ecc_ctl ecc_ctl;
			ecc_ctl.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_ECC_CTL(i));
			ecc_ctl.s.pcam_en = 0;
			ecc_ctl.s.pcam0_cdis = 1;
			ecc_ctl.s.pcam1_cdis = 1;
			cvmx_write_csr_node(node,
				CVMX_PKI_CLX_ECC_CTL(i), ecc_ctl.u64);
		}
	}

	for (field = CVMX_PKI_PCAM_TERM_ETHTYPE0;
		field < CVMX_PKI_PCAM_TERM_ETHTYPE2; field++) {
		bank = field & 0x01;

		index = cvmx_pki_pcam_entry_alloc(node,
			CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d\n",
				node, bank);
			return -1;
		}
		pcam_input.style = 0;
		pcam_input.style_mask = 0;
		pcam_input.field = field;
		pcam_input.field_mask = 0xfd;
		pcam_input.data = 0x81000000;
		pcam_input.data_mask = 0xffff0000;
		pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
		pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_VLAN;
		pcam_action.style_add = 0;
		pcam_action.pointer_advance = 4;
		cvmx_pki_pcam_write_entry(node, index, cl_mask,
			pcam_input, pcam_action);/*cluster_mask in pass2*/

		index = cvmx_pki_pcam_entry_alloc(node,
			CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d\n",
				node, bank);
			return -1;
		}
		pcam_input.data = 0x88a80000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input,
			pcam_action);

		index = cvmx_pki_pcam_entry_alloc(node,
			CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d\n",
				node, bank);
			return -1;
		}
		pcam_input.data = 0x92000000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input,
			pcam_action);/* cluster_mask in pass2*/

		index = cvmx_pki_pcam_entry_alloc(node,
			CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d\n",
				node, bank);
			return -1;
		}
		pcam_input.data = 0x91000000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input,
			pcam_action);
	}
	return 0;
}
EXPORT_SYMBOL(__cvmx_helper_pki_install_dflt_vlan);

static int __cvmx_helper_setup_pki_cluster_groups(int node)
{
	uint64_t cl_mask;
	int cl_group;

	cl_group = cvmx_pki_cluster_grp_alloc(node, pki_dflt_clgrp[node].grp_num);
	if (cl_group == -1) {
		if (pki_dflt_clgrp[node].grp_num == -1)
			return -1;
		else
			return 0; /* cluster already configured, share it */
	}
	cl_mask = pki_dflt_clgrp[node].cluster_mask;
	if (pki_helper_debug)
		cvmx_dprintf("pki-helper: setup pki cluster grp %d with cl_mask 0x%llx\n",
			     (int)cl_group, (unsigned long long)cl_mask);
	cvmx_pki_attach_cluster_to_group(node, cl_group, cl_mask);
	return 0;
}

/**
 * This function sets up pools/auras to be used by PKI
 * @param node    node number
 */
int __cvmx_helper_pki_setup_sso_groups(int node)
{
	cvmx_coremask_t core_mask = CVMX_COREMASK_EMPTY;
	cvmx_xgrp_t xgrp;
	int grp;
	int priority;
	int weight;
	int affinity;
	uint64_t modify_mask;
	uint8_t core_mask_set;

	/* try to reserve sso groups and configure them if they are not configured */
	grp = cvmx_sso_allocate_group_range(node, &pki_dflt_sso_grp[node].group, 1);
	if (grp == CVMX_RESOURCE_ALLOC_FAILED)
		return -1;
	else if (grp == CVMX_RESOURCE_ALREADY_RESERVED)
		return 0; /* sso group already configured, share it */

	xgrp.xgrp = grp;
	priority = pki_dflt_sso_grp[node].priority;
	weight = pki_dflt_sso_grp[node].weight;
	affinity = pki_dflt_sso_grp[node].affinity;
	core_mask_set = pki_dflt_sso_grp[node].core_mask_set;
	cvmx_coremask_set64_node(&core_mask, node, pki_dflt_sso_grp[node].core_mask);
	modify_mask = CVMX_SSO_MODIFY_GROUP_PRIORITY |
			CVMX_SSO_MODIFY_GROUP_WEIGHT |
			CVMX_SSO_MODIFY_GROUP_AFFINITY;
	if (pki_helper_debug)
		cvmx_dprintf("pki-helper: set sso grp %d with priority %d weight %d core_mask 0x%llx\n",
			     grp, priority, weight, (unsigned long long)pki_dflt_sso_grp[node].core_mask);
	cvmx_sso_set_group_priority(node, xgrp, priority, weight,
				    affinity, modify_mask);
	cvmx_sso_set_group_core_affinity(xgrp, &core_mask, core_mask_set);
	return 0;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * This function sets up pools/auras to be used by PKI
 * @param node    node number
 */
static int __cvmx_helper_pki_setup_fpa_pools(int node)
{
	uint64_t buffer_count;
	uint64_t buffer_size;

	if (__cvmx_fpa3_aura_valid(pki_dflt_aura[node].aura))
		return 0; /* aura already configured, share it */

	buffer_count = pki_dflt_pool[node].buffer_count;
	buffer_size = pki_dflt_pool[node].buffer_size;

	if (buffer_count != 0) {
		pki_dflt_pool[node].pool =
			cvmx_fpa3_setup_fill_pool(node,
				pki_dflt_pool[node].pool_num, "PKI POOL DFLT",
				buffer_size, buffer_count, NULL);
		if (!__cvmx_fpa3_pool_valid(pki_dflt_pool[node].pool)) {
			cvmx_printf("ERROR: %s: Failed to allocate pool %d\n",
				__func__, pki_dflt_pool[node].pool_num);
			return -1;
		}
		pki_dflt_pool[node].pool_num = pki_dflt_pool[node].pool.lpool;

		if (pki_helper_debug)
			cvmx_dprintf("%s pool %d with buffer size %d cnt %d\n",
				__func__, pki_dflt_pool[node].pool_num,
				(int)buffer_size, (int)buffer_count);

		pki_dflt_aura[node].pool_num =  pki_dflt_pool[node].pool_num;
		pki_dflt_aura[node].pool =  pki_dflt_pool[node].pool;
	}

	buffer_count = pki_dflt_aura[node].buffer_count;

	if (buffer_count != 0) {
		pki_dflt_aura[node].aura =
			cvmx_fpa3_set_aura_for_pool(
				pki_dflt_aura[node].pool,
				pki_dflt_aura[node].aura_num,
				"PKI DFLT AURA",
				buffer_size, buffer_count);

		if (!__cvmx_fpa3_aura_valid(pki_dflt_aura[node].aura)) {
			cvmx_dprintf("ERROR: %sL Failed to allocate aura %d\n",
				__func__, pki_dflt_aura[node].aura_num);
				return -1;
		}
	}
	return 0;
}
#endif /* ! CVMX_BUILD_FOR_LINUX_KERNEL */

static int __cvmx_helper_setup_pki_qpg_table(int node)
{
	int offset;

	offset = cvmx_pki_qpg_entry_alloc(node, pki_dflt_qpg[node].qpg_base, 1);
	if (offset == CVMX_RESOURCE_ALLOC_FAILED)
		return -1;
	else if (offset == CVMX_RESOURCE_ALREADY_RESERVED)
		return 0; /* share the qpg table entry */
	if (pki_helper_debug)
		cvmx_dprintf("pki-helper: set qpg entry at offset %d with port add %d aura %d grp_ok %d grp_bad %d\n",
			offset, pki_dflt_qpg[node].port_add,
			pki_dflt_qpg[node].aura_num, pki_dflt_qpg[node].grp_ok,
			pki_dflt_qpg[node].grp_bad);
	cvmx_pki_write_qpg_entry(node, offset, &pki_dflt_qpg[node]);
	return 0;
}

int __cvmx_helper_pki_port_setup(int node, int ipd_port)
{
	int interface, index;
	int pknd, style_num;
	int rs;
	struct cvmx_pki_pkind_config pkind_cfg;

	if (!cvmx_pki_dflt_init[node])
		return 0;
	interface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);

	pknd = cvmx_helper_get_pknd(interface, index);
	style_num = pkind_style_map[node][pknd];

	/* try to reserve the style, if it is not configured already, reserve
	and configure it */
	rs = cvmx_pki_style_alloc(node, style_num);
	if (rs < 0) {
		if (rs == CVMX_RESOURCE_ALLOC_FAILED)
			return -1;
	} else {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper: set style %d with default parameters\n", style_num);
		pkind_style_map[node][pknd] = style_num;
		/* configure style with default parameters */
		cvmx_pki_write_style_config(node, style_num, CVMX_PKI_CLUSTER_ALL,
					  &pki_dflt_style[node]);
	}
	if (pki_helper_debug)
		cvmx_dprintf("pki-helper: set pkind %d with initial style %d\n", pknd, style_num);
	/* write pkind configuration */
	pkind_cfg = pki_dflt_pkind[node];
	pkind_cfg.initial_style = style_num;
	cvmx_pki_write_pkind_config(node, pknd, &pkind_cfg);
	return 0;
}

int __cvmx_helper_pki_global_setup(int node)
{
	__cvmx_helper_pki_set_dflt_ltype_map(node);
	if (!cvmx_pki_dflt_init[node])
		return 0;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Setup the packet pools*/
	__cvmx_helper_pki_setup_fpa_pools(node);
#endif
	/*set up default cluster*/
	__cvmx_helper_setup_pki_cluster_groups(node);
	//__cvmx_helper_pki_setup_sso_groups(node);
	__cvmx_helper_setup_pki_qpg_table(node);
	/*
	 * errata PKI-19103 backward compat has only 1 aura
	 * no head line blocking
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		cvmx_pki_buf_ctl_t buf_ctl;
		buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
		buf_ctl.s.fpa_wait = 1;
		cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);
	}
	return 0;
}

void cvmx_helper_pki_set_dflt_pool(int node, int pool, int buffer_size, int buffer_count)
{
	if (pool == 0)
		pool = -1;
	pki_dflt_pool[node].pool_num = pool;
	pki_dflt_pool[node].buffer_size = buffer_size;
	pki_dflt_pool[node].buffer_count = buffer_count;
}

void cvmx_helper_pki_set_dflt_aura(int node, int aura, int pool, int buffer_count)
{
	pki_dflt_aura[node].aura_num = aura;
	pki_dflt_aura[node].pool_num = pool;
	pki_dflt_aura[node].buffer_count = buffer_count;
}

void cvmx_helper_pki_set_dflt_pool_buffer(int node, int buffer_count)
{
	pki_dflt_pool[node].buffer_count = buffer_count;
}

void cvmx_helper_pki_set_dflt_aura_buffer(int node, int buffer_count)
{
	pki_dflt_aura[node].buffer_count = buffer_count;
}

void cvmx_helper_pki_set_dflt_style(int node, struct cvmx_pki_style_config *style_cfg)
{
	pki_dflt_style[node] = *style_cfg;
}

void cvmx_helper_pki_get_dflt_style(int node, struct cvmx_pki_style_config *style_cfg)
{
	*style_cfg = pki_dflt_style[node];
}

void cvmx_helper_pki_set_dflt_qpg(int node, struct cvmx_pki_qpg_config *qpg_cfg)
{
	pki_dflt_qpg[node] = *qpg_cfg;
}

void cvmx_helper_pki_get_dflt_qpg(int node, struct cvmx_pki_qpg_config *qpg_cfg)
{
	*qpg_cfg = pki_dflt_qpg[node];
}

void cvmx_helper_pki_set_dflt_pkind_map(int node, int pkind, int style)
{
	pkind_style_map[node][pkind] = style;
}

void cvmx_helper_pki_no_dflt_init(int node)
{
	cvmx_pki_dflt_init[node] = 0;
}

/**
 * This function Enabled the PKI hardware to
 * start accepting/processing packets.
 *
 * @param node    node number
 */
void cvmx_helper_pki_enable(int node)
{
	if (pki_helper_debug)
		cvmx_dprintf("enable PKI on node %d\n", node);
	__cvmx_helper_pki_install_dflt_vlan(node);
	cvmx_pki_setup_clusters(node);
	cvmx_pki_enable_backpressure(node);
	cvmx_pki_parse_enable(node, 0);
	cvmx_pki_enable(node);
}

/**
 * This function frees up PKI resources consumed by that port.
 * This function should only be called if port resources
 * (fpa pools aura, style qpg entry pcam entry etc.) are not shared
 * @param ipd_port      ipd port number for which resources need to
 *		      be freed.
 */
int cvmx_helper_pki_port_shutdown(int ipd_port)
{
	/* remove pcam entries */
	/* implemet if needed */
	/* __cvmx_pki_port_rsrc_free(node); */
	return 0;
}

/**
 * This function shuts down complete PKI hardware
 * and software resources.
 * @param node	  node number where PKI needs to shutdown.
 */
void cvmx_helper_pki_shutdown(int node)
{
	/* remove pcam entries */
	/* Disable PKI */
	cvmx_pki_disable(node);
	/* Free all prefetched buffers */
	__cvmx_pki_free_ptr(node);
	/* Reset PKI */
	cvmx_pki_reset(node);
	/* Free all the allocated PKI resources
	except fpa pools & aura which will be done in fpa block */
	__cvmx_pki_global_rsrc_free(node);
}

/**
 * This function calculates how mant qpf entries will be needed for
 * a particular QOS.
 * @param qpg_qos       qos value for which entries need to be calculated.
 */
int cvmx_helper_pki_get_num_qpg_entry(enum cvmx_pki_qpg_qos qpg_qos)
{
	if (qpg_qos == CVMX_PKI_QPG_QOS_NONE)
		return 1;
	else if (qpg_qos == CVMX_PKI_QPG_QOS_VLAN || qpg_qos == CVMX_PKI_QPG_QOS_MPLS)
		return 8;
	else if (qpg_qos == CVMX_PKI_QPG_QOS_DSA_SRC)
		return 32;
	else if (qpg_qos == CVMX_PKI_QPG_QOS_DIFFSERV || qpg_qos == CVMX_PKI_QPG_QOS_HIGIG)
		return 64;
	else {
		cvmx_dprintf("ERROR: unrecognized qpg_qos = %d", qpg_qos);
		return 0;
	}
}

/**
 * This function setups the qos table by allocating qpg entry and writing
 * the provided parameters to that entry (offset).
 * @param node	  node number.
 * @param qpg_cfg       pointer to struct containing qpg configuration
 */
int cvmx_helper_pki_set_qpg_entry(int node, struct cvmx_pki_qpg_config *qpg_cfg)
{
	int offset;

	offset = cvmx_pki_qpg_entry_alloc(node, qpg_cfg->qpg_base, 1);
	if (pki_helper_debug)
		cvmx_dprintf("pki-helper:set qpg entry at offset %d \n", offset);
	if (offset == CVMX_RESOURCE_ALREADY_RESERVED) {
		cvmx_dprintf("INFO:setup_qpg_table: offset %d already reserved\n", qpg_cfg->qpg_base);
		return CVMX_RESOURCE_ALREADY_RESERVED;
	} else if (offset == CVMX_RESOURCE_ALLOC_FAILED) {
		cvmx_dprintf("ERROR:setup_qpg_table: no more entries available\n");
		return CVMX_RESOURCE_ALLOC_FAILED;
	}
	qpg_cfg->qpg_base = offset;
	cvmx_pki_write_qpg_entry(node, offset, qpg_cfg);
	return offset;
}

/**
 * This function sets up aura QOS for RED, backpressure and tail-drop.
 *
 * @param node       node number.
 * @param aura       aura to configure.
 * @param ena_red       enable RED based on [DROP] and [PASS] levels
 *			1: enable 0:disable
 * @param pass_thresh   pass threshold for RED.
 * @param drop_thresh   drop threshold for RED
 * @param ena_bp	enable backpressure based on [BP] level.
 *			1:enable 0:disable
 * @param bp_thresh     backpressure threshold.
 * @param ena_drop      enable tail drop.
 *			1:enable 0:disable
 * @return Zero on success. Negative on failure
 * @note the 'node' and 'aura' arguments may be combined in the future
 * to use a compaund cvmx_fpa3_gaura_t structure argument.
 */
int cvmx_helper_setup_aura_qos(int node, int aura, bool ena_red, bool ena_drop,
			       uint64_t pass_thresh, uint64_t drop_thresh,
			       bool ena_bp, uint64_t bp_thresh)
{
	cvmx_fpa3_gaura_t gaura;

	gaura = __cvmx_fpa3_gaura(node, aura);

	ena_red = ena_red | ena_drop;
	cvmx_fpa3_setup_aura_qos(gaura, ena_red, pass_thresh, drop_thresh,
				ena_bp, bp_thresh);
	cvmx_pki_enable_aura_qos(node, aura, ena_red, ena_drop, ena_bp);
	return 0;
}

/**
 * This function maps specified bpid to all the auras from which it can receive bp and
 * then maps that bpid to all the channels, that bpid can asserrt bp on.
 *
 * @param node	  node number.
 * @param aura	  aura number which will back pressure specified bpid.
 * @param bpid	  bpid to map.
 * @param chl_map       array of channels to map to that bpid.
 * @param chl_cnt	number of channel/ports to map to that bpid.
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_pki_map_aura_chl_bpid(int node, uint16_t aura, uint16_t bpid,
				      uint16_t chl_map[], uint16_t chl_cnt)
{
	uint16_t channel;

	if (aura >= CVMX_PKI_NUM_AURA) {
		cvmx_dprintf("ERROR: aura %d is > supported in hw\n", aura);
		return -1;
	}
	if (bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: bpid %d is > supported in hw\n", bpid);
		return -1;
	}
	cvmx_pki_write_aura_bpid(node, aura, bpid);
	while (chl_cnt--) {
		channel = chl_map[chl_cnt];
		if (channel >= CVMX_PKI_NUM_CHANNEL) {
			cvmx_dprintf("ERROR: channel %d is > supported in hw\n", channel);
			return -1;
		}
		cvmx_pki_write_channel_bpid(node, channel, bpid);
	}
	return 0;
}

/** @INTERNAL
 * This function returns the value of port shift required
 * if all the ports on that interface are using same style and
 * configuring qpg_qos != NONE
 */
int __cvmx_helper_pki_port_shift(int xiface, enum cvmx_pki_qpg_qos qpg_qos)
{
	uint8_t num_qos;
	cvmx_helper_interface_mode_t mode = cvmx_helper_interface_get_mode(xiface);

	num_qos = cvmx_helper_pki_get_num_qpg_entry(qpg_qos);
	if ((mode != CVMX_HELPER_INTERFACE_MODE_SGMII) &&
	     (mode != CVMX_HELPER_INTERFACE_MODE_NPI) &&
	     (mode != CVMX_HELPER_INTERFACE_MODE_LOOP)) {
		return ffs(num_qos) - 1;
	     } else if (num_qos <= 16)
		     return 0;
	     else if (num_qos <= 32)
		     return 1;
	     else
		     return 2;
}


int __cvmx_helper_pki_qos_rsrcs(int node, struct cvmx_pki_qos_schd *qossch)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	int rs;

	/* Reserve pool resources */
	if (qossch->pool_per_qos && qossch->pool_num < 0) {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:qos-rsrc: setup pool %d buff_size %d blocks %d\n",
				     qossch->pool_num, (int)qossch->pool_buff_size, (int)qossch->pool_max_buff);

		qossch->_pool = cvmx_fpa3_setup_fill_pool(node,
			qossch->pool_num, qossch->pool_name,
			qossch->pool_buff_size,
			qossch->pool_max_buff, NULL);

		if (!__cvmx_fpa3_pool_valid(qossch->_pool)) {
			cvmx_printf("ERROR: %s POOL %d init failed\n",
				__func__, qossch->pool_num);
			return -1;
		}

		qossch->pool_num = qossch->_pool.lpool;
		if (pki_helper_debug)
			cvmx_dprintf("pool alloced is %d\n", qossch->pool_num);
	}
	/* Reserve aura resources */
	if (qossch->aura_per_qos && qossch->aura_num < 0) {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:qos-rsrc: setup aura %d pool %d blocks %d\n",
				     qossch->aura_num, qossch->pool_num,
				     (int)qossch->aura_buff_cnt);

		qossch->_aura = cvmx_fpa3_set_aura_for_pool(qossch->_pool,
				qossch->aura_num,
				qossch->aura_name,
				qossch->pool_buff_size,
				qossch->aura_buff_cnt);

		if (!__cvmx_fpa3_aura_valid(qossch->_aura)) {
			cvmx_printf("ERROR: %s AURA %d init failed\n",
				__func__, qossch->aura_num);
			return -1;
		}

		qossch->aura_num = qossch->_aura.laura;
		if (pki_helper_debug)
			cvmx_dprintf("aura alloced is %d\n", qossch->aura_num);
	}
	/* Reserve sso group resources */
	/* Find which node work needs to be schedules vinita_to_do to extract node*/
	if (qossch->sso_grp_per_qos && qossch->sso_grp < 0) {
		//unsigned grp_node;
		//grp_node = (abs)(qossch->sso_grp + CVMX_PKI_FIND_AVAILABLE_RSRC);
		rs = cvmx_sso_allocate_group(node);
		if (rs < 0) {
			cvmx_dprintf("pki-helper:qos-rsrc: ERROR: sso grp not available\n");
			return rs;
		}
		qossch->sso_grp = rs | (node<<8);
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:qos-rsrc: sso grp alloced is %d\n", qossch->sso_grp);
	}
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */
	return 0;
}

int __cvmx_helper_pki_port_rsrcs(int node, struct cvmx_pki_prt_schd *prtsch)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	int rs;

	/* Reserve pool resources */
	if (prtsch->pool_per_prt && prtsch->pool_num < 0) {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:port-rsrc: setup pool %d buff_size %d blocks %d\n",
				     prtsch->pool_num, (int)prtsch->pool_buff_size, (int)prtsch->pool_max_buff);

		prtsch->_pool = cvmx_fpa3_setup_fill_pool(node,
			prtsch->pool_num, prtsch->pool_name,
			prtsch->pool_buff_size, prtsch->pool_max_buff, NULL);

		if (!__cvmx_fpa3_pool_valid(prtsch->_pool)) {
			cvmx_printf("ERROR: %s: POOL %d init failed\n",
				__func__, prtsch->pool_num);
			return -1;
		}
		prtsch->pool_num = prtsch->_pool.lpool;
		if (pki_helper_debug)
			cvmx_dprintf("pool alloced is %d\n", prtsch->pool_num);
	}
	/* Reserve aura resources */
	if (prtsch->aura_per_prt && prtsch->aura_num < 0) {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:port-rsrc; setup aura %d pool %d blocks %d\n",
				     prtsch->aura_num, prtsch->pool_num, (int)prtsch->aura_buff_cnt);
		prtsch->_aura = cvmx_fpa3_set_aura_for_pool(prtsch->_pool,
			prtsch->aura_num, prtsch->aura_name,
			prtsch->pool_buff_size,
			prtsch->aura_buff_cnt);

		if (!__cvmx_fpa3_aura_valid(prtsch->_aura)) {
			cvmx_printf("ERROR: %s: AURA %d init failed\n",
				__func__, prtsch->aura_num);
			return -1;
		}
		prtsch->aura_num = prtsch->_aura.laura;

		if (pki_helper_debug)
			cvmx_dprintf("aura alloced is %d\n", prtsch->aura_num);
	}
	/* Reserve sso group resources , vinita_to_do to extract node*/
	if (prtsch->sso_grp_per_prt && prtsch->sso_grp < 0) {
		//unsigned grp_node;
		//grp_node = (abs)(prtsch->sso_grp + CVMX_PKI_FIND_AVAILABLE_RSRC);
		rs = cvmx_sso_allocate_group(node);
		if (rs < 0) {
			cvmx_printf("ERROR: %s: sso grp not available\n", __func__);
			return rs;
		}
		prtsch->sso_grp = rs | (node << 8);
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:port-rsrc: sso grp alloced is %d\n", prtsch->sso_grp);
	}
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */
	return 0;
}

int __cvmx_helper_pki_intf_rsrcs(int node, struct cvmx_pki_intf_schd *intf)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	int rs;

	if (intf->pool_per_intf && intf->pool_num < 0) {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:intf-rsrc: setup pool %d buff_size %d blocks %d\n",
				     intf->pool_num, (int)intf->pool_buff_size, (int)intf->pool_max_buff);
		intf->_pool = cvmx_fpa3_setup_fill_pool(node, intf->pool_num,
			intf->pool_name, intf->pool_buff_size,
			intf->pool_max_buff, NULL);

		if (!__cvmx_fpa3_pool_valid(intf->_pool)) {
			cvmx_printf("ERROR: %s: POOL %d init failed\n",
				__func__, intf->pool_num);
			return -1;
		}
		intf->pool_num = intf->_pool.lpool;

		if (pki_helper_debug)
			cvmx_dprintf("pool alloced is %d\n", intf->pool_num);

	}
	if (intf->aura_per_intf && intf->aura_num < 0) {
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:intf-rsrc: setup aura %d pool %d blocks %d\n",
			     intf->aura_num, intf->pool_num, (int)intf->aura_buff_cnt);
		intf->_aura = cvmx_fpa3_set_aura_for_pool(intf->_pool,
			intf->aura_num, intf->aura_name,
			intf->pool_buff_size,
			intf->aura_buff_cnt);

		if (!__cvmx_fpa3_aura_valid(intf->_aura)) {
			cvmx_printf("ERROR: %s: AURA %d init failed\n",
				__func__, intf->aura_num);

			return -1;
		}

		intf->aura_num = intf->_aura.laura;

		if (pki_helper_debug)
			cvmx_dprintf("aura alloced is %d\n", intf->aura_num);
	}
	/* vinita_to_do to extract node */
	if (intf->sso_grp_per_intf && intf->sso_grp < 0) {
		//unsigned grp_node;
		//grp_node = (abs)(intf->sso_grp + CVMX_PKI_FIND_AVAILABLE_RSRC);
		rs = cvmx_sso_allocate_group(node);
		if (rs < 0) {
			cvmx_printf("ERROR: %s: sso grp not available\n", __func__);
			return rs;
		}
		intf->sso_grp = rs | (node << 8);
	}
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */
	return 0;
}

int __cvmx_helper_pki_set_intf_qpg(int node, int port, int qpg_base, int num_entry,
			   struct cvmx_pki_intf_schd *intfsch)
{
	int offset;
	int entry;
	struct cvmx_pki_qpg_config qpg_cfg;

	memset(&qpg_cfg, 0, sizeof(qpg_cfg));
	if (pki_helper_debug)
		cvmx_dprintf("pki-helper:intf_qpg port %d qpg_base %d num_entry %d",
			     port, qpg_base, num_entry);
	offset = cvmx_pki_qpg_entry_alloc(node, qpg_base, num_entry);
	if (offset == CVMX_RESOURCE_ALREADY_RESERVED) {
		cvmx_dprintf("pki-helper: INFO: qpg entries will be shared\n");
		return offset;
	} else if (offset == CVMX_RESOURCE_ALLOC_FAILED) {
		cvmx_dprintf("pki-helper: ERROR: qpg entries not available\n");
		return offset;
	} else if (intfsch->qpg_base < 0)
		intfsch->qpg_base = offset;
	if (pki_helper_debug)
		cvmx_dprintf("qpg_base allocated is %d\n", offset);
	for (entry = 0; entry < num_entry; entry++) {
		qpg_cfg.port_add = intfsch->prt_s[port].qos_s[entry].port_add;
		qpg_cfg.aura_num = intfsch->prt_s[port].qos_s[entry].aura_num;
		qpg_cfg.grp_ok = intfsch->prt_s[port].qos_s[entry].sso_grp;
		qpg_cfg.grp_bad = intfsch->prt_s[port].qos_s[entry].sso_grp;
		cvmx_pki_write_qpg_entry(node, (offset + entry), &qpg_cfg);
	}
	return offset;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * This function sets up the global pool, aura and sso group
 * resources which application can use between any interfaces
 * and ports.
 * @param node          node number
 * @param gblsch        pointer to struct containing global
 *                      scheduling parameters.
 */
int cvmx_helper_pki_set_gbl_schd(int node, struct cvmx_pki_global_schd *gblsch)
{
	int rs;

	if (gblsch->setup_pool) {
		if (pki_helper_debug)
			cvmx_dprintf("%s: gbl setup global pool %d buff_size %d blocks %d\n",
				__func__, gblsch->pool_num,
				(int)gblsch->pool_buff_size,
				(int)gblsch->pool_max_buff);

		gblsch->_pool =
		    cvmx_fpa3_setup_fill_pool(node, gblsch->pool_num,
			gblsch->pool_name, gblsch->pool_buff_size,
			gblsch->pool_max_buff, NULL);

		if (!__cvmx_fpa3_pool_valid(gblsch->_pool)) {
			cvmx_printf("ERROR: %s: POOL %u:%d unavailable\n",
				__func__, node, gblsch->pool_num);
			return -1;
		}

		gblsch->pool_num = gblsch->_pool.lpool;

		if (pki_helper_debug)
			cvmx_dprintf("pool alloced is %d\n", gblsch->pool_num);
	}
	if (gblsch->setup_aura) {
		if (pki_helper_debug)
			cvmx_dprintf("%s: gbl setup global aura %d pool %d blocks %d\n",
				__func__, gblsch->aura_num, gblsch->pool_num,
				(int)gblsch->aura_buff_cnt);

		gblsch->_aura = cvmx_fpa3_set_aura_for_pool(gblsch->_pool,
			gblsch->aura_num,
			gblsch->aura_name,
			gblsch->pool_buff_size,
			gblsch->aura_buff_cnt);

		if (!__cvmx_fpa3_aura_valid(gblsch->_aura)) {
			cvmx_printf("ERROR: %s: AURA %u:%d unavailable\n",
				__func__, node, gblsch->aura_num);
			return -1;
		}

		gblsch->aura_num = gblsch->_aura.laura;

		if (pki_helper_debug)
			cvmx_dprintf("aura alloced is %d\n", gblsch->aura_num);

	}
	if (gblsch->setup_sso_grp) {
		//unsigned grp_node;
		//grp_node = (abs)(gblsch->setup_sso_grp + CVMX_PKI_FIND_AVAILABLE_RSRC);/*vinita_to_do to extract node*/
		rs = cvmx_sso_allocate_group(node);
		if (rs < 0) {
			cvmx_dprintf("pki-helper:gbl: ERROR: sso grp not available\n");
			return rs;
		}
		gblsch->sso_grp = rs | (node << 8);
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:gbl: sso grp alloced is %d\n", gblsch->sso_grp);
	}
	return 0;
}
#endif /* ! CVMX_BUILD_FOR_LINUX_KERNEL */

/**
 * This function sets up scheduling parameters (pool, aura, sso group etc)
 * of an ipd port.
 * @param ipd_port      ipd port number
 * @param prtsch        pointer to struct containing port's
 *                      scheduling parameters.
 */
int cvmx_helper_pki_init_port(int ipd_port, struct cvmx_pki_prt_schd *prtsch)
{
	int num_qos;
	int qos;
	struct cvmx_pki_qpg_config qpg_cfg;
	struct cvmx_pki_qos_schd *qossch;
	struct cvmx_pki_style_config style_cfg;
	struct cvmx_pki_pkind_config pknd_cfg;
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	int pknd;
	uint16_t mbuff_size;
	int rs;

	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);
	num_qos = cvmx_helper_pki_get_num_qpg_entry(prtsch->qpg_qos);
	mbuff_size = prtsch->pool_buff_size;
	memset(&qpg_cfg, 0, sizeof(qpg_cfg));

	/* Reserve port resources */
	rs = __cvmx_helper_pki_port_rsrcs(xp.node, prtsch);
	if (rs)
		return rs;
	/* Reserve qpg resources */
	if (prtsch->qpg_base < 0) {
		rs = cvmx_pki_qpg_entry_alloc(xp.node, prtsch->qpg_base, num_qos);
		if (rs < 0) {
			cvmx_dprintf("pki-helper:port%d:ERROR: qpg entries not available\n", ipd_port);
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
		prtsch->qpg_base = rs;
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:port-init: to port %d, qpg_base %d allocated\n",
				ipd_port, prtsch->qpg_base);
	}

	if (prtsch->qpg_qos) {
		for (qos = 0; qos < num_qos; qos++) {
			qossch = &prtsch->qos_s[qos];
			if (!qossch->pool_per_qos)
				qossch->pool_num = prtsch->pool_num;
			else if (qossch->pool_buff_size < mbuff_size)
				mbuff_size = qossch->pool_buff_size;
			if (!qossch->aura_per_qos)
				qossch->aura_num = prtsch->aura_num;
			if (!qossch->sso_grp_per_qos)
				qossch->sso_grp = prtsch->sso_grp;

			/* Reserve qos resources */
			rs = __cvmx_helper_pki_qos_rsrcs(xp.node, qossch);
			if (rs)
				return rs;
			qpg_cfg.port_add = qossch->port_add;
			qpg_cfg.aura_num = qossch->aura_num;
			qpg_cfg.grp_ok = qossch->sso_grp;
			qpg_cfg.grp_bad = qossch->sso_grp;
			cvmx_pki_write_qpg_entry(xp.node, prtsch->qpg_base + qos, &qpg_cfg);
			if (pki_helper_debug)
				cvmx_dprintf("pki-helper:port-init: port %d qos %d has port_add %d aura %d grp %d\n",
				ipd_port, qos, qossch->port_add,
				qossch->aura_num, qossch->sso_grp);
		}
	} else
		qpg_cfg.port_add = 0;
		qpg_cfg.aura_num = prtsch->aura_num;
		qpg_cfg.grp_ok = prtsch->sso_grp;
		qpg_cfg.grp_bad = prtsch->sso_grp;
		cvmx_pki_write_qpg_entry(xp.node, prtsch->qpg_base, &qpg_cfg);
		/* Allocate style here and map it to the port */
		rs = cvmx_pki_style_alloc(xp.node, prtsch->style);
		if (rs == CVMX_RESOURCE_ALREADY_RESERVED) {
			cvmx_dprintf("pki-helper: INFO: style will be shared\n");
		} else if (rs == CVMX_RESOURCE_ALLOC_FAILED) {
			cvmx_dprintf("pki-helper: ERROR: style not available\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		} else {
			prtsch->style = rs;
			if (pki_helper_debug)
				cvmx_dprintf("pki-helper:port-init: port %d has style %d\n",
					ipd_port, prtsch->style);
			style_cfg = pki_dflt_style[xp.node];
			style_cfg.parm_cfg.qpg_qos = prtsch->qpg_qos;
			style_cfg.parm_cfg.qpg_base = prtsch->qpg_base;
			style_cfg.parm_cfg.qpg_port_msb = 0;
			style_cfg.parm_cfg.qpg_port_sh = 0;
			style_cfg.parm_cfg.mbuff_size = mbuff_size;
			cvmx_pki_write_style_config(xp.node, prtsch->style,
				CVMX_PKI_CLUSTER_ALL, &style_cfg);
		}
		pknd = cvmx_helper_get_pknd(xiface, cvmx_helper_get_interface_index_num(ipd_port));
		cvmx_pki_read_pkind_config(xp.node, pknd, &pknd_cfg);
		pknd_cfg.initial_style = prtsch->style;
		pknd_cfg.fcs_pres = __cvmx_helper_get_has_fcs(xiface);
		cvmx_pki_write_pkind_config(xp.node, pknd, &pknd_cfg);
		return 0;
}
EXPORT_SYMBOL(cvmx_helper_pki_init_port);

/**
 * This function sets up scheduling parameters (pool, aura, sso group etc)
 * of an interface (all ports/channels on that interface).
 * @param xiface        interface number with node.
 * @param intfsch      pointer to struct containing interface
 *                      scheduling parameters.
 * @param gblsch       pointer to struct containing global scheduling parameters
 *                      (can be NULL if not used)
 */
int cvmx_helper_pki_init_interface(const int xiface, struct cvmx_pki_intf_schd *intfsch,
			    struct cvmx_pki_global_schd *gblsch)
{
	const uint16_t num_ports = cvmx_helper_ports_on_interface(xiface);
	uint8_t qos;
	uint16_t port = num_ports;
	uint8_t port_msb = 0;
	uint8_t port_shift = 0;
	uint16_t num_entry = 0;
	uint8_t num_qos;
	int pknd;
	int rs;
	int has_fcs;
	int ipd_port;
	int qpg_base;
	uint64_t mbuff_size = 0;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	enum cvmx_pki_qpg_qos qpg_qos = CVMX_PKI_QPG_QOS_NONE;
	struct cvmx_pki_qpg_config qpg_cfg;
	struct cvmx_pki_prt_schd *prtsch;
	struct cvmx_pki_qos_schd *qossch;
	struct cvmx_pki_style_config style_cfg;
	struct cvmx_pki_pkind_config pknd_cfg;

	has_fcs = __cvmx_helper_get_has_fcs(xiface);
	memset(&qpg_cfg, 0, sizeof(qpg_cfg));

	if (pki_helper_debug)
		cvmx_dprintf("pki-helper:intf-init:intf%d initialize\n", xiface);

	if (!intfsch->pool_per_intf) {
		if (gblsch != NULL) {
			intfsch->_pool = gblsch->_pool;
			intfsch->pool_num = gblsch->pool_num;
		} else {
			cvmx_dprintf("ERROR:pki-helper:intf-init:intf%d: global scheduling is in use but is NULL\n", xiface);
			return -1;
		}
	} else {
		if (intfsch == NULL) {
			cvmx_dprintf("ERROR:pki-helper:intf-init:intf%d: interface scheduling pointer is NULL\n", xiface);
			return -1;
		}
		mbuff_size = intfsch->pool_buff_size;
	}
	if (!intfsch->aura_per_intf) {
		intfsch->_aura = gblsch->_aura;
		intfsch->aura_num = gblsch->aura_num;
	}
	if (!intfsch->sso_grp_per_intf)
		intfsch->sso_grp = gblsch->sso_grp;

	/* Allocate interface resources */
	rs = __cvmx_helper_pki_intf_rsrcs(xi.node, intfsch);
	if (rs)
		return rs;

	for (port = 0; port < num_ports; port++) {
		prtsch = &intfsch->prt_s[port];

		/* Skip invalid/disabled ports */
		if (!cvmx_helper_is_port_valid(xiface, port) || prtsch->cfg_port)
			continue;

		if (!prtsch->pool_per_prt) {
			prtsch->pool_num = intfsch->pool_num;
			prtsch->_pool = intfsch->_pool;
		} else if (prtsch->pool_buff_size < mbuff_size || !mbuff_size)
			mbuff_size = prtsch->pool_buff_size;
		if (!prtsch->aura_per_prt) {
			prtsch->aura_num = intfsch->aura_num;
			prtsch->_aura = intfsch->_aura;
		}
		if (!prtsch->sso_grp_per_prt)
			prtsch->sso_grp = intfsch->sso_grp;

		rs = __cvmx_helper_pki_port_rsrcs(xi.node, prtsch);
		if (rs)
			return rs;

		/* Port is using qpg qos to schedule packets to differnet aura or sso group */
		num_qos = cvmx_helper_pki_get_num_qpg_entry(prtsch->qpg_qos);
		if (pki_helper_debug)
			cvmx_dprintf("pki-helper:intf-init:intf%d: port %d used qpg_qos=%d\n",
				     xiface, port, prtsch->qpg_qos);

		/* All ports will share the aura from port 0 for the respective qos */
		/* Port 0 should never have this set to TRUE **/
		if (intfsch->qos_share_aura && (port != 0)) {
			if (pki_helper_debug)
				cvmx_dprintf("pki-helper:intf-init:intf%d All ports will share same aura for all qos\n", xiface);
			for (qos = 0; qos < num_qos; qos++) {
				qossch = &prtsch->qos_s[qos];
				prtsch->qpg_qos = intfsch->prt_s[0].qpg_qos;
				qossch->pool_per_qos = intfsch->prt_s[0].qos_s[qos].pool_per_qos;
				qossch->aura_per_qos = intfsch->prt_s[0].qos_s[qos].aura_per_qos;
				qossch->pool_num = intfsch->prt_s[0].qos_s[qos].pool_num;
				qossch->_pool = intfsch->prt_s[0].qos_s[qos]._pool;
				qossch->aura_num = intfsch->prt_s[0].qos_s[qos].aura_num;
				qossch->_aura = intfsch->prt_s[0].qos_s[qos]._aura;
			}

		}
		if (intfsch->qos_share_grp && (port != 0)) {
			if (pki_helper_debug)
				cvmx_dprintf("pki-helper:intf-init:intf%d: All ports will share same sso group for all qos\n",xiface);
			for (qos = 0; qos < num_qos; qos++) {
				qossch = &prtsch->qos_s[qos];
				qossch->sso_grp_per_qos =
					intfsch->prt_s[0].qos_s[qos].sso_grp_per_qos;
				qossch->sso_grp =
					intfsch->prt_s[0].qos_s[qos].sso_grp;
			}
		}
		for (qos = 0; qos < num_qos; qos++) {
			qossch = &prtsch->qos_s[qos];
			if (!qossch->pool_per_qos) {
				qossch->pool_num = prtsch->pool_num;
				qossch->_pool = prtsch->_pool;
				if (pki_helper_debug)
					cvmx_dprintf("pki-helper:intf-init:intf%d: qos %d has pool %d\n",
						xiface, qos, prtsch->pool_num);
			} else if (qossch->pool_buff_size < mbuff_size ||
				    !mbuff_size)
				mbuff_size = qossch->pool_buff_size;
			if (!qossch->aura_per_qos) {
				qossch->aura_num = prtsch->aura_num;
				qossch->_aura = prtsch->_aura;
			}
			if (!qossch->sso_grp_per_qos)
				qossch->sso_grp = prtsch->sso_grp;
			rs = __cvmx_helper_pki_qos_rsrcs(xi.node, qossch);
			if (rs)
				return rs;
		}
	}
	/* Using port shift and port msb to schedule packets from differnt port to differnt
	auras and different sso group */
	/* Using QPG_QOS to schedule packets to different aura and sso group */
	/* If ports needs to send packets to different aura and sso group
	depending on packet qos */
	/* We will need to set up aura and sso group for each port and each qos */
	/* If all ports are using same style, they will be using same qpg_qos so
	check only for port 0*/
	if (intfsch->style_per_intf) {
		if (intfsch->prt_s[0].qpg_qos) { /* all ports using same style will use same qos defined in port 0 config */
			qpg_qos = intfsch->prt_s[0].qpg_qos;
			num_qos = cvmx_helper_pki_get_num_qpg_entry(
				intfsch->prt_s[0].qpg_qos);
			if (intfsch->qos_share_aura && intfsch->qos_share_grp) {
				/* All ports will use same qpg offset so no need for
				port_msb or port shift */
				port_msb = 0;
				port_shift = 0;
				num_entry = num_qos;
				qpg_base = intfsch->qpg_base;
				rs = __cvmx_helper_pki_set_intf_qpg(
					xi.node, 0, qpg_base, num_entry,
					intfsch);
				if (rs == -1)
					return rs;
				intfsch->qpg_base = rs;
			} else {
				port_msb = 8;
				port_shift = __cvmx_helper_pki_port_shift(xiface, intfsch->prt_s[0].qpg_qos);
				if (pki_helper_debug) {
					cvmx_dprintf("pki-helper: num qpg entry needed %d\n", (int)num_entry);
					cvmx_dprintf("pki-helper:port_msb=%d port_shift=%d\n", port_msb, port_shift);
				}
				num_entry = num_qos;
				for (port = 0; port < num_ports; port++) {
					/* Skip invalid/disabled ports */
					prtsch =  &intfsch->prt_s[port];
					if (!cvmx_helper_is_port_valid(xiface, port) || prtsch->cfg_port)
						continue;
					ipd_port = cvmx_helper_get_ipd_port(xiface, port);
					qpg_base = intfsch->qpg_base + ((ipd_port & 0xff) << port_shift);
					rs = __cvmx_helper_pki_set_intf_qpg(
						xi.node, port, qpg_base,
						num_entry, intfsch);
					if (rs == -1)
						return rs;
					prtsch->qpg_base = rs;
				}
				intfsch->qpg_base = intfsch->prt_s[0].qpg_base;
			}
		} else if (intfsch->prt_s[0].aura_per_prt || intfsch->prt_s[0].sso_grp_per_prt) {
			/* Every port is using their own aura or group but no qos */
			port_msb = 8;
			port_shift = 0;
			num_entry = 1;
			if (pki_helper_debug)
				cvmx_dprintf("pki-helper: aura/grp_per_prt: num qpg entry needed %d\n", (int)num_entry);
			for (port = 0; port < num_ports; port++) {
				prtsch =  &intfsch->prt_s[port];
				/* Skip invalid/disabled ports */
				if (!cvmx_helper_is_port_valid(xiface, port) || prtsch->cfg_port)
					continue;
				ipd_port = cvmx_helper_get_ipd_port(xiface, port);
				qpg_base = intfsch->qpg_base + ((ipd_port & 0xff) << port_shift);
				if (pki_helper_debug)
					cvmx_dprintf("port %d intf_q_base=%d q_base= %d\n",
						port, intfsch->qpg_base, qpg_base);
				qpg_base = cvmx_pki_qpg_entry_alloc(xi.node, qpg_base, num_entry);
				if (qpg_base == CVMX_RESOURCE_ALREADY_RESERVED) {
					cvmx_dprintf("pki-helper: INFO: qpg entries will be shared\n");
				} else if (qpg_base == CVMX_RESOURCE_ALLOC_FAILED) {
					cvmx_dprintf("pki-helper: ERROR: qpg entries not available\n");
					return qpg_base;
				} else {
					if (intfsch->qpg_base < 0)
						intfsch->qpg_base = qpg_base;
					prtsch->qpg_base = qpg_base;
				}
				qpg_cfg.port_add = 0;
				qpg_cfg.aura_num = prtsch->aura_num;
				qpg_cfg.grp_ok = prtsch->sso_grp;
				qpg_cfg.grp_bad = prtsch->sso_grp;
				cvmx_pki_write_qpg_entry(xi.node, qpg_base, &qpg_cfg);
			}
			intfsch->qpg_base = intfsch->prt_s[0].qpg_base;
		} else { /* All ports on that intf use same port_add, aura & sso grps */
			/* All ports will use same qpg offset so no need for
			port_msb or port shift */
			port_msb = 0;
			port_shift = 0;
			num_entry = 1;
			qpg_base = intfsch->qpg_base;
			qpg_base = cvmx_pki_qpg_entry_alloc(xi.node, qpg_base, num_entry);
			if (qpg_base == CVMX_RESOURCE_ALREADY_RESERVED) {
				cvmx_dprintf("pki-helper: INFO: qpg entries will be shared\n");
			} else if (qpg_base == CVMX_RESOURCE_ALLOC_FAILED) {
				cvmx_dprintf("pki-helper: ERROR: qpg entries not available\n");
				return qpg_base;
			} else
				intfsch->qpg_base = qpg_base;

			qpg_cfg.port_add = 0;
			qpg_cfg.aura_num = intfsch->aura_num;
			qpg_cfg.grp_ok = intfsch->sso_grp;
			qpg_cfg.grp_bad = intfsch->sso_grp;
			cvmx_pki_write_qpg_entry(xi.node, qpg_base, &qpg_cfg);
		}
		if (!mbuff_size) {
			if (!gblsch->setup_pool) {
				cvmx_dprintf("No pool has setup for intf %d\n",
					xiface);
				return -1;
			}
			mbuff_size = gblsch->pool_buff_size;
			cvmx_dprintf("interface %d on node %d is using global pool\n", xi.interface, xi.node);
		}
		/* Allocate style here and map it to all ports on interface */
		rs = cvmx_pki_style_alloc(xi.node, intfsch->style);
		if (rs == CVMX_RESOURCE_ALREADY_RESERVED) {
			cvmx_dprintf("passthrough: INFO: style will be shared\n");
		} else if (rs == CVMX_RESOURCE_ALLOC_FAILED) {
			cvmx_dprintf("passthrough: ERROR: style not available\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		} else {
			intfsch->style = rs;
			if (pki_helper_debug)
				cvmx_dprintf("style %d allocated intf %d qpg_base %d\n",
					intfsch->style, xiface,
					intfsch->qpg_base);
			style_cfg = pki_dflt_style[xi.node];
			style_cfg.parm_cfg.qpg_qos = qpg_qos;
			style_cfg.parm_cfg.qpg_base = intfsch->qpg_base;
			style_cfg.parm_cfg.qpg_port_msb = port_msb;
			style_cfg.parm_cfg.qpg_port_sh = port_shift;
			style_cfg.parm_cfg.mbuff_size = mbuff_size;
			cvmx_pki_write_style_config(xi.node, intfsch->style,
				CVMX_PKI_CLUSTER_ALL, &style_cfg);
		}
		for (port = 0; port < num_ports; port++) {
			prtsch = &intfsch->prt_s[port];
			/* Skip invalid/disabled ports */
			if (!cvmx_helper_is_port_valid(xiface, port) || prtsch->cfg_port)
				continue;
			prtsch->style = intfsch->style;
			pknd = cvmx_helper_get_pknd(xiface, port);
			cvmx_pki_read_pkind_config(xi.node, pknd, &pknd_cfg);
			pknd_cfg.initial_style = intfsch->style;
			pknd_cfg.fcs_pres = has_fcs;
			cvmx_pki_write_pkind_config(xi.node, pknd, &pknd_cfg);
		}
	} else {
		port_msb = 0;
		port_shift = 0;
		for (port = 0; port < num_ports; port++) {
			prtsch = &intfsch->prt_s[port];
			/* Skip invalid/disabled ports */
			if (!cvmx_helper_is_port_valid(xiface, port) || prtsch->cfg_port)
				continue;
			if (prtsch->qpg_qos && intfsch->qos_share_aura &&
				intfsch->qos_share_grp && port != 0) {
				if (pki_helper_debug)
					cvmx_dprintf("intf %d has all ports share qos aura n grps\n",
						xiface);
				/* Ports have differnet styles but want
				 * to share same qpg entries.
				this might never be the case */
				prtsch->qpg_base = intfsch->prt_s[0].qpg_base;
			}
			ipd_port = cvmx_helper_get_ipd_port(xiface, port);
			cvmx_helper_pki_init_port(ipd_port, prtsch);
		}
	}
	return 0;
}

/**
 * This function gets all the PKI parameters related to that
 * particular port from hardware.
 * @param ipd_port	ipd port number with node to get parameter of
 * @param port_cfg	pointer to structure where to store read parameters
 */
void cvmx_pki_get_port_config(int ipd_port, struct cvmx_pki_port_config *port_cfg)
{
	int interface, index, pknd;
	int style, cl_mask;
	cvmx_pki_icgx_cfg_t pki_cl_msk;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	/* get the pkind used by this ipd port */
	interface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);
	pknd = cvmx_helper_get_pknd(interface, index);

	cvmx_pki_read_pkind_config(xp.node, pknd, &port_cfg->pkind_cfg);
	style = port_cfg->pkind_cfg.initial_style;
	pki_cl_msk.u64 = cvmx_read_csr_node(xp.node, CVMX_PKI_ICGX_CFG(port_cfg->pkind_cfg.cluster_grp));
	cl_mask = pki_cl_msk.s.clusters;
	cvmx_pki_read_style_config(xp.node, style, cl_mask, &port_cfg->style_cfg);
}
EXPORT_SYMBOL(cvmx_pki_get_port_config);

/**
 * This function sets all the PKI parameters related to that
 * particular port in hardware.
 * @param ipd_port	ipd port number with node to get parameter of
 * @param port_cfg	pointer to structure containing port parameters
 */
void cvmx_pki_set_port_config(int ipd_port, struct cvmx_pki_port_config *port_cfg)
{
	int interface, index, pknd;
	int style, cl_mask;
	cvmx_pki_icgx_cfg_t pki_cl_msk;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	/* get the pkind used by this ipd port */
	interface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);
	pknd = cvmx_helper_get_pknd(interface, index);

	if (cvmx_pki_write_pkind_config(xp.node, pknd, &port_cfg->pkind_cfg))
		return;
	style = port_cfg->pkind_cfg.initial_style;
	pki_cl_msk.u64 = cvmx_read_csr_node(xp.node, CVMX_PKI_ICGX_CFG(port_cfg->pkind_cfg.cluster_grp));
	cl_mask = pki_cl_msk.s.clusters;
	cvmx_pki_write_style_config(xp.node, style, cl_mask, &port_cfg->style_cfg);
}
EXPORT_SYMBOL(cvmx_pki_set_port_config);

/**
 * This function displays all the PKI parameters related to that
 * particular port.
 * @param ipd_port	ipd port number to display parameter of
 */
void cvmx_pki_show_port_config(int ipd_port)
{
	int interface, index, pknd;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	interface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);
	pknd = cvmx_helper_get_pknd(interface, index);
	cvmx_dprintf("Showing stats for intf 0x%x port %d------------------\n", interface, index);
	cvmx_pki_show_pkind_attributes(xp.node, pknd);
	cvmx_dprintf("END STAUS------------------------\n\n");
}

void cvmx_helper_pki_errata(int node)
{
	struct cvmx_pki_global_config gbl_cfg;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		cvmx_pki_read_global_config(node, &gbl_cfg);
		gbl_cfg.fpa_wait = CVMX_PKI_WAIT_PKT;
		cvmx_pki_write_global_config(node, &gbl_cfg);
	}
}

static const char *pki_ltype_sprint(int ltype)
{
	switch (ltype) {
	case CVMX_PKI_LTYPE_E_ENET:	return "(ENET)";
	case CVMX_PKI_LTYPE_E_VLAN:	return "(VLAN)";
	case CVMX_PKI_LTYPE_E_SNAP_PAYLD:return "(SNAP_PAYLD)";
	case CVMX_PKI_LTYPE_E_ARP:	return "(ARP)";
	case CVMX_PKI_LTYPE_E_RARP:	return "(RARP)";
	case CVMX_PKI_LTYPE_E_IP4:	return "(IP4)";
	case CVMX_PKI_LTYPE_E_IP4_OPT:	return "(IP4_OPT)";
	case CVMX_PKI_LTYPE_E_IP6:	return "(IP6)";
	case CVMX_PKI_LTYPE_E_IP6_OPT:	return "(IP6_OPT)";
	case CVMX_PKI_LTYPE_E_IPSEC_ESP:return "(IPSEC_ESP)";
	case CVMX_PKI_LTYPE_E_IPFRAG:	return "(IPFRAG)";
	case CVMX_PKI_LTYPE_E_IPCOMP:	return "(IPCOMP)";
	case CVMX_PKI_LTYPE_E_TCP:	return "(TCP)";
	case CVMX_PKI_LTYPE_E_UDP:	return "(UDP)";
	case CVMX_PKI_LTYPE_E_SCTP:	return "(SCTP)";
	case CVMX_PKI_LTYPE_E_UDP_VXLAN:return "(UDP_VXLAN)";
	case CVMX_PKI_LTYPE_E_GRE:	return "(GRE)";
	case CVMX_PKI_LTYPE_E_NVGRE:	return "(NVGRE)";
	case CVMX_PKI_LTYPE_E_GTP:	return "(GTP)";
	default:			return "";
	}
}

void cvmx_pki_dump_wqe(const cvmx_wqe_78xx_t *wqp)
{
	int i;
	/* it is not cvmx_shared so per core only */
	static uint64_t count;

	cvmx_dprintf("Wqe entry for packet %lld\n", (unsigned long long)count++);
	cvmx_dprintf("    WORD%02d: %016llx", (int)0, (unsigned long long)wqp->word0.u64);
	cvmx_dprintf(" aura=0x%x", wqp->word0.aura);
	cvmx_dprintf(" apad=%d", wqp->word0.apad);
	cvmx_dprintf(" chan=0x%x", wqp->word0.channel);
	cvmx_dprintf(" bufs=%d" , wqp->word0.bufs);
	cvmx_dprintf(" style=0x%x" , wqp->word0.style);
	cvmx_dprintf(" pknd=0x%x" , wqp->word0.pknd);
	cvmx_dprintf("\n");
	cvmx_dprintf("    WORD%02d: %016llx", (int)1, (unsigned long long)wqp->word1.u64);
	cvmx_dprintf(" len=%d" , wqp->word1.len);
	cvmx_dprintf(" grp=0x%x" , wqp->word1.grp);
	cvmx_dprintf(" tt=%s", OCT_TAG_TYPE_STRING(wqp->word1.tag_type));
	cvmx_dprintf(" tag=0x%08x" , wqp->word1.tag);
	cvmx_dprintf("\n");
	if (wqp->word2.u64) {
		cvmx_dprintf("    WORD%02d: %016llx"  , (int)2, (unsigned long long)wqp->word2.u64);
		if (wqp->word2.le_hdr_type)
			cvmx_dprintf(" [LAE]");
		if (wqp->word2.lb_hdr_type)
			cvmx_dprintf(" lbty=%d"  "%s",
				wqp->word2.lb_hdr_type, pki_ltype_sprint(wqp->word2.lb_hdr_type));
		if (wqp->word2.lc_hdr_type)
			cvmx_dprintf(" lcty=%d"  "%s",
				wqp->word2.lc_hdr_type, pki_ltype_sprint(wqp->word2.lc_hdr_type));
		if (wqp->word2.ld_hdr_type)
			cvmx_dprintf(" ldty=%d"  "%s",
				wqp->word2.ld_hdr_type, pki_ltype_sprint(wqp->word2.ld_hdr_type));
		if (wqp->word2.le_hdr_type)
			cvmx_dprintf(" lety=%d"  "%s",
				wqp->word2.le_hdr_type, pki_ltype_sprint(wqp->word2.le_hdr_type));
		if (wqp->word2.lf_hdr_type)
			cvmx_dprintf(" lfty=%d"  "%s",
				wqp->word2.lf_hdr_type, pki_ltype_sprint(wqp->word2.lf_hdr_type));
		if (wqp->word2.lg_hdr_type)
			cvmx_dprintf(" lgty=%d"  "%s",
				wqp->word2.lg_hdr_type, pki_ltype_sprint(wqp->word2.lg_hdr_type));
		if (wqp->word2.pcam_flag1)
			cvmx_dprintf(" PF1");
		if (wqp->word2.pcam_flag2)
			cvmx_dprintf(" PF2");
		if (wqp->word2.pcam_flag3)
			cvmx_dprintf(" PF3");
		if (wqp->word2.pcam_flag4)
			cvmx_dprintf(" PF4");
		if (wqp->word2.vlan_valid || wqp->word2.vlan_stacked) {
			if (wqp->word2.vlan_valid)
				cvmx_dprintf(" vlan valid");
			if (wqp->word2.vlan_stacked)
				cvmx_dprintf(" vlan stacked");
			cvmx_dprintf(" ");
		}
		if (wqp->word2.stat_inc)
			cvmx_dprintf(" stat_inc");
		if (wqp->word2.is_frag)
			cvmx_dprintf(" L3 Fragment");
		if (wqp->word2.is_l3_bcast)
			cvmx_dprintf(" L3 Broadcast");
		if (wqp->word2.is_l3_mcast)
			cvmx_dprintf(" L3 Multicast");
		if (wqp->word2.is_l2_bcast)
			cvmx_dprintf(" L2 Broadcast");
		if (wqp->word2.is_l2_mcast)
			cvmx_dprintf(" L2 Multicast");
		if (wqp->word2.is_raw)
			cvmx_dprintf(" RAW");
		if (wqp->word2.err_level || wqp->word2.err_code) {
			cvmx_dprintf(" errlev=%d" , wqp->word2.err_level);
			cvmx_dprintf(" opcode=0x%x" , wqp->word2.err_code);
		}
		cvmx_dprintf("\n");
	}
	cvmx_dprintf("    WORD%02d: %016llx", (int)3, (unsigned long long)wqp->packet_ptr.u64);

	cvmx_dprintf(" size=%d" , wqp->packet_ptr.size);
	cvmx_dprintf(" addr=0x%llx" , (unsigned long long)wqp->packet_ptr.addr);

	cvmx_dprintf("\n");
	if (wqp->word4.u64) {
		cvmx_dprintf("    WORD%02d: %016llx", (int)4, (unsigned long long)wqp->word4.u64);
		if (wqp->word4.ptr_layer_a)
			cvmx_dprintf(" laptr=%d" , wqp->word4.ptr_layer_a);
		if (wqp->word4.ptr_layer_b)
			cvmx_dprintf(" lbptr=%d" , wqp->word4.ptr_layer_b);
		if (wqp->word4.ptr_layer_c)
			cvmx_dprintf(" lcptr=%d" , wqp->word4.ptr_layer_c);
		if (wqp->word4.ptr_layer_d)
			cvmx_dprintf(" ldptr=%d" , wqp->word4.ptr_layer_d);
		if (wqp->word4.ptr_layer_e)
			cvmx_dprintf(" leptr=%d" , wqp->word4.ptr_layer_e);
		if (wqp->word4.ptr_layer_f)
			cvmx_dprintf(" lfptr=%d" , wqp->word4.ptr_layer_f);
		if (wqp->word4.ptr_layer_g)
			cvmx_dprintf(" lgptr=%d" , wqp->word4.ptr_layer_g);
		if (wqp->word4.ptr_vlan)
			cvmx_dprintf(" vlptr=%d" , wqp->word4.ptr_vlan);
		cvmx_dprintf("\n");
	}
	for (i = 0; i < 10; ++i) {
		if (wqp->wqe_data[i])
			cvmx_dprintf("    WORD%02d: %016llx"  "\n",
				i+5, (unsigned long long)wqp->wqe_data[i]);
	}
}



/**
 * Modifies maximum frame length to check.
 * It modifies the global frame length set used by this port, any other
 * port using the same set will get affected too.
 * @param ipd_port	ipd port for which to modify max len.
 * @param max_size	maximum frame length
 */
void cvmx_pki_set_max_frm_len(int ipd_port, uint32_t max_size)
{
	/* On CN78XX frame check is enabled for a style n and
	PKI_CLX_STYLE_CFG[minmax_sel] selects which set of
	MAXLEN/MINLEN to use. */
	int interface, index, pknd;
	cvmx_pki_clx_stylex_cfg_t style_cfg;
	cvmx_pki_frm_len_chkx_t frame_len;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);
	int cluster = 0;
	int style;
	int sel;

	/* get the pkind used by this ipd port */
	interface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);
	pknd = cvmx_helper_get_pknd(interface, index);

	style = cvmx_pki_get_pkind_style(xp.node, pknd);
	style_cfg.u64 = cvmx_read_csr_node(xp.node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
	sel = style_cfg.s.minmax_sel;
	frame_len.u64 = cvmx_read_csr(CVMX_PKI_FRM_LEN_CHKX(sel));
	frame_len.s.maxlen = max_size;
	cvmx_write_csr_node(xp.node, CVMX_PKI_FRM_LEN_CHKX(sel), frame_len.u64);
}

/**
 * This function sets up all th eports of particular interface
 * for chosen fcs mode. (only use for backward compatibility).
 * New application can control it via init_interface calls.
 * @param node		node number.
 * @param interface	interface number.
 * @param nports	number of ports
 * @param has_fcs	1 -- enable fcs check and fcs strip.
 *			0 -- disable fcs check.
 */
void cvmx_helper_pki_set_fcs_op(int node, int interface, int nports, int has_fcs)
{
	int index;
	int pknd;
	int cluster = 0;
	cvmx_pki_clx_pkindx_cfg_t pkind_cfg;

	for (index = 0; index < nports; index++) {
		pknd = cvmx_helper_get_pknd(interface, index);
		while (cluster < CVMX_PKI_NUM_CLUSTER) {
			/*find the cluster in use pass2*/
			pkind_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_CFG(pknd, cluster));
			pkind_cfg.s.fcs_pres = has_fcs;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PKINDX_CFG(pknd, cluster), pkind_cfg.u64);
			cluster++;
		}
		/* make sure fcs_strip and fcs_check is also enable/disable for the style used by that port*/
		cvmx_pki_endis_fcs_check(node, pknd, has_fcs, has_fcs);
		cluster = 0;
	}
}

/**
 * This function sets the wqe buffer mode of all ports. First packet data buffer can reside
 * either in same buffer as wqe OR it can go in separate buffer. If used the later mode,
 * make sure software allocate enough buffers to now have wqe separate from packet data.
 * @param node			node number.
 * @param pkt_outside_wqe	0 = The packet link pointer will be at word [FIRST_SKIP]
 *				immediately followed by packet data, in the same buffer
 *				as the work queue entry.
 *				1 = The packet link pointer will be at word [FIRST_SKIP] in a new
 *				buffer separate from the work queue entry. Words following the
 *				WQE in the same cache line will be zeroed, other lines in the
 *				buffer will not be modified and will retain stale data (from the
 *				buffer’s previous use). This setting may decrease the peak PKI
 *				performance by up to half on small packets.
 */
void cvmx_helper_pki_set_wqe_mode(int node, bool pkt_outside_wqe)
{
	int interface, port, pknd;
	int num_intf, num_ports;
	uint64_t style;

	/* get the pkind used by this ipd port */
	num_intf = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_intf; interface++) {
		num_ports = cvmx_helper_ports_on_interface(interface);
		/*Skip invalid/disabled interfaces */
		if (num_ports <= 0)
			continue;
		for (port = 0; port < num_ports; port++) {
			pknd = cvmx_helper_get_pknd(interface, port);
			style = cvmx_pki_get_pkind_style(node, pknd);
			cvmx_pki_set_wqe_mode(node, style, pkt_outside_wqe);
		}
	}
}

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to
 * be in little-endian. Does not change the WQE header format, which is
 * properly endian neutral.
 * @param node		node number.
 */
void cvmx_helper_pki_set_little_endian(int node)
{
	int interface, port, pknd;
	int num_intf, num_ports;
	uint64_t style;

	/* get the pkind used by this ipd port */
	num_intf = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_intf; interface++) {
		num_ports = cvmx_helper_ports_on_interface(interface);
		/*Skip invalid/disabled interfaces */
		if (num_ports <= 0)
			continue;
		for (port = 0; port < num_ports; port++) {
			pknd = cvmx_helper_get_pknd(interface, port);
			style = cvmx_pki_get_pkind_style(node, pknd);
			cvmx_pki_set_little_endian(node, style);
		}
	}
}
