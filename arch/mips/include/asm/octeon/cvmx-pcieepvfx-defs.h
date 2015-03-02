/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
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
 * cvmx-pcieepvfx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pcieepvfx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PCIEEPVFX_DEFS_H__
#define __CVMX_PCIEEPVFX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG000(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG000(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000000ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG000(block_id) (0x0000050000000000ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG001(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG001(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000004ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG001(block_id) (0x0000050000000004ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG002(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG002(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000008ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG002(block_id) (0x0000050000000008ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG003(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG003(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000000Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG003(block_id) (0x000005000000000Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG004(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG004(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000010ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG004(block_id) (0x0000050000000010ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG005(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG005(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000014ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG005(block_id) (0x0000050000000014ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG006(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG006(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000018ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG006(block_id) (0x0000050000000018ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG007(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG007(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000001Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG007(block_id) (0x000005000000001Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG008(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG008(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000020ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG008(block_id) (0x0000050000000020ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG009(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG009(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000024ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG009(block_id) (0x0000050000000024ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG010(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG010(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000028ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG010(block_id) (0x0000050000000028ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG011(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG011(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000002Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG011(block_id) (0x000005000000002Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG012(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG012(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000030ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG012(block_id) (0x0000050000000030ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG013(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG013(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000034ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG013(block_id) (0x0000050000000034ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG015(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG015(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000003Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG015(block_id) (0x000005000000003Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG028(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG028(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000070ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG028(block_id) (0x0000050000000070ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG029(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG029(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000074ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG029(block_id) (0x0000050000000074ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG030(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG030(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000078ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG030(block_id) (0x0000050000000078ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG031(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG031(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000007Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG031(block_id) (0x000005000000007Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG032(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG032(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000080ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG032(block_id) (0x0000050000000080ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG037(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG037(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000094ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG037(block_id) (0x0000050000000094ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG038(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG038(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000098ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG038(block_id) (0x0000050000000098ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG039(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG039(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000009Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG039(block_id) (0x000005000000009Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG040(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG040(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000A0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG040(block_id) (0x00000500000000A0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG044(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG044(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000B0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG044(block_id) (0x00000500000000B0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG045(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG045(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000B4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG045(block_id) (0x00000500000000B4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG046(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG046(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000B8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG046(block_id) (0x00000500000000B8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG048(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG048(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000C0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG048(block_id) (0x00000500000000C0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG049(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG049(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000C4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG049(block_id) (0x00000500000000C4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG064(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG064(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000100ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG064(block_id) (0x0000050000000100ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG082(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG082(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000148ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG082(block_id) (0x0000050000000148ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG083(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG083(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000014Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG083(block_id) (0x000005000000014Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG448(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG448(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000700ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG448(block_id) (0x0000050000000700ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG449(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG449(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000704ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG449(block_id) (0x0000050000000704ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG450(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG450(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000708ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG450(block_id) (0x0000050000000708ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG451(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG451(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000070Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG451(block_id) (0x000005000000070Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG452(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG452(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000710ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG452(block_id) (0x0000050000000710ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG453(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG453(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000714ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG453(block_id) (0x0000050000000714ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG454(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG454(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000718ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG454(block_id) (0x0000050000000718ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG455(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG455(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000071Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG455(block_id) (0x000005000000071Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG456(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG456(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000720ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG456(block_id) (0x0000050000000720ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG458(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG458(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000728ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG458(block_id) (0x0000050000000728ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG459(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG459(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000072Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG459(block_id) (0x000005000000072Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG460(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG460(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000730ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG460(block_id) (0x0000050000000730ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG461(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG461(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000734ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG461(block_id) (0x0000050000000734ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG462(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG462(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000738ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG462(block_id) (0x0000050000000738ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG463(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG463(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000073Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG463(block_id) (0x000005000000073Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG464(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG464(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000740ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG464(block_id) (0x0000050000000740ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG465(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG465(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000744ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG465(block_id) (0x0000050000000744ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG466(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG466(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000748ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG466(block_id) (0x0000050000000748ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG467(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG467(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000074Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG467(block_id) (0x000005000000074Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG468(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG468(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000750ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG468(block_id) (0x0000050000000750ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG490(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG490(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000007A8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG490(block_id) (0x00000500000007A8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG491(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG491(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000007ACull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG491(block_id) (0x00000500000007ACull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG492(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG492(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000007B0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG492(block_id) (0x00000500000007B0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG515(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG515(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000080Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG515(block_id) (0x000005000000080Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG516(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG516(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000810ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG516(block_id) (0x0000050000000810ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG517(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG517(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000814ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG517(block_id) (0x0000050000000814ull + ((block_id) & 3) * 0x100000000ull)
#endif

/**
 * cvmx_pcieepvf#_cfg000
 *
 * This register contains the first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg000 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg000_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t devid                        : 16; /**< Device ID. */
	uint32_t vendid                       : 16; /**< Vendor ID. */
#else
	uint32_t vendid                       : 16;
	uint32_t devid                        : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg000_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg000 cvmx_pcieepvfx_cfg000_t;

/**
 * cvmx_pcieepvf#_cfg001
 *
 * This register contains the second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg001 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg001_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dpe                          : 1;  /**< Detected parity error. */
	uint32_t sse                          : 1;  /**< Signaled system error. */
	uint32_t rma                          : 1;  /**< Received master abort. */
	uint32_t rta                          : 1;  /**< Received target abort. */
	uint32_t sta                          : 1;  /**< Signaled target abort. */
	uint32_t devt                         : 2;  /**< DEVSEL timing. Not applicable for PCI Express. Hardwired to 0x0. */
	uint32_t mdpe                         : 1;  /**< Master data parity error. */
	uint32_t fbb                          : 1;  /**< Fast back-to-back capable. Not applicable for PCI Express. Hardwired to 0. */
	uint32_t reserved_22_22               : 1;
	uint32_t m66                          : 1;  /**< 66 MHz capable. Not applicable for PCI Express. Hardwired to 0. */
	uint32_t cl                           : 1;  /**< Capabilities list. Indicates presence of an extended capability item. Hardwired to 1. */
	uint32_t i_stat                       : 1;  /**< INTx status. */
	uint32_t reserved_11_18               : 8;
	uint32_t i_dis                        : 1;  /**< INTx assertion disable. */
	uint32_t fbbe                         : 1;  /**< Fast back-to-back transaction enable. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t see                          : 1;  /**< SERR# enable. */
	uint32_t ids_wcc                      : 1;  /**< IDSEL stepping/wait cycle control. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t per                          : 1;  /**< Parity error response. */
	uint32_t vps                          : 1;  /**< VGA palette snoop. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t mwice                        : 1;  /**< Memory write and invalidate. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t scse                         : 1;  /**< Special cycle enable. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t me                           : 1;  /**< Bus master enable. */
	uint32_t msae                         : 1;  /**< Memory space access enable. */
	uint32_t isae                         : 1;  /**< I/O space access enable. */
#else
	uint32_t isae                         : 1;
	uint32_t msae                         : 1;
	uint32_t me                           : 1;
	uint32_t scse                         : 1;
	uint32_t mwice                        : 1;
	uint32_t vps                          : 1;
	uint32_t per                          : 1;
	uint32_t ids_wcc                      : 1;
	uint32_t see                          : 1;
	uint32_t fbbe                         : 1;
	uint32_t i_dis                        : 1;
	uint32_t reserved_11_18               : 8;
	uint32_t i_stat                       : 1;
	uint32_t cl                           : 1;
	uint32_t m66                          : 1;
	uint32_t reserved_22_22               : 1;
	uint32_t fbb                          : 1;
	uint32_t mdpe                         : 1;
	uint32_t devt                         : 2;
	uint32_t sta                          : 1;
	uint32_t rta                          : 1;
	uint32_t rma                          : 1;
	uint32_t sse                          : 1;
	uint32_t dpe                          : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg001_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg001 cvmx_pcieepvfx_cfg001_t;

/**
 * cvmx_pcieepvf#_cfg002
 *
 * This register contains the third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg002 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg002_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bcc                          : 8;  /**< Base class code. */
	uint32_t sc                           : 8;  /**< Subclass code. */
	uint32_t pi                           : 8;  /**< Programming interface. */
	uint32_t rid                          : 8;  /**< Revision ID, writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t rid                          : 8;
	uint32_t pi                           : 8;
	uint32_t sc                           : 8;
	uint32_t bcc                          : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg002_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg002 cvmx_pcieepvfx_cfg002_t;

/**
 * cvmx_pcieepvf#_cfg003
 *
 * This register contains the fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg003 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg003_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bist                         : 8;  /**< The BIST register functions are not supported. All 8 bits of the BIST register are
                                                         hardwired to 0x0. */
	uint32_t mfd                          : 1;  /**< Multi function device. */
	uint32_t chf                          : 7;  /**< Configuration header format. Hardwired to 0x0 for type 0. */
	uint32_t lt                           : 8;  /**< Master latency timer. Not applicable for PCI Express, hardwired to 0x0. */
	uint32_t cls                          : 8;  /**< Cache line size. The cache line size register is R/W for legacy compatibility purposes and
                                                         is not applicable to PCI Express device functionality. Writing to the cache line size
                                                         register does not impact functionality of the PCI Express bus. */
