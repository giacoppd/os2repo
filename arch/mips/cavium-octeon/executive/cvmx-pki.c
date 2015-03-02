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
 * PKI Support.
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki-defs.h>
#include <asm/octeon/cvmx-pki.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-pki-cluster.h>
#else
#include "cvmx.h"
#include "cvmx-version.h"
#include "cvmx-error.h"
#include "cvmx-pki-defs.h"
#include "cvmx-pki.h"
#include "cvmx-fpa3.h"
#include "cvmx-pki-cluster.h"
#endif


/**
 * This function enables pki
 * @param node	node to enable pki in.
 */
void cvmx_pki_enable(int node)
{

	cvmx_pki_sft_rst_t pki_sft_rst;
	cvmx_pki_buf_ctl_t pki_en;

	pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);

	while (pki_sft_rst.s.busy != 0)
		pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);

	pki_en.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	if (pki_en.s.pki_en)
		cvmx_dprintf("Warning: Enabling PKI when PKI already enabled.\n");

	pki_en.s.pki_en = 1;

	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_en.u64);

}
EXPORT_SYMBOL(cvmx_pki_enable);

/**
 * This function disables pki
 * @param node	node to disable pki in.
 */
void cvmx_pki_disable(int node)
{
	cvmx_pki_buf_ctl_t pki_en;
	pki_en.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	pki_en.s.pki_en = 0;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_en.u64);

}
EXPORT_SYMBOL(cvmx_pki_disable);

/**
 * This function soft resets pki
 * @param node	node to enable pki in.
 */
void cvmx_pki_reset(int node)
{
	cvmx_pki_sft_rst_t pki_sft_rst;

	pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);

	while (pki_sft_rst.s.active != 0)
		pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);
	pki_sft_rst.s.rst = 1;
	cvmx_write_csr_node(node, CVMX_PKI_SFT_RST, pki_sft_rst.u64);
	while (pki_sft_rst.s.busy != 0)
		pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);
}

/**
 * This function sets the clusters in PKI
 * @param node	node to set clusters in.
 */
int cvmx_pki_setup_clusters(int node)
{
	int i;

	for (i = 0; i < cvmx_pki_cluster_code_length; i++)
		cvmx_write_csr_node(node, CVMX_PKI_IMEMX(i), cvmx_pki_cluster_code_default[i]);

	return 0;
}
EXPORT_SYMBOL(cvmx_pki_setup_clusters);

/**
 * @INTERNAL
 * This function is called by cvmx_helper_shutdown() to extract
 * all FPA buffers out of the PKI. After this function
 * completes, all FPA buffers that were prefetched by PKI
 * wil be in the apropriate FPA pool. This functions does not reset
 * PKI.
 * WARNING: It is very important that PKI be
 * reset soon after a call to this function.
 */
void __cvmx_pki_free_ptr(int node)
{
	cvmx_pki_buf_ctl_t buf_ctl;
	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	/*Disable buffering any data*/
	buf_ctl.s.pkt_off = 1;
	/*diable caching of any data and return all the prefetched buffers to fpa*/
	buf_ctl.s.fpa_cac_dis = 1;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);
}

/**
 * This function reads global configuration of PKI block.
 * @param node		      node number.
 * @param gbl_cfg	      pointer to struct to read global configuration
 */
void cvmx_pki_read_global_config(int node, struct cvmx_pki_global_config *gbl_cfg)
{
	cvmx_pki_stat_ctl_t stat_ctl;
	cvmx_pki_icgx_cfg_t pki_cl_grp;
	cvmx_pki_gbl_pen_t gbl_pen_reg;
	cvmx_pki_tag_secret_t tag_secret_reg;
	cvmx_pki_frm_len_chkx_t frm_len_chk;
	cvmx_pki_buf_ctl_t buf_ctl;
	int cl_grp;
	int id;

	stat_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_STAT_CTL);
	gbl_cfg->stat_mode = stat_ctl.s.mode;

	for (cl_grp = 0; cl_grp < CVMX_PKI_NUM_CLUSTER_GROUP; cl_grp++) {
		pki_cl_grp.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_ICGX_CFG(cl_grp));
		gbl_cfg->cluster_mask[cl_grp] = pki_cl_grp.s.clusters;
	}
	gbl_pen_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_GBL_PEN);
	gbl_cfg->gbl_pen.virt_pen = gbl_pen_reg.s.virt_pen;
	gbl_cfg->gbl_pen.clg_pen = gbl_pen_reg.s.clg_pen;
	gbl_cfg->gbl_pen.cl2_pen = gbl_pen_reg.s.cl2_pen;
	gbl_cfg->gbl_pen.l4_pen = gbl_pen_reg.s.l4_pen;
	gbl_cfg->gbl_pen.il3_pen = gbl_pen_reg.s.il3_pen;
	gbl_cfg->gbl_pen.l3_pen = gbl_pen_reg.s.l3_pen;
	gbl_cfg->gbl_pen.mpls_pen = gbl_pen_reg.s.mpls_pen;
	gbl_cfg->gbl_pen.fulc_pen = gbl_pen_reg.s.fulc_pen;
	gbl_cfg->gbl_pen.dsa_pen = gbl_pen_reg.s.dsa_pen;
	gbl_cfg->gbl_pen.hg_pen = gbl_pen_reg.s.hg_pen;

	tag_secret_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_SECRET);
	gbl_cfg->tag_secret.dst6 = tag_secret_reg.s.dst6;
	gbl_cfg->tag_secret.src6 = tag_secret_reg.s.src6;
	gbl_cfg->tag_secret.dst = tag_secret_reg.s.dst;
	gbl_cfg->tag_secret.src = tag_secret_reg.s.src;

	for (id = 0; id < CVMX_PKI_NUM_FRAME_CHECK; id++) {
		frm_len_chk.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_FRM_LEN_CHKX(id));
		gbl_cfg->frm_len[id].maxlen = frm_len_chk.s.maxlen;
		gbl_cfg->frm_len[id].minlen = frm_len_chk.s.minlen;
	}

	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	gbl_cfg->fpa_wait = buf_ctl.s.fpa_wait;
}

