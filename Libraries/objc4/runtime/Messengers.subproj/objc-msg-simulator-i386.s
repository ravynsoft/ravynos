/*
 * Copyright (c) 1999-2009 Apple Inc.  All Rights Reserved.
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
#if defined(__i386__)  &&  TARGET_OS_SIMULATOR

#include "objc-config.h"

.data

// _objc_restartableRanges is used by method dispatch
// to get the critical regions for which method caches 
// cannot be garbage collected.

.macro RestartableEntry
	.long	$0
	.long	0
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
	RestartableEntry _objc_msgSend_stret
	RestartableEntry _objc_msgSendSuper
	RestartableEntry _objc_msgSendSuper2
	RestartableEntry _objc_msgSendSuper_stret
	RestartableEntry _objc_msgSendSuper2_stret
	RestartableEntry _objc_msgLookup
	RestartableEntry _objc_msgLookup_fpret
	RestartableEntry _objc_msgLookup_stret
	RestartableEntry _objc_msgLookupSuper2
	RestartableEntry _objc_msgLookupSuper2_stret
	.fill	16, 1, 0


/********************************************************************
 * Names for relative labels
 * DO NOT USE THESE LABELS ELSEWHERE
 * Reserved labels: 5: 6: 7: 8: 9:
 ********************************************************************/
#define LCacheMiss 	5
#define LCacheMiss_f 	5f
#define LCacheMiss_b 	5b
#define LNilTestDone 	6
#define LNilTestDone_f 	6f
#define LNilTestDone_b 	6b
#define LNilTestSlow 	7
#define LNilTestSlow_f 	7f
#define LNilTestSlow_b 	7b
#define LGetIsaDone 	8
#define LGetIsaDone_f 	8f
#define LGetIsaDone_b 	8b
#define LGetIsaSlow 	9
#define LGetIsaSlow_f 	9f
#define LGetIsaSlow_b 	9b

/********************************************************************
 * Macro parameters
 ********************************************************************/


#define NORMAL 0
#define FPRET 1
#define STRET 2

#define CALL 100
#define GETIMP 101
#define LOOKUP 102


/********************************************************************
 *
 * Structure definitions.
 *
 ********************************************************************/

// Offsets from %esp
#define self            4
#define super           4
#define selector        8
#define marg_size       12
#define marg_list       16
#define first_arg       12

#define struct_addr     4

#define self_stret      8
#define super_stret     8
#define selector_stret  12
#define marg_size_stret 16
#define marg_list_stret 20

// objc_super parameter to sendSuper
#define receiver        0
#define class           4

// Selected field offsets in class structure
#define isa             0
#define superclass	4
#define cache_buckets	8
#define cache_mask	12

// Method cache
#define cached_sel	0
#define cached_imp	4

// Method descriptor
#define method_name     0
#define method_imp      8


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
	.align	2, 0x90
$0:
.endmacro

.macro STATIC_ENTRY
	.text
	.private_extern	$0
	.align	4, 0x90
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
	.long $0
	.set  LUnwind$0, LExit$0 - $0
	.long LUnwind$0
	.long $1
	.long 0	 /* no personality */
	.long 0  /* no LSDA */
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

	pushl	%ebp
	movl	%esp, %ebp

	subl	$$(8+5*16), %esp

	movdqa  %xmm3, 4*16(%esp)
	movdqa  %xmm2, 3*16(%esp)
	movdqa  %xmm1, 2*16(%esp)
	movdqa  %xmm0, 1*16(%esp)

.endmacro


//////////////////////////////////////////////////////////////////////
//
// RESTORE_REGS
//
// Restore all argument registers and pop the stack frame created by
// SAVE_REGS.
//////////////////////////////////////////////////////////////////////

.macro RESTORE_REGS

	movdqa  4*16(%esp), %xmm3
	movdqa  3*16(%esp), %xmm2
	movdqa  2*16(%esp), %xmm1
	movdqa  1*16(%esp), %xmm0

	leave