#else
	uint32_t cls                          : 8;
	uint32_t lt                           : 8;
	uint32_t chf                          : 7;
	uint32_t mfd                          : 1;
	uint32_t bist                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg003_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg003 cvmx_pcieepvfx_cfg003_t;

/**
 * cvmx_pcieepvf#_cfg004
 *
 * This register contains the fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg004 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg004_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg004_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg004 cvmx_pcieepvfx_cfg004_t;

/**
 * cvmx_pcieepvf#_cfg005
 *
 * This register contains the sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg005 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg005_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg005_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg005 cvmx_pcieepvfx_cfg005_t;

/**
 * cvmx_pcieepvf#_cfg006
 *
 * This register contains the seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg006 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg006_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg006_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg006 cvmx_pcieepvfx_cfg006_t;

/**
 * cvmx_pcieepvf#_cfg007
 *
 * This register contains the eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg007 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg007_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg007_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg007 cvmx_pcieepvfx_cfg007_t;

/**
 * cvmx_pcieepvf#_cfg008
 *
 * This register contains the ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg008 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg008_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg008_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg008 cvmx_pcieepvfx_cfg008_t;

/**
 * cvmx_pcieepvf#_cfg009
 *
 * This register contains the tenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg009 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg009_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg009_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg009 cvmx_pcieepvfx_cfg009_t;

/**
 * cvmx_pcieepvf#_cfg010
 *
 * This register contains the eleventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg010 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg010_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cisp                         : 32; /**< CardBus CIS pointer. */
#else
	uint32_t cisp                         : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg010_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg010 cvmx_pcieepvfx_cfg010_t;

/**
 * cvmx_pcieepvf#_cfg011
 *
 * This register contains the twelfth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg011 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg011_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ssid                         : 16; /**< Subsystem ID. Assigned by PCI-SIG. */
	uint32_t ssvid                        : 16; /**< Subsystem vendor ID. Assigned by PCI-SIG. */
#else
	uint32_t ssvid                        : 16;
	uint32_t ssid                         : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg011_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg011 cvmx_pcieepvfx_cfg011_t;

/**
 * cvmx_pcieepvf#_cfg012
 *
 * This register contains the thirteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg012 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg012_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t eraddr                       : 16; /**< Expansion ROM address. */
	uint32_t reserved_1_15                : 15;
	uint32_t er_en                        : 1;  /**< Expansion ROM enable. */
#else
	uint32_t er_en                        : 1;
	uint32_t reserved_1_15                : 15;
	uint32_t eraddr                       : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg012_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg012 cvmx_pcieepvfx_cfg012_t;

/**
 * cvmx_pcieepvf#_cfg013
 *
 * This register contains the fourteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg013 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg013_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t cp                           : 8;  /**< First capability pointer. Points to the PCI Express Capability Pointer structure (VF's). */
#else
	uint32_t cp                           : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg013_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg013 cvmx_pcieepvfx_cfg013_t;

/**
 * cvmx_pcieepvf#_cfg015
 *
 * This register contains the sixteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg015 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg015_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ml                           : 8;  /**< Maximum latency (hardwired to 0x0). */
	uint32_t mg                           : 8;  /**< Minimum grant (hardwired to 0x0). */
	uint32_t inta                         : 8;  /**< Interrupt pin. */
	uint32_t il                           : 8;  /**< Interrupt line. */
#else
	uint32_t il                           : 8;
	uint32_t inta                         : 8;
	uint32_t mg                           : 8;
	uint32_t ml                           : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg015_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg015 cvmx_pcieepvfx_cfg015_t;

/**
 * cvmx_pcieepvf#_cfg028
 *
 * This register contains the twenty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg028 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg028_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t imn                          : 5;  /**< Interrupt message number. */
	uint32_t si                           : 1;  /**< Slot implemented. */
	uint32_t dpt                          : 4;  /**< Device port type. */
	uint32_t pciecv                       : 4;  /**< PCI Express capability version. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. Points to the MSI-X capabilities by default. */
	uint32_t pcieid                       : 8;  /**< PCI Express capability ID. */
#else
	uint32_t pcieid                       : 8;
	uint32_t ncp                          : 8;
	uint32_t pciecv                       : 4;
	uint32_t dpt                          : 4;
	uint32_t si                           : 1;
	uint32_t imn                          : 5;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg028_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg028 cvmx_pcieepvfx_cfg028_t;

/**
 * cvmx_pcieepvf#_cfg029
 *
 * This register contains the thirtieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg029 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg029_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t flr_cap                      : 1;  /**< Function level reset capability. Set to 1 for SR-IOV core. */
	uint32_t cspls                        : 2;  /**< Captured slot power limit scale. From message from RC, upstream port only. */
	uint32_t csplv                        : 8;  /**< Captured slot power limit value. From message from RC, upstream port only. */
	uint32_t reserved_16_17               : 2;
	uint32_t rber                         : 1;  /**< Role-based error reporting. */
	uint32_t reserved_12_14               : 3;
	uint32_t el1al                        : 3;  /**< Endpoint L1 acceptable latency. */
	uint32_t el0al                        : 3;  /**< Endpoint L0s acceptable latency. */
	uint32_t etfs                         : 1;  /**< Extended tag field supported. */
	uint32_t pfs                          : 2;  /**< Phantom function supported. */
	uint32_t mpss                         : 3;  /**< Max_Payload_Size supported. */
#else
	uint32_t mpss                         : 3;
	uint32_t pfs                          : 2;
	uint32_t etfs                         : 1;
	uint32_t el0al                        : 3;
	uint32_t el1al                        : 3;
	uint32_t reserved_12_14               : 3;
	uint32_t rber                         : 1;
	uint32_t reserved_16_17               : 2;
	uint32_t csplv                        : 8;
	uint32_t cspls                        : 2;
	uint32_t flr_cap                      : 1;
	uint32_t reserved_29_31               : 3;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg029_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg029 cvmx_pcieepvfx_cfg029_t;

/**
 * cvmx_pcieepvf#_cfg030
 *
 * This register contains the thirty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg030 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg030_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t tp                           : 1;  /**< Transaction pending. Set to 1 when nonposted requests are not yet completed and set to 0
                                                         when they are completed. */
	uint32_t ap_d                         : 1;  /**< Aux power detected. Set to 1 if Aux power detected. */
	uint32_t ur_d                         : 1;  /**< Unsupported request detected. Errors are logged in this register regardless of whether or
                                                         not error reporting is enabled in the device control register. UR_D occurs when we receive
                                                         something unsupported. Unsupported requests are nonfatal errors, so UR_D should cause
                                                         NFE_D. Receiving a vendor-defined message should cause an unsupported request. */
	uint32_t fe_d                         : 1;  /**< Fatal error detected. All fatal errors are non-function specific and get reported only in the PF. */
	uint32_t nfe_d                        : 1;  /**< Nonfatal error detected. Errors are logged in this register regardless of whether or not
                                                         error reporting is enabled in the device control register. This field is set if we receive
                                                         any of the errors in
                                                         PCIEEP()_CFG066 that has a severity set to nonfatal and does not meet advisory
                                                         nonfatal criteria, which most poisoned TLPs should. */
	uint32_t ce_d                         : 1;  /**< Correctable error detected. All correctable errors are non-function specific and get
                                                         reported only in the PF. */
	uint32_t i_flr                        : 1;  /**< Initiate function level reset (not supported). */
	uint32_t mrrs                         : 3;  /**< Max read request size.
                                                         0x0 = 128 bytes.
                                                         0x1 = 256 bytes.
                                                         0x2 = 512 bytes.
                                                         0x3 = 1024 bytes.
                                                         0x4 = 2048 bytes.
                                                         0x5 = 4096 bytes. */
	uint32_t ns_en                        : 1;  /**< Enable no snoop. */
	uint32_t ap_en                        : 1;  /**< AUX power PM enable. */
	uint32_t pf_en                        : 1;  /**< Phantom function enable. */
	uint32_t etf_en                       : 1;  /**< Extended tag field enable. */
	uint32_t mps                          : 3;  /**< Max payload size. Legal values: 0x0 = 128 B, 0x1 = 256 B.
                                                         Larger sizes are not supported by CNXXXX.
                                                         DPI_SLI_PRT()_CFG[MPS] must be set to the same value as this field for proper
                                                         functionality. */
	uint32_t ro_en                        : 1;  /**< Enable relaxed ordering. */
	uint32_t ur_en                        : 1;  /**< Unsupported request reporting enable. */
	uint32_t fe_en                        : 1;  /**< Fatal error reporting enable. */
	uint32_t nfe_en                       : 1;  /**< Nonfatal error reporting enable. */
	uint32_t ce_en                        : 1;  /**< Correctable error reporting enable. */
