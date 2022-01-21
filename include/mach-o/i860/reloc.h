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
 * Relocation types used in the I860 implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as discribed
 * above and their r_type is RELOC_VANILLA.  The rest of the relocation types
 * are for instructions.  Since they are for instructions the r_address field
 * indicates the 32 bit instruction that the relocation is to be preformed on.
 * The fields r_pcrel and r_length are ignored for non-RELOC_VANILLA r_types
 * except RELOC_SPLIT0 as discribed below.
 */
enum reloc_type_i860
{
    I860_RELOC_VANILLA,	/* generic relocation as discribed above */

    I860_RELOC_PAIR,	/* Only follows a I860_RELOC_HIGH or a
			 * I860_RELOC_HIGHADJ and only the r_address has any
			 * meaning.
			 */ 
    I860_RELOC_HIGH,	/* The low 16 bits of the instruction contains the high
			 * 16 bits of the item being refered to.  This
			 * relocation type must be followed by a I860_RELOC_PAIR
			 * relocation type.  The low 16 bits of the item being
			 * refered to is stored in the r_address of the
			 * I860_RELOC_PAIR entry.
			 */
    I860_RELOC_LOW0,	/* For all of these the low 16 bits of the instruction*/
    I860_RELOC_LOW1,	/* (minus the low 0, 1, 2, 3, or 4 bits) contain the  */
    I860_RELOC_LOW2,	/* low 16 bits of the item being refered to.  The bits*/
    I860_RELOC_LOW3,	/* of the reference that are missing are 0 and the    */
    I860_RELOC_LOW4,	/* bits in the instruction are part of the encoding of*/
			/* instruction.  The resulting low 16 bits of the item*/
			/* being refered to is sign extended to 32 bits.      */

    I860_RELOC_SPLIT0,	/* For all of these the bits 20-14 and bits 10-0 of   */
    I860_RELOC_SPLIT1,	/* the instruction (minus the low 0, 1 or 2 bits)     */
    I860_RELOC_SPLIT2,	/* contain the low 16 bits of the item to being       */
			/* refered to.  The bits of the reference that are    */
			/* missing are 0 and the bits of the instruction are  */
			/* part of the encoding of the instruction.  The      */
			/* resulting low 16 bits of the item being relocated  */
			/* is sign extened to 32 bits.  A special case of the */
			/* I860_RELOC_SPLIT0 is when r_pcrel is non-zero (for */
			/* branch displacements).  In this case the 16 bits   */
			/* from the instruction is a 32 bit word displacement.*/

    I860_RELOC_HIGHADJ,	/* Same as the RELOC_HIGH except the low 16 bits and the
			 * high 16 bits are added together with the low 16 bits
			 * sign extened first.  This means if bit 15 of the low
			 * 16 bits is set the high 16 bits stored in the
			 * instruction will be adjusted.
			 */
    I860_RELOC_BRADDR,	/* The low 26 bits of the instruction is a 32 bit
			 * word displacement from the pc to the item to being
			 * refered to.
			 */
    I860_RELOC_SECTDIFF /* a PAIR follows with subtract symbol value */
};