/**
 * This function writes max and min frame lengths to hardware which can be used
 * to check the size of frame arrived.There are 2 possible combination which are
 * indicated by id field.
 * @param node		node number.
 * @param id		choose which frame len register to write to
 * @param len_chk	struct containing Byte count for max-sized/min-sized frame check.
 *
 */
static void cvmx_pki_write_frame_len(int node, int id,
	struct cvmx_pki_frame_len len_chk)
{
	cvmx_pki_frm_len_chkx_t frm_len_chk;
	frm_len_chk.u64 = cvmx_read_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
	frm_len_chk.s.maxlen = len_chk.maxlen;
	frm_len_chk.s.minlen = len_chk.minlen;
	cvmx_write_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id), frm_len_chk.u64);
}

/**
 * This function writes global configuration of PKI into hw.
 * @param node		      node number.
 * @param gbl_cfg	      pointer to struct to global configuration
 */
void cvmx_pki_write_global_config(int node, struct cvmx_pki_global_config *gbl_cfg)
{
	cvmx_pki_stat_ctl_t stat_ctl;
	cvmx_pki_buf_ctl_t buf_ctl;
	int cl_grp;

	for (cl_grp = 0; cl_grp < CVMX_PKI_NUM_CLUSTER_GROUP; cl_grp++)
		cvmx_pki_attach_cluster_to_group(node, cl_grp, gbl_cfg->cluster_mask[cl_grp]);

	stat_ctl.u64 = 0;
	stat_ctl.s.mode = gbl_cfg->stat_mode;
	cvmx_write_csr_node(node, CVMX_PKI_STAT_CTL, stat_ctl.u64);

	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	buf_ctl.s.fpa_wait = gbl_cfg->fpa_wait;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);

	cvmx_pki_write_global_parse(node, gbl_cfg->gbl_pen);
	cvmx_pki_write_tag_secret(node, gbl_cfg->tag_secret);
	cvmx_pki_write_frame_len(node, 0, gbl_cfg->frm_len[0]);
	cvmx_pki_write_frame_len(node, 1, gbl_cfg->frm_len[1]);
}

/**
 * This function reads per pkind parameters in hardware which defines how
  the incoming packet is processed.
 * @param node		      node number.
 * @param pkind               PKI supports a large number of incoming interfaces
 *                            and packets arriving on different interfaces or channels
 *                            may want to be processed differently. PKI uses the pkind to
 *                            determine how the incoming packet is processed.
 * @param pkind_cfg	      pointer to struct conatining pkind configuration read from hw
 */
int cvmx_pki_read_pkind_config(int node, int pkind, struct cvmx_pki_pkind_config *pkind_cfg)
{
	int cluster = 0;
	uint64_t cl_mask;
	cvmx_pki_pkindx_icgsel_t pkind_clsel;
	cvmx_pki_clx_pkindx_style_t pkind_cfg_style;
	cvmx_pki_icgx_cfg_t pki_cl_grp;
	cvmx_pki_clx_pkindx_cfg_t pknd_cfg_reg;
	cvmx_pki_clx_pkindx_skip_t pknd_skip_reg;

	pkind_clsel.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_PKINDX_ICGSEL(pkind));
	pki_cl_grp.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_ICGX_CFG(pkind_clsel.s.icg));
	pkind_cfg->cluster_grp = (uint8_t)pkind_clsel.s.icg;
	cl_mask = (uint64_t)pki_cl_grp.s.clusters;
	cluster = __builtin_ffsll(cl_mask) - 1;

	pkind_cfg_style.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
	pkind_cfg->initial_parse_mode = pkind_cfg_style.s.pm;
	pkind_cfg->initial_style = pkind_cfg_style.s.style;

	pknd_cfg_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
	pkind_cfg->fcs_pres = pknd_cfg_reg.s.fcs_pres;
	pkind_cfg->parse_en.inst_hdr = pknd_cfg_reg.s.inst_hdr;
	pkind_cfg->parse_en.mpls_en = pknd_cfg_reg.s.mpls_en;
	pkind_cfg->parse_en.lg_custom = pknd_cfg_reg.s.lg_custom;
	pkind_cfg->parse_en.fulc_en = pknd_cfg_reg.s.fulc_en;
	pkind_cfg->parse_en.dsa_en = pknd_cfg_reg.s.dsa_en;
	pkind_cfg->parse_en.hg2_en = pknd_cfg_reg.s.hg2_en;
	pkind_cfg->parse_en.hg_en = pknd_cfg_reg.s.hg_en;

	pknd_skip_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
	pkind_cfg->fcs_skip = pknd_skip_reg.s.fcs_skip;
	pkind_cfg->inst_skip = pknd_skip_reg.s.inst_skip;

	return 0;
}

/**
 * This function writes per pkind parameters in hardware which defines how
  the incoming packet is processed.
 * @param node		node number.
 * @param pkind		PKI supports a large number of incoming interfaces
 *			and packets arriving on different interfaces or channels
 *			may want to be processed differently. PKI uses the pkind to
 *			determine how the incoming packet is processed.
 * @param pkind_cfg	pointer to struct conatining pkind configuration
 * 			need to be written in hw
 */