#else
	uint32_t ce_en                        : 1;
	uint32_t nfe_en                       : 1;
	uint32_t fe_en                        : 1;
	uint32_t ur_en                        : 1;
	uint32_t ro_en                        : 1;
	uint32_t mps                          : 3;
	uint32_t etf_en                       : 1;
	uint32_t pf_en                        : 1;
	uint32_t ap_en                        : 1;
	uint32_t ns_en                        : 1;
	uint32_t mrrs                         : 3;
	uint32_t i_flr                        : 1;
	uint32_t ce_d                         : 1;
	uint32_t nfe_d                        : 1;
	uint32_t fe_d                         : 1;
	uint32_t ur_d                         : 1;
	uint32_t ap_d                         : 1;
	uint32_t tp                           : 1;
	uint32_t reserved_22_31               : 10;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg030_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg030 cvmx_pcieepvfx_cfg030_t;

/**
 * cvmx_pcieepvf#_cfg031
 *
 * This register contains the thirty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg031 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg031_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pnum                         : 8;  /**< Port number. */
	uint32_t reserved_23_23               : 1;
	uint32_t aspm                         : 1;  /**< ASPM optionality compliance. */
	uint32_t lbnc                         : 1;  /**< Link bandwidth notification capability. Set to 0 for endpoint devices. */
	uint32_t dllarc                       : 1;  /**< Data link layer active reporting capable. */
	uint32_t sderc                        : 1;  /**< Surprise down error reporting capable. Not supported; hardwired to 0. */
	uint32_t cpm                          : 1;  /**< Clock power management. The default value is the value that software specifies during
                                                         hardware configuration. */
	uint32_t l1el                         : 3;  /**< L1 exit latency. The default value is the value that software specifies during hardware
                                                         configuration. */
	uint32_t l0el                         : 3;  /**< L0s exit latency. The default value is the value that software specifies during hardware
                                                         configuration. */
	uint32_t aslpms                       : 2;  /**< Active state link PM support. The default value is the value that software specifies
                                                         during hardware configuration. */
	uint32_t mlw                          : 6;  /**< Maximum link width.
                                                         The reset value of this field is determined by the value read from the PEM
                                                         csr PEM()_CFG[LANES8]. If LANES8 is set the reset value is 0x4, otherwise 0x8.
                                                         This field is writable through PEM()_CFG_WR. */
	uint32_t mls                          : 4;  /**< Maximum link speed. The reset value of this field is controlled by the value read from
                                                         PEM()_CFG[MD].
                                                         _ MD is 0x0, reset to 0x1: 2.5 GHz supported.
                                                         _ MD is 0x1, reset to 0x2: 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x2, reset to 0x3: 8.0 Ghz, 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x3, reset to 0x3: 8.0 Ghz, 5.0 GHz and 2.5 GHz supported (RC Mode).
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t mls                          : 4;
	uint32_t mlw                          : 6;
	uint32_t aslpms                       : 2;
	uint32_t l0el                         : 3;
	uint32_t l1el                         : 3;
	uint32_t cpm                          : 1;
	uint32_t sderc                        : 1;
	uint32_t dllarc                       : 1;
	uint32_t lbnc                         : 1;
	uint32_t aspm                         : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t pnum                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg031_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg031 cvmx_pcieepvfx_cfg031_t;

/**
 * cvmx_pcieepvf#_cfg032
 *
 * This register contains the thirty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg032 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg032_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lab                          : 1;  /**< Link autonomous bandwidth status. */
	uint32_t lbm                          : 1;  /**< Link bandwidth management status. */
	uint32_t dlla                         : 1;  /**< Data link layer active. Not applicable for an upstream port or endpoint device, hardwired to 0. */
	uint32_t scc                          : 1;  /**< Slot clock configuration. Indicates that the component uses the same physical reference
                                                         clock that the platform provides on the connector. */
	uint32_t lt                           : 1;  /**< Link training. Not applicable for an upstream port or endpoint device, hardwired to 0. */
	uint32_t reserved_26_26               : 1;
	uint32_t nlw                          : 6;  /**< Negotiated link width. Set automatically by hardware after Link initialization. */
	uint32_t ls                           : 4;  /**< Link speed.
                                                         0x1 = The negotiated link speed: 2.5 Gbps.
                                                         0x2 = The negotiated link speed: 5.0 Gbps.
                                                         0x4 = The negotiated link speed: 8.0 Gbps. */
	uint32_t reserved_12_15               : 4;
	uint32_t lab_int_enb                  : 1;  /**< Link autonomous bandwidth interrupt enable. This bit is not applicable and is reserved for
                                                         endpoints. */
	uint32_t lbm_int_enb                  : 1;  /**< Link bandwidth management interrupt enable. This bit is not applicable and is reserved for
                                                         endpoints. */
	uint32_t hawd                         : 1;  /**< Hardware autonomous width disable (not supported). */
	uint32_t ecpm                         : 1;  /**< Enable clock power management. Hardwired to 0 if clock power management is disabled in the
                                                         link capabilities register. */
	uint32_t es                           : 1;  /**< Extended synch. */
	uint32_t ccc                          : 1;  /**< Common clock configuration. */
	uint32_t rl                           : 1;  /**< Retrain link. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t ld                           : 1;  /**< Link disable. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t rcb                          : 1;  /**< Read completion boundary (RCB). */
	uint32_t reserved_2_2                 : 1;
	uint32_t aslpc                        : 2;  /**< Active state link PM control. */
#else
	uint32_t aslpc                        : 2;
	uint32_t reserved_2_2                 : 1;
	uint32_t rcb                          : 1;
	uint32_t ld                           : 1;
	uint32_t rl                           : 1;
	uint32_t ccc                          : 1;
	uint32_t es                           : 1;
	uint32_t ecpm                         : 1;
	uint32_t hawd                         : 1;
	uint32_t lbm_int_enb                  : 1;
	uint32_t lab_int_enb                  : 1;
	uint32_t reserved_12_15               : 4;
	uint32_t ls                           : 4;
	uint32_t nlw                          : 6;
	uint32_t reserved_26_26               : 1;
	uint32_t lt                           : 1;
	uint32_t scc                          : 1;
	uint32_t dlla                         : 1;
	uint32_t lbm                          : 1;
	uint32_t lab                          : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg032_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg032 cvmx_pcieepvfx_cfg032_t;

/**
 * cvmx_pcieepvf#_cfg037
 *
 * This register contains the thirty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg037 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg037_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< Max end-end TLP prefixes.
                                                         0x1 = 1.
                                                         0x2 = 2.
                                                         0x3 = 3.
                                                         0x0 = 4. */
	uint32_t eetps                        : 1;  /**< End-end TLP prefix supported (not supported). */
	uint32_t effs                         : 1;  /**< Extended fmt field supported (not supported). */
	uint32_t obffs                        : 2;  /**< Optimized buffer flush fill (OBFF) supported (not supported). */
	uint32_t reserved_14_17               : 4;
	uint32_t tphs                         : 2;  /**< TPH completer supported (not supported). */
	uint32_t ltrs                         : 1;  /**< Latency tolerance reporting (LTR) mechanism supported (not supported). */
	uint32_t noroprpr                     : 1;  /**< No RO-enabled PR-PR passing. (This bit applies to RCs.) */
	uint32_t atom128s                     : 1;  /**< 128-bit AtomicOp supported (not supported). */
	uint32_t atom64s                      : 1;  /**< 64-bit AtomicOp supported (not supported). */
	uint32_t atom32s                      : 1;  /**< 32-bit AtomicOp supported (not supported). */
	uint32_t atom_ops                     : 1;  /**< AtomicOp routing supported (not applicable for EP). */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported. */
	uint32_t ctds                         : 1;  /**< Completion timeout disable supported. */
	uint32_t ctrs                         : 4;  /**< Completion timeout ranges supported. */
#else
	uint32_t ctrs                         : 4;
	uint32_t ctds                         : 1;
	uint32_t ari                          : 1;
	uint32_t atom_ops                     : 1;
	uint32_t atom32s                      : 1;
	uint32_t atom64s                      : 1;
	uint32_t atom128s                     : 1;
	uint32_t noroprpr                     : 1;
	uint32_t ltrs                         : 1;
	uint32_t tphs                         : 2;
	uint32_t reserved_14_17               : 4;
	uint32_t obffs                        : 2;
	uint32_t effs                         : 1;
	uint32_t eetps                        : 1;
	uint32_t meetp                        : 2;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg037_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg037 cvmx_pcieepvfx_cfg037_t;

