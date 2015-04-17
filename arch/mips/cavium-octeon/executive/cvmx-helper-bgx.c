/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
 * Functions to configure the BGX MAC.
 *
 * <hr>$Id$<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-bgxx-defs.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#else
#include "cvmx.h"
#include "cvmx-helper.h"
#include "cvmx-helper-bgx.h"
#include "cvmx-helper-board.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-qlm.h"
#endif

int __cvmx_helper_bgx_enumerate(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	int qlm;
	enum cvmx_qlm_mode mode;

	/*
	 * Check the QLM is configured correctly for SGMII, verify the
	 * speed as well as the mode.
	 */
	qlm = cvmx_qlm_interface(xiface);
	if (qlm == -1)
		return 0;

	mode = cvmx_qlm_get_mode_cn78xx(node, qlm);
	if (mode == CVMX_QLM_MODE_SGMII) {
		return 4;
	} else if (mode == CVMX_QLM_MODE_XAUI
		   || mode == CVMX_QLM_MODE_XLAUI
		   || mode == CVMX_QLM_MODE_40G_KR4) {
		return 1;
	} else if (mode == CVMX_QLM_MODE_RXAUI) {
		return 2;
	} else if (mode == CVMX_QLM_MODE_XFI
		   || mode == CVMX_QLM_MODE_10G_KR) {
		return 4;
	} else
		return 0;
}

/**
 * @INTERNAL
 * Disable the BGX port
 *
 * @param xipd_port IPD port of the BGX interface to disable
 */
void cvmx_helper_bgx_disable(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_cmrx_config_t cmr_config;

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0) || index)
		cmr_config.s.enable = 0;
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
}



/**
 * @INTERNAL
 * Configure the bgx mac.
 *
 * @param xiface Interface to bring up
 * @param index  port on interface to bring up
 */
static void __cvmx_bgx_common_init(int xiface, int index)
{
	cvmx_bgxx_cmrx_config_t	cmr_config;
	cvmx_bgxx_cmr_rx_lmacs_t bgx_cmr_rx_lmacs;
	cvmx_bgxx_cmr_tx_lmacs_t bgx_cmr_tx_lmacs;
	cvmx_helper_interface_mode_t mode;
	int num_ports;
	int lmac_type = 0;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int lane_to_sds = 0;

	num_ports = cvmx_helper_ports_on_interface(xiface);
	mode = cvmx_helper_interface_get_mode(xiface);

	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		lmac_type = 0;
		lane_to_sds = 1;
		break;
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		lmac_type = 1;
		lane_to_sds = 0xe4;
		break;
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		lmac_type = 2;
		break;
	case CVMX_HELPER_INTERFACE_MODE_XFI:
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
		lmac_type = 3;
		lane_to_sds = 1;
		break;
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
		lmac_type = 4;
		lane_to_sds = 0xe4;
		break;
	default:
		break;
	}

	/* Set mode and lanes for all interface ports */
	cmr_config.u64 =
		cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
	cmr_config.s.enable = 0;
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cmr_config.s.lmac_type = lmac_type;
	cmr_config.s.lane_to_sds = ((lane_to_sds == 1) ? index
				: ((lane_to_sds == 0)
				? (index ? 0xe : 4) : lane_to_sds));
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);

	if (index == 0) {
		bgx_cmr_rx_lmacs.u64 = 0;
		bgx_cmr_rx_lmacs.s.lmacs = num_ports;
		cvmx_write_csr_node(node, CVMX_BGXX_CMR_RX_LMACS(interface), bgx_cmr_rx_lmacs.u64);

		bgx_cmr_tx_lmacs.u64 = 0;
		bgx_cmr_tx_lmacs.s.lmacs = num_ports;
		cvmx_write_csr_node(node, CVMX_BGXX_CMR_TX_LMACS(interface), bgx_cmr_tx_lmacs.u64);
	}
}

static void __cvmx_bgx_common_init_pknd(int xiface, int index)
{
	int num_ports;
	int num_chl = 16; /*modify it to 64 for xlaui and xaui*/
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int pknd;
	cvmx_bgxx_cmrx_rx_bp_on_t bgx_rx_bp_on;
	cvmx_bgxx_cmrx_rx_id_map_t cmr_rx_id_map;
	cvmx_bgxx_cmr_chan_msk_and_t chan_msk_and;
	cvmx_bgxx_cmr_chan_msk_or_t chan_msk_or;

	num_ports = cvmx_helper_ports_on_interface(xiface);
	/* Modify bp_on mark, depending on number of LMACS on that interface
	and write it for every port */
	bgx_rx_bp_on.u64 = 0;
	bgx_rx_bp_on.s.mark = (CVMX_BGX_RX_FIFO_SIZE / (num_ports * 4 * 16));

	/* Setup pkind */
	pknd = cvmx_helper_get_pknd(xiface, index);
	cmr_rx_id_map.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ID_MAP(index, interface));
	cmr_rx_id_map.s.pknd = pknd;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_ID_MAP(index, interface),
			    cmr_rx_id_map.u64);
	/* Set backpressure channel mask AND/OR registers */
	chan_msk_and.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_AND(interface));
	chan_msk_or.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_OR(interface));
	chan_msk_and.s.msk_and |= ((1 << num_chl) - 1) << (16 * index);
	chan_msk_or.s.msk_or |= ((1 << num_chl) - 1) << (16 * index);
	cvmx_write_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_AND(interface), chan_msk_and.u64);
	cvmx_write_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_OR(interface), chan_msk_or.u64);
	/* set rx back pressure (bp_on) on value */
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_BP_ON(index, interface), bgx_rx_bp_on.u64);
}