int cvmx_pki_write_pkind_config(int node, int pkind, struct cvmx_pki_pkind_config *pkind_cfg)
{
	int cluster = 0;
	uint64_t cluster_mask;

	cvmx_pki_pkindx_icgsel_t pkind_clsel;
	cvmx_pki_clx_pkindx_style_t pkind_cfg_style;
	cvmx_pki_icgx_cfg_t pki_cl_grp;
	cvmx_pki_clx_pkindx_cfg_t pknd_cfg_reg;
	cvmx_pki_clx_pkindx_skip_t pknd_skip_reg;


	if (pkind >= CVMX_PKI_NUM_PKIND || pkind_cfg->cluster_grp >= CVMX_PKI_NUM_CLUSTER_GROUP
		  || pkind_cfg->initial_style >= CVMX_PKI_NUM_FINAL_STYLE) {
		cvmx_dprintf("ERROR: Configuring PKIND pkind = %d cluster_group = %d style = %d\n",
			     pkind, pkind_cfg->cluster_grp, pkind_cfg->initial_style);
		return -1;
	}
	pkind_clsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	pkind_clsel.s.icg = pkind_cfg->cluster_grp;
	cvmx_write_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind), pkind_clsel.u64);

	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(pkind_cfg->cluster_grp));
	cluster_mask = (uint64_t)pki_cl_grp.s.clusters;
	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			pkind_cfg_style.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			pkind_cfg_style.s.pm = pkind_cfg->initial_parse_mode;
			pkind_cfg_style.s.style = pkind_cfg->initial_style;
			cvmx_write_csr_node(node,
				CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster),
				pkind_cfg_style.u64);
			pknd_cfg_reg.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
			pknd_cfg_reg.s.fcs_pres = pkind_cfg->fcs_pres;
			pknd_cfg_reg.s.inst_hdr = pkind_cfg->parse_en.inst_hdr;
			pknd_cfg_reg.s.mpls_en = pkind_cfg->parse_en.mpls_en;
			pknd_cfg_reg.s.lg_custom = pkind_cfg->parse_en.lg_custom;
			pknd_cfg_reg.s.fulc_en = pkind_cfg->parse_en.fulc_en;
			pknd_cfg_reg.s.dsa_en = pkind_cfg->parse_en.dsa_en;
			pknd_cfg_reg.s.hg2_en = pkind_cfg->parse_en.hg2_en;
			pknd_cfg_reg.s.hg_en = pkind_cfg->parse_en.hg_en;
			cvmx_write_csr_node(node,
				CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster),
				pknd_cfg_reg.u64);
			pknd_skip_reg.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
			pknd_skip_reg.s.fcs_skip = pkind_cfg->fcs_skip;
			pknd_skip_reg.s.inst_skip = pkind_cfg->inst_skip;
			cvmx_write_csr_node(node,
				CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster),
				pknd_skip_reg.u64);
		}
		cluster++;
	}
	return 0;
}

 /** This function reads parameters associated with tag configuration in hardware.
 * @param node		node number.
 * @param style		style to configure tag for
 * @param cluster_mask	Mask of clusters to configure the style for.
 * @param tag_cfg	pointer to tag configuration struct.
 */
void cvmx_pki_read_tag_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_tag_cfg *tag_cfg)
{
	cvmx_pki_clx_stylex_cfg2_t style_cfg2_reg;
	cvmx_pki_clx_stylex_alg_t style_alg_reg;
	int cluster = __builtin_ffsll(cluster_mask) - 1;

	style_cfg2_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
	style_alg_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_STYLEX_ALG(style, cluster));

	tag_cfg->tag_fields.layer_g_src = style_cfg2_reg.s.tag_src_lg;
	tag_cfg->tag_fields.layer_f_src = style_cfg2_reg.s.tag_src_lf;
	tag_cfg->tag_fields.layer_e_src = style_cfg2_reg.s.tag_src_le;
	tag_cfg->tag_fields.layer_d_src = style_cfg2_reg.s.tag_src_ld;
	tag_cfg->tag_fields.layer_c_src = style_cfg2_reg.s.tag_src_lc;
	tag_cfg->tag_fields.layer_b_src = style_cfg2_reg.s.tag_src_lb;
	tag_cfg->tag_fields.layer_g_dst = style_cfg2_reg.s.tag_dst_lg;
	tag_cfg->tag_fields.layer_f_dst = style_cfg2_reg.s.tag_dst_lf;
	tag_cfg->tag_fields.layer_e_dst = style_cfg2_reg.s.tag_dst_le;
	tag_cfg->tag_fields.layer_d_dst = style_cfg2_reg.s.tag_dst_ld;
	tag_cfg->tag_fields.layer_c_dst = style_cfg2_reg.s.tag_dst_lc;
	tag_cfg->tag_fields.layer_b_dst = style_cfg2_reg.s.tag_dst_lb;
	tag_cfg->tag_fields.tag_vni = style_alg_reg.s.tag_vni;
	tag_cfg->tag_fields.tag_gtp = style_alg_reg.s.tag_gtp;
	tag_cfg->tag_fields.tag_spi = style_alg_reg.s.tag_spi;
	tag_cfg->tag_fields.tag_sync = style_alg_reg.s.tag_syn;
	tag_cfg->tag_fields.ip_prot_nexthdr = style_alg_reg.s.tag_pctl;
	tag_cfg->tag_fields.second_vlan = style_alg_reg.s.tag_vs1;
	tag_cfg->tag_fields.first_vlan = style_alg_reg.s.tag_vs0;
	tag_cfg->tag_fields.mpls_label = style_alg_reg.s.tag_mpls0;
	tag_cfg->tag_fields.input_port = style_alg_reg.s.tag_prt;

	/** TO_DO get mask tag*/
}

 /** This function writes/configures parameters associated with tag configuration in hardware.
 * @param node	              node number.
 * @param style		      style to configure tag for
 * @param cluster_mask	      Mask of clusters to configure the style for.
 * @param tag_cfg	      pointer to taf configuration struct.
 */
