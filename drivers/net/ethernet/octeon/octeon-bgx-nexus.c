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
#include <linux/ctype.h>

#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-bgxx-defs.h>

#include "octeon-bgx.h"

static atomic_t bgx_nexus_once[4];
static atomic_t request_ethernet3_once;

static char *mix_port;
module_param(mix_port, charp, S_IRUGO);
MODULE_PARM_DESC(mix_port, "Specifies which ports connect to MIX interfaces.");

#define MAX_NODES		2
#define MAX_MIX_PER_NODE	2
#define MAX_BGX_PER_NODE	6
#define MAX_LMAC_PER_NODE	4

#define MAX_MIX			(MAX_NODES * MAX_MIX_PER_NODE)

/* mix_port_lmac:		Describes a lmac that connects to a mix port.
 *				The lmac must be on the same node as the mix.
 *
 *  node:			Node of the lmac.
 *  bgx:			Bgx of the lmac.
 *  lmac:			Lmac index.
 */
struct mix_port_lmac {
	int	node;
	int	bgx;
	int	lmac;
};

/* mix_ports_lmacs contains all the lmacs connected to mix ports */
static struct mix_port_lmac mix_port_lmacs[MAX_MIX];

/* is_lmac_to_mix:		Search the list of lmacs connected to mix'es
 *				for a match.
 *
 *  node:			Numa node of lmac to search for.
 *  bgx:			Bgx of lmac to search for.
 *  lmac:			Lmac index to search for.
 *
 *  returns:			true if the lmac is connected to a mix, false
 *				otherwise.
 */
static bool is_lmac_to_mix(int node, int bgx, int lmac)
{
	int	i;

	for (i = 0; i < MAX_MIX; i++) {
		if (mix_port_lmacs[i].node == node &&
		    mix_port_lmacs[i].bgx == bgx &&
		    mix_port_lmacs[i].lmac == lmac)
			return true;
	}

	return false;
}

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

	reg = of_get_property(pdev->dev.of_node, "reg", NULL);
	addr = of_translate_address(pdev->dev.of_node, reg);
	interface = (addr >> 24) & 0xf;
	numa_node = (addr >> 36) & 0x7;

	/* One time initialization */
	if (atomic_cmpxchg(bgx_nexus_once + numa_node, 0, 1) == 0)
		__cvmx_helper_init_port_config_data(numa_node);

	__cvmx_helper_bgx_probe(cvmx_helper_node_interface_to_xiface(numa_node, interface));

	/* Assign 8 CAM entries per LMAC */
	for (i = 0; i < 32; i++) {
		union cvmx_bgxx_cmr_rx_adrx_cam adr_cam;
		adr_cam.u64 = 0;
		adr_cam.s.id = i >> 3;
		cvmx_write_csr_node(numa_node, CVMX_BGXX_CMR_RX_ADRX_CAM(i, interface), adr_cam.u64);
	}

	for_each_available_child_of_node(pdev->dev.of_node, child) {
		bool is_mix = false;
		union cvmx_bgxx_cmrx_config cmr_config;

		if (!of_device_is_compatible(child, "cavium,octeon-7890-bgx-port"))
			continue;
		r = of_property_read_u32(child, "reg", &port);
		if (r)
			return -ENODEV;

		is_mix = is_lmac_to_mix(numa_node, interface, port);

		/* Connect to PKI/PKO */
		cmr_config.u64 = cvmx_read_csr_node(numa_node, CVMX_BGXX_CMRX_CONFIG(port, interface));
		cmr_config.s.mix_en = is_mix ? 1 : 0;
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

		pki_dev = platform_device_register_data(&new_dev->dev,
							is_mix ? "octeon_mgmt" : "ethernet-mac-pki",
							pki_id++,
							&platform_data, sizeof(platform_data));
		dev_info(&pdev->dev, "Created %s %u: %p\n", is_mix ? "MIX" : "PKI", pki_dev->id, pki_dev);
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

/* bgx_mix_init_from_fdt:	Initialize the list of lmacs that connect to mix
 *				ports from information in the device tree.
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_mix_init_from_fdt(void)
{
	struct device_node	*node;
	struct device_node	*parent = NULL;
	int			mix = 0;

	for_each_compatible_node(node, NULL, "cavium,octeon-7890-mix") {
		struct device_node	*lmac_fdt_node;
		const __be32		*reg;
		u64			addr;

		/* Get the fdt node of the lmac connected to this mix */
		lmac_fdt_node = of_parse_phandle(node, "cavium,mac-handle", 0);
		if (!lmac_fdt_node)
			goto err;

		/* Get the numa node and bgx of the lmac */
		parent = of_get_parent(lmac_fdt_node);
		if (!parent)
			goto err;
		reg = of_get_property(parent, "reg", NULL);
		if (!reg)
			goto err;
		addr = of_translate_address(parent, reg);
		of_node_put(parent);
		parent = NULL;

		mix_port_lmacs[mix].node = (addr >> 36) & 0x7;
		mix_port_lmacs[mix].bgx = (addr >> 24) & 0xf;

		/* Get the lmac index */
		reg = of_get_property(lmac_fdt_node, "reg", NULL);
		if (!reg)
			goto err;

		mix_port_lmacs[mix].lmac = *reg;

		mix++;
		if (mix >= MAX_MIX)
			break;
	}

	return 0;
 err:
	pr_warn("Invalid device tree mix port information\n");
	for (mix = 0; mix < MAX_MIX; mix++) {
		mix_port_lmacs[mix].node = -1;
		mix_port_lmacs[mix].bgx = -1;
		mix_port_lmacs[mix].lmac = -1;
	}
	if (parent)
		of_node_put(parent);

	return -EINVAL;
}

