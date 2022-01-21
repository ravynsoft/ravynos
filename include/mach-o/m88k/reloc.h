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
/*
 * Relocation types used in the m88k implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as discribed
 * above and their r_type is RELOC_VANILLA.  The rest of the relocation types
 * are for instructions.  Since they are for instructions the r_address field
 * indicates the 32 bit instruction that the relocation is to be preformed on.
 * The fields r_pcrel and r_length are ignored for non-RELOC_VANILLA r_types.
 */
enum reloc_type_m88k
{
    M88K_RELOC_VANILLA,	/* generic relocation as discribed above */
    M88K_RELOC_PAIR,	/* the second relocation entry of a pair */
    M88K_RELOC_PC16,
    M88K_RELOC_PC26,
    M88K_RELOC_HI16,	/* a PAIR follows with the low half */
    M88K_RELOC_LO16,	/* a PAIR follows with the high half */
    M88K_RELOC_SECTDIFF,/* a PAIR follows with subtract symbol value */
    M88K_RELOC_PB_LA_PTR/* prebound lazy pointer */
};