/**
 * cvmx_pcieepvf#_cfg038
 *
 * This register contains the thirty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg038 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg038_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t eetpb                        : 1;  /**< Unsupported end-end TLP prefix blocking. */
	uint32_t obffe                        : 2;  /**< Optimized buffer flush fill (OBFF) enable (not supported). */
	uint32_t reserved_10_12               : 3;
	uint32_t id0_cp                       : 1;  /**< ID based ordering completion enable (not supported). */
	uint32_t id0_rq                       : 1;  /**< ID based ordering request enable (not supported). */
	uint32_t atom_op_eb                   : 1;  /**< AtomicOp egress blocking (not supported). */
	uint32_t atom_op                      : 1;  /**< AtomicOp requester enable (not supported). */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported (not supported). */
	uint32_t ctd                          : 1;  /**< Completion timeout disable. */
	uint32_t ctv                          : 4;  /**< Completion timeout value. Completion timeout programming is not supported. Completion
                                                         timeout is the range of 16 ms to 55 ms. */
#else
	uint32_t ctv                          : 4;
	uint32_t ctd                          : 1;
	uint32_t ari                          : 1;
	uint32_t atom_op                      : 1;
	uint32_t atom_op_eb                   : 1;
	uint32_t id0_rq                       : 1;
	uint32_t id0_cp                       : 1;
	uint32_t reserved_10_12               : 3;
	uint32_t obffe                        : 2;
	uint32_t eetpb                        : 1;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg038_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg038 cvmx_pcieepvfx_cfg038_t;

/**
 * cvmx_pcieepvf#_cfg039
 *
 * This register contains the fortieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg039 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg039_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t cls                          : 1;  /**< Crosslink supported. */
	uint32_t slsv                         : 7;  /**< Supported link speeds vector. Indicates the supported link speeds of the associated port.
                                                         For each bit, a value of 1b indicates that the corresponding link speed is supported;
                                                         otherwise, the link speed is not supported. Bit definitions are:
                                                         _ Bit <1> =  2.5 GT/s.
                                                         _ Bit <2> = 5.0 GT/s.
                                                         _ Bit <3> = 8.0 GT/s.
                                                         _ Bits <7:4> are reserved.
                                                         The reset value of this field is controlled by the value read from PEM()_CFG[MD].
                                                         _ MD is 0x0, reset to 0x1: 2.5 GHz supported.
                                                         _ MD is 0x1, reset to 0x3: 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x2, reset to 0x7: 8.0 Ghz, 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x3, reset to 0x7: 8.0 Ghz, 5.0 GHz and 2.5 GHz supported (RC Mode). */
	uint32_t reserved_0_0                 : 1;
#else
	uint32_t reserved_0_0                 : 1;
	uint32_t slsv                         : 7;
	uint32_t cls                          : 1;
	uint32_t reserved_9_31                : 23;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg039_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg039 cvmx_pcieepvfx_cfg039_t;

/**
 * cvmx_pcieepvf#_cfg040
 *
 * This register contains the forty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg040 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg040_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t cdl                          : 1;  /**< Current deemphasis level. When the link is operating at 5 GT/s speed, this bit reflects
                                                         the level of deemphasis. Encodings:
                                                         1 = -3.5 dB.
                                                         0 = -6 dB.
                                                         The value in this bit is undefined when the link is operating at 2.5 GT/s speed. */
	uint32_t reserved_13_15               : 3;
	uint32_t cde                          : 1;  /**< Compliance deemphasis. This bit sets the deemphasis level in polling. Compliance state if
                                                         the entry occurred due to the Tx compliance receive bit being 1. Encodings:
                                                         1 = -3.5 dB.
                                                         0 = -6 dB.
                                                         When the link is operating at 2.5 GT/s, the setting of this bit has no effect. */
	uint32_t csos                         : 1;  /**< Compliance SOS. When set to 1, the LTSSM is required to send SKP ordered sets periodically
                                                         in between the (modified) compliance patterns.
                                                         When the link is operating at 2.5 GT/s, the setting of this bit has no effect. */
	uint32_t emc                          : 1;  /**< Enter modified compliance. When this bit is set to 1, the device transmits a modified
                                                         compliance pattern if the LTSSM enters polling compliance state. */
	uint32_t tm                           : 3;  /**< Transmit margin. This field controls the value of the non-deemphasized voltage level at
                                                         the transmitter pins:
                                                         0x0 =  800-1200 mV for full swing 400-600 mV for half-swing.
                                                         0x1-0x2 = Values must be monotonic with a nonzero slope.
                                                         0x3 = 200-400 mV for full-swing and 100-200 mV for halfswing.
                                                         0x4-0x7 = Reserved.
                                                         This field is reset to 0x0 on entry to the LTSSM polling compliance substate. When
                                                         operating in 5.0 GT/s mode with full swing, the deemphasis ratio must be maintained within
                                                         +/- 1 dB from the specification-defined operational value either -3.5 or -6 dB. */
	uint32_t sde                          : 1;  /**< Selectable deemphasis. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t hasd                         : 1;  /**< Hardware autonomous speed disable. When asserted, the application must disable hardware
                                                         from changing the link speed for device-specific reasons other than attempting to correct
                                                         unreliable link operation by reducing link speed. Initial transition to the highest
                                                         supported common link speed is not blocked by this signal. */
	uint32_t ec                           : 1;  /**< Enter compliance. Software is permitted to force a link to enter compliance mode at the
                                                         speed indicated in the target link speed field by setting this bit to 1 in both components
                                                         on a link and then initiating a hot reset on the link. */
	uint32_t tls                          : 4;  /**< Target link speed. For downstream ports, this field sets an upper limit on link
                                                         operational speed by restricting the values advertised by the upstream component in its
                                                         training sequences:
                                                         0x1 = 2.5 Gb/s target link speed.
                                                         0x2 = 5 Gb/s target link speed.
                                                         0x4 = 8 Gb/s target link speed (not supported).
                                                         All other encodings are reserved.
                                                         If a value is written to this field that does not correspond to a speed included in the
                                                         supported link speeds field, the result is undefined.
                                                         For both upstream and downstream ports, this field is used to set the target compliance
                                                         mode speed when software is using the enter compliance bit to force a link into compliance
                                                         mode.
                                                         The reset value of this field is controlled by the value read from PEM()_CFG[MD].
                                                         _ MD is 0x0, reset to 0x1: 2.5 GHz supported.
                                                         _ MD is 0x1, reset to 0x2: 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x2, reset to 0x3: 8.0 Ghz, 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x3, reset to 0x3: 8.0 Ghz, 5.0 GHz and 2.5 GHz supported (RC Mode). */
#else
	uint32_t tls                          : 4;
	uint32_t ec                           : 1;
	uint32_t hasd                         : 1;
	uint32_t sde                          : 1;
	uint32_t tm                           : 3;
	uint32_t emc                          : 1;
	uint32_t csos                         : 1;
	uint32_t cde                          : 1;
	uint32_t reserved_13_15               : 3;
	uint32_t cdl                          : 1;
	uint32_t reserved_17_31               : 15;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg040_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg040 cvmx_pcieepvfx_cfg040_t;

/**
 * cvmx_pcieepvf#_cfg044
 *
 * This register contains the forty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg044 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg044_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixen                       : 1;  /**< MSI-X enable. If MSI-X is enabled, MSI and INTx must be disabled. */
	uint32_t funm                         : 1;  /**< Function mask.
                                                         0 = Each vectors mask bit determines whether the vector is masked or not.
                                                         1 = All vectors associated with the function are masked, regardless of their respective
                                                         per-vector mask bits. */
	uint32_t reserved_27_29               : 3;
	uint32_t msixts                       : 11; /**< MSI-X table size encoded as (table size - 1). */
	uint32_t ncp                          : 8;  /**< Next capability pointer. Points to the PCI power management capability registers. */
	uint32_t msixcid                      : 8;  /**< MSI-X Capability ID. */
#else
	uint32_t msixcid                      : 8;
	uint32_t ncp                          : 8;
	uint32_t msixts                       : 11;
	uint32_t reserved_27_29               : 3;
	uint32_t funm                         : 1;
	uint32_t msixen                       : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg044_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg044 cvmx_pcieepvfx_cfg044_t;

/**
 * cvmx_pcieepvf#_cfg045
 *
 * This register contains the forty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg045 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg045_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixtoffs                    : 29; /**< MSI-X table offset register. Base address of the MSI-X Table, as an offset from the base
                                                         address of the BAR indicated by the Table BIR bits. */
	uint32_t msixtbir                     : 3;  /**< MSI-X table BAR indicator register (BIR). Indicates which BAR is used to map the MSI-X
                                                         table into memory space. */
