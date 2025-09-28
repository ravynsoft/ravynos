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
 *   2021-12-13 Adapted for ravynOS - Zoe Knox
 * Modifications Copyright (c) 2021 Zoe Knox
 */
 
 

/*
 * This is the interface for the stub_binding_helper for x86_64:
 * The caller has pushed the address of the a lazy pointer to be filled in with
 * the value for the defined symbol and pushed the address of the the mach
 * header this pointer comes from.
 *
 * sp+8	address of lazy pointer
 * sp+0	address of mach header
 *
 * All parameters registers must be preserved.
 * 
 * After the symbol has been resolved and the pointer filled in this is to pop
 * these arguments off the stack and jump to the address of the defined symbol.
 */
#define MH_PARAM_BP			8
#define LP_PARAM_BP			16

#define RDI_SAVE			0
#define RSI_SAVE			8
#define RDX_SAVE			16
#define RCX_SAVE			24
#define R8_SAVE				32
#define R9_SAVE				40
#define RAX_SAVE			48
#define XMMM0_SAVE			64    /* 16-byte align */
#define XMMM1_SAVE			80
#define XMMM2_SAVE			96
#define XMMM3_SAVE			112
#define XMMM4_SAVE			128
#define XMMM5_SAVE			144
#define XMMM6_SAVE			160
#define XMMM7_SAVE			176
#define STACK_SIZE			192 /*  (XMMM7_SAVE+16) must be 16 byte aligned too */
    
    .text
    .align 2,0x90
    .globl _stub_binding_helper
_stub_binding_helper:
	pushq		%rbp
	movq		%rsp,%rbp
	subq		$STACK_SIZE,%rsp	# at this point stack is 16-byte aligned because two meta-parameters where pushed
	movq		%rdi,RDI_SAVE(%rsp)	# save registers that might be used as parameters
	movq		%rsi,RSI_SAVE(%rsp)
	movq		%rdx,RDX_SAVE(%rsp)
	movq		%rcx,RCX_SAVE(%rsp)
	movq		%r8,R8_SAVE(%rsp)
	movq		%r9,R9_SAVE(%rsp)
	movq		%rax,RAX_SAVE(%rsp)
	movdqa		%xmm0,XMMM0_SAVE(%rsp)
	movdqa		%xmm1,XMMM1_SAVE(%rsp)
	movdqa		%xmm2,XMMM2_SAVE(%rsp)
	movdqa		%xmm3,XMMM3_SAVE(%rsp)
	movdqa		%xmm4,XMMM4_SAVE(%rsp)
	movdqa		%xmm5,XMMM5_SAVE(%rsp)
	movdqa		%xmm6,XMMM6_SAVE(%rsp)
	movdqa		%xmm7,XMMM7_SAVE(%rsp)
	movq		MH_PARAM_BP(%rbp),%rdi	# call dyld_macho_bind_stub(mh, lazy_ptr)
	movq		LP_PARAM_BP(%rbp),%rsi
	call		dyld_macho_bind_stub
	movq		%rax,%r11		# save target
	movdqa		XMMM0_SAVE(%rsp),%xmm0	# restore registers
	movdqa		XMMM1_SAVE(%rsp),%xmm1
	movdqa		XMMM2_SAVE(%rsp),%xmm2
	movdqa		XMMM3_SAVE(%rsp),%xmm3
	movdqa		XMMM4_SAVE(%rsp),%xmm4
	movdqa		XMMM5_SAVE(%rsp),%xmm5
	movdqa		XMMM6_SAVE(%rsp),%xmm6
	movdqa		XMMM7_SAVE(%rsp),%xmm7
	movq		RDI_SAVE(%rsp),%rdi
	movq		RSI_SAVE(%rsp),%rsi
	movq		RDX_SAVE(%rsp),%rdx
	movq		RCX_SAVE(%rsp),%rcx
	movq		R8_SAVE(%rsp),%r8
	movq		R9_SAVE(%rsp),%r9
	movq		RAX_SAVE(%rsp),%rax
	addq		$STACK_SIZE,%rsp
	popq		%rbp
	addq		$16,%rsp		# remove meta-parameters
	jmp		*%r11			# jmp to target