.endmacro
/////////////////////////////////////////////////////////////////////
//
// CacheLookup	return-type, caller
//
// Locate the implementation for a selector in a class method cache.
//
// Takes: 
//	  $0 = NORMAL, FPRET, STRET
//	  $1 = CALL, LOOKUP, GETIMP
//	  ecx = selector to search for
//	  edx = class to search
//
// On exit: ecx clobbered
//	    (found) calls or returns IMP in eax, eq/ne set for forwarding
//	    (not found) jumps to LCacheMiss, class still in edx
//
/////////////////////////////////////////////////////////////////////

.macro CacheHit

	// CacheHit must always be preceded by a not-taken `jne` instruction
	// in case the imp is _objc_msgForward_impcache.

	// eax = found bucket
	
.if $1 == GETIMP
	movl	cached_imp(%eax), %eax	// return imp
	cmpl	$$0, %eax
	jz	9f		// don't xor a nil imp
	xorl	%edx, %eax	// xor the isa with the imp
9:	ret

.else

.if $1 == CALL
	xorl	cached_imp(%eax), %edx	// xor imp and isa
.if $0 != STRET
	// ne already set for forwarding by `xor`
.else
	cmp	%eax, %eax		// set eq for stret forwarding
.endif
	jmp *%edx	// call imp

.elseif $1 == LOOKUP
	movl	cached_imp(%eax), %eax	// return imp
	xorl	%edx, %eax	// xor isa into imp
	ret

.else
.abort oops
.endif

.endif

.endmacro


.macro	CacheLookup

	movzwl	cache_mask(%edx), %eax		// eax = mask
	andl	%ecx, %eax		// eax = SEL & mask
	shll	$$3, %eax		// eax = offset = (SEL & mask) * 8
	addl	cache_buckets(%edx), %eax  // eax = bucket = buckets+offset
	cmpl	cached_sel(%eax), %ecx	// if (bucket->sel != SEL)
	jne	1f			//     scan more
	// The `jne` above sets flags for CacheHit
	CacheHit $0, $1			// call or return imp

1:
	// loop
	cmpl	$$1, cached_sel(%eax)
	jbe	3f			// if (bucket->sel <= 1) wrap or miss
	
	addl	$$8, %eax		// bucket++
2:
	cmpl	cached_sel(%eax), %ecx	// if (bucket->sel != sel)
	jne	1b			//     scan more
	// The `jne` above sets flags for CacheHit
	CacheHit $0, $1			// call or return imp

3:	
	// wrap or miss
	jb	LCacheMiss_f		// if (bucket->sel < 1) cache miss
	// wrap
	movl	cached_imp(%eax), %eax	// bucket->imp is really first bucket
	jmp	2f

	// Clone scanning loop to miss instead of hang when cache is corrupt.
	// The slow path may detect any corruption and halt later.

1:
	// loop
	cmpl	$$1, cached_sel(%eax)
	jbe	3f			// if (bucket->sel <= 1) wrap or miss
	
	addl	$$8, %eax		// bucket++
2:
	cmpl	cached_sel(%eax), %ecx	// if (bucket->sel != sel)
	jne	1b			//     scan more
	// The `jne` above sets flags for CacheHit
	CacheHit $0, $1			// call or return imp

3:	
	// double wrap or miss
	jmp	LCacheMiss_f
	
.endmacro


/////////////////////////////////////////////////////////////////////
//
// MethodTableLookup NORMAL|STRET
//
// Takes:
//	  receiver (not struct objc_super) and selector on stack
// 	  edx = class to search
//
// On exit: IMP in eax, eq/ne set for forwarding
//
/////////////////////////////////////////////////////////////////////

.macro MethodTableLookup
	SAVE_REGS

.if $0 == NORMAL
	movl	self+4(%ebp), %eax
	movl    selector+4(%ebp), %ecx
.else
	movl	self_stret+4(%ebp), %eax
	movl    selector_stret+4(%ebp), %ecx