#else
	uint32_t msixtbir                     : 3;
	uint32_t msixtoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg045_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg045 cvmx_pcieepvfx_cfg045_t;

/**
 * cvmx_pcieepvf#_cfg046
 *
 * This register contains the forty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg046 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg046_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixpoffs                    : 29; /**< MSI-X table offset register. Base address of the MSI-X PBA, as an offset from the base
                                                         address of the BAR indicated by the table PBA bits. */
	uint32_t msixpbir                     : 3;  /**< MSI-X PBA BAR indicator register (BIR). Indicates which BAR is used to map the MSI-X
                                                         pending bit array into memory space.
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t msixpbir                     : 3;
	uint32_t msixpoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg046_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg046 cvmx_pcieepvfx_cfg046_t;

/**
 * cvmx_pcieepvf#_cfg048
 *
 * This register contains the forty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg048 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg048_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmes                         : 5;  /**< PME_Support.
                                                         _ Bit 11: If set, PME Messages can be generated from D0.
                                                         _ Bit 12: If set, PME Messages can be generated from D1.
                                                         _ Bit 13: If set, PME Messages can be generated from D2.
                                                         _ Bit 14: If set, PME Messages can be generated from D3hot.
                                                         _ Bit 15: If set, PME Messages can be generated from D3cold. */
	uint32_t d2s                          : 1;  /**< D2 support. */
	uint32_t d1s                          : 1;  /**< D1 support. */
	uint32_t auxc                         : 3;  /**< AUX current. */
	uint32_t dsi                          : 1;  /**< Device specific initialization (DSI). */
	uint32_t reserved_20_20               : 1;
	uint32_t pme_clock                    : 1;  /**< PME clock, hardwired to 0. */
	uint32_t pmsv                         : 3;  /**< Power management specification version. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. */
	uint32_t pmcid                        : 8;  /**< Power management capability ID. */
#else
	uint32_t pmcid                        : 8;
	uint32_t ncp                          : 8;
	uint32_t pmsv                         : 3;
	uint32_t pme_clock                    : 1;
	uint32_t reserved_20_20               : 1;
	uint32_t dsi                          : 1;
	uint32_t auxc                         : 3;
	uint32_t d1s                          : 1;
	uint32_t d2s                          : 1;
	uint32_t pmes                         : 5;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg048_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg048 cvmx_pcieepvfx_cfg048_t;

/**
 * cvmx_pcieepvf#_cfg049
 *
 * This register contains the fiftieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg049 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg049_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmdia                        : 8;  /**< Data register for additional information (not supported) */
	uint32_t bpccee                       : 1;  /**< Bus power/clock control enable, hardwired to 0. */
	uint32_t bd3h                         : 1;  /**< B2/B3 support, hardwired to 0. */
	uint32_t reserved_16_21               : 6;
	uint32_t pmess                        : 1;  /**< PME status. Indicates whether or not a previously enabled PME event occurred. */
	uint32_t pmedsia                      : 2;  /**< Data scale (not supported). */
	uint32_t pmds                         : 4;  /**< Data select (not supported). */
	uint32_t pmeens                       : 1;  /**< PME enable. A value of 1 indicates that the device is enabled to generate PME. */
	uint32_t reserved_4_7                 : 4;
	uint32_t nsr                          : 1;  /**< No soft reset. */
	uint32_t reserved_2_2                 : 1;
	uint32_t ps                           : 2;  /**< Power state. Controls the device power state:
                                                         0x0 = D0.
                                                         0x1 = D1.
                                                         0x2 = D2.
                                                         0x3 = D3.
                                                         The written value is ignored if the specific state is not supported. */
#else
	uint32_t ps                           : 2;
	uint32_t reserved_2_2                 : 1;
	uint32_t nsr                          : 1;
	uint32_t reserved_4_7                 : 4;
	uint32_t pmeens                       : 1;
	uint32_t pmds                         : 4;
	uint32_t pmedsia                      : 2;
	uint32_t pmess                        : 1;
	uint32_t reserved_16_21               : 6;
	uint32_t bd3h                         : 1;
	uint32_t bpccee                       : 1;
	uint32_t pmdia                        : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg049_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg049 cvmx_pcieepvfx_cfg049_t;

/**
 * cvmx_pcieepvf#_cfg064
 *
 * This register contains the sixty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg064 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg064_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. Points to the ARI capabilities by default. */
	uint32_t cv                           : 4;  /**< Capability version */
	uint32_t pcieec                       : 16; /**< PCI Express extended capability */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg064_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg064 cvmx_pcieepvfx_cfg064_t;

/**
 * cvmx_pcieepvf#_cfg082
 *
 * This register contains the eighty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg082 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg082_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t ariid                        : 16; /**< PCIE Express extended capability */
#else
	uint32_t ariid                        : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg082_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg082 cvmx_pcieepvfx_cfg082_t;

/**
 * cvmx_pcieepvf#_cfg083
 *
 * This register contains the eighty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg083 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg083_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t fg                           : 4;  /**< Function group. */
	uint32_t reserved_18_19               : 2;
	uint32_t acsfge                       : 1;  /**< ACS function groups enable (A). */
	uint32_t mfvcfge                      : 1;  /**< MFVC function groups enable (M). */
	uint32_t nfn                          : 8;  /**< Next function number. */
	uint32_t reserved_2_7                 : 6;
	uint32_t acsfgc                       : 1;  /**< ACS function groups capability. */
	uint32_t mfvcfgc                      : 1;  /**< MFVC function groups capability. */
#else
	uint32_t mfvcfgc                      : 1;
	uint32_t acsfgc                       : 1;
	uint32_t reserved_2_7                 : 6;
	uint32_t nfn                          : 8;
	uint32_t mfvcfge                      : 1;
	uint32_t acsfge                       : 1;
	uint32_t reserved_18_19               : 2;
	uint32_t fg                           : 4;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg083_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg083 cvmx_pcieepvfx_cfg083_t;

/**
 * cvmx_pcieepvf#_cfg448
 *
 * This register contains the four hundred forty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg448 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg448_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rtl                          : 16; /**< Replay time limit. The replay timer expires when it reaches this limit. The PCI Express
                                                         bus initiates a replay upon reception of a nak or when the replay timer expires. This
                                                         value is set correctly by the hardware out of reset or when the negotiated link width or
                                                         payload size changes. If the user changes this value through a CSR write or by an EEPROM
                                                         load, they should refer to the PCIe specification for the correct value. */
	uint32_t rtltl                        : 16; /**< Round trip latency time limit. The ack/nak latency timer expires when it reaches this
                                                         limit. This value is set correctly by the hardware out of reset or when the negotiated
                                                         link width or payload size changes. If the user changes this value through a CSR write or
                                                         by an EEPROM load, they should refer to the PCIe specification for the correct value. */
#else
	uint32_t rtltl                        : 16;
	uint32_t rtl                          : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg448_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg448 cvmx_pcieepvfx_cfg448_t;

/**
 * cvmx_pcieepvf#_cfg449
 *
 * This register contains the four hundred fiftieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg449 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg449_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t omr                          : 32; /**< Other message register. This register can be used for either of the following purposes:
                                                         * To send a specific PCI Express message, the application writes the payload of the
                                                         message into this register, then sets bit 0 of the port link control register to send the
                                                         message.
                                                         * To store a corruption pattern for corrupting the LCRC on all TLPs, the application
                                                         places a 32-bit corruption pattern into this register and enables this function by setting
                                                         bit 25 of the port link control register. When enabled, the transmit LCRC result is XORed
                                                         with this pattern before inserting it into the packet. */
#else
	uint32_t omr                          : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg449_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg449 cvmx_pcieepvfx_cfg449_t;