/**
 * @INTERNAL
 * Probe a SGMII interface and determine the number of ports
 * connected to it. The SGMII interface should still be down after
 * this call. This is used by interfaces using the bgx mac.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_bgx_probe(int xiface)
{
	int num_ports = cvmx_helper_ports_on_interface(xiface);
	int index;

	for (index = 0; index < num_ports; index++)
		__cvmx_bgx_common_init(xiface, index);
	return __cvmx_helper_bgx_enumerate(xiface);
}
EXPORT_SYMBOL(__cvmx_helper_bgx_probe);

/**
 * @INTERNAL
 * Perform initialization required only once for an SGMII port.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_one_time(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	const uint64_t clock_mhz = cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) / 1000000;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	cvmx_bgxx_gmp_pcs_linkx_timer_t gmp_timer;

	if (!cvmx_helper_is_port_valid(interface, index))
		return 0;

	/*
	 * Write PCS*_LINK*_TIMER_COUNT_REG[COUNT] with the
	 * appropriate value. 1000BASE-X specifies a 10ms
	 * interval. SGMII specifies a 1.6ms interval.
	 */
	gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
	/* Adjust the MAC mode if requested by device tree */
	gmp_misc_ctl.s.mac_phy =
		cvmx_helper_get_mac_phy_mode(xiface, index);
	gmp_misc_ctl.s.mode =
		cvmx_helper_get_1000x_mode(xiface, index);
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface), gmp_misc_ctl.u64);

	gmp_timer.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, interface));
	if (gmp_misc_ctl.s.mode)
		/* 1000BASE-X */
		gmp_timer.s.count = (10000ull * clock_mhz) >> 10;
	else
		/* SGMII */
		gmp_timer.s.count = (1600ull * clock_mhz) >> 10;

	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, interface), gmp_timer.u64);

	/*
	 * Write the advertisement register to be used as the
	 * tx_Config_Reg<D15:D0> of the autonegotiation.  In
	 * 1000BASE-X mode, tx_Config_Reg<D15:D0> is PCS*_AN*_ADV_REG.
	 * In SGMII PHY mode, tx_Config_Reg<D15:D0> is
	 * PCS*_SGM*_AN_ADV_REG.  In SGMII MAC mode,
	 * tx_Config_Reg<D15:D0> is the fixed value 0x4001, so this
	 * step can be skipped.
	 */
	if (gmp_misc_ctl.s.mode) {
		/* 1000BASE-X */
		cvmx_bgxx_gmp_pcs_anx_adv_t gmp_an_adv;
		gmp_an_adv.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_ANX_ADV(index, interface));
		gmp_an_adv.s.rem_flt = 0;
		gmp_an_adv.s.pause = 3;
		gmp_an_adv.s.hfd = 1;
		gmp_an_adv.s.fd = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_ANX_ADV(index, interface), gmp_an_adv.u64);
	} else {
		if (gmp_misc_ctl.s.mac_phy) {
			/* PHY Mode */
			cvmx_bgxx_gmp_pcs_sgmx_an_adv_t gmp_sgmx_an_adv;
			gmp_sgmx_an_adv.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(index, interface));
			gmp_sgmx_an_adv.s.dup = 1;
			gmp_sgmx_an_adv.s.speed = 2;
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(index, interface),
				       gmp_sgmx_an_adv.u64);
		} else {
			/* MAC Mode - Nothing to do */
		}
	}
	return 0;
}

/**
 * @INTERNAL
 * Bring up the SGMII interface to be ready for packet I/O but
 * leave I/O disabled using the GMX override. This function
 * follows the bringup documented in 10.6.3 of the manual.
 *
 * @param xiface Interface to bringup
 * @param num_ports Number of ports on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init(int xiface, int num_ports)
{
	int index;
	int do_link_set = 1;

	for (index = 0; index < num_ports; index++) {
		int xipd_port = cvmx_helper_get_ipd_port(xiface, index);

		__cvmx_helper_bgx_port_init(xipd_port, 0);

		if (!cvmx_helper_is_port_valid(xiface, index))
			continue;


#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		/*
		 * Linux kernel driver will call ....link_set with the
		 * proper link state. In the simulator there is no
		 * link state polling and hence it is set from
		 * here.
		 */
		if (!(cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM))
			do_link_set = 0;
#endif
		if (do_link_set)
			__cvmx_helper_bgx_sgmii_link_set(xipd_port,
					__cvmx_helper_bgx_sgmii_link_get(xipd_port));
	}

	return 0;
}