.endif
	
	// lookUpImpOrForward(obj, sel, cls, LOOKUP_INITIALIZE | LOOKUP_RESOLVER)
	movl	$$3,  12(%esp)		// LOOKUP_INITIALIZE | LOOKUP_RESOLVER
	movl	%edx, 8(%esp)		// class
	movl	%ecx, 4(%esp)		// selector
	movl	%eax, 0(%esp)		// receiver
	call	_lookUpImpOrForward

	// imp in eax

.if $0 == NORMAL
	test	%eax, %eax	// set ne for stret forwarding
.else
	cmp	%eax, %eax	// set eq for nonstret forwarding
.endif

	RESTORE_REGS

.endmacro


/////////////////////////////////////////////////////////////////////
//
// NilTest return-type
//
// Takes:	$0 = NORMAL or FPRET or STRET
//		eax = receiver
//
// On exit: 	Loads non-nil receiver in eax and self(esp) or self_stret(esp),
//		or returns zero.
//
// NilTestReturnZero return-type
//
// Takes:	$0 = NORMAL or FPRET or STRET
//		eax = receiver
//
// On exit: 	Loads non-nil receiver in eax and self(esp) or self_stret(esp),
//		or returns zero.
//
// NilTestReturnIMP return-type
//
// Takes:	$0 = NORMAL or FPRET or STRET
//		eax = receiver
//
// On exit: 	Loads non-nil receiver in eax and self(esp) or self_stret(esp),
//		or returns an IMP in eax that returns zero.
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
.endmacro

.macro ZeroReturnSTRET
	// empty
.endmacro

	STATIC_ENTRY __objc_msgNil
	ZeroReturn
	ret
	END_ENTRY __objc_msgNil

	STATIC_ENTRY __objc_msgNil_fpret
	ZeroReturnFPRET
	ret
	END_ENTRY __objc_msgNil_fpret

	STATIC_ENTRY __objc_msgNil_stret
	ZeroReturnSTRET
	ret $4
	END_ENTRY __objc_msgNil_stret


.macro NilTest
	testl	%eax, %eax
	jz	LNilTestSlow_f
LNilTestDone:
.endmacro

.macro NilTestReturnZero
	.align 3
LNilTestSlow:

.if $0 == NORMAL
	ZeroReturn
	ret
.elseif $0 == FPRET
	ZeroReturnFPRET
	ret
.elseif $0 == STRET
	ZeroReturnSTRET
	ret $$4
.else
.abort oops
.endif
.endmacro

.macro NilTestReturnIMP
	.align 3
LNilTestSlow:

	call	1f
1:	pop	%eax
.if $0 == NORMAL
	leal	__objc_msgNil-1b(%eax), %eax
.elseif $0 == FPRET
	leal	__objc_msgNil_fpret-1b(%eax), %eax
.elseif $0 == STRET
	leal	__objc_msgNil_stret-1b(%eax), %eax
.else
.abort oops
.endif
	ret
.endmacro


/********************************************************************
 * IMP _cache_getImp(Class cls, SEL sel)
 *
 * If found, returns method implementation.
 * If not found, returns NULL.
 ********************************************************************/

	STATIC_ENTRY _cache_getImp

// load the class and selector
	movl    selector(%esp), %ecx
	movl	self(%esp), %edx

	CacheLookup NORMAL, GETIMP	// returns IMP on success

LCacheMiss:
// cache miss, return nil
	xorl    %eax, %eax
	ret

	END_ENTRY _cache_getImp


/********************************************************************
 *
 * id objc_msgSend(id self, SEL _cmd, ...);
 * IMP objc_msgLookup(id self, SEL _cmd, ...);
 *
 * objc_msgLookup ABI:
 * IMP returned in eax
 * Forwarding returned in Z flag
 * edx reserved for our use but not used
 *
 ********************************************************************/

	ENTRY _objc_msgSend
	UNWIND _objc_msgSend, NoFrame
	
	movl    selector(%esp), %ecx
	movl	self(%esp), %eax

	NilTest NORMAL

	movl	isa(%eax), %edx		// class = self->isa
	CacheLookup NORMAL, CALL	// calls IMP on success

	NilTestReturnZero NORMAL