/**
 * cvmx_pcieepvf#_cfg450
 *
 * This register contains the four hundred fifty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg450 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg450_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lpec                         : 8;  /**< Low power entrance count. The power management state waits this many clock cycles for the
                                                         associated completion of a CfgWr to PCIEEP()_CFG017 register, power state (PS) field
                                                         register
                                                         to go low-power. This register is intended for applications that do not let the PCI
                                                         Express bus handle a completion for configuration request to the power management control
                                                         and status (PCIEP()_CFG017) register. */
	uint32_t reserved_22_23               : 2;
	uint32_t link_state                   : 6;  /**< Link state. The link state that the PCI Express bus is forced to when bit 15 (force link)
                                                         is set. State encoding:
                                                         0x0 = DETECT_QUIET.
                                                         0x1 = DETECT_ACT.
                                                         0x2 = POLL_ACTIVE.
                                                         0x3 = POLL_COMPLIANCE.
                                                         0x4 = POLL_CONFIG.
                                                         0x5 = PRE_DETECT_QUIET.
                                                         0x6 = DETECT_WAIT.
                                                         0x7 = CFG_LINKWD_START.
                                                         0x8 = CFG_LINKWD_ACEPT.
                                                         0x9 = CFG_LANENUM_WAIT.
                                                         0xA = CFG_LANENUM_ACEPT.
                                                         0xB = CFG_COMPLETE.
                                                         0xC = CFG_IDLE.
                                                         0xD = RCVRY_LOCK.
                                                         0xE = RCVRY_SPEED.
                                                         0xF = RCVRY_RCVRCFG.
                                                         0x10 = RCVRY_IDLE.
                                                         0x11 = L0.
                                                         0x12 = L0S.
                                                         0x13 = L123_SEND_EIDLE.
                                                         0x14 = L1_IDLE.
                                                         0x15 = L2_IDLE.
                                                         0x16 = L2_WAKE.
                                                         0x17 = DISABLED_ENTRY.
                                                         0x18 = DISABLED_IDLE.
                                                         0x19 = DISABLED.
                                                         0x1A = LPBK_ENTRY.
                                                         0x1B = LPBK_ACTIVE.
                                                         0x1C = LPBK_EXIT.
                                                         0x1D = LPBK_EXIT_TIMEOUT.
                                                         0x1E = HOT_RESET_ENTRY.
                                                         0x1F = HOT_RESET. */
	uint32_t force_link                   : 1;  /**< Force link. Forces the link to the state specified by the LINK_STATE field. The force link
                                                         pulse triggers link renegotiation.
                                                         As the force link is a pulse, writing a 1 to it does trigger the forced link state event,
                                                         even though reading it always returns a 0. */
	uint32_t reserved_8_14                : 7;
	uint32_t link_num                     : 8;  /**< Link number. Not used for endpoint. */
#else
	uint32_t link_num                     : 8;
	uint32_t reserved_8_14                : 7;
	uint32_t force_link                   : 1;
	uint32_t link_state                   : 6;
	uint32_t reserved_22_23               : 2;
	uint32_t lpec                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg450_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg450 cvmx_pcieepvfx_cfg450_t;

/**
 * cvmx_pcieepvf#_cfg451
 *
 * This register contains the four hundred fifty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg451 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg451_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t easpml1                      : 1;  /**< Enter ASPM L1 without receive in L0s. Allow core to enter ASPM L1 even when link partner
                                                         did not go to L0s (receive is not in L0s). When not set, core goes to ASPM L1 only after
                                                         idle period, during which both receive and transmit are in L0s. */
	uint32_t l1el                         : 3;  /**< L1 entrance latency. Values correspond to:
                                                         0x0 = 1 ms.
                                                         0x1 = 2 ms.
                                                         0x2 = 4 ms.
                                                         0x3 = 8 ms.
                                                         0x4 = 16 ms.
                                                         0x5 = 32 ms.
                                                         0x6 or 0x7 = 64 ms. */
	uint32_t l0el                         : 3;  /**< L0s entrance latency. Values correspond to:
                                                         0x0 = 1 ms.
                                                         0x1 = 2 ms.
                                                         0x2 = 3 ms.
                                                         0x3 = 4 ms.
                                                         0x4 = 5 ms.
                                                         0x5 = 6 ms.
                                                         0x6 or 0x7 = 7 ms. */
	uint32_t n_fts_cc                     : 8;  /**< N_FTS when common clock is used.
                                                         The number of fast training sequence (FTS) ordered sets to be transmitted when
                                                         transitioning from L0s to L0. The maximum number of FTS ordered sets that a component can
                                                         request is 255.
                                                         A value of zero is not supported; a value of zero can cause the LTSSM to go into the
                                                         recovery state when exiting from L0s. */
	uint32_t n_fts                        : 8;  /**< N_FTS. The number of fast training sequence (FTS) ordered sets to be transmitted when
                                                         transitioning from L0s to L0. The maximum number of FTS ordered sets that a component can
                                                         request is 255.
                                                         A value of zero is not supported; a value of zero can cause the LTSSM to go into the
                                                         recovery state when exiting from L0s. */
	uint32_t ack_freq                     : 8;  /**< Ack frequency. The number of pending Acks specified here (up to 255) before sending an Ack. */
#else
	uint32_t ack_freq                     : 8;
	uint32_t n_fts                        : 8;
	uint32_t n_fts_cc                     : 8;
	uint32_t l0el                         : 3;
	uint32_t l1el                         : 3;
	uint32_t easpml1                      : 1;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg451_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg451 cvmx_pcieepvfx_cfg451_t;

/**
 * cvmx_pcieepvf#_cfg452
 *
 * This register contains the four hundred fifty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg452 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg452_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t lme                          : 6;  /**< Link mode enable set as follows:
                                                         0x1 = x1.
                                                         0x3 = x2.
                                                         0x7 = x4.
                                                         0xF = x8.
                                                         0x1F = x16 (not supported).
                                                         0x3F = x32 (not supported).
                                                         This field indicates the maximum number of lanes supported by the PCIe port. The value can
                                                         be set less than 0xF to limit the number of lanes that the PCIe will attempt to use. If
                                                         the value of 0xF set by the hardware is not desired, this field can be programmed to a
                                                         smaller value (i.e. EEPROM). See also PCIEEP()_CFG031[MLW].
                                                         The value of this field does not indicate the number of lanes in use by the PCIe. This
                                                         field sets the maximum number of lanes in the PCIe core that could be used. As per the
                                                         PCIe specification, the PCIe core can negotiate a smaller link width, so all of x8, x4,
                                                         x2, and x1 are supported when
                                                         LME = 0xF, for example. */
	uint32_t reserved_12_15               : 4;
	uint32_t link_rate                    : 4;  /**< Reserved. */
	uint32_t flm                          : 1;  /**< Fast link mode. Sets all internal timers to fast mode for simulation purposes. If during
                                                         an EEPROM load, the first word loaded is 0xFFFFFFFF, the EEPROM load is terminated and
                                                         this bit is set. */
	uint32_t reserved_6_6                 : 1;
	uint32_t dllle                        : 1;  /**< DLL link enable. Enables link initialization. If DLL link enable = 0, the PCI Express bus
                                                         does not transmit InitFC DLLPs and does not establish a link. */
	uint32_t reserved_4_4                 : 1;
	uint32_t ra                           : 1;  /**< Reset assert. Triggers a recovery and forces the LTSSM to the hot reset state (downstream
                                                         port only). */
	uint32_t le                           : 1;  /**< Loopback enable. Initiate loopback mode as a master. On a 0->1 transition, the PCIe core
                                                         sends TS ordered sets with the loopback bit set to cause the link partner to enter into
                                                         loopback mode as a slave. Normal transmission is not possible when LE=1. To exit loopback
                                                         mode, take the link through a reset sequence. */
	uint32_t sd                           : 1;  /**< Scramble disable. Setting this bit turns off data scrambling. */
	uint32_t omr                          : 1;  /**< Other message request. When software writes a 1 to this bit, the PCI Express bus transmits
                                                         the message contained in the other message register. */
#else
	uint32_t omr                          : 1;
	uint32_t sd                           : 1;
	uint32_t le                           : 1;
	uint32_t ra                           : 1;
	uint32_t reserved_4_4                 : 1;
	uint32_t dllle                        : 1;
	uint32_t reserved_6_6                 : 1;
	uint32_t flm                          : 1;
	uint32_t link_rate                    : 4;
	uint32_t reserved_12_15               : 4;
	uint32_t lme                          : 6;
	uint32_t reserved_22_31               : 10;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg452_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg452 cvmx_pcieepvfx_cfg452_t;

/**
 * cvmx_pcieepvf#_cfg453
 *
 * This register contains the four hundred fifty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg453 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg453_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dlld                         : 1;  /**< Disable lane-to-lane deskew. Disables the internal lane-to-lane deskew logic. */
	uint32_t reserved_26_30               : 5;
	uint32_t ack_nak                      : 1;  /**< Ack/Nak disable. Prevents the PCI Express bus from sending Ack and Nak DLLPs. */
	uint32_t fcd                          : 1;  /**< Flow control disable. Prevents the PCI Express bus from sending FC DLLPs. */
	uint32_t ilst                         : 24; /**< Insert lane skew for transmit. Causes skew between lanes for test purposes. There are
                                                         three bits per lane. The value is in units of one symbol time. For example, the value 010b
                                                         for a lane forces a skew of two symbol times for that lane. The maximum skew value for any
                                                         lane is 5 symbol times. */
