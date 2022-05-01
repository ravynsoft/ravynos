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
 *   2021-12-13 Adapted for ravynOS - Zoe Knox
 * Modifications Copyright (c) 2021 Zoe Knox
 */
 
//#include <TargetConditionals.h>
 
//#include <System/machine/cpu_capabilities.h>


#define RET_ADDR_RBP			24
#define LP_PARAM_RBP			16
#define MH_PARAM_RBP			8
#define OLD_RBP_RBP				0

#define RDI_SAVE_RBP			-8
#define RSI_SAVE_RBP			-16
#define RDX_SAVE_RBP			-24
#define RCX_SAVE_RBP			-32
#define RBX_SAVE_RBP			-40
#define RAX_SAVE_RBP			-48
#define R8_SAVE_RBP 			-56
#define R9_SAVE_RBP 			-64
#define STATIC_STACK_SIZE		256	// extra padding to allow it to be 64-byte aligned

#define XMM0_SAVE_RSP			0x00
#define XMM1_SAVE_RSP			0x10
#define XMM2_SAVE_RSP			0x20
#define XMM3_SAVE_RSP			0x30
#define XMM4_SAVE_RSP			0x40
#define XMM5_SAVE_RSP			0x50
#define XMM6_SAVE_RSP			0x60
#define XMM7_SAVE_RSP			0x70


 /*    
 * sp+16 return address
 * sp+8  lazy binding info offset
 * sp+0	 address of ImageLoader cache
 */
    .align 2,0x90
    .globl dyld_stub_binder
dyld_stub_binder:
	pushq		%rbp
	test		$0xF,%rsp				# at this point stack should be 16-byte aligned
	jne			_stack_not_16_byte_aligned_error
	movq		%rsp,%rbp
	subq		$STATIC_STACK_SIZE,%rsp
	movq		%rdi,RDI_SAVE_RBP(%rbp)	# save registers that might be used as parameters
	movq		%rsi,RSI_SAVE_RBP(%rbp)
	movq		%rdx,RDX_SAVE_RBP(%rbp)
	movq		%rcx,RCX_SAVE_RBP(%rbp)
	movq		%rbx,RBX_SAVE_RBP(%rbp)
	movq		%rax,RAX_SAVE_RBP(%rbp)
	movq		%r8, R8_SAVE_RBP(%rbp)
	movq		%r9, R9_SAVE_RBP(%rbp)

	cmpl		$0, _inited(%rip)
	jne			Linited
	movl		$0x01,%eax
	cpuid		# get cpu features to check on xsave instruction support
	andl		$0x08000000,%ecx		# check OSXSAVE bit
	movl		%ecx,_hasXSave(%rip)
	cmpl		$0, %ecx
	jne			LxsaveInfo
	movl		$1, _inited(%rip)
	jmp			Lsse

LxsaveInfo:
	movl		$0x0D,%eax
	movl		$0x00,%ecx
	cpuid		# get xsave parameter info
	movl		%eax,_features_lo32(%rip)
	movl		%edx,_features_hi32(%rip)
	movl		%ecx,_bufferSize32(%rip)
	movl		$1, _inited(%rip)

Linited:
	cmpl		$0, _hasXSave(%rip)
	jne			Lxsave

Lsse:
	subq		$128, %rsp
	movdqa      %xmm0, XMM0_SAVE_RSP(%rsp)
	movdqa      %xmm1, XMM1_SAVE_RSP(%rsp)
	movdqa      %xmm2, XMM2_SAVE_RSP(%rsp)
	movdqa      %xmm3, XMM3_SAVE_RSP(%rsp)
	movdqa      %xmm4, XMM4_SAVE_RSP(%rsp)
	movdqa      %xmm5, XMM5_SAVE_RSP(%rsp)
	movdqa      %xmm6, XMM6_SAVE_RSP(%rsp)
	movdqa      %xmm7, XMM7_SAVE_RSP(%rsp)
	jmp			Lbind

Lxsave:
	movl		_bufferSize32(%rip),%eax
	movq		%rsp, %rdi
	subq		%rax, %rdi				# stack alloc buffer
	andq		$-64, %rdi				# 64-byte align stack
	movq		%rdi, %rsp
	# xsave requires buffer to be zero'ed out
	movq		$0, %rcx
	movq		%rdi, %r8
	movq		%rdi, %r9
	addq		%rax, %r9
Lz:	movq		%rcx, (%r8)
	addq		$8, %r8
	cmpq		%r8,%r9
	ja			Lz

	movl		_features_lo32(%rip),%eax
	movl		_features_hi32(%rip),%edx
	# call xsave with buffer on stack and eax:edx flag bits
	# note: do not use xsaveopt, it assumes you are using the same
	# buffer as previous xsaves, and this thread is on the same cpu.
	xsave		(%rsp)

Lbind:
	movq		MH_PARAM_RBP(%rbp),%rdi	# call dyld_macho_fast_lazy_bind(loadercache, lazyinfo)
	movq		LP_PARAM_RBP(%rbp),%rsi
	call		dyld_macho_fast_lazy_bind
	movq		%rax,%r11		# copy jump target

	cmpl		$0, _hasXSave(%rip)
	jne			Lxrstror

	movdqa      XMM0_SAVE_RSP(%rsp),%xmm0
	movdqa      XMM1_SAVE_RSP(%rsp),%xmm1
	movdqa      XMM2_SAVE_RSP(%rsp),%xmm2
	movdqa      XMM3_SAVE_RSP(%rsp),%xmm3
	movdqa      XMM4_SAVE_RSP(%rsp),%xmm4
	movdqa      XMM5_SAVE_RSP(%rsp),%xmm5
	movdqa      XMM6_SAVE_RSP(%rsp),%xmm6
	movdqa      XMM7_SAVE_RSP(%rsp),%xmm7
	jmp			Ldone

Lxrstror:
	movl		_features_lo32(%rip),%eax
	movl		_features_hi32(%rip),%edx
	# call xsave with buffer on stack and eax:edx flag bits
	xrstor		(%rsp)

Ldone:
	movq		RDI_SAVE_RBP(%rbp),%rdi
	movq		RSI_SAVE_RBP(%rbp),%rsi
	movq		RDX_SAVE_RBP(%rbp),%rdx
	movq		RCX_SAVE_RBP(%rbp),%rcx
	movq		RBX_SAVE_RBP(%rbp),%rbx
	movq		RAX_SAVE_RBP(%rbp),%rax
	movq		R8_SAVE_RBP(%rbp),%r8
	movq		R9_SAVE_RBP(%rbp),%r9
	movq		%rbp,%rsp
	popq		%rbp
	addq		$16,%rsp		# remove meta-parameters
	jmp			*%r11			# jmp to target

_stack_not_16_byte_aligned_error:
	movdqa      %xmm0, 0(%rsp)
	int3

	.data
# Cached info from cpuid.  These must be lazily evaluated.
# You cannot initalize these from _dyld_initializer() because
# that function is called from another dylib...
_inited:			.long 0
_features_lo32:		.long 0
_features_hi32:		.long 0
_bufferSize32:		.long 0
_hasXSave:			.long 0