LCacheMiss:
	// isa still in edx
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSend


	ENTRY _objc_msgLookup
	UNWIND _objc_msgLookup, NoFrame
	
	movl    selector(%esp), %ecx
	movl	self(%esp), %eax

	NilTest NORMAL

	movl	isa(%eax), %edx		// class = self->isa
	CacheLookup NORMAL, LOOKUP	// returns IMP on success

	NilTestReturnIMP NORMAL

LCacheMiss:
	// isa still in edx
	jmp	__objc_msgLookup_uncached

	END_ENTRY _objc_msgLookup


/********************************************************************
 *
 * id objc_msgSendSuper(struct objc_super *super, SEL _cmd, ...);
 *
 ********************************************************************/

	ENTRY _objc_msgSendSuper
	UNWIND _objc_msgSendSuper, NoFrame

	movl    selector(%esp), %ecx
	movl	super(%esp), %eax	// struct objc_super
	movl	class(%eax), %edx	// struct objc_super->class
	movl	receiver(%eax), %eax	// struct objc_super->receiver
	movl	%eax, super(%esp)	// replace super arg with receiver
	CacheLookup NORMAL, CALL	// calls IMP on success

LCacheMiss:	
	// class still in edx
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSendSuper


/********************************************************************
 *
 * id objc_msgSendSuper2(struct objc_super *super, SEL _cmd, ...);
 * IMP objc_msgLookupSuper2(struct objc_super *super, SEL _cmd, ...);
 *
 ********************************************************************/

	ENTRY _objc_msgSendSuper2
	UNWIND _objc_msgSendSuper2, NoFrame

	movl    selector(%esp), %ecx
	movl	super(%esp), %eax	// struct objc_super
	movl	class(%eax), %edx	// struct objc_super->class
	movl	receiver(%eax), %eax	// struct objc_super->receiver
	movl	%eax, super(%esp)	// replace super arg with receiver
	movl	superclass(%edx), %edx	// edx = objc_super->class->super_class
	CacheLookup NORMAL, CALL	// calls IMP on success

LCacheMiss:
	// class still in edx
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSendSuper2


	ENTRY _objc_msgLookupSuper2
	UNWIND _objc_msgLookupSuper2, NoFrame

	movl    selector(%esp), %ecx
	movl	super(%esp), %eax	// struct objc_super
	movl	class(%eax), %edx	// struct objc_super->class
	movl	receiver(%eax), %eax	// struct objc_super->receiver
	movl	%eax, super(%esp)	// replace super arg with receiver
	movl	superclass(%edx), %edx	// edx = objc_super->class->super_class
	CacheLookup NORMAL, LOOKUP	// returns IMP on success

LCacheMiss:
	// class still in edx
	jmp	__objc_msgLookup_uncached

	END_ENTRY _objc_msgLookupSuper2


/********************************************************************
 *
 * double objc_msgSend_fpret(id self, SEL _cmd, ...);
 * IMP objc_msgLookup_fpret(id self, SEL _cmd, ...);
 *
 ********************************************************************/

	ENTRY _objc_msgSend_fpret
	UNWIND _objc_msgSend_fpret, NoFrame

	movl    selector(%esp), %ecx
	movl	self(%esp), %eax

	NilTest FPRET

	movl	isa(%eax), %edx		// class = self->isa
	CacheLookup FPRET, CALL		// calls IMP on success

	NilTestReturnZero FPRET
	
LCacheMiss:	
	// class still in edx
	jmp	__objc_msgSend_uncached

	END_ENTRY _objc_msgSend_fpret


	ENTRY _objc_msgLookup_fpret
	UNWIND _objc_msgLookup_fpret, NoFrame

	movl    selector(%esp), %ecx
	movl	self(%esp), %eax

	NilTest FPRET

	movl	isa(%eax), %edx		// class = self->isa
	CacheLookup FPRET, LOOKUP	// returns IMP on success

	NilTestReturnIMP FPRET
	
LCacheMiss:	
	// class still in edx
	jmp	__objc_msgLookup_uncached

	END_ENTRY _objc_msgLookup_fpret
	