void cvmx_pki_write_tag_config(int node, int style, uint64_t cluster_mask,
			       struct cvmx_pki_style_tag_cfg *tag_cfg)
{
	cvmx_pki_clx_stylex_cfg2_t style_cfg2_reg;
	cvmx_pki_clx_stylex_alg_t style_alg_reg;
	int cluster = 0;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			style_cfg2_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			style_cfg2_reg.s.tag_src_lg = tag_cfg->tag_fields.layer_g_src;
			style_cfg2_reg.s.tag_src_lf = tag_cfg->tag_fields.layer_f_src;
			style_cfg2_reg.s.tag_src_le = tag_cfg->tag_fields.layer_e_src;
			style_cfg2_reg.s.tag_src_ld = tag_cfg->tag_fields.layer_d_src;
			style_cfg2_reg.s.tag_src_lc = tag_cfg->tag_fields.layer_c_src;
			style_cfg2_reg.s.tag_src_lb = tag_cfg->tag_fields.layer_b_src;
			style_cfg2_reg.s.tag_dst_lg = tag_cfg->tag_fields.layer_g_dst;
			style_cfg2_reg.s.tag_dst_lf = tag_cfg->tag_fields.layer_f_dst;
			style_cfg2_reg.s.tag_dst_le = tag_cfg->tag_fields.layer_e_dst;
			style_cfg2_reg.s.tag_dst_ld = tag_cfg->tag_fields.layer_d_dst;
			style_cfg2_reg.s.tag_dst_lc = tag_cfg->tag_fields.layer_c_dst;
			style_cfg2_reg.s.tag_dst_lb = tag_cfg->tag_fields.layer_b_dst;
			cvmx_write_csr_node(node,
				CVMX_PKI_CLX_STYLEX_CFG2(style, cluster),
				style_cfg2_reg.u64);
			style_alg_reg.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg_reg.s.tag_vni = tag_cfg->tag_fields.tag_vni;
			style_alg_reg.s.tag_gtp = tag_cfg->tag_fields.tag_gtp;
			style_alg_reg.s.tag_spi = tag_cfg->tag_fields.tag_spi;
			style_alg_reg.s.tag_syn = tag_cfg->tag_fields.tag_sync;
			style_alg_reg.s.tag_pctl = tag_cfg->tag_fields.ip_prot_nexthdr;
			style_alg_reg.s.tag_vs1 = tag_cfg->tag_fields.second_vlan;
			style_alg_reg.s.tag_vs0 = tag_cfg->tag_fields.first_vlan;
			style_alg_reg.s.tag_mpls0 = tag_cfg->tag_fields.mpls_label;
			style_alg_reg.s.tag_prt = tag_cfg->tag_fields.input_port;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster), style_alg_reg.u64);

			/* TO_DO add mask tag */
		}
		cluster++;
	}
}

/**
 * This function reads parameters associated with style in hardware.
 * @param node		node number.
 * @param style		style to read from.
 * @param cluster_mask	Mask of clusters style belongs to.
 * @param style_cfg	pointer to style config struct.
 */
void cvmx_pki_read_style_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_config *style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t style_cfg_reg;
	cvmx_pki_clx_stylex_cfg2_t style_cfg2_reg;
	cvmx_pki_clx_stylex_alg_t style_alg_reg;
	cvmx_pki_stylex_buf_t style_buf_reg;
	int cluster = __builtin_ffsll(cluster_mask) - 1;

	style_cfg_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
	style_cfg2_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
	style_alg_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
	style_buf_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_STYLEX_BUF(style));

	style_cfg->parm_cfg.ip6_udp_opt = style_cfg_reg.s.ip6_udp_opt;
	style_cfg->parm_cfg.lenerr_en = style_cfg_reg.s.lenerr_en;
	style_cfg->parm_cfg.lenerr_eqpad = style_cfg_reg.s.lenerr_eqpad;
	style_cfg->parm_cfg.maxerr_en = style_cfg_reg.s.maxerr_en;
	style_cfg->parm_cfg.minerr_en = style_cfg_reg.s.minerr_en;
	style_cfg->parm_cfg.fcs_chk = style_cfg_reg.s.fcs_chk;
	style_cfg->parm_cfg.fcs_strip = style_cfg_reg.s.fcs_strip;
	style_cfg->parm_cfg.minmax_sel = style_cfg_reg.s.minmax_sel;
	style_cfg->parm_cfg.qpg_base = style_cfg_reg.s.qpg_base;
	style_cfg->parm_cfg.qpg_dis_padd = style_cfg_reg.s.qpg_dis_padd;
	style_cfg->parm_cfg.qpg_dis_aura = style_cfg_reg.s.qpg_dis_aura;
	style_cfg->parm_cfg.qpg_dis_grp = style_cfg_reg.s.qpg_dis_grp;
	style_cfg->parm_cfg.qpg_dis_grptag = style_cfg_reg.s.qpg_dis_grptag;
	style_cfg->parm_cfg.rawdrp = style_cfg_reg.s.rawdrp;
	style_cfg->parm_cfg.force_drop = style_cfg_reg.s.drop;
	style_cfg->parm_cfg.nodrop = style_cfg_reg.s.nodrop;

	style_cfg->parm_cfg.len_lg = style_cfg2_reg.s.len_lg;
	style_cfg->parm_cfg.len_lf = style_cfg2_reg.s.len_lf;
	style_cfg->parm_cfg.len_le = style_cfg2_reg.s.len_le;
	style_cfg->parm_cfg.len_ld = style_cfg2_reg.s.len_ld;
	style_cfg->parm_cfg.len_lc = style_cfg2_reg.s.len_lc;
	style_cfg->parm_cfg.len_lb = style_cfg2_reg.s.len_lb;
	style_cfg->parm_cfg.csum_lg = style_cfg2_reg.s.csum_lg;
	style_cfg->parm_cfg.csum_lf = style_cfg2_reg.s.csum_lf;
	style_cfg->parm_cfg.csum_le = style_cfg2_reg.s.csum_le;
	style_cfg->parm_cfg.csum_ld = style_cfg2_reg.s.csum_ld;
	style_cfg->parm_cfg.csum_lc = style_cfg2_reg.s.csum_lc;
	style_cfg->parm_cfg.csum_lb = style_cfg2_reg.s.csum_lb;

	style_cfg->parm_cfg.qpg_qos = style_alg_reg.s.qpg_qos;
	style_cfg->parm_cfg.tag_type = style_alg_reg.s.tt;
	style_cfg->parm_cfg.apad_nip = style_alg_reg.s.apad_nip;
	style_cfg->parm_cfg.qpg_port_sh = style_alg_reg.s.qpg_port_sh;
	style_cfg->parm_cfg.qpg_port_msb = style_alg_reg.s.qpg_port_msb;
	style_cfg->parm_cfg.wqe_vs = style_alg_reg.s.wqe_vs;

	style_cfg->parm_cfg.pkt_lend = style_buf_reg.s.pkt_lend;
	style_cfg->parm_cfg.wqe_hsz = style_buf_reg.s.wqe_hsz;
	style_cfg->parm_cfg.wqe_skip = style_buf_reg.s.wqe_skip * 128;
	style_cfg->parm_cfg.first_skip = style_buf_reg.s.first_skip * 8;
	style_cfg->parm_cfg.later_skip = style_buf_reg.s.later_skip * 8;
	style_cfg->parm_cfg.cache_mode = style_buf_reg.s.opc_mode;
	style_cfg->parm_cfg.mbuff_size = style_buf_reg.s.mb_size * 8;
	style_cfg->parm_cfg.dis_wq_dat = style_buf_reg.s.dis_wq_dat;

	cvmx_pki_read_tag_config(node, style, cluster_mask, &style_cfg->tag_cfg);
}


