/*
 * Code to process dynamic relocations in the kernel.
 *
 * Copyright 2008 Paul Mackerras, IBM Corp.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include <asm/ppc_asm.h>

RELA = 7
RELASZ = 8
RELAENT = 9

STN_UNDEF = 0

R_PPC64_ADDR16_LO = 4
R_PPC64_ADDR16_HI = 5
R_PPC64_RELATIVE = 22
R_PPC64_ADDR16_HIGHER = 39
R_PPC64_ADDR16_HIGHEST = 41

/*
 * r3 = desired final address of kernel
 */
_GLOBAL(relocate)
	mflr	r0
	bcl	20,31,$+4
0:	mflr	r12		/* r12 has runtime addr of label 0 */
	mtlr	r0
	ld	r11,(p_dyn - 0b)(r12)
	add	r11,r11,r12	/* r11 has runtime addr of .dynamic section */
	ld	r9,(p_rela - 0b)(r12)
	add	r9,r9,r12	/* r9 has runtime addr of .rela.dyn section */
	ld	r10,(p_st - 0b)(r12)
	add	r10,r10,r12	/* r10 has runtime addr of _stext */
	ld	r5,(p_sym - 0b)(r12)
	add	r5,r5,r12	/* r5 has runtime addr of .dynsym section */

	/*
	 * Scan the dynamic section for the RELA and RELACOUNT entries.
	 */
	li	r7,0
	li	r8,0
	li	r4,0
1:	ld	r6,0(r11)	/* get tag */
	cmpdi	r6,0
	beq	4f		/* end of list */
	cmpdi	r6,RELA
	bne	relasz
	ld	r7,8(r11)	/* get RELA pointer in r7 */
	b	3f
relasz:	cmpdi	r6,RELASZ
	bne	relaent
	ld	r8,8(r11)	/* get RELASZ in r8 */
	b	3f
relaent:cmpdi	r6,RELAENT
	bne	3f
	ld	r4,8(r11)	/* get RELAENT value in r4 */
3:	addi	r11,r11,16
	b	1b
4:	cmpdi	r7,0		/* check we have both RELA and RELACOUNT */
	cmpdi	cr1,r8,0
	cmpdi	cr5,r4,0
	beq	6f
	beq	cr1,6f
	beq	cr5,6f
	divd	r8,r8,r4	/* RELA count */

	/*
	 * Work out linktime address of _stext and hence the
	 * relocation offset to be applied.
	 * cur_offset [r7] = rela.run [r9] - rela.link [r7]
	 * _stext.link [r10] = _stext.run [r10] - cur_offset [r7]
	 * final_offset [r3] = _stext.final [r3] - _stext.link [r10]
	 */
	subf	r7,r7,r9	/* cur_offset */
	subf	r10,r7,r10
	subf	r3,r10,r3	/* final_offset */

	/*
	 * Run through the list of relocations and process the
	 * R_PPC64_RELATIVE ones.
	 */
	mtctr	r8
5:	lwz	r0,12(9)	/* ELF64_R_TYPE(reloc->r_info) */
	lwz	r4,8(r9)	/* ELF64_R_SYM(reloc->r_info) */
	cmpwi	r4,STN_UNDEF	/* sym == STN_UNDEF ? */
	beq	get_type
	/* Find the value of the symbol at index(r4) */
	mulli	r4,r4,24	/* r4 = r4 * sizeof(ELF64_Sym) */
	add	r4,r4,r5	/* r4 = &__dyn_sym[Index] */
	ld	r4,8(r4)	/* r4 = __dyn_sym[Index].st_value */

get_type:
	cmpwi	r0,R_PPC64_RELATIVE
	bne	addr16_highest
	ld	r6,0(r9)	/* reloc->r_offset */
	ld	r0,16(r9)	/* reloc->r_addend */
	add	r0,r0,r3
	stdx	r0,r7,r6
	b	nextrela
addr16_highest:
	cmpwi	r0,R_PPC64_ADDR16_HIGHEST
	bne	addr16_higher
	ld	r6,0(r9)	/* reloc->r_offset */
	ld	r0,16(r9)	/* reloc->r_addend */
	add	r0,r0,r3
	add	r0,r0,r4	/* r0 = (S + A + Offset) */
	extrdi	r0,r0,16,0	/* r0 = (r0 >> 48) & 0xffff) */
	b	store_half
addr16_higher:
	cmpwi	r0,R_PPC64_ADDR16_HIGHER
	bne	addr16_hi
	ld	r6,0(r9)	/* reloc->r_offset */
	ld	r0,16(r9)	/* reloc->r_addend */
	add	r0,r0,r3
	add	r0,r0,r4	/* r0 = (S + A + Offset) */
	extrdi	r0,r0,16,16	/* r0 = (r0 >> 32) & 0xffff) */
	b	store_half
addr16_hi:
	cmpwi	r0,R_PPC64_ADDR16_HI
	bne	addr16_lo
	ld	r6,0(r9)	/* reloc->r_offset */
	ld	r0,16(r9)	/* reloc->r_addend */
	add	r0,r0,r3
	add	r0,r0,r4	/* r0 = (S + A + Offset) */
	extrdi	r0,r0,16,32	/* r0 = (r0 >> 16) & 0xffff) */
	b	store_half
addr16_lo:
	cmpwi	r0,R_PPC64_ADDR16_LO
	bne	nextrela
	ld	r6,0(r9)	/* reloc->r_offset */
	ld	r0,16(r9)	/* reloc->r_addend */
	add	r0,r0,r3
	add	r0,r0,r4	/* r0 = (S + A + Offset) */
	extrdi	r0,r0,16,48	/* r0 = r0 & 0xffff) */

	/* Store half word */
store_half:
	sthx	r0,r6,r7	/* memory[r6 + r7] = (u16)r0 */

nextrela:
	dcbst	r6,r7
	sync
	icbi	r6,r7
	addi	r9,r9,24
	bdnz	5b

6:	sync
	isync
	blr

.balign 8
p_dyn:	.llong	__dynamic_start - 0b
p_rela:	.llong	__rela_dyn_start - 0b
p_st:	.llong	_stext - 0b
p_sym:	.llong  __dynamic_symtab - 0b