/**
 * @INTERNAL
 * Bringup and enable a SGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using
 * the bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_enable(int xiface)
{
	int num_ports;

	num_ports = cvmx_helper_ports_on_interface(xiface);
	__cvmx_helper_bgx_sgmii_hardware_init(xiface, num_ports);

	return 0;
}

/**
 * @INTERNAL
 * Initialize the SERTES link for the first time or after a loss
 * of link.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_link(int xiface, int index)
{
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	int phy_mode, mode_1000x;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	gmp_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
	/* Take PCS through a reset sequence */
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		gmp_control.s.reset = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface),
		       					     gmp_control.u64);

		/* Wait until GMP_PCS_MRX_CONTROL[reset] comes out of reset */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface),
				cvmx_bgxx_gmp_pcs_mrx_control_t, reset, ==, 0, 10000)) {
			cvmx_dprintf("SGMII%d: Timeout waiting for port %d to finish reset\n", interface, index);
			return -1;
		}
	}

	/* Write GMP_PCS_MR*_CONTROL[RST_AN]=1 to ensure a fresh SGMII
	   negotiation starts. */
	gmp_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
	gmp_control.s.rst_an = 1;
	gmp_control.s.an_en = 1;
	gmp_control.s.pwr_dn = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface),
		       gmp_control.u64);


	phy_mode = cvmx_helper_get_mac_phy_mode(xiface, index);
	mode_1000x = cvmx_helper_get_1000x_mode(xiface, index);

	gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
	gmp_misc_ctl.s.mac_phy = phy_mode;
	gmp_misc_ctl.s.mode = mode_1000x;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface), gmp_misc_ctl.u64);

	if (phy_mode)
		/* In PHY mode we can't query the link status so we just
		   assume that the link is up */
		return 0;

	/* Wait for GMP_PCS_MRX_CONTROL[an_cpt] to be set, indicating that
	   SGMII autonegotiation is complete. In MAC mode this isn't an
	   ethernet link, but a link between OCTEON and PHY. */

	if ((cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) &&
	     CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_PCS_MRX_STATUS(index, interface),
				   cvmx_bgxx_gmp_pcs_mrx_status_t, an_cpt,
				   ==, 1, 10000)) {
		cvmx_dprintf("SGMII%d: Port %d link timeout\n", interface, index);
		return -1;
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure an SGMII link to the specified speed after the SERTES
 * link is up.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 * @param link_info Link state to configure
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_link_speed(int xiface,
							    int index,
							    cvmx_helper_link_info_t link_info)
{
	int is_enabled = 1;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_miscx_ctl;
	cvmx_bgxx_gmp_gmi_prtx_cfg_t gmp_prtx_cfg;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	/* Errata bgx-22429*/
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || index) {
		/* Disable GMX before we make any changes. Remember the enable state */
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
		is_enabled = cmr_config.s.enable;
		cmr_config.s.enable = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
	}

	/* Wait for GMX to be idle */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, interface),
				  cvmx_bgxx_gmp_gmi_prtx_cfg_t, rx_idle, ==, 1, 10000) ||
	    CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, interface),
				  cvmx_bgxx_gmp_gmi_prtx_cfg_t, tx_idle, ==, 1, 10000)) {
		cvmx_dprintf("SGMII%d:%d: Timeout waiting for port %d to be idle\n",
			     node, interface, index);
		return -1;
	}

	/* Read GMX CFG again to make sure the disable completed */
	gmp_prtx_cfg.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, interface));

	/*
	 * Get the misc control for PCS. We will need to set the
	 * duplication amount.
	 */
	gmp_miscx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));

	/*
	 * Use GMXENO to force the link down if the status we get says
	 * it should be down.
	 */
	gmp_miscx_ctl.s.gmxeno = !link_info.s.link_up;

	/* Only change the duplex setting if the link is up */
	if (link_info.s.link_up)
		gmp_prtx_cfg.s.duplex = link_info.s.full_duplex;

	/* Do speed based setting for GMX */
	switch (link_info.s.speed) {
	case 10:
		gmp_prtx_cfg.s.speed = 0;
		gmp_prtx_cfg.s.speed_msb = 1;
		gmp_prtx_cfg.s.slottime = 0;
		/* Setting from GMX-603 */
		gmp_miscx_ctl.s.samp_pt = 25;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SLOT(index, interface), 64);
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, interface), 0);
		break;
	case 100:
		gmp_prtx_cfg.s.speed = 0;
		gmp_prtx_cfg.s.speed_msb = 0;
		gmp_prtx_cfg.s.slottime = 0;
		gmp_miscx_ctl.s.samp_pt = 0x5;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SLOT(index, interface), 64);
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, interface), 0);
		break;
	case 1000:
		gmp_prtx_cfg.s.speed = 1;
		gmp_prtx_cfg.s.speed_msb = 0;
		gmp_prtx_cfg.s.slottime = 1;
		gmp_miscx_ctl.s.samp_pt = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SLOT(index, interface), 512);
		if (gmp_prtx_cfg.s.duplex)
			/* full duplex */
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, interface), 0);
		else
			/* half duplex */
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, interface), 8192);
		break;
	default:
		break;
	}

	/* Write the new misc control for PCS */
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface),
		       gmp_miscx_ctl.u64);

	/* Write the new GMX settings with the port still disabled */
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, interface), gmp_prtx_cfg.u64);

	/* Read GMX CFG again to make sure the config completed */
	cvmx_read_csr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, interface));

	/* Restore the enabled/disabled state */
	/* bgx-22429 */
	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || index)
		cmr_config.s.enable = is_enabled;
#ifndef CVMX_BUILD_FOR_UBOOT
	cmr_config.s.data_pkt_tx_en = 1;
	cmr_config.s.data_pkt_rx_en = 1;
#endif
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);

	return 0;
}

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t __cvmx_helper_bgx_sgmii_link_get(int xipd_port)
{
	cvmx_helper_link_info_t result;
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	int speed = 1000;

	result.u64 = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return result;

	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM) {
		/* The simulator gives you a simulated 1Gbps full duplex link */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = speed;
		return result;
	}

	speed = cvmx_qlm_get_gbaud_mhz(0) * 8 / 10;

	gmp_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
	if (gmp_control.s.loopbck1) {
		/* Force 1Gbps full duplex link for internal loopback */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = speed;
		return result;
	}

	gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
	if (gmp_misc_ctl.s.mac_phy) {
		/* PHY Mode */
		/* Note that this also works for 1000base-X mode */

		result.s.speed = speed;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		return result;
	} else {
		/* MAC Mode */
		result = __cvmx_helper_board_link_get(xipd_port);
	}
	return result;
}