/**
 * This function writes/configures parameters associated with style in hardware.
 * @param node	              node number.
 * @param style		      style to configure.
 * @param cluster_mask	      Mask of clusters to configure the style for.
 * @param style_cfg	      pointer to style config struct.
 */
void cvmx_pki_write_style_config(int node, uint64_t style, uint64_t cluster_mask,
			    struct cvmx_pki_style_config *style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t style_cfg_reg;
	cvmx_pki_clx_stylex_cfg2_t style_cfg2_reg;
	cvmx_pki_clx_stylex_alg_t style_alg_reg;
	cvmx_pki_stylex_buf_t     style_buf_reg;
	int cluster = 0;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			style_cfg_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
			style_cfg_reg.s.ip6_udp_opt = style_cfg->parm_cfg.ip6_udp_opt;
			style_cfg_reg.s.lenerr_en = style_cfg->parm_cfg.lenerr_en;
			style_cfg_reg.s.lenerr_eqpad = style_cfg->parm_cfg.lenerr_eqpad;
			style_cfg_reg.s.maxerr_en = style_cfg->parm_cfg.maxerr_en;
			style_cfg_reg.s.minerr_en = style_cfg->parm_cfg.minerr_en;
			style_cfg_reg.s.fcs_chk = style_cfg->parm_cfg.fcs_chk;
			style_cfg_reg.s.fcs_strip = style_cfg->parm_cfg.fcs_strip;
			style_cfg_reg.s.minmax_sel = style_cfg->parm_cfg.minmax_sel;
			style_cfg_reg.s.qpg_base = style_cfg->parm_cfg.qpg_base;
			style_cfg_reg.s.qpg_dis_padd = style_cfg->parm_cfg.qpg_dis_padd;
			style_cfg_reg.s.qpg_dis_aura = style_cfg->parm_cfg.qpg_dis_aura;
			style_cfg_reg.s.qpg_dis_grp = style_cfg->parm_cfg.qpg_dis_grp;
			style_cfg_reg.s.qpg_dis_grptag = style_cfg->parm_cfg.qpg_dis_grptag;
			style_cfg_reg.s.rawdrp = style_cfg->parm_cfg.rawdrp;
			style_cfg_reg.s.drop = style_cfg->parm_cfg.force_drop;
			style_cfg_reg.s.nodrop = style_cfg->parm_cfg.nodrop;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg_reg.u64);

			style_cfg2_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			style_cfg2_reg.s.len_lg = style_cfg->parm_cfg.len_lg;
			style_cfg2_reg.s.len_lf = style_cfg->parm_cfg.len_lf;
			style_cfg2_reg.s.len_le = style_cfg->parm_cfg.len_le;
			style_cfg2_reg.s.len_ld = style_cfg->parm_cfg.len_ld;
			style_cfg2_reg.s.len_lc = style_cfg->parm_cfg.len_lc;
			style_cfg2_reg.s.len_lb = style_cfg->parm_cfg.len_lb;
			style_cfg2_reg.s.csum_lg = style_cfg->parm_cfg.csum_lg;
			style_cfg2_reg.s.csum_lf = style_cfg->parm_cfg.csum_lf;
			style_cfg2_reg.s.csum_le = style_cfg->parm_cfg.csum_le;
			style_cfg2_reg.s.csum_ld = style_cfg->parm_cfg.csum_ld;
			style_cfg2_reg.s.csum_lc = style_cfg->parm_cfg.csum_lc;
			style_cfg2_reg.s.csum_lb = style_cfg->parm_cfg.csum_lb;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster), style_cfg2_reg.u64);

			style_alg_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg_reg.s.qpg_qos = style_cfg->parm_cfg.qpg_qos;
			style_alg_reg.s.tt = style_cfg->parm_cfg.tag_type;
			style_alg_reg.s.apad_nip = style_cfg->parm_cfg.apad_nip;
			style_alg_reg.s.qpg_port_sh = style_cfg->parm_cfg.qpg_port_sh;
			style_alg_reg.s.qpg_port_msb = style_cfg->parm_cfg.qpg_port_msb;
			style_alg_reg.s.wqe_vs = style_cfg->parm_cfg.wqe_vs;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster), style_alg_reg.u64);
		}
		cluster++;
	}
	style_buf_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf_reg.s.pkt_lend = style_cfg->parm_cfg.pkt_lend;
	style_buf_reg.s.wqe_hsz = style_cfg->parm_cfg.wqe_hsz;
	style_buf_reg.s.wqe_skip = (style_cfg->parm_cfg.wqe_skip)/128;
	style_buf_reg.s.first_skip = (style_cfg->parm_cfg.first_skip)/8;
	style_buf_reg.s.later_skip = style_cfg->parm_cfg.later_skip/8;
	style_buf_reg.s.opc_mode = style_cfg->parm_cfg.cache_mode;
	style_buf_reg.s.mb_size = (style_cfg->parm_cfg.mbuff_size)/8;
	style_buf_reg.s.dis_wq_dat = style_cfg->parm_cfg.dis_wq_dat;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf_reg.u64);

	cvmx_pki_write_tag_config(node, style, cluster_mask, &style_cfg->tag_cfg);
}

