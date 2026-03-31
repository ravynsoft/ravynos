/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
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

#include <TargetConditionals.h>
#if __x86_64__  &&  TARGET_OS_SIMULATOR  &&  !TARGET_OS_IOSMAC

/********************************************************************
 ********************************************************************
 **
 **  objc-msg-x86_64.s - x86-64 code to support objc messaging.
 **
 ********************************************************************
 ********************************************************************/

.data

// _objc_restartableRanges is used by method dispatch
// to get the critical regions for which method caches 
// cannot be garbage collected.

.macro RestartableEntry
	.quad	$0
	.short	LExit$0 - $0
	.short	0xffff // The simulator doesn't support kernel based recovery
	.long	0
.endmacro

	.align 4
	.private_extern _objc_restartableRanges
_objc_restartableRanges:
	RestartableEntry _cache_getImp
	RestartableEntry _objc_msgSend
	RestartableEntry _objc_msgSend_fpret
	RestartableEntry _objc_msgSend_fp2ret
	RestartableEntry _objc_msgSend_stret
	RestartableEntry _objc_msgSendSuper
	RestartableEntry _objc_msgSendSuper_stret
	RestartableEntry _objc_msgSendSuper2
	RestartableEntry _objc_msgSendSuper2_stret
	RestartableEntry _objc_msgLookup
	RestartableEntry _objc_msgLookup_fpret
	RestartableEntry _objc_msgLookup_fp2ret
	RestartableEntry _objc_msgLookup_stret
	RestartableEntry _objc_msgLookupSuper2
	RestartableEntry _objc_msgLookupSuper2_stret
	.fill	16, 1, 0


/********************************************************************
 * Recommended multi-byte NOP instructions
 * (Intel 64 and IA-32 Architectures Software Developer's Manual Volume 2B)
 ********************************************************************/
#define nop1 .byte 0x90
#define nop2 .byte 0x66,0x90
#define nop3 .byte 0x0F,0x1F,0x00
#define nop4 .byte 0x0F,0x1F,0x40,0x00
#define nop5 .byte 0x0F,0x1F,0x44,0x00,0x00
#define nop6 .byte 0x66,0x0F,0x1F,0x44,0x00,0x00
#define nop7 .byte 0x0F,0x1F,0x80,0x00,0x00,0x00,0x00
#define nop8 .byte 0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00
#define nop9 .byte 0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00


/********************************************************************
 * Names for parameter registers.
 ********************************************************************/

#define a1  rdi
#define a1d edi
#define a1b dil
#define a2  rsi
#define a2d esi
#define a2b sil
#define a3  rdx
#define a3d edx
#define a3b dl
#define a4  rcx
#define a4d ecx
#define a5  r8
#define a5d r8d
#define a6  r9
#define a6d r9d


/********************************************************************
 * Names for relative labels
 * DO NOT USE THESE LABELS ELSEWHERE
 * Reserved labels: 6: 7: 8: 9:
 ********************************************************************/
#define LCacheMiss 	6
#define LCacheMiss_f 	6f
#define LCacheMiss_b 	6b
#define LGetIsaDone 	7
#define LGetIsaDone_f 	7f
#define LGetIsaDone_b 	7b
#define LNilOrTagged 	8
#define LNilOrTagged_f 	8f
#define LNilOrTagged_b 	8b
#define LNil		9
#define LNil_f		9f
#define LNil_b		9b

/********************************************************************
 * Macro parameters
 ********************************************************************/

#define NORMAL 0
#define FPRET 1
#define FP2RET 2
#define STRET 3

#define CALL 100
#define GETIMP 101
#define LOOKUP 102


/********************************************************************
 *
 * Structure definitions.
 *
 ********************************************************************/

// objc_super parameter to sendSuper
#define receiver 	0
#define class 		8

// Selected field offsets in class structure
// #define isa		0    USE GetIsa INSTEAD

// Method descriptor
#define method_name 	0
#define method_imp 	16

// Method cache
#define cached_sel 	0
#define cached_imp 	8