#else
	uint32_t ilst                         : 24;
	uint32_t fcd                          : 1;
	uint32_t ack_nak                      : 1;
	uint32_t reserved_26_30               : 5;
	uint32_t dlld                         : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg453_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg453 cvmx_pcieepvfx_cfg453_t;

/**
 * cvmx_pcieepvf#_cfg454
 *
 * This register contains the four hundred fifty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg454 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg454_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t tmfcwt                       : 5;  /**< Used to be 'timer modifier for flow control watchdog timer.' This field is no longer used.
                                                         and has moved to the queue status register -- PCIEEP()_CFG463. This field remains to
                                                         prevent software from breaking. */
	uint32_t tmanlt                       : 5;  /**< Timer modifier for Ack/Nak latency timer. Increases the timer value for the Ack/Nak
                                                         latency timer, in increments of 64 clock cycles. */
	uint32_t tmrt                         : 5;  /**< Timer modifier for replay timer. Increases the timer value for the replay timer, in
                                                         increments of 64 clock cycles. */
	uint32_t reserved_8_13                : 6;
	uint32_t mfuncn                       : 8;  /**< Max number of functions supported. */
#else
	uint32_t mfuncn                       : 8;
	uint32_t reserved_8_13                : 6;
	uint32_t tmrt                         : 5;
	uint32_t tmanlt                       : 5;
	uint32_t tmfcwt                       : 5;
	uint32_t reserved_29_31               : 3;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg454_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg454 cvmx_pcieepvfx_cfg454_t;

/**
 * cvmx_pcieepvf#_cfg455
 *
 * This register contains the four hundred fifty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg455 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg455_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t m_cfg0_filt                  : 1;  /**< Mask filtering of received configuration requests (RC mode only). */
	uint32_t m_io_filt                    : 1;  /**< Mask filtering of received I/O requests (RC mode only). */
	uint32_t msg_ctrl                     : 1;  /**< Message control. The application must not change this field. */
	uint32_t m_cpl_ecrc_filt              : 1;  /**< Mask ECRC error filtering for completions. */
	uint32_t m_ecrc_filt                  : 1;  /**< Mask ECRC error filtering. */
	uint32_t m_cpl_len_err                : 1;  /**< Mask length mismatch error for received completions. */
	uint32_t m_cpl_attr_err               : 1;  /**< Mask attributes mismatch error for received completions. */
	uint32_t m_cpl_tc_err                 : 1;  /**< Mask traffic class mismatch error for received completions. */
	uint32_t m_cpl_fun_err                : 1;  /**< Mask function mismatch error for received completions. */
	uint32_t m_cpl_rid_err                : 1;  /**< Mask requester ID mismatch error for received completions. */
	uint32_t m_cpl_tag_err                : 1;  /**< Mask tag error rules for received completions. */
	uint32_t m_lk_filt                    : 1;  /**< Mask locked request filtering. */
	uint32_t m_cfg1_filt                  : 1;  /**< Mask type 1 configuration request filtering. */
	uint32_t m_bar_match                  : 1;  /**< Mask BAR match filtering. */
	uint32_t m_pois_filt                  : 1;  /**< Mask poisoned TLP filtering. */
	uint32_t m_fun                        : 1;  /**< Mask function. */
	uint32_t dfcwt                        : 1;  /**< Disable FC watchdog timer. */
	uint32_t reserved_11_14               : 4;
	uint32_t skpiv                        : 11; /**< SKP interval value. */
#else
	uint32_t skpiv                        : 11;
	uint32_t reserved_11_14               : 4;
	uint32_t dfcwt                        : 1;
	uint32_t m_fun                        : 1;
	uint32_t m_pois_filt                  : 1;
	uint32_t m_bar_match                  : 1;
	uint32_t m_cfg1_filt                  : 1;
	uint32_t m_lk_filt                    : 1;
	uint32_t m_cpl_tag_err                : 1;
	uint32_t m_cpl_rid_err                : 1;
	uint32_t m_cpl_fun_err                : 1;
	uint32_t m_cpl_tc_err                 : 1;
	uint32_t m_cpl_attr_err               : 1;
	uint32_t m_cpl_len_err                : 1;
	uint32_t m_ecrc_filt                  : 1;
	uint32_t m_cpl_ecrc_filt              : 1;
	uint32_t msg_ctrl                     : 1;
	uint32_t m_io_filt                    : 1;
	uint32_t m_cfg0_filt                  : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg455_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg455 cvmx_pcieepvfx_cfg455_t;

/**
 * cvmx_pcieepvf#_cfg456
 *
 * This register contains the four hundred fifty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg456 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg456_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t m_handle_flush               : 1;  /**< Mask core filter to handle flush request. */
	uint32_t m_dabort_4ucpl               : 1;  /**< Mask DLLP abort for unexpected CPL. */
	uint32_t m_vend1_drp                  : 1;  /**< Mask vendor MSG type 1 dropped silently. */
	uint32_t m_vend0_drp                  : 1;  /**< Mask vendor MSG type 0 dropped with UR error reporting. */
#else
	uint32_t m_vend0_drp                  : 1;
	uint32_t m_vend1_drp                  : 1;
	uint32_t m_dabort_4ucpl               : 1;
	uint32_t m_handle_flush               : 1;
	uint32_t reserved_4_31                : 28;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg456_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg456 cvmx_pcieepvfx_cfg456_t;

/**
 * cvmx_pcieepvf#_cfg458
 *
 * This register contains the four hundred fifty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg458 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg458_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_l32                 : 32; /**< Debug info lower 32 bits. */
#else
	uint32_t dbg_info_l32                 : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg458_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg458 cvmx_pcieepvfx_cfg458_t;

/**
 * cvmx_pcieepvf#_cfg459
 *
 * This register contains the four hundred sixtieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg459 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg459_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_u32                 : 32; /**< Debug info upper 32 bits. */
#else
	uint32_t dbg_info_u32                 : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg459_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg459 cvmx_pcieepvfx_cfg459_t;

/**
 * cvmx_pcieepvf#_cfg460
 *
 * This register contains the four hundred sixty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg460 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg460_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tphfcc                       : 8;  /**< Transmit posted header FC Credits. The posted header credits advertised by the receiver at
                                                         the other end of the link, updated with each UpdateFC DLLP. */
	uint32_t tpdfcc                       : 12; /**< Transmit posted data FC credits. The posted data credits advertised by the receiver at the
                                                         other end of the link, updated with each UpdateFC DLLP. */
#else
	uint32_t tpdfcc                       : 12;
	uint32_t tphfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg460_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg460 cvmx_pcieepvfx_cfg460_t;

/**
 * cvmx_pcieepvf#_cfg461
 *
 * This register contains the four hundred sixty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg461 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg461_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit nonposted header FC credits. The nonposted header credits advertised by the
                                                         receiver at the other end of the link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit nonposted data FC credits. The nonposted data credits advertised by the receiver
                                                         at the other end of the link, updated with each UpdateFC DLLP. */
#else
	uint32_t tcdfcc                       : 12;
	uint32_t tchfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg461_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg461 cvmx_pcieepvfx_cfg461_t;

/**
 * cvmx_pcieepvf#_cfg462
 *
 * This register contains the four hundred sixty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg462 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg462_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit completion header FC credits. The completion header credits advertised by the
                                                         receiver at the other end of the link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit completion data FC credits. The completion data credits advertised by the
                                                         receiver at the other end of the link, updated with each UpdateFC DLLP. */
#else
	uint32_t tcdfcc                       : 12;
	uint32_t tchfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg462_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg462 cvmx_pcieepvfx_cfg462_t;

/**
 * cvmx_pcieepvf#_cfg463
 *
 * This register contains the four hundred sixty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg463 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg463_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t fcltoe                       : 1;  /**< FC latency timer override enable. When this bit is set, the value from
                                                         PCIEEPVF()_CFG463[FCLTOV] will override the FC latency timer value that the core
                                                         calculates according to the PCIe specification. */
	uint32_t reserved_29_30               : 2;
	uint32_t fcltov                       : 13; /**< FC latency timer override value. When you set PCIEEPVF()_CFG463[FCLTOE], the value in
                                                         this
                                                         field will override the FC latency timer value that the core calculates according to the
                                                         PCIe specification. */
	uint32_t reserved_3_15                : 13;
	uint32_t rqne                         : 1;  /**< Received queue not empty. Indicates there is data in one or more of the receive buffers. */
	uint32_t trbne                        : 1;  /**< Transmit retry buffer not empty. Indicates that there is data in the transmit retry buffer. */
	uint32_t rtlpfccnr                    : 1;  /**< Received TLP FC credits not returned. Indicates that the PCI Express bus has sent a TLP
                                                         but has not yet received an UpdateFC DLLP indicating that the credits for that TLP have
                                                         been restored by the receiver at the other end of the link. */