int cvmx_helper_bgx_errata_22429(int xipd_port, int link_up)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	int num_ports = cvmx_helper_ports_on_interface(xiface);

	/* Errata does not apply for only 1 LMAC */
	if (num_ports == 1 || !OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		return 0;

	/* Errata BGX-22429 */
	if (link_up) {
		if (!index) /*does not apply for port 0*/
			return 0;
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(0, interface));
		if (!cmr_config.s.enable) {
			cmr_config.s.enable = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(0, interface), cmr_config.u64);
		}
	} else {
		if (index) /*does not apply to port 1-3 */
			return 0;
		while (--num_ports) {
			cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(num_ports, interface));
			if (cmr_config.s.enable)
				return -1;
		}
	}
	return 0;
}

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_link_set(int xipd_port,
				 cvmx_helper_link_info_t link_info)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	cvmx_helper_bgx_errata_22429(xipd_port, link_info.s.link_up);

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
	if (link_info.s.link_up) {
		cmr_config.s.enable = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
	} else {
		cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
		cmr_config.s.data_pkt_tx_en = 0;
		cmr_config.s.data_pkt_rx_en = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
		gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));

		/* Disable autonegotiation only when MAC mode. */
		if (gmp_misc_ctl.s.mac_phy == 0) {
			cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;

			gmp_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
			gmp_control.s.an_en = 0;
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface), gmp_control.u64);
			cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
		}
		/*
		 * Use GMXENO to force the link down it will get
		 * reenabled later...
		 */
		gmp_misc_ctl.s.gmxeno = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface),
			       gmp_misc_ctl.u64);
		cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
		return 0;
	}
	return __cvmx_helper_bgx_sgmii_hardware_init_link_speed(xiface, index, link_info);
}


/**
 * @INTERNAL
 * Bringup XAUI interface. After this call packet I/O should be
 * fully functional.
 *
 * @param index port on interface to bring up
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_xaui_init(int index, int xiface)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_spux_control1_t spu_control1;
	cvmx_bgxx_spux_an_control_t spu_an_control;
	cvmx_bgxx_spux_an_adv_t spu_an_adv;
	cvmx_bgxx_spux_fec_control_t spu_fec_control;
	cvmx_bgxx_spu_dbg_control_t spu_dbg_control;
	cvmx_bgxx_smux_tx_append_t  smu_tx_append;
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_helper_interface_mode_t mode;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int use_auto_neg = 0;
	int use_training = 0;
	int xipd_port = cvmx_helper_get_ipd_port(xiface, index);

	mode = cvmx_helper_interface_get_mode(xiface);

	if (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR
	    || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) {
		use_training = 1;
		use_auto_neg = 0;
	}

	/* NOTE: This code was moved first, out of order compared to the HRM
	   because the RESET causes all SPU registers to loose their value */
	/* 4. Next, bring up the SMU/SPU and the BGX reconciliation layer logic: */
	/* 4a. Take SMU/SPU through a reset sequence. Write
	   BGX(0..5)_SPU(0..3)_CONTROL1[RESET] = 1. Read
	   BGX(0..5)_SPU(0..3)_CONTROL1[RESET] until it changes value to 0. Keep
	   BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1 to disable
	   reception. */
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface));
		spu_control1.s.reset = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);

		/* 1. Wait for PCS to come out of reset */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_CONTROL1(index, interface),
				cvmx_bgxx_spux_control1_t, reset, ==, 0, 10000)) {
			cvmx_dprintf("BGX%d:%d: SPU stuck in reset\n", node, interface);
			return -1;
		}

		/* 2. Write BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] to 0,
		      BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 1 and
		      BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1. */
		if (!cvmx_helper_bgx_errata_22429(xipd_port, 0)) {
			cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
			cmr_config.s.enable = 0;
			cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
		}
		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface));
		spu_control1.s.lo_pwr = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);

		spu_misc_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface));
		spu_misc_control.s.rx_packet_dis = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface), spu_misc_control.u64);

		/* 3. At this point, it may be appropriate to disable all BGX and SMU/SPU
		    interrupts, as a number of them will occur during bring-up of the Link.
		    - zero BGX(0..5)_SMU(0..3)_RX_INT
		    - zero BGX(0..5)_SMU(0..3)_TX_INT
		    - zero BGX(0..5)_SPU(0..3)_INT */
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_RX_INT(index, interface),
			cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_INT(index, interface)));
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_INT(index, interface),
			cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_INT(index, interface)));
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, interface),
			cvmx_read_csr_node(node, CVMX_BGXX_SPUX_INT(index, interface)));

		/* 4. Configure the BGX LMAC. */
		/* 4a. Configure the LMAC type (40GBASE-R/10GBASE-R/RXAUI/XAUI) and
		     SerDes selection in the BGX(0..5)_CMR(0..3)_CONFIG register, but keep
		     the ENABLE, DATA_PKT_TX_EN and DATA_PKT_RX_EN bits clear. */
		/* Already done in bgx_setup_one_time */

		/* 4b. Write BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 1 and
		     BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1. */
		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface));
		spu_control1.s.lo_pwr = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);

		spu_misc_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface));
		spu_misc_control.s.rx_packet_dis = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface), spu_misc_control.u64);

		/* 4b. Initialize the selected SerDes lane(s) in the QLM. See Section
		      28.1.2.2 in the GSER chapter. */
		/* Already done in QLM setup */

		/* 4c. For 10GBASE-KR or 40GBASE-KR, enable link training by writing
		     BGX(0..5)_SPU(0..3)_BR_PMD_CONTROL[TRAIN_EN] = 1. */
		if (use_training) {
			cvmx_bgxx_spux_br_pmd_control_t spu_br_pmd_control;
			spu_br_pmd_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface));
			spu_br_pmd_control.s.train_en = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface), spu_br_pmd_control.u64);

		}
	} else { /* enable for simulator */
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
		cmr_config.s.enable = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
	}

	/* 4d. Program all other relevant BGX configuration while
	       BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] = 0. This includes all things
	       described in this chapter. */
	/* Always add FCS to PAUSE frames */
	smu_tx_append.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_APPEND(index, interface));
	smu_tx_append.s.fcs_d = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_APPEND(index, interface), smu_tx_append.u64);

	/* 4e. If Forward Error Correction is desired for 10GBASE-R or 40GBASE-R,
	       enable it by writing BGX(0..5)_SPU(0..3)_FEC_CONTROL[FEC_EN] = 1. */
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		/* FEC is optional for 10GBASE-KR, 40GBASE-KR4, and XLAUI. We're going
		to disable it by default */
		spu_fec_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface));
		spu_fec_control.s.fec_en = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface), spu_fec_control.u64);

		/* 4f. If Auto-Negotiation is desired, configure and enable
		      Auto-Negotiation as described in Section 33.6.2. */
		spu_an_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, interface));
		spu_an_control.s.an_en = use_auto_neg;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, interface), spu_an_control.u64);

		spu_fec_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_FEC_CONTROL(index, interface));
		spu_an_adv.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_ADV(index, interface));
		spu_an_adv.s.fec_req = spu_fec_control.s.fec_en;
		spu_an_adv.s.fec_able = 1;
		spu_an_adv.s.a100g_cr10 = 0;
		spu_an_adv.s.a40g_cr4 = 0;
		spu_an_adv.s.a40g_kr4 = (mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) ;
		spu_an_adv.s.a10g_kr = (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR) ;
		spu_an_adv.s.a10g_kx4 = 0;
		spu_an_adv.s.a1g_kx = 0;
		spu_an_adv.s.rf = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_ADV(index, interface), spu_an_adv.u64);

		/* 3. Set BGX(0..5)_SPU_DBG_CONTROL[AN_ARB_LINK_CHK_EN] = 1. */
		spu_dbg_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(interface));
		spu_dbg_control.s.an_arb_link_chk_en = use_auto_neg;
		cvmx_write_csr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(interface), spu_dbg_control.u64);

		/* 4. Execute the link bring-up sequence in Section 33.6.3. */

		/* 5. If the auto-negotiation protocol is successful,
		    BGX(0..5)_SPU(0..3)_AN_ADV[AN_COMPLETE] is set along with
		    BGX(0..5)_SPU(0..3)_INT[AN_COMPLETE] when the link is up. */

		/* 3h. Set BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] = 1 and
		    BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 0 to enable the LMAC. */
		cvmx_helper_bgx_errata_22429(xipd_port, 1);
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
		cmr_config.s.enable = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);

		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface));
		spu_control1.s.lo_pwr = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);
	}

	/* 4g. Set the polarity and lane swapping of the QLM SerDes. Refer to
	   Section 33.4.1, BGX(0..5)_SPU(0..3)_MISC_CONTROL[XOR_TXPLRT,XOR_RXPLRT]
	   and BGX(0..5)_SPU(0..3)_MISC_CONTROL[TXPLRT,RXPLRT]. */

	/* 4c. Write BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 0. */
	spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface));
	spu_control1.s.lo_pwr = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);

	/* 4d. Select Deficit Idle Count mode and unidirectional enable/disable
	   via BGX(0..5)_SMU(0..3)_TX_CTL[DIC_EN,UNI_EN]. */
	smu_tx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, interface));
	smu_tx_ctl.s.dic_en = 1;
	smu_tx_ctl.s.uni_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, interface), smu_tx_ctl.u64);

	return 0;
}