//////////////////////////////////////////////////////////////////////
//
// ENTRY		functionName
//
// Assembly directives to begin an exported function.
//
// Takes: functionName - name of the exported function
//////////////////////////////////////////////////////////////////////

.macro ENTRY
	.text
	.globl	$0
	.align	6, 0x90
$0:
.endmacro

.macro STATIC_ENTRY
	.text
	.private_extern	$0
	.align	2, 0x90
$0:
.endmacro

//////////////////////////////////////////////////////////////////////
//
// END_ENTRY	functionName
//
// Assembly directives to end an exported function.  Just a placeholder,
// a close-parenthesis for ENTRY, until it is needed for something.
//
// Takes: functionName - name of the exported function
//////////////////////////////////////////////////////////////////////

.macro END_ENTRY
LExit$0:
.endmacro


 /********************************************************************
 * UNWIND name, flags
 * Unwind info generation	
 ********************************************************************/
.macro UNWIND
	.section __LD,__compact_unwind,regular,debug
	.quad $0
	.set  LUnwind$0, LExit$0 - $0
	.long LUnwind$0
	.long $1
	.quad 0	 /* no personality */
	.quad 0  /* no LSDA */
	.text
.endmacro

#define NoFrame 0x02010000  // no frame, no SP adjustment except return address
#define FrameWithNoSaves 0x01000000  // frame, no non-volatile saves


//////////////////////////////////////////////////////////////////////
//
// SAVE_REGS
//
// Create a stack frame and save all argument registers in preparation
// for a function call.
//////////////////////////////////////////////////////////////////////

.macro SAVE_REGS

	push	%rbp
	mov	%rsp, %rbp

	sub	$$0x80+8, %rsp		// +8 for alignment

	movdqa	%xmm0, -0x80(%rbp)
	push	%rax			// might be xmm parameter count
	movdqa	%xmm1, -0x70(%rbp)
	push	%a1
	movdqa	%xmm2, -0x60(%rbp)
	push	%a2
	movdqa	%xmm3, -0x50(%rbp)
	push	%a3
	movdqa	%xmm4, -0x40(%rbp)
	push	%a4
	movdqa	%xmm5, -0x30(%rbp)
	push	%a5
	movdqa	%xmm6, -0x20(%rbp)
	push	%a6
	movdqa	%xmm7, -0x10(%rbp)

.endmacro


//////////////////////////////////////////////////////////////////////
//
// RESTORE_REGS
//
// Restore all argument registers and pop the stack frame created by
// SAVE_REGS.
//////////////////////////////////////////////////////////////////////

.macro RESTORE_REGS

	movdqa	-0x80(%rbp), %xmm0
	pop	%a6
	movdqa	-0x70(%rbp), %xmm1
	pop	%a5
	movdqa	-0x60(%rbp), %xmm2
	pop	%a4
	movdqa	-0x50(%rbp), %xmm3
	pop	%a3
	movdqa	-0x40(%rbp), %xmm4
	pop	%a2
	movdqa	-0x30(%rbp), %xmm5
	pop	%a1
	movdqa	-0x20(%rbp), %xmm6
	pop	%rax
	movdqa	-0x10(%rbp), %xmm7
	leave

.endmacro


/////////////////////////////////////////////////////////////////////
//
// CacheLookup	return-type, caller
//
// Locate the implementation for a class in a selector's method cache.
//
// Takes: 
//	  $0 = NORMAL, FPRET, FP2RET, STRET
//	  $1 = CALL, LOOKUP, GETIMP
//	  a1 or a2 (STRET) = receiver
//	  a2 or a3 (STRET) = selector
//	  r10 = class to search
//
// On exit: r10 clobbered
//	    (found) calls or returns IMP in r11, eq/ne set for forwarding
//	    (not found) jumps to LCacheMiss, class still in r10
//
/////////////////////////////////////////////////////////////////////

.macro CacheHit

	// r11 = found bucket
	
.if $1 == GETIMP
	movq	cached_imp(%r11), %rax	// return imp
	cmpq	$$0, %rax
 	jz	9f			// don't xor a nil imp
	xorq	%r10, %rax		// xor the isa with the imp
