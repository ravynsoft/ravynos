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
/*    reloc.h   -  assemble for Sparc    */
/*    Defines machine specific relocation entries */

#ifndef SPARC_RELOC_INCLUDED
#define SPARC_RELOC_INCLUDED

/*
 * Relocation types used in the sparc implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as discribed
 * in <mach-o/reloc.h> and their r_type is SPARC_RELOC_VANILLA.  The rest of the
 * relocation types are for instructions.  Since they are for instructions the
 * r_address field indicates the 32 bit instruction that the relocation is to
 * be preformed on.  The field r_pcrel is set only for the SPARC_RELOC_WDISP22
 * and SPARC_RELOC_WDISP30.  And r_length is set to long for all
 * non-RELOC_VANILLA r_types.
 */
enum reloc_type_sparc
{
	SPARC_RELOC_VANILLA,	/* vanilla relocation */
	SPARC_RELOC_PAIR,	/* the second relocation entry of a pair */
	SPARC_RELOC_HI22,	/* 22 high bits (sethi) (has pair) */
	SPARC_RELOC_LO10,	/* 10 low bits (has pair) */
	SPARC_RELOC_WDISP22,	/* 22 bit PC relative displacement */
	SPARC_RELOC_WDISP30,	/* 30 bit PC relative displacement */
	SPARC_RELOC_SECTDIFF,	/* a PAIR follows with subtract symbol value */
	SPARC_RELOC_HI22_SECTDIFF,
	SPARC_RELOC_LO10_SECTDIFF,
	SPARC_RELOC_PB_LA_PTR	/* prebound lazy pointer */
};
#endif    /* SPARC_RELOC_INCLUDED */