/********************************************************************
 *
 * void objc_msgSend_stret(void *st_addr, id self, SEL _cmd, ...);
 * IMP objc_msgLookup_stret(void *st_addr, id self, SEL _cmd, ...);
 *
 ********************************************************************/

	ENTRY _objc_msgSend_stret
	UNWIND _objc_msgSend_stret, NoFrame

	movl	selector_stret(%esp), %ecx
	movl	self_stret(%esp), %eax

	NilTest STRET

	movl	isa(%eax), %edx		// class = self->isa
	CacheLookup STRET, CALL		// calls IMP on success

	NilTestReturnZero STRET
	
LCacheMiss:
	// class still in edx
	jmp	__objc_msgSend_stret_uncached

	END_ENTRY _objc_msgSend_stret


	ENTRY _objc_msgLookup_stret
	UNWIND _objc_msgLookup_stret, NoFrame

	movl	selector_stret(%esp), %ecx
	movl	self_stret(%esp), %eax

	NilTest STRET

	movl	isa(%eax), %edx		// class = self->isa
	CacheLookup STRET, LOOKUP	// returns IMP on success

	NilTestReturnIMP STRET
	
LCacheMiss:
	// class still in edx
	jmp	__objc_msgLookup_stret_uncached

	END_ENTRY _objc_msgLookup_stret

	
/********************************************************************
 *
 * void objc_msgSendSuper_stret(void *st_addr, struct objc_super *super, SEL _cmd, ...);
 *
 ********************************************************************/

	ENTRY _objc_msgSendSuper_stret
	UNWIND _objc_msgSendSuper_stret, NoFrame

	movl	selector_stret(%esp), %ecx
	movl	super_stret(%esp), %eax	// struct objc_super
	movl	class(%eax), %edx	// struct objc_super->class
	movl	receiver(%eax), %eax	// struct objc_super->receiver
	movl	%eax, super_stret(%esp)	// replace super arg with receiver
	CacheLookup STRET, CALL		// calls IMP on success

LCacheMiss:
	// class still in edx
	jmp	__objc_msgSend_stret_uncached

	END_ENTRY _objc_msgSendSuper_stret


/********************************************************************
 *
 * void objc_msgSendSuper2_stret(void *st_addr, struct objc_super *super, SEL _cmd, ...);
 * IMP objc_msgLookupSuper2_stret(void *st_addr, struct objc_super *super, SEL _cmd, ...);
 *
 ********************************************************************/

	ENTRY _objc_msgSendSuper2_stret
	UNWIND _objc_msgSendSuper2_stret, NoFrame

	movl	selector_stret(%esp), %ecx
	movl	super_stret(%esp), %eax	// struct objc_super
	movl	class(%eax), %edx	// struct objc_super->class
	movl	receiver(%eax), %eax	// struct objc_super->receiver
	movl	%eax, super_stret(%esp)	// replace super arg with receiver
	mov	superclass(%edx), %edx	// edx = objc_super->class->super_class
	CacheLookup STRET, CALL		// calls IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// class still in edx
	jmp	__objc_msgSend_stret_uncached

	END_ENTRY _objc_msgSendSuper2_stret


	ENTRY _objc_msgLookupSuper2_stret
	UNWIND _objc_msgLookupSuper2_stret, NoFrame

	movl	selector_stret(%esp), %ecx
	movl	super_stret(%esp), %eax	// struct objc_super
	movl	class(%eax), %edx	// struct objc_super->class
	movl	receiver(%eax), %eax	// struct objc_super->receiver
	movl	%eax, super_stret(%esp)	// replace super arg with receiver
	mov	superclass(%edx), %edx	// edx = objc_super->class->super_class
	CacheLookup STRET, LOOKUP	// returns IMP on success

// cache miss: go search the method lists
LCacheMiss:
	// class still in edx
	jmp	__objc_msgLookup_stret_uncached

	END_ENTRY _objc_msgLookupSuper2_stret