/* bgx_mix_init_from_param:	Initialize the list of lmacs that connect to mix
 *				ports from information in the "mix_port"
 *				parameter. The mix_port parameter format is as
 *				follows:
 *				mix_port=nbl
 *				where:
 *					n = node
 *					b = bgx
 *					l = lmac
 *				There can be up to 4 lmacs defined separated by
 *				commas. For example to select node0, bgx0, lmac0
 *				and node0, bgx4, lamc0, the mix_port parameter
 *				would be: mix_port=000,040
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_mix_init_from_param(void)
{
	char	*p = mix_port;
	int	mix = 0;
	int	i;

	while (*p) {
		int	node = -1;
		int	bgx = -1;
		int	lmac = -1;

		if (strlen(p) < 3)
			goto err;

		/* Get the numa node */
		if (!isdigit(*p))
			goto err;
		node = *p - '0';
		if (node >= MAX_NODES)
			goto err;

		/* Get the bgx */
		p++;
		if (!isdigit(*p))
			goto err;
		bgx = *p - '0';
		if (bgx >= MAX_BGX_PER_NODE)
			goto err;

		/* Get the lmac index */
		p++;
		if (!isdigit(*p))
			goto err;
		lmac = *p - '0';
		if (lmac >= 2)
			goto err;

		/* Only one lmac0 and one lmac1 per node is supported */
		for (i = 0; i < MAX_MIX; i++) {
			if (mix_port_lmacs[i].node == node &&
			    mix_port_lmacs[i].lmac == lmac)
				goto err;
		}

		mix_port_lmacs[mix].node = node;
		mix_port_lmacs[mix].bgx = bgx;
		mix_port_lmacs[mix].lmac = lmac;

		p++;
		if (*p == ',')
			p++;

		mix++;
		if (mix >= MAX_MIX)
			break;
	}

	return 0;
 err:
	pr_warn("Invalid parameter mix_port=%s\n", mix_port);
	for (mix = 0; mix < MAX_MIX; mix++) {
		mix_port_lmacs[mix].node = -1;
		mix_port_lmacs[mix].bgx = -1;
		mix_port_lmacs[mix].lmac = -1;
	}
	return -EINVAL;
}

/* bgx_mix_port_lmacs_init:	Initialize the mix_port_lmacs variable with the
 *				lmacs that connect to mic ports.
 *
 *  returns:			Zero on success, error otherwise.
 */
static int bgx_mix_port_lmacs_init(void)
{
	int	mix;

	/* Start with no mix ports configured */
	for (mix = 0; mix < MAX_MIX; mix++) {
		mix_port_lmacs[mix].node = -1;
		mix_port_lmacs[mix].bgx = -1;
		mix_port_lmacs[mix].lmac = -1;
	}

	/* Check if no mix port should be configured */
	if (mix_port && !strcmp(mix_port, "none"))
		return 0;

	/* Configure the mix ports using information from the device tree if no
	 * parameter was passed. Otherwise, use the information in the module
	 * parameter.
	 */
	if (!mix_port)
		bgx_mix_init_from_fdt();
	else
		bgx_mix_init_from_param();

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

static int __init bgx_driver_init(void)
{
	int r;

	bgx_mix_port_lmacs_init();

	r = platform_driver_register(&bgx_driver);

	return r;
}

static void __exit bgx_driver_exit(void)
{
	platform_driver_unregister(&bgx_driver);
}

module_init(bgx_driver_init);
module_exit(bgx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Networks <support@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks BGX MAC Nexus driver.");