9:	ret

.else

.if $1 == CALL
	movq	cached_imp(%r11), %r11	// load imp
	xorq	%r10, %r11			// xor imp and isa
.if $0 != STRET
	// ne already set for forwarding by `xor`
.else
	cmp	%r11, %r11		// set eq for stret forwarding
.endif
	jmp	*%r11			// call imp

.elseif $1 == LOOKUP
	movq	cached_imp(%r11), %r11
	xorq	%r10, %r11		// return imp ^ isa
	ret
	
.else
.abort oops
.endif

.endif

.endmacro


.macro	CacheLookup
.if $0 != STRET
	movq	%a2, %r11		// r11 = _cmd
.else
	movq	%a3, %r11		// r11 = _cmd
.endif
	andl	24(%r10), %r11d		// r11 = _cmd & class->cache.mask
	shlq	$$4, %r11		// r11 = offset = (_cmd & mask)<<4
	addq	16(%r10), %r11		// r11 = class->cache.buckets + offset

.if $0 != STRET
	cmpq	cached_sel(%r11), %a2	// if (bucket->sel != _cmd)
.else
	cmpq	cached_sel(%r11), %a3	// if (bucket->sel != _cmd)
.endif
	jne 	1f			//     scan more
	CacheHit $0, $1			// call or return imp

1:
	// loop
	cmpq	$$1, cached_sel(%r11)
	jbe	3f			// if (bucket->sel <= 1) wrap or miss

	addq	$$16, %r11		// bucket++
2:	
.if $0 != STRET
	cmpq	cached_sel(%r11), %a2	// if (bucket->sel != _cmd)
.else
	cmpq	cached_sel(%r11), %a3	// if (bucket->sel != _cmd)
.endif
	jne 	1b			//     scan more
	CacheHit $0, $1			// call or return imp

3:
	// wrap or miss
	jb	LCacheMiss_f		// if (bucket->sel < 1) cache miss
	// wrap
	movq	cached_imp(%r11), %r11	// bucket->imp is really first bucket
	jmp 	2f

	// Clone scanning loop to miss instead of hang when cache is corrupt.
	// The slow path may detect any corruption and halt later.

1:
	// loop
	cmpq	$$1, cached_sel(%r11)
	jbe	3f			// if (bucket->sel <= 1) wrap or miss

	addq	$$16, %r11		// bucket++
2:	
.if $0 != STRET
	cmpq	cached_sel(%r11), %a2	// if (bucket->sel != _cmd)
.else
	cmpq	cached_sel(%r11), %a3	// if (bucket->sel != _cmd)
.endif
	jne 	1b			//     scan more
	CacheHit $0, $1			// call or return imp

3:
	// double wrap or miss
	jmp	LCacheMiss_f

.endmacro


/////////////////////////////////////////////////////////////////////
//
// MethodTableLookup NORMAL|STRET
//
// Takes:	a1 or a2 (STRET) = receiver
//		a2 or a3 (STRET) = selector to search for
// 		r10 = class to search
//
// On exit: imp in %r11, eq/ne set for forwarding
//
/////////////////////////////////////////////////////////////////////

.macro MethodTableLookup

	SAVE_REGS

	// lookUpImpOrForward(obj, sel, cls, LOOKUP_INITIALIZE | LOOKUP_RESOLVER)
.if $0 == NORMAL
	// receiver already in a1
	// selector already in a2
.else
	movq	%a2, %a1
	movq	%a3, %a2
.endif
	movq	%r10, %a3
	movl	$$3, %a4d
	call	_lookUpImpOrForward

	// IMP is now in %rax
	movq	%rax, %r11

	RESTORE_REGS

.if $0 == NORMAL
	test	%r11, %r11		// set ne for stret forwarding
.else
	cmp	%r11, %r11		// set eq for nonstret forwarding
.endif

.endmacro