/**
 * This function reads qpg entry at specified offset from qpg table
 *
 * @param node		node number
 * @param offset	offset in qpg table to read from.
 * @param qpg_cfg	pointer to structure containing qpg values
 */
int cvmx_pki_read_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;
	if (offset >= CVMX_PKI_NUM_QPG_ENTRY) {
		cvmx_dprintf("ERROR: qpg offset %d is >= 2048\n", offset);
		return -1;
	}
	qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(offset));
	qpg_cfg->aura_num = qpg_tbl.s.laura;
	qpg_cfg->port_add = qpg_tbl.s.padd;
	qpg_cfg->grp_ok = qpg_tbl.s.grp_ok;
	qpg_cfg->grp_bad = qpg_tbl.s.grp_bad;
	qpg_cfg->grptag_ok = qpg_tbl.s.grptag_ok;
	qpg_cfg->grptag_bad = qpg_tbl.s.grptag_bad;
	return 0;
}

/**
 * This function writes qpg entry at specified offset in qpg table
 *
 * @param node		node number
 * @param offset	offset in qpg table to write to.
 * @param qpg_cfg	pointer to stricture containing qpg values
 */
void cvmx_pki_write_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;
	qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(offset));
	qpg_tbl.s.padd = qpg_cfg->port_add;
	qpg_tbl.s.laura = qpg_cfg->aura_num;
	qpg_tbl.s.grp_ok = qpg_cfg->grp_ok;
	qpg_tbl.s.grp_bad = qpg_cfg->grp_bad;
	qpg_tbl.s.grptag_ok = qpg_cfg->grptag_ok;
	qpg_tbl.s.grptag_bad = qpg_cfg->grptag_bad;
	cvmx_write_csr_node(node, CVMX_PKI_QPG_TBLX(offset), qpg_tbl.u64);
}


/**
 * This function writes pcam entry at given offset in pcam table in hardware
 *
 * @param node		node number.
 * @param index		offset in pcam table.
 * @param cluster_mask	Mask of clusters in which to write pcam entry.
 * @param input 	input keys to pcam match passed as struct.
 * @param action	pcam match action passed as struct
 *
 */
int cvmx_pki_pcam_write_entry(int node, int index, uint64_t cluster_mask,
	struct cvmx_pki_pcam_input input, struct cvmx_pki_pcam_action action)
{
	int bank;
	int cluster = 0;
	cvmx_pki_clx_pcamx_termx_t	pcam_term;
	cvmx_pki_clx_pcamx_matchx_t	pcam_match;
	cvmx_pki_clx_pcamx_actionx_t	pcam_action;

	if (index >= CVMX_PKI_TOTAL_PCAM_ENTRY) {
		cvmx_dprintf("\nERROR: Invalid pcam entry %d", index);
		return -1;
	}
	bank = (int)(input.field & 0x01);
	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			pcam_term.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			pcam_term.s.valid = 0;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index), pcam_term.u64);
			pcam_match.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index));
			pcam_match.s.data1 = input.data & input.data_mask;
			pcam_match.s.data0 = (~input.data) & input.data_mask;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index), pcam_match.u64);
			pcam_action.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index));
			pcam_action.s.pmc = action.parse_mode_chg;
			pcam_action.s.style_add = action.style_add;
			pcam_action.s.pf = action.parse_flag_set;
			pcam_action.s.setty = action.layer_type_set;
			pcam_action.s.advance = action.pointer_advance;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index), pcam_action.u64);
			pcam_term.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			pcam_term.s.term1 = input.field & input.field_mask;
			pcam_term.s.term0 = (~input.field) & input.field_mask;
			pcam_term.s.style1 = input.style & input.style_mask;
			pcam_term.s.style0 = (~input.style) & input.style_mask;
			pcam_term.s.valid = 1;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index), pcam_term.u64);
		}
		cluster++;
	}
	return 0;
}

/**
 * Enables/Disabled QoS (RED Drop, Tail Drop & backpressure) for the
 * PKI aura.
 * @param node		node number
 * @param aura		to enable/disable QoS on.
 * @param ena_red	Enable/Disable RED drop between pass and drop level
 *			1-enable 0-disable
 * @param ena_drop	Enable/disable tail drop when max drop level exceeds
 *			1-enable 0-disable
 * @param ena_red	Enable/Disable asserting backpressure on bpid when
 *			max DROP level exceeds.
 *			1-enable 0-disable
 */
int cvmx_pki_enable_aura_qos(int node, int aura, bool ena_red,
	bool ena_drop, bool ena_bp)
{
	cvmx_pki_aurax_cfg_t pki_aura_cfg;

	if (aura >= CVMX_PKI_NUM_AURA) {
		cvmx_dprintf("ERROR: PKI config aura_qos aura = %d", aura);
		return -1;
	}
	pki_aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
	pki_aura_cfg.s.ena_red = ena_red;
	pki_aura_cfg.s.ena_drop = ena_drop;
	pki_aura_cfg.s.ena_bp = ena_bp;
	cvmx_write_csr_node(node, CVMX_PKI_AURAX_CFG(aura), pki_aura_cfg.u64);
	return 0;
}

/**
 * Configures the bpid on which, specified aura will
 * assert backpressure.
 * Each bpid receives backpressure from auras.
 * Multiple auras can backpressure single bpid.
 * @param node   node number
 * @param aura   number which will assert backpressure on that bpid.
 * @param bpid   to assert backpressure on.
 */