int __cvmx_helper_bgx_port_init(int xipd_port, int phy_pres)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_interface_get_mode(xiface);

	__cvmx_bgx_common_init_pknd(xiface, index);

	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII) {
		cvmx_bgxx_gmp_gmi_txx_thresh_t gmi_tx_thresh;
		cvmx_bgxx_gmp_gmi_txx_append_t gmp_txx_append;
		cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_t gmp_sgmii_ctl;

		/* Set TX Threshold */
		gmi_tx_thresh.u64 = 0;
		gmi_tx_thresh.s.cnt = 0x20;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_THRESH(index, interface),
				    gmi_tx_thresh.u64);
		__cvmx_helper_bgx_sgmii_hardware_init_one_time(xiface, index);
		gmp_txx_append.u64 = cvmx_read_csr_node(node,
					CVMX_BGXX_GMP_GMI_TXX_APPEND(index, interface));
		gmp_sgmii_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(index, interface));
		gmp_sgmii_ctl.s.align = gmp_txx_append.s.preamble ? 0 : 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(index, interface),
			    gmp_sgmii_ctl.u64);

	} else {
		int res;
		cvmx_bgxx_smux_tx_thresh_t smu_tx_thresh;

		res = __cvmx_helper_bgx_xaui_init(index, xiface);
		if (res == -1) {
			cvmx_dprintf("Failed to enable XAUI for %d:BGX(%d,%d)\n", node, interface, index);
			return res;
		}

		smu_tx_thresh.u64 = 0;
		/* Hopefully big enough to avoid underrun, but not too
		* big to adversly effect shaping.
		*/
		smu_tx_thresh.s.cnt = 0x100;
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_THRESH(index, interface),
				    smu_tx_thresh.u64);
		/* Set disparity for RXAUI interface as described in the
		Marvell RXAUI Interface specification. */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI && phy_pres) {
			cvmx_bgxx_spux_misc_control_t misc_control;
			misc_control.u64 = cvmx_read_csr_node(node,
					CVMX_BGXX_SPUX_MISC_CONTROL(index, interface));
			misc_control.s.intlv_rdisp = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface),
					    misc_control.u64);
		}
	}
	return 0;
}
EXPORT_SYMBOL(__cvmx_helper_bgx_port_init);


