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
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-global-resources.h>
#else
#include "cvmx.h"
#include "cvmx-pow.h"
#include "cvmx-global-resources.h"
#endif

static struct global_resource_tag get_sso_resource_tag(int node)
{
	switch(node) {
	case 0:
		return cvmx_get_gr_tag('c','v','m','_','s','s','o','_','0','0','.','.','.','.','.','.');
	case 1:
		return cvmx_get_gr_tag('c','v','m','_','s','s','o','_','0','1','.','.','.','.','.','.');
	case 2:
		return cvmx_get_gr_tag('c','v','m','_','s','s','o','_','0','2','.','.','.','.','.','.');
	case 3:
		return cvmx_get_gr_tag('c','v','m','_','s','s','o','_','0','3','.','.','.','.','.','.');
	default:
		/* Add a panic?? */
		return cvmx_get_gr_tag('i','n','v','a','l','i','d','.','.','.','.','.','.','.','.','.');
	}
}

int cvmx_sso_allocate_group_range(int node, int *base_group, int count)
{
	int num_grp;
	int start;
	uint64_t owner = 0;
	struct global_resource_tag tag = get_sso_resource_tag(node);

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		num_grp = 256;
	else if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		num_grp = 64;
	else
		num_grp = 16;

	if (cvmx_create_global_resource_range(tag, num_grp) != 0) {
		cvmx_dprintf("ERROR: failed to create sso global resource for node=%d\n", node);
		return -1;
	}

	if (*base_group >= 0) {
		start = cvmx_reserve_global_resource_range(tag, owner, *base_group, count);
		if (start != *base_group)
			return CVMX_RESOURCE_ALREADY_RESERVED;
		else
			return 0;
	} else {
		start = cvmx_allocate_global_resource_range(tag, owner, count, 1);
		if (start < 0) {
			return CVMX_RESOURCE_ALREADY_RESERVED;
		} else {
			*base_group = start;
			return 0;
		}
	}
}
EXPORT_SYMBOL(cvmx_sso_allocate_group_range);

int cvmx_sso_allocate_group(int node)
{
	int r;
	int grp = -1;

	r = cvmx_sso_allocate_group_range(node, &grp, 1);

	return r == 0 ? grp : -1;
}
EXPORT_SYMBOL(cvmx_sso_allocate_group);
