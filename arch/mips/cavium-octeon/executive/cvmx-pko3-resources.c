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

/**
 * @file
 *
 * PKO resources.
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pko3.h>
#include "asm/octeon/cvmx-global-resources.h"
#include <asm/octeon/cvmx-pko3-resources.h>
#include "asm/octeon/cvmx-range.h"
#else
#include "cvmx.h"
#include "cvmx-pko3.h"
#include "cvmx-global-resources.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-range.h"
#endif

#define CVMX_GR_TAG_PKO_PORT_QUEUES(x)   cvmx_get_gr_tag('c','v','m','_','p','k','o','p','o','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L2_QUEUES(x)   	 cvmx_get_gr_tag('c','v','m','_','p','k','o','l','2','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L3_QUEUES(x)     cvmx_get_gr_tag('c','v','m','_','p','k','o','l','3','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L4_QUEUES(x)     cvmx_get_gr_tag('c','v','m','_','p','k','o','l','4','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L5_QUEUES(x)     cvmx_get_gr_tag('c','v','m','_','p','k','o','l','5','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_DESCR_QUEUES(x)  cvmx_get_gr_tag('c','v','m','_','p','k','o','d','e','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_PORT_INDEX(x)  	 cvmx_get_gr_tag('c','v','m','_','p','k','o','p','i','d','_',(x+'0'),'.','.','.','.')

const int cvmx_pko_num_queues_78XX[CVMX_PKO_NUM_QUEUE_LEVELS] = 
{
	[CVMX_PKO_PORT_QUEUES] = 32,
	[CVMX_PKO_L2_QUEUES] = 512,
	[CVMX_PKO_L3_QUEUES] = 512,
	[CVMX_PKO_L4_QUEUES] = 1024,
	[CVMX_PKO_L5_QUEUES] = 1024,
	[CVMX_PKO_DESCR_QUEUES] = 1024
};

static inline int __cvmx_pko3_get_num_queues(int level)
{
	if(OCTEON_IS_MODEL(OCTEON_CN78XX))
		return cvmx_pko_num_queues_78XX[level];
	return -1;
}

static inline struct global_resource_tag __cvmx_pko_get_queues_resource_tag(int node, int queue_level)
{
	switch(queue_level) {
		case CVMX_PKO_PORT_QUEUES:
			return CVMX_GR_TAG_PKO_PORT_QUEUES(node);
		case CVMX_PKO_L2_QUEUES:
			return CVMX_GR_TAG_PKO_L2_QUEUES(node);
		case CVMX_PKO_L3_QUEUES:
			return CVMX_GR_TAG_PKO_L3_QUEUES(node);
		case CVMX_PKO_L4_QUEUES:
			return CVMX_GR_TAG_PKO_L4_QUEUES(node);
		case CVMX_PKO_L5_QUEUES:
			return CVMX_GR_TAG_PKO_L5_QUEUES(node);
		case CVMX_PKO_DESCR_QUEUES:
			return CVMX_GR_TAG_PKO_DESCR_QUEUES(node);
		default:
			return CVMX_GR_TAG_INVALID;
	}
}

/**
 * Allocate or reserve a pko resource - called by wrapper functions
 * @param tag processed global resource tag
 * @param base_queue if specified the queue to reserve
 * @param owner to be specified for resource
 * @param num_queues to allocate
 * @param max_num_queues for global resource
 */
int cvmx_pko_alloc_global_resource(struct global_resource_tag tag, int base_queue, int owner, int num_queues, int max_num_queues)
{
	int res;
	if(cvmx_create_global_resource_range(tag, max_num_queues)) {
		cvmx_dprintf("ERROR: Failed to create PKO3 resource: %lx-%lx\n",
			(unsigned long) tag.hi, (unsigned long) tag.lo);
		return -1;
	}
	if(base_queue >= 0) {
		res = cvmx_reserve_global_resource_range(tag, owner, base_queue, num_queues);
	} else {
		res = cvmx_allocate_global_resource_range(tag, owner, num_queues, 1);
	}
	if(res < 0) {
		cvmx_dprintf("ERROR: Failed to %s PKO3 tag %lx:%lx, %i %i %i %i.\n",
			((base_queue < 0) ? "allocate" : "reserve"),
			(unsigned long) tag.hi, (unsigned long) tag.lo,
			base_queue, owner, num_queues, max_num_queues);
		return -1;
	}

	return res;
}

/**
 * Allocate or reserve PKO queues - wrapper for cvmx_pko_alloc_global_resource
 *
 * @param node on which to allocate/reserve PKO queues
 * @param level of PKO queue
 * @param owner of reserved/allocated resources
 * @param base_queue to start reservation/allocatation
 * @param num_queues number of queues to be allocated
 * @return 0 on success, -1 on failure
 */
int cvmx_pko_alloc_queues(int node, int level, int owner, int base_queue, int num_queues)
{
	struct global_resource_tag tag = __cvmx_pko_get_queues_resource_tag(node, level);
	int max_num_queues = __cvmx_pko3_get_num_queues(level);

	return cvmx_pko_alloc_global_resource(tag, base_queue, owner, num_queues, max_num_queues);
}
EXPORT_SYMBOL(cvmx_pko_alloc_queues);

/**
 * Free an allocated/reserved PKO queues for a certain level and owner
 *
 * @param node on which to allocate/reserve PKO queues
 * @param level of PKO queue
 * @param owner of reserved/allocated resources
 * @return 0 on success, -1 on failure
 */
int cvmx_pko_free_queues(int node, int level, int owner)
{
	struct global_resource_tag tag = __cvmx_pko_get_queues_resource_tag(node, level);

	return cvmx_free_global_resource_range_with_owner(tag, owner);
}
EXPORT_SYMBOL(cvmx_pko_free_queues);