int cvmx_pki_write_aura_bpid(int node, int aura, int bpid)
{
	cvmx_pki_aurax_cfg_t pki_aura_cfg;

	if (aura >= CVMX_PKI_NUM_AURA || bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: PKI config aura_bp aura = %d bpid = %d",
			aura, bpid);
		return -1;
	}
	pki_aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
	pki_aura_cfg.s.bpid = bpid;
	cvmx_write_csr_node(node, CVMX_PKI_AURAX_CFG(aura), pki_aura_cfg.u64);
	return 0;
}

/**
 * Configures the channel which will receive backpressure
 * from the specified bpid.
 * Each channel listens for backpressure on a specific bpid.
 * Each bpid can backpressure multiple channels.
 * @param node	node number
 * @param bpid	bpid from which, channel will receive backpressure.
 * @param channel channel numner to receive backpressue.
 */
int cvmx_pki_write_channel_bpid(int node, int channel, int bpid)
{
	cvmx_pki_chanx_cfg_t pki_chan_cfg;

	if (channel >= CVMX_PKI_NUM_CHANNEL || bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: PKI config channel_bp channel = %d bpid = %d",
			channel, bpid);
		return -1;
	}

	pki_chan_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CHANX_CFG(channel));
	pki_chan_cfg.s.bpid = bpid;
	cvmx_write_csr_node(node, CVMX_PKI_CHANX_CFG(channel), pki_chan_cfg.u64);
	return 0;
}

/**
 * This function gives the initial style used by that pkind.
 * @param node	node number.
 * @param pkind	pkind number.
 */
int cvmx_pki_get_pkind_style(int node, int pkind)
{
	int cluster = 0;

	cvmx_pki_clx_pkindx_style_t pkind_style;

	pkind_style.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));

	return pkind_style.s.style;
}

/**
 * This function sets the wqe buffer mode. First packet data buffer can reside
 * either in same buffer as wqe OR it can go in separate buffer. If used the later mode,
 * make sure software allocate enough buffers to now have wqe separate from packet data.
 * @param node		node number.
 * @param style		style to configure.
 * @param pkt_outside_wqe.	0 = The packet link pointer will be at word [FIRST_SKIP]
 *				immediately followed by packet data, in the same buffer
 *				as the work queue entry.
 *				1 = The packet link pointer will be at word [FIRST_SKIP] in a new
 *				buffer separate from the work queue entry. Words following the
 *				WQE in the same cache line will be zeroed, other lines in the
 *				buffer will not be modified and will retain stale data (from the
 * 				buffer’s previous use). This setting may decrease the peak PKI
 *				performance by up to half on small packets.
 */
void cvmx_pki_set_wqe_mode(int node, uint64_t style, bool pkt_outside_wqe)
{
	cvmx_pki_stylex_buf_t style_buf_reg;

	style_buf_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf_reg.s.dis_wq_dat = pkt_outside_wqe;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf_reg.u64);
}

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to
 * be in little-endian. Does not change the WQE header format, which is
 * properly endian neutral.
 * @param node		node number.
 * @param style 	style to configure.
 */
void cvmx_pki_set_little_endian(int node, uint64_t style)
{
	cvmx_pki_stylex_buf_t style_buf_reg;

	style_buf_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf_reg.s.pkt_lend = 1;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf_reg.u64);
}

/**
 * Enables/Disables fcs check and fcs stripping on the pkind.
 * @param node		node number
 * @param pknd		pkind to apply settings on.
 * @param fcs_chk	enable/disable fcs check.
 *			1 -- enable fcs error check.
 *			0 -- disable fcs error check.
 * @param fcs_strip	Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes
 *			1 -- strip L2 FCS.
 *			0 -- Do not strip L2 FCS.
 */
void cvmx_pki_endis_fcs_check(int node, int pknd, bool fcs_chk, bool fcs_strip)
{
	int style;
	int cluster = 0;
	cvmx_pki_clx_pkindx_style_t pkind_style;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		pkind_style.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pkind_style.s.style;
		style_cfg.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.fcs_chk = fcs_chk;
		style_cfg.s.fcs_strip = fcs_strip;
		cvmx_write_csr_node(node,
			CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg.u64);
		cluster++;
	}
}

/**
 * Enables/Disables l2 length error check and max & min frame length checks
 * @param node		node number
 * @param pknd		pkind to disable error for.
 * @param l2len_err	L2 length error check enable.
 * @param maxframe_err	Max frame error check enable.
 * @param minframe_err	Min frame error check enable.
 *			1 -- Enabel err checks
 *			0 -- Disable error checks
 */
void cvmx_pki_endis_l2_errs(int node, int pknd, bool l2len_err,
			 bool maxframe_err, bool minframe_err)
{
	int style;
	int cluster = 0;
	cvmx_pki_clx_pkindx_style_t pkind_style;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		pkind_style.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pkind_style.s.style;
		style_cfg.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.lenerr_en = l2len_err;
		style_cfg.s.maxerr_en = maxframe_err;
		style_cfg.s.minerr_en = minframe_err;
		cvmx_write_csr_node(node,
			CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg.u64);
		cluster++;
	}
}

/**
 * Disables maximum & minimum frame length checks
 * @param node   node number
 * @param pknd	 pkind to disable error for.
 */
void cvmx_pki_dis_frame_len_chk(int node, int pknd)
{
	int style;
	int cluster = 0;
	cvmx_pki_clx_pkindx_style_t pkind_style;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		pkind_style.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pkind_style.s.style;
		style_cfg.u64 = cvmx_read_csr_node(node,
			CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.maxerr_en = 0;
		style_cfg.s.minerr_en = 0;
		cvmx_write_csr_node(node,
			CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg.u64);
		cluster++;
	}
}


/**
 * This function shows the qpg table entries,
 * read directly from hardware.
 * @param node	node number
 */