/////////////////////////////////////////////////////////////////////
//
// GetIsaCheckNil return-type
// GetIsaSupport return-type
// NilTestReturnZero return-type
// NilTestReturnIMP return-type
//
// Sets r10 = obj->isa.
// Looks up the real class if receiver is a tagged pointer object.
// Returns zero or a zero-returning IMP if obj is nil.
//
// Takes:	$0 = NORMAL or FPRET or FP2RET or STRET
//		a1 or a2 (STRET) = receiver
//
// On exit from GetIsaCheckNil:
//		r10 = receiver->isa
//		r11 is clobbered
//
/////////////////////////////////////////////////////////////////////

.macro ZeroReturn
	xorl	%eax, %eax
	xorl	%edx, %edx
	xorps	%xmm0, %xmm0
	xorps	%xmm1, %xmm1
.endmacro

.macro ZeroReturnFPRET
	fldz
	ZeroReturn
.endmacro

.macro ZeroReturnFP2RET
	fldz
	fldz
	ZeroReturn
.endmacro

.macro ZeroReturnSTRET
	// rax gets the struct-return address as passed in rdi
	movq	%rdi, %rax
.endmacro

	STATIC_ENTRY __objc_msgNil
	ZeroReturn
	ret
	END_ENTRY __objc_msgNil

	STATIC_ENTRY __objc_msgNil_fpret
	ZeroReturnFPRET
	ret
	END_ENTRY __objc_msgNil_fpret

	STATIC_ENTRY __objc_msgNil_fp2ret
	ZeroReturnFP2RET
	ret
	END_ENTRY __objc_msgNil_fp2ret

	STATIC_ENTRY __objc_msgNil_stret
	ZeroReturnSTRET
	ret
	END_ENTRY __objc_msgNil_stret


.macro GetIsaCheckNil
.if $0 != STRET
	testq	%a1, %a1
.else
	testq	%a2, %a2
.endif
	jle	LNilOrTagged_f	// MSB tagged pointer looks negative

.if $0 != STRET
	movq	(%a1), %r10	// r10 = isa
.else
	movq	(%a2), %r10	// r10 = isa
.endif

LGetIsaDone:
.endmacro


.macro GetIsaSupport
	.align 3
LNilOrTagged:
	jz	LNil_f		// flags set by GetIsaCheckNil
.if $0 != STRET
	movq	%a1, %r11
.else
	movq	%a2, %r11
.endif
	// basic tagged
	shrq	$$60, %r11
	leaq	_objc_debug_taggedpointer_classes(%rip), %r10
	movq	(%r10, %r11, 8), %r10	// read isa from table
	leaq    _OBJC_CLASS_$___NSUnrecognizedTaggedPointer(%rip), %r11
	cmp    %r10, %r11
	jne	LGetIsaDone_b
	// ext tagged
.if $0 != STRET
	movq	%a1, %r11
.else
	movq	%a2, %r11
.endif
	shrq	$$52, %r11
	andl	$$0xff, %r11d
	leaq	_objc_debug_taggedpointer_ext_classes(%rip), %r10
	movq	(%r10, %r11, 8), %r10	// read isa from table
	jmp	LGetIsaDone_b	
.endmacro


.macro NilTestReturnZero
LNil:
.if $0 == NORMAL
	ZeroReturn
.elseif $0 == FPRET
	ZeroReturnFPRET
.elseif $0 == FP2RET
	ZeroReturnFP2RET
.elseif $0 == STRET
	ZeroReturnSTRET
.else
.abort oops
.endif
	ret
.endmacro


.macro NilTestReturnIMP
LNil:	
.if $0 == NORMAL
	leaq	__objc_msgNil(%rip), %r11
.elseif $0 == FPRET
	leaq	__objc_msgNil_fpret(%rip), %r11
.elseif $0 == FP2RET
	leaq	__objc_msgNil_fp2ret(%rip), %r11
.elseif $0 == STRET
	leaq	__objc_msgNil_stret(%rip), %r11
.else
.abort oops
.endif
	ret
.endmacro


/********************************************************************
 * IMP cache_getImp(Class cls, SEL sel)
 *
 * On entry:	a1 = class whose cache is to be searched
 *		a2 = selector to search for
 *
 * If found, returns method implementation.
 * If not found, returns NULL.
 ********************************************************************/

	STATIC_ENTRY _cache_getImp

