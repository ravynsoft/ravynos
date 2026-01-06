/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*    reloc.h   -  assemble for HP-PA    */
/*    Defines machine specific relocation entries */

#ifndef HPPA_RELOC_INCLUDED
#define HPPA_RELOC_INCLUDED

/*
 * Relocation types used in the hppa implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as discribed
 * in <mach-o/reloc.h> and their r_type is HPPA_RELOC_VANILLA.  The rest of the
 * relocation types are for instructions.  Since they are for instructions the
 * r_address field indicates the 32 bit instruction that the relocation is to
 * be preformed on.  The field r_pcrel is set only for the HPPA_RELOC_BR17.
 * And r_length is set to long for all non-RELOC_VANILLA r_types.
 */
enum reloc_type_hppa
{
    HPPA_RELOC_VANILLA,	/* generic relocation as discribed in <mach-o/reloc.h>*/
    HPPA_RELOC_PAIR,	/* the second relocation entry of a pair */
    HPPA_RELOC_HI21,	/* a PAIR follows with the low part */
    HPPA_RELOC_LO14,	/* a PAIR follows with the high part */
    HPPA_RELOC_BR17,	/* 17 bit branch displacement (to a word address)
			   a PAIR follows with the high part */
    HPPA_RELOC_BL17,	/* a bl instruction (overlow causes an error) */
    HPPA_RELOC_JBSR, 	/* a bl instruction that is targeted at a long branch
			   stub, a PAIR follows with the high part */
    HPPA_RELOC_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    HPPA_RELOC_HI21_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    HPPA_RELOC_LO14_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    HPPA_RELOC_PB_LA_PTR	/* prebound lazy pointer */
};

/*
 * For the HI and LO relocation types the two parts of the relocated expression
 * (symbol + offset) are calculated as follows:
 *
 *	rounded = round(offset, 0x2000);
 *	left21 =   (symbol + rounded) & 0xfffff800;
 *	right14 = ((symbol + rounded) & 0x000007ff) + (offset - rounded);
 *
 * This allows the left part to be shared between references with different
 * offsets as long as the rounded offsets are the same.
 *
 * The HPPA_RELOC_BR17 r_type also uses the above calculation and the right14
 * bits, sign extened to fill the displacement, and converted to a word
 * displacement by droping the low bits (after checking they are zero).
 */

/*
 * For relocation types that use pairs the part of the relocated expression that
 * is not stored in the instruction is stored in the r_address feild of the
 * PAIR's entry.
 *
 * All low parts are stored as sign extened byte addressed values in the PAIR's
 * r_address field as 32 bit values.  This allows the HI21 not to have to know
 * which type of low it is used with.
 *
 * The high parts are left justified 21 bit values zero filled to 32 bits and 
 * stored in the PAIR's r_address field.
 */

/*
 * The instructions that use the non-RELOC_VANILLA r_types are and the r_types
 * they use are as follows:
 *	instructions	r_type
 *
 *	LDIL,ADDIL	HPPA_RELOC_HI21
 *	LDx, STx, LDO	HPPA_RELOC_LO14
 *	BE, BLE		HPPA_RELOC_BR17
 *	BL		HPPA_RELOC_BL17
 *
 * For the HPPA_RELOC_JBSR the BL instruction must be targeted at a long branch
 * stub that can be reached with 17 bits of signed word displacement.  Also the
 * stub must be in the same block as the BL instruction so that scattered
 * loading done by the link editor will not move them apart.  For example in
 * assembly code:
 *	jbsr	foo,%r2,L1	; creates a bl inst with a HPPA_RELOC_JBSR
 *				;  relocation entry for the symbol foo and the
 *				;  instruction is targeted to L1
 *	...
 * L1:	ldil	L'foo,%r1	; a HPPA_RELOC_HI21 entry for symbol foo
 *	ble,n	R'foo(%sr4,%r1)	; a HPPA_RELOC_BR17 entry for symbol foo
 */

#endif    /* HPPA_RELOC_INCLUDED */
