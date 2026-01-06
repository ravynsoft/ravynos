/*
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
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

#include "offsets.h"

#if defined(__x86_64__)

#include <mach/i386/syscall_sw.h>

#ifndef VARIANT_DYLD

	.align 2, 0x90
	.globl _start_wqthread
_start_wqthread:
	// This routine is never called directly by user code, jumped from kernel
	// Push a sentinel frame, so backtracers know when to stop.
	push   $0
	push   %rbp
	mov    %rsp,%rbp
	sub    $16,%rsp      // align the stack
	call   __pthread_wqthread
	ud2 // never returns

	.align 2, 0x90
	.globl _thread_start
_thread_start:
	// This routine is never called directly by user code, jumped from kernel
	// Push a sentinel frame, so backtracers know when to stop.
	push   $0
	push   %rbp
	mov    %rsp,%rbp
	sub    $16,%rsp		// align the stack
	call   __pthread_start
	leave
	ret

	.align 2, 0x90
	.globl _thread_chkstk_darwin
_thread_chkstk_darwin:
	.globl ____chkstk_darwin
____chkstk_darwin: // %rax == alloca size
	pushq  %rcx
	leaq   0x10(%rsp), %rcx

	// validate that the frame pointer is on our stack (no alt stack)
	cmpq   %rcx, %gs:_PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET
	jb     Lprobe
	cmpq   %rcx, %gs:_PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET
	jae    Lprobe

	// validate alloca size
	subq   %rax, %rcx
	jb     Lcrash
	cmpq   %rcx, %gs:_PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET
	ja     Lcrash

	popq   %rcx
	retq

Lcrash:
	// POSIX mandates that stack overflow crashes with SIGSEGV
	// so load an address in the guard page and dereference it
	movq   %gs:_PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET, %rcx
	testq  %rcx, -8(%rcx)
	// if main_thread caused stack growth with setrlimit()
	// fall into Lprobe and eventually cause SIGSEGV.

Lprobe:
	// probe the stack when it's not ours (altstack or some shenanigan)
	cmpq   $0x1000, %rax
	jb     Lend
	pushq  %rax
Lloop:
	subq   $0x1000, %rcx
	testq  %rcx, (%rcx)
	subq   $0x1000, %rax
	cmpq   $0x1000, %rax
	ja     Lloop
	popq   %rax
Lend:
	subq   %rax, %rcx
	testq  %rcx, (%rcx)

	popq   %rcx
	retq

#endif

#elif defined(__i386__)

#include <mach/i386/syscall_sw.h>

#ifndef VARIANT_DYLD

	.align 2, 0x90
	.globl _start_wqthread
_start_wqthread:
	// This routine is never called directly by user code, jumped from kernel
	// Push a sentinel frame, so backtracers know when to stop.
	push   $0
	push   %ebp
	mov    %esp,%ebp
	sub    $24,%esp         // align the stack
	mov    %esi,20(%esp)    //arg5
	mov    %edi,16(%esp)    //arg5
	mov    %edx,12(%esp)    //arg4
	mov    %ecx,8(%esp)             //arg3
	mov    %ebx,4(%esp)             //arg2
	mov    %eax,(%esp)              //arg1
	call   __pthread_wqthread
	ud2 // never returns

	.align 2, 0x90
	.globl _thread_start
_thread_start:
	// This routine is never called directly by user code, jumped from kernel
	// Push a sentinel frame, so backtracers know when to stop.
	push   $0
	push   %ebp
	mov    %esp,%ebp
	sub    $24,%esp         // align the stack
	mov    %esi,20(%esp)    //arg6
	mov    %edi,16(%esp)    //arg5
	mov    %edx,12(%esp)    //arg4
	mov    %ecx,8(%esp)     //arg3
	mov    %ebx,4(%esp)     //arg2
	mov    %eax,(%esp)      //arg1
	call   __pthread_start
	leave
	ret

	.align 2, 0x90
	.globl _thread_chkstk_darwin
_thread_chkstk_darwin:
	.globl ____chkstk_darwin
____chkstk_darwin: // %eax == alloca size
	pushl  %ecx
	pushl  %edx
	leal   0xc(%esp), %ecx

	// validate that the frame pointer is on our stack (no alt stack)
	movl   %gs:0x0, %edx    // pthread_self()
	cmpl   %ecx, _PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET(%edx)
	jb     Lprobe
	movl   _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET(%edx), %edx
	cmpl   %ecx, %edx
	jae    Lprobe

	// validate alloca size
	subl   %eax, %ecx
	jb     Lcrash
	cmpl   %ecx, %edx
	ja     Lcrash

	popl   %edx
	popl   %ecx
	retl

Lcrash:
	// POSIX mandates that stack overflow crashes with SIGSEGV
	// so load an address in the guard page and dereference it
	movl   %gs:0x0, %ecx    // pthread_self()
	movl   _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET(%ecx), %ecx
	testl  %ecx, -4(%ecx)
	// if main_thread caused stack growth with setrlimit()
	// fall into Lprobe and eventually cause SIGSEGV.

Lprobe:
	// probe the stack when it's not ours (altstack or some shenanigan)
	cmpl   $0x1000, %eax
	jb     Lend
	pushl  %eax
Lloop:
	subl   $0x1000, %ecx
	testl  %ecx, (%ecx)
	subl   $0x1000, %eax
	cmpl   $0x1000, %eax
	ja     Lloop
	popl   %eax
Lend:
	subl   %eax, %ecx
	testl  %ecx, (%ecx)

	popl   %edx
	popl   %ecx
	retl

#endif

#elif defined(__arm__)

#include <mach/arm/syscall_sw.h>

#ifndef VARIANT_DYLD

// This routine is never called directly by user code, jumped from kernel
// args 0 to 3 are already in the regs 0 to 3
// should set stack with the 2 extra args before calling pthread_wqthread()
// arg4 is in r[4]
// arg5 is in r[5]

	.text
	.align 2
	.globl _start_wqthread
_start_wqthread:
// Push a sentinel frame, so backtracers know when to stop.
	mov ip, #0
	str ip, [sp, #-4]!
	str ip, [sp, #-4]!
	stmfd sp!, {r4, r5}
	bl __pthread_wqthread
	trap // never returns

	.text
	.align 2
	.globl _thread_start
_thread_start:
// Push a sentinel frame, so backtracers know when to stop.
	mov ip, #0
	str ip, [sp, #-4]!
	str ip, [sp, #-4]!
	stmfd sp!, {r4, r5}
	bl __pthread_start
// Stackshots will show the routine that happens to link immediately following
// _start_wqthread.  So we add an extra instruction (nop) to make stackshots
// more readable.
	nop

#endif

#elif defined(__arm64__)

#include <mach/arm/syscall_sw.h>

#ifndef VARIANT_DYLD

// This routine is never called directly by user code, jumped from kernel
// args 0 to 5 in registers.
	.text
	.align 2
	.globl _start_wqthread
_start_wqthread:
// Push a sentinel frame, so backtracers know when to stop.
	stp xzr, xzr, [sp, #-16]!
	bl __pthread_wqthread
	brk #1 // never returns

	.text
	.align 2
	.globl _thread_start
_thread_start:
// Push a sentinel frame, so backtracers know when to stop.
	stp xzr, xzr, [sp, #-16]!
	bl __pthread_start
	nop

	.text
	.align 2
	.globl _thread_chkstk_darwin
_thread_chkstk_darwin:
	.globl ____chkstk_darwin
____chkstk_darwin: // %w9/x9 == alloca size
	stp     x10, x11, [sp, #-16]

	// validate that the frame pointer is on our stack (no alt stack)
	mrs     x10, TPIDRRO_EL0
	and     x10, x10, #0xfffffffffffffff8

	// (%sp - pthread_self()->stackaddr) > 0 ?
#if defined(__ARM64_ARCH_8_32__)
	ubfx    x9, x9, #0, #32
	ldur    w11, [x10, _PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET]
#else
	ldur    x11, [x10, _PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET]
#endif
	subs    x11, sp, x11
	b.hs    Lprobe

	// %sp <= pthread_self()->stackbottom ?
#if defined(__ARM64_ARCH_8_32__)
	ldur    w11, [x10, _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET]
#else
	ldur    x11, [x10, _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET]
#endif
	mov     x10, sp
	cmp     x10, x11
	b.ls    Lprobe

	// %sp - (uintptr_t)%x9 < pthread_self()->stackbottom ?
	subs    x10, x10, x9
	b.lo    Lcrash
	cmp     x10, x11
	b.lo    Lcrash

Lexit:
	ldp     x10, x11, [sp, #-16]
	ret

Lcrash:
	// POSIX mandates that stack overflow crashes with SIGSEGV
	// so load an address in the guard page and dereference it
	//
	// x11 contains pthread_self()->stackbottom already
	ldr     x11, [x11, #-8]
	// if main_thread caused stack growth with setrlimit()
	// fall into Lprobe and eventually cause SIGSEGV.

Lprobe:
	mov     x10, sp
	cmp     x9, #0x1000
	b.lo    Lend
Lloop:
	sub     x10, x10, #0x1000
	ldr     x11, [x10]
	sub     x9, x9, #0x1000
	cmp     x9, #0x1000
	b.hi    Lloop
Lend:
	sub     x10, x10, x9
	ldr     x11, [x10]
	b       Lexit

#endif

#else
#error Unsupported architecture
#endif