// do lookup
	movq	%a1, %r10		// move class to r10 for CacheLookup
	CacheLookup NORMAL, GETIMP	// returns IMP on success

LCacheMiss:
// cache miss, return nil
	xorl	%eax, %eax
	ret

	END_ENTRY _cache_getImp


/********************************************************************
 *
 * id objc_msgSend(id self, SEL	_cmd,...);
 * IMP objc_msgLookup(id self, SEL _cmd, ...);
 *
 * objc_msgLookup ABI:
 * IMP returned in r11
 * Forwarding returned in Z flag
 * r10 reserved for our use but not used
 *
 ********************************************************************/
	
	.data
	.align 3
	.globl _objc_debug_taggedpointer_classes
_objc_debug_taggedpointer_classes:
	.fill 16, 8, 0
	.globl _objc_debug_taggedpointer_ext_classes
_objc_debug_taggedpointer_ext_classes:
	.fill 256, 8, 0

	ENTRY _objc_msgSend
	UNWIND _objc_msgSend, NoFrame

	GetIsaCheckNil NORMAL		// r10 = self->isa, or return zero
	CacheLookup NORMAL, CALL	// calls IMP on success

	GetIsaSupport NORMAL
	NilTestReturnZero NORMAL

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSend

	
	ENTRY _objc_msgLookup

	GetIsaCheckNil NORMAL		// r10 = self->isa, or return zero IMP
	CacheLookup NORMAL, LOOKUP	// returns IMP on success

	GetIsaSupport NORMAL
	NilTestReturnIMP NORMAL

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgLookup_uncached

	END_ENTRY _objc_msgLookup

	
	ENTRY _objc_msgSend_fixup
	int3
	END_ENTRY _objc_msgSend_fixup

	
	STATIC_ENTRY _objc_msgSend_fixedup
	// Load _cmd from the message_ref
	movq	8(%a2), %a2
	jmp	_objc_msgSend
	END_ENTRY _objc_msgSend_fixedup

	
/********************************************************************
 *
 * id objc_msgSendSuper(struct objc_super *super, SEL _cmd,...);
 *
 * struct objc_super {
 *		id	receiver;
 *		Class	class;
 * };
 ********************************************************************/
	
	ENTRY _objc_msgSendSuper
	UNWIND _objc_msgSendSuper, NoFrame
	
// search the cache (objc_super in %a1)
	movq	class(%a1), %r10	// class = objc_super->class
	movq	receiver(%a1), %a1	// load real receiver
	CacheLookup NORMAL, CALL	// calls IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// class still in r10
	jmp	__objc_msgSend_uncached
	
	END_ENTRY _objc_msgSendSuper


/********************************************************************
 * id objc_msgSendSuper2
 ********************************************************************/

	ENTRY _objc_msgSendSuper2
	UNWIND _objc_msgSendSuper2, NoFrame
	
	// objc_super->class is superclass of class to search
	
// search the cache (objc_super in %a1)
	movq	class(%a1), %r10	// cls = objc_super->class
	movq	receiver(%a1), %a1	// load real receiver
	movq	8(%r10), %r10		// cls = class->superclass
	CacheLookup NORMAL, CALL	// calls IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// superclass still in r10
	jmp	__objc_msgSend_uncached
	
	END_ENTRY _objc_msgSendSuper2


	ENTRY _objc_msgLookupSuper2
	
	// objc_super->class is superclass of class to search
	
// search the cache (objc_super in %a1)
	movq	class(%a1), %r10	// cls = objc_super->class
	movq	receiver(%a1), %a1	// load real receiver
	movq	8(%r10), %r10		// cls = class->superclass
	CacheLookup NORMAL, LOOKUP	// returns IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// superclass still in r10
	jmp	__objc_msgLookup_uncached
	
	END_ENTRY _objc_msgLookupSuper2


	ENTRY _objc_msgSendSuper2_fixup
	int3
	END_ENTRY _objc_msgSendSuper2_fixup

	
	STATIC_ENTRY _objc_msgSendSuper2_fixedup
	// Load _cmd from the message_ref
	movq	8(%a2), %a2
	jmp 	_objc_msgSendSuper2
	END_ENTRY _objc_msgSendSuper2_fixedup


