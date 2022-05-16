/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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
 *
 * THIS CODE HAS BEEN MODIFIED AS FOLLOWS
 *   2022-5-16 Adapted for ravynOS - Zoe Knox
 * Modifications Copyright (c) 2022 Zoe Knox
 */
 


 /*    
  * sp+0	lazy binding info offset
  * sp+8	address of ImageLoader cache
  */
	.text
	.align 2
	.globl	dyld_stub_binder
dyld_stub_binder:
	stp		fp, lr, [sp, #-16]!
	mov		fp, sp
	sub		sp, sp, #240
	stp		x0,x1, [fp, #-16]	; x0-x7 are int parameter registers
	stp		x2,x3, [fp, #-32]
	stp		x4,x5, [fp, #-48]
	stp		x6,x7, [fp, #-64]
	stp		x8,x9, [fp, #-80]	; x8 is used for struct returns
	stp		q0,q1, [fp, #-128]	; q0-q7 are vector/fp parameter registers
	stp		q2,q3, [fp, #-160]
	stp		q4,q5, [fp, #-192]
	stp		q6,q7, [fp, #-224]

	ldr		x0, [fp, #24]	; move address ImageLoader cache to 1st parameter
	ldr		x1, [fp, #16]	; move lazy info offset 2nd parameter
	; call dyld::fastBindLazySymbol(loadercache, lazyinfo)
	bl		__Z21_dyld_fast_stub_entryPvl
	mov		x16,x0			; save target function address in lr
	
	; restore parameter registers
	ldp		x0,x1, [fp, #-16]
	ldp		x2,x3, [fp, #-32]
	ldp		x4,x5, [fp, #-48]
	ldp		x6,x7, [fp, #-64]
	ldp		x8,x9, [fp, #-80]
	ldp		q0,q1, [fp, #-128]
	ldp		q2,q3, [fp, #-160]
	ldp		q4,q5, [fp, #-192]
	ldp		q6,q7, [fp, #-224]
	
	mov		sp, fp
	ldp		fp, lr, [sp], #16
	add		sp, sp, #16	; remove meta-parameters
	br      x16