/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_sgmii_configure_loopback(int xipd_port, int enable_internal,
					   int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_mrx_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	gmp_mrx_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
	gmp_mrx_control.s.loopbck1 = enable_internal;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface), gmp_mrx_control.u64);

	gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
	gmp_misc_ctl.s.loopbck2 = enable_external;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface), gmp_misc_ctl.u64);

	__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);

	return 0;
}

static int __cvmx_helper_bgx_xaui_link_init(int index, int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_bgxx_spux_status2_t spu_status2;
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_helper_interface_mode_t mode;
	int use_training = 0;

	mode = cvmx_helper_interface_get_mode(xiface);

	if (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)
		use_training = 1;

	/* Disable packet reception */
	spu_misc_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface));
	spu_misc_control.s.rx_packet_dis = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface), spu_misc_control.u64);

	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		if (use_training) {
			spu_int.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_INT(index, interface));
			if (!spu_int.s.training_done) {
				cvmx_bgxx_spux_br_pmd_control_t pmd_control;
				/* Clear the training interrupts (W1C) */

				spu_int.u64 = 0;
				spu_int.s.training_failure = 1;
				spu_int.s.training_done = 1;
				cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, interface), spu_int.u64);

				/* Restart training */
				pmd_control.u64 = cvmx_read_csr_node(node,
							CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface));
				pmd_control.s.train_restart = 1;
				cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface), pmd_control.u64);

				/*cvmx_dprintf("Restarting link training\n"); */
				return -1;
			}
		}

		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_CONTROL1(index, interface),
					  cvmx_bgxx_spux_control1_t, reset, ==, 0, 10000)) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: PCS in reset", node, interface, index);
			return -1;
		}

		if (mode == CVMX_HELPER_INTERFACE_MODE_XFI
		    || mode == CVMX_HELPER_INTERFACE_MODE_XLAUI
		    || mode == CVMX_HELPER_INTERFACE_MODE_10G_KR
		    || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) {
			if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_BR_STATUS1(index, interface),
					  cvmx_bgxx_spux_br_status1_t, blk_lock, ==, 1, 10000)) {
				cvmx_dprintf("ERROR: %d:BGX%d:%d: BASE-R PCS block not locked\n", node, interface, index);
                		return -1;
			}
		} else {
			/* (5) Check to make sure that the link appears up and stable. */
			/* Wait for PCS to be aligned */
			if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_BX_STATUS(index, interface),
				  cvmx_bgxx_spux_bx_status_t, alignd, ==, 1, 10000)) {
				cvmx_dprintf("ERROR: %d:BGX%d:%d: PCS not aligned\n", node, interface, index);
				return -1;
			}
		}

		/* Clear rcvflt bit (latching high) and read it back */
		spu_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, interface));
		spu_status2.s.rcvflt = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, interface), spu_status2.u64);

		spu_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, interface));
		if (spu_status2.s.rcvflt) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: Receive fault, need to retry\n",
					node, interface, index);

			if (use_training) {
				cvmx_bgxx_spux_br_pmd_control_t pmd_control;
				spu_int.u64 = 0;
				spu_int.s.training_failure = 1;
				spu_int.s.training_done = 1;
				cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, interface), spu_int.u64);

				/* Restart training */
				pmd_control.u64 = cvmx_read_csr_node(node,
							CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface));
				pmd_control.s.train_restart = 1;
				cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface), pmd_control.u64);
			}
			/*cvmx_dprintf("training restarting\n"); */
			return -1;
		}

		/* Wait for MAC RX to be ready */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SMUX_RX_CTL(index, interface),
					  cvmx_bgxx_smux_rx_ctl_t, status, ==, 0, 10000)) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: RX not ready\n", node, interface, index);
			return -1;
		}

		/* Wait for BGX RX to be idle */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SMUX_CTRL(index, interface),
				  cvmx_bgxx_smux_ctrl_t, rx_idle, ==, 1, 10000)) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: RX not idle\n", node, interface, index);
			return -1;
		}

		/* Wait for GMX TX to be idle */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SMUX_CTRL(index, interface),
				  cvmx_bgxx_smux_ctrl_t, tx_idle, ==, 1, 10000)) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: TX not idle\n", node, interface, index);
			return -1;
		}

		/* rcvflt should still be 0 */
		spu_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, interface));
		if (spu_status2.s.rcvflt) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: Receive fault, need to retry\n", node, interface, index);
			return -1;
		}

		/* Receive link is latching low. Force it high and verify it */
		spu_status1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, interface));
		spu_status1.s.rcv_lnk = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, interface), spu_status1.u64);

		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_STATUS1(index, interface),
				cvmx_bgxx_spux_status1_t, rcv_lnk, ==, 1, 10000)) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: Receive link down\n", node, interface, index);
			return -1;
		}
	}

	/* (7) Enable packet transmit and receive */
	spu_misc_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface));
	spu_misc_control.s.rx_packet_dis = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, interface), spu_misc_control.u64);

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
	cmr_config.s.data_pkt_tx_en = 1;
	cmr_config.s.data_pkt_rx_en = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);

	return 0;
}