#else
	uint32_t rtlpfccnr                    : 1;
	uint32_t trbne                        : 1;
	uint32_t rqne                         : 1;
	uint32_t reserved_3_15                : 13;
	uint32_t fcltov                       : 13;
	uint32_t reserved_29_30               : 2;
	uint32_t fcltoe                       : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg463_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg463 cvmx_pcieepvfx_cfg463_t;

/**
 * cvmx_pcieepvf#_cfg464
 *
 * This register contains the four hundred sixty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg464 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg464_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc3                      : 8;  /**< WRR Weight for VC3. */
	uint32_t wrr_vc2                      : 8;  /**< WRR Weight for VC2. */
	uint32_t wrr_vc1                      : 8;  /**< WRR Weight for VC1. */
	uint32_t wrr_vc0                      : 8;  /**< WRR Weight for VC0. */
#else
	uint32_t wrr_vc0                      : 8;
	uint32_t wrr_vc1                      : 8;
	uint32_t wrr_vc2                      : 8;
	uint32_t wrr_vc3                      : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg464_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg464 cvmx_pcieepvfx_cfg464_t;

/**
 * cvmx_pcieepvf#_cfg465
 *
 * This register contains the four hundred sixty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg465 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg465_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc7                      : 8;  /**< WRR Weight for VC7. */
	uint32_t wrr_vc6                      : 8;  /**< WRR Weight for VC6. */
	uint32_t wrr_vc5                      : 8;  /**< WRR Weight for VC5. */
	uint32_t wrr_vc4                      : 8;  /**< WRR Weight for VC4. */
#else
	uint32_t wrr_vc4                      : 8;
	uint32_t wrr_vc5                      : 8;
	uint32_t wrr_vc6                      : 8;
	uint32_t wrr_vc7                      : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg465_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg465 cvmx_pcieepvfx_cfg465_t;

/**
 * cvmx_pcieepvf#_cfg466
 *
 * This register contains the four hundred sixty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg466 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg466_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rx_queue_order               : 1;  /**< VC ordering for receive queues. Determines the VC ordering rule for the receive queues,
                                                         used only in the segmented-buffer configuration, writable through PEM()_CFG_WR:
                                                         0 = Round robin.
                                                         1 = Strict ordering, higher numbered VCs have higher priority. */
	uint32_t type_ordering                : 1;  /**< TLP type ordering for VC0. Determines the TLP type ordering rule for VC0 receive queues,
                                                         used only in the segmented-buffer configuration:
                                                         0 = Strict ordering for received TLPs: Posted, then Completion, then Nonposted.
                                                         1 = Ordering of received TLPs follows the rules in PCI Express Base Specification */
	uint32_t reserved_24_29               : 6;
	uint32_t queue_mode                   : 3;  /**< VC0 posted TLP queue mode. The operating mode of the posted receive queue for VC0, used
                                                         only in the segmented-buffer configuration, writable through PEM()_CFG_WR.
                                                         However, the application must not change this field.
                                                         Only one bit can be set at a time:
                                                         _ Bit 23 = Bypass.
                                                         _ Bit 22 = Cut-through.
                                                         _ Bit 21 = Store-and-forward. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 posted header credits. The number of initial posted header credits for VC0, used for
                                                         all receive queue buffer configurations. */
	uint32_t data_credits                 : 12; /**< VC0 posted data credits. The number of initial posted data credits for VC0, used for all
                                                         receive queue buffer configurations. */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_29               : 6;
	uint32_t type_ordering                : 1;
	uint32_t rx_queue_order               : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg466_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg466 cvmx_pcieepvfx_cfg466_t;

/**
 * cvmx_pcieepvf#_cfg467
 *
 * This register contains the four hundred sixty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg467 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg467_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< VC0 nonposted TLP queue mode. The operating mode of the nonposted receive queue for VC0,
                                                         used only in the segmented-buffer configuration. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field.
                                                         Only one bit can be set at a time:
                                                         _ Bit 23 = Bypass.
                                                         _ Bit 22 = Cut-through.
                                                         _ Bit 21 = Store-and-forward. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 nonposted header credits. The number of initial nonposted header credits for VC0, used
                                                         for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 non-posted data credits. The number of initial nonposted data credits for VC0, used
                                                         for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg467_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg467 cvmx_pcieepvfx_cfg467_t;

/**
 * cvmx_pcieepvf#_cfg468
 *
 * This register contains the four hundred sixty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg468 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg468_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< VC0 completion TLP queue mode. The operating mode of the completion receive queue for VC0,
                                                         used only in the segmented-buffer configuration.
                                                         Only one bit can be set at a time:
                                                         Bit 23 = Bypass
                                                         Bit 22 = Cut-through
                                                         Bit 21 = Store-and-forward
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 completion header credits. The number of initial completion header credits for VC0,
                                                         used for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 completion data credits. The number of initial completion data credits for VC0, used
                                                         for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg468_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg468 cvmx_pcieepvfx_cfg468_t;

/**
 * cvmx_pcieepvf#_cfg490
 *
 * This register contains the four hundred ninety-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg490 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg490_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 posted header queue depth. Sets the number of entries in the posted header queue for
                                                         VC0 when using the segmented-buffer configuration. */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 posted data queue depth. Sets the number of entries in the posted data queue for VC0
                                                         when using the segmented-buffer configuration. */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg490_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg490 cvmx_pcieepvfx_cfg490_t;

/**
 * cvmx_pcieepvf#_cfg491
 *
 * This register contains the four hundred ninety-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg491 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg491_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 nonposted header queue depth. Sets the number of entries in the nonposted header queue
                                                         for VC0 when using the segmented-buffer configuration. */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 nonposted data queue depth. Sets the number of entries in the nonposted data queue for
                                                         VC0 when using the segmented-buffer configuration. */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg491_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg491 cvmx_pcieepvfx_cfg491_t;

/**
 * cvmx_pcieepvf#_cfg492
 *
 * This register contains the four hundred ninety-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg492 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg492_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 completion header queue depth. Sets the number of entries in the completion header
                                                         queue for VC0 when using the segmented-buffer configuration. */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 completion data queue depth. Sets the number of entries in the completion data queue
                                                         for VC0 when using the segmented-buffer configuration. */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg492_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg492 cvmx_pcieepvfx_cfg492_t;

/**
 * cvmx_pcieepvf#_cfg515
 *
 * This register contains the five hundred sixteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg515 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg515_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t s_d_e                        : 1;  /**< SEL_DE_EMPHASIS. Used to set the deemphasis level for upstream ports. */
	uint32_t ctcrb                        : 1;  /**< Config Tx compliance receive bit. When set to 1, signals LTSSM to transmit TS ordered sets
                                                         with the compliance receive bit assert (equal to 1). */
	uint32_t cpyts                        : 1;  /**< Config PHY Tx swing. Indicates the voltage level that the PHY should drive. When set to 1,
                                                         indicates full swing. When set to 0, indicates low swing. */
	uint32_t dsc                          : 1;  /**< Directed speed change. A write of 1 initiates a speed change; always reads as zero. */
	uint32_t le                           : 9;  /**< Lane enable. Indicates the number of lanes to check for exit from electrical idle in
                                                         Polling.Active and Polling.Compliance. 0x1 = x1, 0x2 = x2, etc. Used to limit the maximum
                                                         link width to ignore broken lanes that detect a receiver, but will not exit electrical
                                                         idle and would otherwise prevent a valid link from being configured. */
	uint32_t n_fts                        : 8;  /**< N_FTS. Sets the number of fast training sequences (N_FTS) that the core advertises as its
                                                         N_FTS during GEN2 Link training. This value is used to inform the link partner about the
                                                         PHY's ability to recover synchronization after a low power state.
                                                         Do not set N_FTS to zero; doing so can cause the LTSSM to go into the recovery state when
                                                         exiting from L0s. */
#else
	uint32_t n_fts                        : 8;
	uint32_t le                           : 9;
	uint32_t dsc                          : 1;
	uint32_t cpyts                        : 1;
	uint32_t ctcrb                        : 1;
	uint32_t s_d_e                        : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg515_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg515 cvmx_pcieepvfx_cfg515_t;

/**
 * cvmx_pcieepvf#_cfg516
 *
 * This register contains the five hundred seventeenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg516 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg516_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_stat                     : 32; /**< PHY status. */
#else
	uint32_t phy_stat                     : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg516_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg516 cvmx_pcieepvfx_cfg516_t;

/**
 * cvmx_pcieepvf#_cfg517
 *
 * This register contains the five hundred eighteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg517 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg517_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_ctrl                     : 32; /**< PHY control. */
#else
	uint32_t phy_ctrl                     : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg517_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg517 cvmx_pcieepvfx_cfg517_t;

#endif