void cvmx_pki_show_qpg_entries(int node, uint16_t num_entry)
{
	int index;
	cvmx_pki_qpg_tblx_t qpg_tbl;

	if (num_entry > CVMX_PKI_NUM_QPG_ENTRY)
		num_entry = CVMX_PKI_NUM_QPG_ENTRY;
	for (index = 0; index < num_entry; index++) {
		qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(index));
		cvmx_dprintf("\n%d	", index);
		cvmx_dprintf("PADD %-16lu",
			(unsigned long)qpg_tbl.s.padd);
		cvmx_dprintf("GRP_OK %-16lu",
			(unsigned long)qpg_tbl.s.grp_ok);
		cvmx_dprintf("GRP_BAD %-16lu",
			(unsigned long)qpg_tbl.s.grp_bad);
		cvmx_dprintf("LAURA %-16lu",
			(unsigned long)qpg_tbl.s.laura);
	}
}

/**
 * This function shows the pcam table in raw format,
 * read directly from hardware.
 * @param node    node number
 */
void cvmx_pki_show_pcam_entries(int node)
{
	int cluster;
	int index;
	int bank;

	for (cluster = 0; cluster < 4; cluster++) {
		for (bank = 0; bank < 2; bank++) {
			cvmx_dprintf("\n--------------Cluster %1d Bank %1d-------------\n", cluster, bank);
			cvmx_dprintf("index         TERM                 DATA,                ACTION");
			for (index = 0; index < CVMX_PKI_NUM_PCAM_ENTRY; index++) {
				cvmx_dprintf("\n%d", index);
				cvmx_dprintf("             %-16lx",
				    (unsigned long)cvmx_read_csr_node(node,
					CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index)));
				cvmx_dprintf("     %-16lx",
				     (unsigned long)cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index)));
				cvmx_dprintf("     %-16lx",
				     (unsigned long)cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index)));
			}
		}
	}
}

/**
 * This function shows the valid entries in readable format,
 * read directly from hardware.
 * @param node    node number
 */
void cvmx_pki_show_valid_pcam_entries(int node)
{
	int cluster;
	int index;
	int bank;
	cvmx_pki_clx_pcamx_termx_t	pcam_term;
	cvmx_pki_clx_pcamx_matchx_t	pcam_match;
	cvmx_pki_clx_pcamx_actionx_t	pcam_action;

	for (cluster = 0; cluster < 4; cluster++) {
		for (bank = 0; bank < 2; bank++) {
			cvmx_dprintf("\n--------------Cluster %1d Bank %1d---------------------\n", cluster, bank);
			cvmx_dprintf("%-10s%-17s%-19s%-18s", "index",
				     "TERM1:TERM0", "Style1:Style0", "Data1:Data0");
			cvmx_dprintf("%-6s", "ACTION[pmc:style_add:pf:setty:advance]");
			for (index = 0; index < CVMX_PKI_NUM_PCAM_ENTRY; index++) {
				pcam_term.u64 = cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
				if (pcam_term.s.valid) {
					pcam_match.u64 = cvmx_read_csr_node(node,
							CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index));
					pcam_action.u64 = cvmx_read_csr_node(node,
							CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index));
					cvmx_dprintf("\n%-13d", index);
					cvmx_dprintf("%-2x:%x", pcam_term.s.term1, pcam_term.s.term0);
					cvmx_dprintf("     	      %-2x:%x", pcam_term.s.style1, pcam_term.s.style0);
					cvmx_dprintf("        %-8x:%x", pcam_match.s.data1, pcam_match.s.data0);
					cvmx_dprintf("            %-2x:%-2x       :%-1x :%2x   :%-2x",
						pcam_action.s.pmc, pcam_action.s.style_add, pcam_action.s.pf, pcam_action.s.setty, pcam_action.s.advance);

				}
			}
		}
	}
}

/**
 * This function shows the pkind attributes in readable format,
 * read directly from hardware.
 * @param node    node number
 */
void cvmx_pki_show_pkind_attributes(int node, int pkind)
{
	int cluster = 0;
	int index;
	cvmx_pki_pkindx_icgsel_t pkind_clsel;
	cvmx_pki_clx_pkindx_style_t pkind_cfg_style;
	cvmx_pki_icgx_cfg_t pki_cl_grp;
	cvmx_pki_clx_stylex_cfg_t style_cfg;
	cvmx_pki_clx_stylex_alg_t style_alg;

	if (pkind >= CVMX_PKI_NUM_PKIND) {
		cvmx_dprintf("ERROR: PKIND %d is beyond range\n", pkind);
		return;
	}
	cvmx_dprintf("Showing stats for pkind %d------------------\n", pkind);
	pkind_clsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	cvmx_dprintf("cluster group:	%d\n", pkind_clsel.s.icg);
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(pkind_clsel.s.icg));
	cvmx_dprintf("cluster mask of the group:	0x%x\n", pki_cl_grp.s.clusters);

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (pki_cl_grp.s.clusters & (0x01L << cluster)) {
			cvmx_dprintf("pkind %d config 0x%llx\n",
				pkind,
				(unsigned long long)cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster)));
			pkind_cfg_style.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			cvmx_dprintf("initial parse Mode: %d\n", pkind_cfg_style.s.pm);
			cvmx_dprintf("initial_style: %d\n", pkind_cfg_style.s.style);
			style_alg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(pkind_cfg_style.s.style, cluster));
			cvmx_dprintf("style_alg: 0x%llx\n", (unsigned long long)style_alg.u64);
			style_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(pkind_cfg_style.s.style, cluster));
			cvmx_dprintf("style_cfg: 0x%llx\n", (unsigned long long)style_cfg.u64);
			cvmx_dprintf("style_cfg2: 0x%llx\n",
				(unsigned long long)cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(pkind_cfg_style.s.style, cluster)));
			cvmx_dprintf("style_buf: 0x%llx\n",
				(unsigned long long)cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(pkind_cfg_style.s.style)));
			break;
		}
	}
	cvmx_dprintf("qpg base: %d\n", style_cfg.s.qpg_base);
	cvmx_dprintf("qpg qos: %d\n", style_alg.s.qpg_qos);
	for (index = 0; index < 8; index++) {
		cvmx_dprintf("qpg index %d: 0x%llx\n", (index+style_cfg.s.qpg_base),
			(unsigned long long)cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(style_cfg.s.qpg_base+index)));
	}
}