int __cvmx_helper_bgx_xaui_enable(int xiface)
{
	int index;
	int num_ports = cvmx_helper_ports_on_interface(xiface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_interface_get_mode(xiface);

	for (index = 0; index < num_ports; index++) {
		int res;
		int xipd_port = cvmx_helper_get_ipd_port(xiface, index);
		int phy_pres;

		/* Set disparity for RXAUI interface as described in the
		   Marvell RXAUI Interface specification. */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI &&
				  (cvmx_helper_get_port_phy_present(xiface, index)))
			phy_pres = 1;
		else
			phy_pres = 0;
		if (__cvmx_helper_bgx_port_init(xipd_port, phy_pres))
			return -1;

		res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
		if (res == -1) {
			cvmx_dprintf("Failed to get %d:BGX(%d,%d) link\n", node, interface, index);
			continue;
		}
	}
	return 0;
}

cvmx_helper_link_info_t __cvmx_helper_bgx_xaui_link_get(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_bgxx_smux_rx_ctl_t smu_rx_ctl;
	cvmx_helper_link_info_t result;

	result.u64 = 0;

	spu_status1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, interface));
	smu_tx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, interface));
	smu_rx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_CTL(index, interface));

	if ((smu_tx_ctl.s.ls == 0)
	    && (smu_rx_ctl.s.status == 0)
	    && (spu_status1.s.rcv_lnk)) {
		int lanes;
		int qlm = cvmx_qlm_interface(xiface);
		uint64_t speed;
		cvmx_helper_interface_mode_t mode;
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		speed = cvmx_qlm_get_gbaud_mhz(qlm);
		mode = cvmx_helper_interface_get_mode(xiface);
		lanes = 4 / cvmx_helper_ports_on_interface(xiface);

		switch(mode) {
		case CVMX_HELPER_INTERFACE_MODE_XFI:
		case CVMX_HELPER_INTERFACE_MODE_XLAUI:
		case CVMX_HELPER_INTERFACE_MODE_10G_KR:
		case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
			/* Using 64b66b symbol encoding */
			speed = (speed * 64 + 33) / 66;
			break;
		default:
			/* Using 8b10b symbol encoding */
			speed = (speed * 8 + 5) / 10;
			break;
		}
		speed *= lanes;
		result.s.speed = speed;
	} else {
		int res;
		res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
		if (res == -1) {
			cvmx_dprintf("Failed to get %d:BGX(%d,%d) link\n", node, interface, index);
			return result;
		}
	}

	return result;
}

int __cvmx_helper_bgx_xaui_link_set(int xipd_port, cvmx_helper_link_info_t link_info)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_bgxx_smux_rx_ctl_t smu_rx_ctl;
	cvmx_bgxx_spux_status1_t spu_status1;

	smu_tx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, interface));
	smu_rx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_CTL(index, interface));
	spu_status1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, interface));

	/* If the link shouldn't be up, then just return */
	if (!link_info.s.link_up)
		return 0;

	/* Do nothing if both RX and TX are happy */
	if ((smu_tx_ctl.s.ls == 0) && (smu_rx_ctl.s.status == 0) && spu_status1.s.rcv_lnk)
		return 0;

	/* Bring the link up */
	return __cvmx_helper_bgx_xaui_link_init(index, xiface);
}

int __cvmx_helper_bgx_xaui_configure_loopback(int xipd_port,
						     int enable_internal,
						     int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int interface = xi.interface;
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_spux_control1_t spu_control1;
	cvmx_bgxx_smux_ext_loopback_t smu_ext_loopback;

	/* Set the internal loop */
	spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface));
	spu_control1.s.loopbck = enable_internal;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, interface), spu_control1.u64);
	/* Set the external loop */
	smu_ext_loopback.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_EXT_LOOPBACK(index, interface));
	smu_ext_loopback.s.en = enable_external;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_EXT_LOOPBACK(index, interface), smu_ext_loopback.u64);

	return __cvmx_helper_bgx_xaui_link_init(index, xiface);
}

/**
 * @INTERNAL
 * Configure Priority-Based Flow Control (a.k.a. PFC/CBFC)
 * on a specific BGX interface/port.
 */
void __cvmx_helper_bgx_xaui_config_pfc(unsigned node,
		unsigned interface, unsigned port, bool pfc_enable)
{
	cvmx_bgxx_smux_cbfc_ctl_t cbfc_ctl;

	cbfc_ctl.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_SMUX_CBFC_CTL(port, interface)
		);

	/* Enable all PFC controls if requiested */
	cbfc_ctl.s.rx_en = pfc_enable;
	cbfc_ctl.s.tx_en = pfc_enable;
#if 0
	cbfc_ctl.s.bck_en = 1;
	cbfc_ctl.s.phys_en = 0xff;
	cbfc_ctl.s.logl_en = 0xff;
	cbfc_ctl.s.drp_en = pfc_enable;
#endif
#ifdef DEBUG
	printf("%s: CVMX_BGXX_SMUX_CBFC_CTL(%d,%d)=%#llx\n",
		__func__, port, interface, (unsigned long long)cbfc_ctl.u64);
#endif
	cvmx_write_csr_node(node,
		CVMX_BGXX_SMUX_CBFC_CTL(port, interface),
		cbfc_ctl.u64);
}


/**
 * This function control how the hardware handles incoming PAUSE
 * packets. The most common modes of operation:
 * ctl_bck = 1, ctl_drp = 1: hardware handles everything
 * ctl_bck = 0, ctl_drp = 0: software sees all PAUSE frames
 * ctl_bck = 0, ctl_drp = 1: all PAUSE frames are completely ignored
 * @param node		node number.
 * @param interface	interface number
 * @param port		port number
 * @param ctl_bck	1: Forward PAUSE information to TX block
 * @param ctl_drp	1: Drop control PAUSE frames.
 */
