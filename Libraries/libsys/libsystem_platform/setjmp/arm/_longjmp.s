/*
 * Copyright (c) 1999-2018 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1998-2008 Apple Inc. All rights reserved.
 *
 *	Implements _longjmp()
 *
 */

#include <architecture/arm/asm_help.h>
#include "_setjmp.h"
#include <arm/arch.h>
#include <os/tsd.h>

/*	int _longjmp(jmp_buf env, int val); */
            
ENTRY_POINT(__longjmp)
	movs 	r12, r1
	ldmia	r0!, { r1-r6, r8, r10-r11 }
	vldmia	r0, { d8-d15 }
	movs	r0, r12
	moveq	r0, #1
	_OS_PTR_MUNGE_TOKEN(r12, r12)
	_OS_PTR_UNMUNGE(r7, r1, r12) // fp
	_OS_PTR_UNMUNGE(lr, r2, r12)
	_OS_PTR_UNMUNGE(sp, r3, r12)
	bx		lr