/********************************************************************
 *
 * double objc_msgSend_fpret(id self, SEL _cmd,...);
 * Used for `long double` return only. `float` and `double` use objc_msgSend.
 *
 ********************************************************************/

	ENTRY _objc_msgSend_fpret
	UNWIND _objc_msgSend_fpret, NoFrame
	
	GetIsaCheckNil FPRET		// r10 = self->isa, or return zero
	CacheLookup FPRET, CALL		// calls IMP on success

	GetIsaSupport FPRET
	NilTestReturnZero FPRET

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSend_fpret


	ENTRY _objc_msgLookup_fpret
	
	GetIsaCheckNil FPRET		// r10 = self->isa, or return zero IMP
	CacheLookup FPRET, LOOKUP	// returns IMP on success

	GetIsaSupport FPRET
	NilTestReturnIMP FPRET

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgLookup_uncached

	END_ENTRY _objc_msgLookup_fpret

	
	ENTRY _objc_msgSend_fpret_fixup
	int3
	END_ENTRY _objc_msgSend_fpret_fixup

	
	STATIC_ENTRY _objc_msgSend_fpret_fixedup
	// Load _cmd from the message_ref
	movq	8(%a2), %a2
	jmp	_objc_msgSend_fpret
	END_ENTRY _objc_msgSend_fpret_fixedup


/********************************************************************
 *
 * double objc_msgSend_fp2ret(id self, SEL _cmd,...);
 * Used for `complex long double` return only.
 *
 ********************************************************************/

	ENTRY _objc_msgSend_fp2ret
	UNWIND _objc_msgSend_fp2ret, NoFrame
	
	GetIsaCheckNil FP2RET		// r10 = self->isa, or return zero
	CacheLookup FP2RET, CALL	// calls IMP on success

	GetIsaSupport FP2RET
	NilTestReturnZero FP2RET
	
// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSend_fp2ret


	ENTRY _objc_msgLookup_fp2ret
	
	GetIsaCheckNil FP2RET		// r10 = self->isa, or return zero IMP
	CacheLookup FP2RET, LOOKUP	// returns IMP on success

	GetIsaSupport FP2RET
	NilTestReturnIMP FP2RET

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgLookup_uncached

	END_ENTRY _objc_msgLookup_fp2ret


	ENTRY _objc_msgSend_fp2ret_fixup
	int3
	END_ENTRY _objc_msgSend_fp2ret_fixup

	
	STATIC_ENTRY _objc_msgSend_fp2ret_fixedup
	// Load _cmd from the message_ref
	movq	8(%a2), %a2
	jmp	_objc_msgSend_fp2ret
	END_ENTRY _objc_msgSend_fp2ret_fixedup


/********************************************************************
 *
 * void	objc_msgSend_stret(void *st_addr, id self, SEL _cmd, ...);
 *
 * objc_msgSend_stret is the struct-return form of msgSend.
 * The ABI calls for %a1 to be used as the address of the structure
 * being returned, with the parameters in the succeeding locations.
 *
 * On entry:	%a1 is the address where the structure is returned,
 *		%a2 is the message receiver,
 *		%a3 is the selector
 ********************************************************************/

	ENTRY _objc_msgSend_stret
	UNWIND _objc_msgSend_stret, NoFrame

	GetIsaCheckNil STRET		// r10 = self->isa, or return zero
	CacheLookup STRET, CALL		// calls IMP on success

	GetIsaSupport STRET
	NilTestReturnZero STRET

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgSend_stret_uncached

	END_ENTRY _objc_msgSend_stret


	ENTRY _objc_msgLookup_stret
	
	GetIsaCheckNil STRET		// r10 = self->isa, or return zero IMP
	CacheLookup STRET, LOOKUP	// returns IMP on success

	GetIsaSupport STRET
	NilTestReturnIMP STRET