/********************************************************************
 *
 * _objc_msgSend_uncached
 * _objc_msgSend_stret_uncached
 * _objc_msgLookup_uncached
 * _objc_msgLookup_stret_uncached
 *
 * The uncached method lookup.
 *
 ********************************************************************/

	STATIC_ENTRY __objc_msgSend_uncached
	UNWIND __objc_msgSend_uncached, FrameWithNoSaves

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band edx is the searched class

	// edx is already the class to search
	MethodTableLookup NORMAL
	jmp	*%eax		// call imp

	END_ENTRY __objc_msgSend_uncached

	
	STATIC_ENTRY __objc_msgSend_stret_uncached
	UNWIND __objc_msgSend_stret_uncached, FrameWithNoSaves

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band edx is the searched class

	// edx is already the class to search
	MethodTableLookup STRET
	jmp	*%eax		// call imp

	END_ENTRY __objc_msgSend_stret_uncached


	STATIC_ENTRY __objc_msgLookup_uncached
	UNWIND __objc_msgLookup_uncached, FrameWithNoSaves

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band edx is the searched class

	// edx is already the class to search
	MethodTableLookup NORMAL	// eax = IMP
	ret

	END_ENTRY __objc_msgLookup_uncached

	
	STATIC_ENTRY __objc_msgLookup_stret_uncached
	UNWIND __objc_msgLookup_stret_uncached, FrameWithNoSaves

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band edx is the searched class

	// edx is already the class to search
	MethodTableLookup STRET		// eax = IMP
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

	.non_lazy_symbol_pointer
L_forward_handler:
	.indirect_symbol __objc_forward_handler
	.long 0
L_forward_stret_handler:
	.indirect_symbol __objc_forward_stret_handler
	.long 0

	STATIC_ENTRY __objc_msgForward_impcache
	// Method cache version
	
	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band condition register is NE for stret, EQ otherwise.

	je	__objc_msgForward_stret
	jmp	__objc_msgForward
	
	END_ENTRY _objc_msgForward_impcache

	
	ENTRY __objc_msgForward
	// Non-struct return version

	call	1f
1:	popl	%edx
	movl	L_forward_handler-1b(%edx), %edx
	jmp	*(%edx)

	END_ENTRY __objc_msgForward


	ENTRY __objc_msgForward_stret
	// Struct return version

	call	1f
1:	popl	%edx
	movl	L_forward_stret_handler-1b(%edx), %edx
	jmp	*(%edx)

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


	ENTRY _objc_msgSend_noarg
	jmp	_objc_msgSend
	END_ENTRY _objc_msgSend_noarg
	

	ENTRY _method_invoke

	// See if this is a small method.
	testb	$1, selector(%esp)
	jnz	L_method_invoke_small

	// We can directly load the IMP from big methods.
	movl	selector(%esp), %ecx
	movl	method_name(%ecx), %edx
	movl	method_imp(%ecx), %eax
	movl	%edx, selector(%esp)
	jmp	*%eax

L_method_invoke_small:
	// Small methods require a call to handle swizzling.
	SAVE_REGS

	movl	selector+4(%ebp), %eax
	movl	%eax, 0(%esp)
	call	__method_getImplementationAndName
	RESTORE_REGS
	movl	%edx, selector(%esp)
	jmp	*%eax

	END_ENTRY _method_invoke


	ENTRY _method_invoke_stret

	// See if this is a small method.
	testb	$1, selector_stret(%esp)
	jnz	L_method_invoke_stret_small

	// We can directly load the IMP from big methods.
	movl	selector_stret(%esp), %ecx
	movl	method_name(%ecx), %edx
	movl	method_imp(%ecx), %eax
	movl	%edx, selector_stret(%esp)
	jmp	*%eax
	
L_method_invoke_stret_small:
	// Small methods require a call to handle swizzling.
	SAVE_REGS

	movl	selector_stret+4(%ebp), %eax
	movl	%eax, 0(%esp)
	call	__method_getImplementationAndName
	RESTORE_REGS
	movl	%edx, selector_stret(%esp)
	jmp	*%eax

	END_ENTRY _method_invoke_stret
	

.section __DATA,__objc_msg_break
.long 0
.long 0

#endif
