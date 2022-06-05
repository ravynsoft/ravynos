/*
 * Copyright (c) 1999-2006 Apple Computer, Inc. All rights reserved.
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
 * This is the interface for the old stub_binding_helper for ARM,
 * adapted to 64-bit aarch64 for ravynOS.
 * The caller has pushed the address of the a lazy pointer to be filled in with
 * the value for the defined symbol and pushed the address of the the mach
 * header this pointer comes from.
 *
 * sp+8        address of lazy pointer
 * sp+0        address of mach header
 * 
 * After the symbol has been resolved and the pointer filled in this is to pop
 * these arguments off the stack and jump to the address of the defined symbol.
 */
  
	.text
	.align 2
	.globl	_stub_binding_helper
_stub_binding_helper:
	stp	fp, lr, [sp, #-16]!
	mov	fp, sp
	sub	sp, sp, #240
	stp	x0,x1, [fp, #-16]
	stp	x2,x3, [fp, #-32]
	stp	x4,x5, [fp, #-48]
	stp	x6,x7, [fp, #-64]
	stp	x8,x9, [fp, #-80]
	stp	q0,q1, [fp, #-128]
	stp	q2,q3, [fp, #-160]
	stp	q4,q5, [fp, #-192]
	stp	q6,q7, [fp, #-224]

	ldr	x0, [fp, #24]			// move address of mach header to 1st parameter
	ldr	x1, [fp, #16]			// move address of lazy pointer to 2nd parameter

	// call dyld::bindLazySymbol(mh, lazy_symbol_pointer_address)
	bl	dyld_macho_fast_lazy_bind
	mov	x16, x0
	
	ldp	x0,x1, [fp, #-16]
	ldp	x2,x3, [fp, #-32]
	ldp	x4,x5, [fp, #-48]
	ldp	x6,x7, [fp, #-64]
	ldp	x8,x9, [fp, #-80]
	ldp	q0,q1, [fp, #-128]
	ldp	q2,q3, [fp, #-160]
	ldp	q4,q5, [fp, #-192]
	ldp	q6,q7, [fp, #-224]

	mov	sp, fp
	ldp	fp, lr, [sp], #16
	add	sp, sp, #16			// remove meta-parameters

	br	x16    				// jump to the symbol`s address that was bound