// cache miss: go search the method lists
LCacheMiss:
	// isa still in r10
	jmp	__objc_msgLookup_stret_uncached

	END_ENTRY _objc_msgLookup_stret


	ENTRY _objc_msgSend_stret_fixup
	int3
	END_ENTRY _objc_msgSend_stret_fixup


	STATIC_ENTRY _objc_msgSend_stret_fixedup
	// Load _cmd from the message_ref
	movq	8(%a3), %a3
	jmp	_objc_msgSend_stret
	END_ENTRY _objc_msgSend_stret_fixedup


/********************************************************************
 *
 * void objc_msgSendSuper_stret(void *st_addr, struct objc_super *super, SEL _cmd, ...);
 *
 * struct objc_super {
 *		id	receiver;
 *		Class	class;
 * };
 *
 * objc_msgSendSuper_stret is the struct-return form of msgSendSuper.
 * The ABI calls for (sp+4) to be used as the address of the structure
 * being returned, with the parameters in the succeeding registers.
 *
 * On entry:	%a1 is the address where the structure is returned,
 *		%a2 is the address of the objc_super structure,
 *		%a3 is the selector
 *
 ********************************************************************/

	ENTRY _objc_msgSendSuper_stret
	UNWIND _objc_msgSendSuper_stret, NoFrame
	
// search the cache (objc_super in %a2)
	movq	class(%a2), %r10	// class = objc_super->class
	movq	receiver(%a2), %a2	// load real receiver
	CacheLookup STRET, CALL		// calls IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// class still in r10
	jmp	__objc_msgSend_stret_uncached
	
	END_ENTRY _objc_msgSendSuper_stret


/********************************************************************
 * id objc_msgSendSuper2_stret
 ********************************************************************/

	ENTRY _objc_msgSendSuper2_stret
	UNWIND _objc_msgSendSuper2_stret, NoFrame
	
// search the cache (objc_super in %a2)
	movq	class(%a2), %r10	// class = objc_super->class
	movq	receiver(%a2), %a2	// load real receiver
	movq	8(%r10), %r10		// class = class->superclass
	CacheLookup STRET, CALL		// calls IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// superclass still in r10
	jmp	__objc_msgSend_stret_uncached

	END_ENTRY _objc_msgSendSuper2_stret


	ENTRY _objc_msgLookupSuper2_stret
	
// search the cache (objc_super in %a2)
	movq	class(%a2), %r10	// class = objc_super->class
	movq	receiver(%a2), %a2	// load real receiver
	movq	8(%r10), %r10		// class = class->superclass
	CacheLookup STRET, LOOKUP	// returns IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// superclass still in r10
	jmp	__objc_msgLookup_stret_uncached

	END_ENTRY _objc_msgLookupSuper2_stret

	
	ENTRY _objc_msgSendSuper2_stret_fixup
	int3
	END_ENTRY _objc_msgSendSuper2_stret_fixup

	
	STATIC_ENTRY _objc_msgSendSuper2_stret_fixedup
	// Load _cmd from the message_ref
	movq	8(%a3), %a3
	jmp	_objc_msgSendSuper2_stret
	END_ENTRY _objc_msgSendSuper2_stret_fixedup


/********************************************************************
 *
 * _objc_msgSend_uncached
 * _objc_msgSend_stret_uncached
 * The uncached method lookup.
 *
 ********************************************************************/

	STATIC_ENTRY __objc_msgSend_uncached
	UNWIND __objc_msgSend_uncached, FrameWithNoSaves
	
	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r10 is the searched class

	// r10 is already the class to search
	MethodTableLookup NORMAL	// r11 = IMP
	jmp	*%r11			// goto *imp

	END_ENTRY __objc_msgSend_uncached

	
	STATIC_ENTRY __objc_msgSend_stret_uncached
	UNWIND __objc_msgSend_stret_uncached, FrameWithNoSaves
	
	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r10 is the searched class

	// r10 is already the class to search
	MethodTableLookup STRET		// r11 = IMP
	jmp	*%r11			// goto *imp

	END_ENTRY __objc_msgSend_stret_uncached

	
	STATIC_ENTRY __objc_msgLookup_uncached
	UNWIND __objc_msgLookup_uncached, FrameWithNoSaves
	
	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r10 is the searched class

	// r10 is already the class to search
	MethodTableLookup NORMAL	// r11 = IMP
	ret

	END_ENTRY __objc_msgLookup_uncached

	
	STATIC_ENTRY __objc_msgLookup_stret_uncached
	UNWIND __objc_msgLookup_stret_uncached, FrameWithNoSaves
	
	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r10 is the searched class

	// r10 is already the class to search
	MethodTableLookup STRET		// r11 = IMP
	ret

	END_ENTRY __objc_msgLookup_stret_uncached

	