void cvmx_helper_bgx_rx_pause_ctl(unsigned node, unsigned interface,
			unsigned port, unsigned ctl_bck, unsigned ctl_drp)
{
	cvmx_bgxx_smux_rx_frm_ctl_t frm_ctl;

	frm_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_CTL(port, interface));
	frm_ctl.s.ctl_bck = ctl_bck;
	frm_ctl.s.ctl_drp = ctl_drp;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_RX_CTL(port, interface), frm_ctl.u64);
}

/**
 * This function configures the receive action taken for multicast, broadcast
 * and dmac filter match packets.
 * @param node		node number.
 * @param interface	interface number
 * @param port		port number
 * @param cam_accept	0: reject packets on dmac filter match
 *                      1: accept packet on dmac filter match
 * @param mcast_mode	0x0 = Force reject all multicast packets
 *                      0x1 = Force accept all multicast packets
 *                      0x2 = Use the address filter CAM
 * @param bcast_accept  0 = Reject all broadcast packets
 *                      1 = Accept all broadcast packets
 */
void cvmx_helper_bgx_rx_adr_ctl(unsigned node, unsigned interface, unsigned port,
                                 unsigned cam_accept, unsigned mcast_mode, unsigned bcast_accept)
{
        cvmx_bgxx_cmrx_rx_adr_ctl_t adr_ctl;

        adr_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(port, interface));
        adr_ctl.s.cam_accept = cam_accept;
        adr_ctl.s.mcst_mode = mcast_mode;
        adr_ctl.s.bcst_accept = bcast_accept;

        cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(port, interface), adr_ctl.u64);
}

/**
 * Function to control the generation of FCS, padding by the BGX
 *
 */
void cvmx_helper_bgx_tx_options(unsigned node,
	unsigned interface, unsigned index,
	bool fcs_enable, bool pad_enable)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_cmrx_config_t cmr_config0;
	cvmx_bgxx_gmp_gmi_txx_append_t gmp_txx_append;
	cvmx_bgxx_gmp_gmi_txx_min_pkt_t gmp_min_pkt;
	cvmx_bgxx_smux_tx_min_pkt_t smu_min_pkt;
	cvmx_bgxx_smux_tx_append_t  smu_tx_append;

	cmr_config0.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_CMRX_CONFIG(0, interface));
	cmr_config.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_CMRX_CONFIG(index, interface));

	/* Temp: initial mode setting is only applied to LMAC(0), use that */
	if (cmr_config0.s.lmac_type != cmr_config.s.lmac_type) {
		cvmx_dprintf("WARNING: %s: "
			"%d:BGX(%d).CMR(0).LMAC_TYPE != BGX(%d).CMR(%d).LMAC_TYPE\n",
			__func__, node, interface, interface, index);
		cmr_config.s.lmac_type = cmr_config0.s.lmac_type;
	}

	if (cmr_config.s.lmac_type == 0) {
		gmp_min_pkt.u64 = 0;
		/* per HRM Sec 34.3.4.4 */
		gmp_min_pkt.s.min_size = 59;
		cvmx_write_csr_node(node,
                        CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(index, interface),
			gmp_min_pkt.u64);
		gmp_txx_append.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_TXX_APPEND(index, interface));
		gmp_txx_append.s.fcs = fcs_enable;
		gmp_txx_append.s.pad = pad_enable;
		cvmx_write_csr_node(node,
			CVMX_BGXX_GMP_GMI_TXX_APPEND(index, interface),
			gmp_txx_append.u64);
	} else {
		smu_min_pkt.u64 = 0;
		/* HRM Sec 33.3.4.3 should read 64 */
		 smu_min_pkt.s.min_size = 0x40;
		cvmx_write_csr_node(node,
                        CVMX_BGXX_SMUX_TX_MIN_PKT(index, interface),
			smu_min_pkt.u64);
		smu_tx_append.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_TX_APPEND(index, interface));
		smu_tx_append.s.fcs_c = fcs_enable;
		smu_tx_append.s.pad = pad_enable;
		cvmx_write_csr_node(node,
			CVMX_BGXX_SMUX_TX_APPEND(index, interface),
			smu_tx_append.u64);
	}
}

/**
 * Set mac for the ipd_port
 *
 * @param xipd_port ipd_port to set the mac
 * @param bcst      If set, accept all broadcast packets
 * @param mcst      Multicast mode
 * 		    0 = Force reject all multicast packets
 * 		    1 = Force accept all multicast packets
 * 		    2 = use the address filter CAM.
 * @param mac       mac address for the ipd_port
 */
void cvmx_helper_bgx_set_mac(int xipd_port, int bcst, int mcst, uint64_t mac)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int index = (xipd_port >> 4) & 0xf;
	cvmx_bgxx_cmr_rx_adrx_cam_t adr_cam;
	cvmx_bgxx_cmrx_rx_adr_ctl_t adr_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	int saved_state;

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface));
	saved_state = cmr_config.s.enable;
	cmr_config.s.enable = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);

	/* Set the mac */
	adr_cam.u64 = 0;
	adr_cam.s.id = index;
	adr_cam.s.en = 1;
	adr_cam.s.adr = mac;
	cvmx_write_csr_node(node, CVMX_BGXX_CMR_RX_ADRX_CAM(index * 8, interface), adr_cam.u64);

	adr_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, interface));
	adr_ctl.s.cam_accept = 1;  /* Accept the packet on DMAC CAM address */
	adr_ctl.s.mcst_mode = mcst;   /* Use the address filter CAM */
	adr_ctl.s.bcst_accept = bcst; /* Accept all broadcast packets */
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, interface), adr_ctl.u64);
	/* Set SMAC for PAUSE frames */
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_SMACX(index, interface), mac);

	/* Restore back the interface state */
	cmr_config.s.enable = saved_state;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
}