/********************************************************************
*
* id _objc_msgForward(id self, SEL _cmd,...);
*
* _objc_msgForward and _objc_msgForward_stret are the externally-callable
*   functions returned by things like method_getImplementation().
* _objc_msgForward_impcache is the function pointer actually stored in
*   method caches.
*
********************************************************************/

	STATIC_ENTRY __objc_msgForward_impcache
	// Method cache version

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band condition register is NE for stret, EQ otherwise.

	je	__objc_msgForward_stret
	jmp	__objc_msgForward

	END_ENTRY __objc_msgForward_impcache
	
	
	ENTRY __objc_msgForward
	// Non-stret version

	movq	__objc_forward_handler(%rip), %r11
	jmp	*%r11

	END_ENTRY __objc_msgForward


	ENTRY __objc_msgForward_stret
	// Struct-return version

	movq	__objc_forward_stret_handler(%rip), %r11
	jmp	*%r11

	END_ENTRY __objc_msgForward_stret


	ENTRY _objc_msgSend_debug
	jmp	_objc_msgSend
	END_ENTRY _objc_msgSend_debug

	ENTRY _objc_msgSendSuper2_debug
	jmp	_objc_msgSendSuper2
	END_ENTRY _objc_msgSendSuper2_debug

	ENTRY _objc_msgSend_stret_debug
	jmp	_objc_msgSend_stret
	END_ENTRY _objc_msgSend_stret_debug

	ENTRY _objc_msgSendSuper2_stret_debug
	jmp	_objc_msgSendSuper2_stret
	END_ENTRY _objc_msgSendSuper2_stret_debug

	ENTRY _objc_msgSend_fpret_debug
	jmp	_objc_msgSend_fpret
	END_ENTRY _objc_msgSend_fpret_debug

	ENTRY _objc_msgSend_fp2ret_debug
	jmp	_objc_msgSend_fp2ret
	END_ENTRY _objc_msgSend_fp2ret_debug


	ENTRY _objc_msgSend_noarg
	jmp	_objc_msgSend
	END_ENTRY _objc_msgSend_noarg


	ENTRY _method_invoke

	// See if this is a small method.
	testb	$1, %a2b
	jnz	L_method_invoke_small

	// We can directly load the IMP from big methods.
	movq	method_imp(%a2), %r11
	movq	method_name(%a2), %a2
	jmp	*%r11

L_method_invoke_small:
	// Small methods require a call to handle swizzling.
	SAVE_REGS
	movq	%a2, %a1
	call	__method_getImplementationAndName
	movq	%rdx, %r10
	movq	%rax, %r11
	RESTORE_REGS
	movq	%r10, %a2
	jmp	*%r11

	END_ENTRY _method_invoke


	ENTRY _method_invoke_stret

	// See if this is a small method.
	testb	$1, %a3b
	jnz	L_method_invoke_stret_small

	// We can directly load the IMP from big methods.
	movq	method_imp(%a3), %r11
	movq	method_name(%a3), %a3
	jmp	*%r11

L_method_invoke_stret_small:
	// Small methods require a call to handle swizzling.
	SAVE_REGS
	movq	%a3, %a1
	call	__method_getImplementationAndName
	movq	%rdx, %r10
	movq	%rax, %r11
	RESTORE_REGS
	movq	%r10, %a3
	jmp	*%r11

	END_ENTRY _method_invoke_stret


.section __DATA,__objc_msg_break
.quad 0
.quad 0

#endif
