/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2007 Apple Computer, Inc.  All Rights Reserved.
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
/********************************************************************
 * 
 *  objc-msg-arm.s - ARM code to support objc messaging
 *
 ********************************************************************/

#ifdef __arm__
	
#include <arm/arch.h>
#include "objc-config.h"
#include "isa.h"

#ifndef _ARM_ARCH_7
#   error requires armv7
#endif

// Set FP=1 on architectures that pass parameters in floating-point registers
#if __ARM_ARCH_7K__
#   define FP 1
#else
#   define FP 0
#endif

#if FP

#   if !__ARM_NEON__
#       error sorry
#   endif

#   define FP_RETURN_ZERO \
	vmov.i32  q0, #0  ; \
	vmov.i32  q1, #0  ; \
	vmov.i32  q2, #0  ; \
	vmov.i32  q3, #0

#   define FP_SAVE \
	vpush	{q0-q3}

#   define FP_RESTORE \
	vpop	{q0-q3}

#else

#   define FP_RETURN_ZERO
#   define FP_SAVE
#   define FP_RESTORE

#endif

.syntax unified	
	
#define MI_EXTERN(var) \
	.non_lazy_symbol_pointer                        ;\
L##var##$$non_lazy_ptr:                                 ;\
	.indirect_symbol var                            ;\
	.long 0

#define MI_GET_EXTERN(reg,var)  \
	movw	reg, :lower16:(L##var##$$non_lazy_ptr-7f-4)  ;\
	movt	reg, :upper16:(L##var##$$non_lazy_ptr-7f-4)  ;\
7:	add	reg, pc                                      ;\
	ldr	reg, [reg]
	
#define MI_GET_ADDRESS(reg,var)  \
	movw	reg, :lower16:(var-7f-4)  ;\
	movt	reg, :upper16:(var-7f-4)  ;\
7:	add	reg, pc                                     ;\


.data

#if SUPPORT_INDEXED_ISA

	.align 2
	.globl _objc_indexed_classes
_objc_indexed_classes:
	.fill ISA_INDEX_COUNT, 4, 0

#endif



// _objc_restartableRanges is used by method dispatch
// caching code to figure out whether any threads are actively 
// in the cache for dispatching.  The labels surround the asm code
// that do cache lookups.  The tables are zero-terminated.

.macro RestartableEntry
	.long	LLookupStart$0
	.long	0
	.short	LLookupEnd$0 - LLookupStart$0
	.short	0xffff // poor ol' armv7 doesn't support kernel based recovery
	.long	0
.endmacro

	.align 4
	.private_extern _objc_restartableRanges
_objc_restartableRanges:
	RestartableEntry _cache_getImp
	RestartableEntry _objc_msgSend
	RestartableEntry _objc_msgSend_stret
	RestartableEntry _objc_msgSendSuper
	RestartableEntry _objc_msgSendSuper_stret
	RestartableEntry _objc_msgSendSuper2
	RestartableEntry _objc_msgSendSuper2_stret
	RestartableEntry _objc_msgLookup
	RestartableEntry _objc_msgLookup_stret
	RestartableEntry _objc_msgLookupSuper2
	RestartableEntry _objc_msgLookupSuper2_stret
	.fill	16, 1, 0

	
/********************************************************************
 * Names for relative labels
 * DO NOT USE THESE LABELS ELSEWHERE
 * Reserved labels: 6: 7: 8: 9:
 ********************************************************************/
// 6: used by CacheLookup
// 7: used by MI_GET_ADDRESS etc
// 8: used by CacheLookup
#define LNilReceiver 	9
#define LNilReceiver_f 	9f
#define LNilReceiver_b 	9b


/********************************************************************
 * Macro parameters
 ********************************************************************/

#define NORMAL 0
#define STRET 1


/********************************************************************
 *
 * Structure definitions.
 *
 ********************************************************************/

/* objc_super parameter to sendSuper */
#define RECEIVER         0
#define CLASS            4

/* Selected field offsets in class structure */
#define ISA              0
#define SUPERCLASS       4
#define CACHE            8
#define CACHE_MASK      12

/* Field offsets in method cache bucket */
#define CACHED_SEL       0
#define CACHED_IMP       4

/* Selected field offsets in method structure */
#define METHOD_NAME      0
#define METHOD_TYPES     4
#define METHOD_IMP       8


//////////////////////////////////////////////////////////////////////
//
// ENTRY		functionName
//
// Assembly directives to begin an exported function.
//
// Takes: functionName - name of the exported function
//////////////////////////////////////////////////////////////////////

.macro ENTRY /* name */
	.text
	.thumb
	.align 5
	.globl $0
	.thumb_func
$0:	
.endmacro

.macro STATIC_ENTRY /*name*/
	.text
	.thumb
	.align 5
	.private_extern $0
	.thumb_func
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

.macro END_ENTRY /* name */
LExit$0:	
.endmacro


//////////////////////////////////////////////////////////////////////
//
// SAVE_REGS
//
// Create a stack frame and save all argument registers in preparation
// for a function call.
//////////////////////////////////////////////////////////////////////

.macro SAVE_REGS

	stmfd	sp!, {r0-r3,r7,lr}
	add	r7, sp, #16
	sub	sp, #8			// align stack
	FP_SAVE

.endmacro


//////////////////////////////////////////////////////////////////////
//
// RESTORE_REGS
//
// Restore all argument registers and pop the stack frame created by
// SAVE_REGS.
//////////////////////////////////////////////////////////////////////

.macro RESTORE_REGS

	FP_RESTORE
	add	sp, #8			// align stack
	ldmfd	sp!, {r0-r3,r7,lr}

.endmacro

/////////////////////////////////////////////////////////////////////
//
// CacheLookup	NORMAL|STRET <function>
// CacheLookup2	NORMAL|STRET <function>
//
// Locate the implementation for a selector in a class's method cache.
//
// Takes: 
//	  $0 = NORMAL, STRET
//	  r0 or r1 (STRET) = receiver
//	  r1 or r2 (STRET) = selector
//	  r9 = class to search in
//
// On exit: r9 clobbered
//	    (found) continues after CacheLookup, IMP in r12, eq set
//	    (not found) continues after CacheLookup2
//
/////////////////////////////////////////////////////////////////////
	
.macro CacheLookup
	//
	// Restart protocol:
	//
	//   As soon as we're past the LLookupStart$1 label we may have loaded
	//   an invalid cache pointer or mask.
	//
	//   When task_restartable_ranges_synchronize() is called,
	//   (or when a signal hits us) before we're past LLookupEnd$1,
	//   then our PC will be reset to LCacheMiss$1 which forcefully
	//   jumps to the cache-miss codepath.
	//
	//   It is assumed that the CacheMiss codepath starts right at the end
	//   of CacheLookup2 and will re-setup the registers to meet the cache-miss
	//   requirements:
	//
	//   GETIMP:
	//     The cache-miss is just returning NULL (setting r9 to 0)
	//
	//   NORMAL and STRET:
	//   - r0 or r1 (STRET) contains the receiver
	//   - r1 or r2 (STRET) contains the selector
	//   - r9 contains the isa (reloaded from r0/r1)
	//   - other registers are set as per calling conventions
	//
LLookupStart$1:

	ldrh	r12, [r9, #CACHE_MASK]	// r12 = mask
	ldr	r9, [r9, #CACHE]	// r9 = buckets
.if $0 == STRET
	and	r12, r12, r2		// r12 = index = SEL & mask
.else
	and	r12, r12, r1		// r12 = index = SEL & mask
.endif
	add	r9, r9, r12, LSL #3	// r9 = bucket = buckets+index*8
	ldr	r12, [r9, #CACHED_SEL]	// r12 = bucket->sel
6:
.if $0 == STRET
	teq	r12, r2
.else
	teq	r12, r1
.endif
	bne	8f
	ldr	r12, [r9, #CACHED_IMP]	// r12 = bucket->imp

.if $0 == STRET
	tst	r12, r12		// set ne for stret forwarding
.else
	// eq already set for nonstret forwarding by `teq` above
.endif

.endmacro

.macro CacheLookup2
#if CACHED_SEL != 0
#   error this code requires that SEL be at offset 0
#endif
8:	
	cmp	r12, #1
	blo	LCacheMiss$1		// if (bucket->sel == 0) cache miss
	it	eq			// if (bucket->sel == 1) cache wrap
	ldreq	r9, [r9, #CACHED_IMP]	// bucket->imp is before first bucket
	ldr	r12, [r9, #8]!		// r12 = (++bucket)->sel
	b	6b

LLookupEnd$1:
LCacheMiss$1:

.endmacro

/////////////////////////////////////////////////////////////////////
//
// GetClassFromIsa	return-type
//
// Given an Isa, return the class for the Isa.
//
// Takes:
//	  r9 = class
//
// On exit: r12 clobbered
//          r9 contains the class for this Isa.
//
/////////////////////////////////////////////////////////////////////
.macro GetClassFromIsa

#if SUPPORT_INDEXED_ISA
	// Note: We are doing a little wasted work here to load values we might not
	// need.  Branching turns out to be even worse when performance was measured.
	MI_GET_ADDRESS(r12, _objc_indexed_classes)
	tst.w	r9, #ISA_INDEX_IS_NPI_MASK
	itt	ne
	ubfxne	r9, r9, #ISA_INDEX_SHIFT, #ISA_INDEX_BITS
	ldrne.w	r9, [r12, r9, lsl #2]
#endif

.endmacro


/********************************************************************
 * IMP cache_getImp(Class cls, SEL sel)
 *
 * On entry:    r0 = class whose cache is to be searched
 *              r1 = selector to search for
 *
 * If found, returns method implementation.
 * If not found, returns NULL.
 ********************************************************************/

	STATIC_ENTRY _cache_getImp

	mov	r9, r0
	CacheLookup NORMAL, _cache_getImp
	// cache hit, IMP in r12
	mov	r0, r12
	bx	lr			// return imp
	
	CacheLookup2 GETIMP, _cache_getImp
	// cache miss, return nil
	mov	r0, #0
	bx	lr

	END_ENTRY _cache_getImp


/********************************************************************
 *
 * id objc_msgSend(id self, SEL _cmd, ...);
 * IMP objc_msgLookup(id self, SEL _cmd, ...);
 * 
 * objc_msgLookup ABI:
 * IMP returned in r12
 * Forwarding returned in Z flag
 * r9 reserved for our use but not used
 *
 ********************************************************************/

	ENTRY _objc_msgSend
	
	cbz	r0, LNilReceiver_f

	ldr	r9, [r0]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	CacheLookup NORMAL, _objc_msgSend
	// cache hit, IMP in r12, eq already set for nonstret forwarding
	bx	r12			// call imp

	CacheLookup2 NORMAL, _objc_msgSend
	// cache miss
	ldr	r9, [r0]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	b	__objc_msgSend_uncached

LNilReceiver:
	// r0 is already zero
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	FP_RETURN_ZERO
	bx	lr	

	END_ENTRY _objc_msgSend

	
	ENTRY _objc_msgLookup

	cbz	r0, LNilReceiver_f

	ldr	r9, [r0]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	CacheLookup NORMAL, _objc_msgLookup
	// cache hit, IMP in r12, eq already set for nonstret forwarding
	bx	lr

	CacheLookup2 NORMAL, _objc_msgLookup
	// cache miss
	ldr	r9, [r0]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	b	__objc_msgLookup_uncached

LNilReceiver:
	MI_GET_ADDRESS(r12, __objc_msgNil)
	bx	lr

	END_ENTRY _objc_msgLookup


	STATIC_ENTRY __objc_msgNil
	
	// r0 is already zero
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	FP_RETURN_ZERO
	bx	lr
	
	END_ENTRY __objc_msgNil


/********************************************************************
 * void objc_msgSend_stret(void *st_addr, id self, SEL op, ...);
 * IMP objc_msgLookup_stret(void *st_addr, id self, SEL op, ...);
 *
 * objc_msgSend_stret is the struct-return form of msgSend.
 * The ABI calls for r0 to be used as the address of the structure
 * being returned, with the parameters in the succeeding registers.
 *
 * On entry: r0 is the address where the structure is returned,
 *           r1 is the message receiver,
 *           r2 is the selector
 ********************************************************************/

	ENTRY _objc_msgSend_stret
	
	cbz	r1, LNilReceiver_f

	ldr	r9, [r1]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	CacheLookup STRET, _objc_msgSend_stret
	// cache hit, IMP in r12, ne already set for stret forwarding
	bx	r12

	CacheLookup2 STRET, _objc_msgSend_stret
	// cache miss
	ldr	r9, [r1]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	b	__objc_msgSend_stret_uncached

LNilReceiver:
	bx	lr
	
	END_ENTRY _objc_msgSend_stret


	ENTRY _objc_msgLookup_stret
	
	cbz	r1, LNilReceiver_f

	ldr	r9, [r1]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	CacheLookup STRET, _objc_msgLookup_stret
	// cache hit, IMP in r12, ne already set for stret forwarding
	bx	lr

	CacheLookup2 STRET, _objc_msgLookup_stret
	// cache miss
	ldr	r9, [r1]		// r9 = self->isa
	GetClassFromIsa			// r9 = class
	b	__objc_msgLookup_stret_uncached

LNilReceiver:
	MI_GET_ADDRESS(r12, __objc_msgNil_stret)
	bx	lr

	END_ENTRY _objc_msgLookup_stret


	STATIC_ENTRY __objc_msgNil_stret
	
	bx	lr

	END_ENTRY __objc_msgNil_stret


/********************************************************************
 * id objc_msgSendSuper(struct objc_super *super, SEL op, ...)
 *
 * struct objc_super {
 *     id receiver;
 *     Class cls;	// the class to search
 * }
 ********************************************************************/

	ENTRY _objc_msgSendSuper
	
	ldr	r9, [r0, #CLASS]	// r9 = struct super->class
	CacheLookup NORMAL, _objc_msgSendSuper
	// cache hit, IMP in r12, eq already set for nonstret forwarding
	ldr	r0, [r0, #RECEIVER]	// load real receiver
	bx	r12			// call imp

	CacheLookup2 NORMAL, _objc_msgSendSuper
	// cache miss
	ldr	r9, [r0, #CLASS]	// r9 = struct super->class
	ldr	r0, [r0, #RECEIVER]	// load real receiver
	b	__objc_msgSend_uncached
	
	END_ENTRY _objc_msgSendSuper


/********************************************************************
 * id objc_msgSendSuper2(struct objc_super *super, SEL op, ...)
 *
 * struct objc_super {
 *     id receiver;
 *     Class cls;	// SUBCLASS of the class to search
 * }
 ********************************************************************/
	
	ENTRY _objc_msgSendSuper2
	
	ldr	r9, [r0, #CLASS]	// class = struct super->class
	ldr	r9, [r9, #SUPERCLASS]   // class = class->superclass
	CacheLookup NORMAL, _objc_msgSendSuper2
	// cache hit, IMP in r12, eq already set for nonstret forwarding
	ldr	r0, [r0, #RECEIVER]	// load real receiver
	bx	r12			// call imp

	CacheLookup2 NORMAL, _objc_msgSendSuper2
	// cache miss
	ldr	r9, [r0, #CLASS]	// class = struct super->class
	ldr	r9, [r9, #SUPERCLASS]   // class = class->superclass
	ldr	r0, [r0, #RECEIVER]	// load real receiver
	b	__objc_msgSend_uncached
	
	END_ENTRY _objc_msgSendSuper2

	
	ENTRY _objc_msgLookupSuper2
	
	ldr	r9, [r0, #CLASS]	// class = struct super->class
	ldr	r9, [r9, #SUPERCLASS]   // class = class->superclass
	CacheLookup NORMAL, _objc_msgLookupSuper2
	// cache hit, IMP in r12, eq already set for nonstret forwarding
	ldr	r0, [r0, #RECEIVER]	// load real receiver
	bx	lr

	CacheLookup2 NORMAL, _objc_msgLookupSuper2
	// cache miss
	ldr	r9, [r0, #CLASS]
	ldr	r9, [r9, #SUPERCLASS]	// r9 = class to search
	ldr	r0, [r0, #RECEIVER]	// load real receiver
	b	__objc_msgLookup_uncached
	
	END_ENTRY _objc_msgLookupSuper2


/********************************************************************
 * void objc_msgSendSuper_stret(void *st_addr, objc_super *self, SEL op, ...);
 *
 * objc_msgSendSuper_stret is the struct-return form of msgSendSuper.
 * The ABI calls for r0 to be used as the address of the structure
 * being returned, with the parameters in the succeeding registers.
 *
 * On entry: r0 is the address where the structure is returned,
 *           r1 is the address of the objc_super structure,
 *           r2 is the selector
 ********************************************************************/

	ENTRY _objc_msgSendSuper_stret
	
	ldr	r9, [r1, #CLASS]	// r9 = struct super->class
	CacheLookup STRET, _objc_msgSendSuper_stret
	// cache hit, IMP in r12, ne already set for stret forwarding
	ldr	r1, [r1, #RECEIVER]	// load real receiver
	bx	r12			// call imp

	CacheLookup2 STRET, _objc_msgSendSuper_stret
	// cache miss
	ldr	r9, [r1, #CLASS]	// r9 = struct super->class
	ldr	r1, [r1, #RECEIVER]	// load real receiver
	b	__objc_msgSend_stret_uncached

	END_ENTRY _objc_msgSendSuper_stret


/********************************************************************
 * id objc_msgSendSuper2_stret
 ********************************************************************/

	ENTRY _objc_msgSendSuper2_stret
	
	ldr	r9, [r1, #CLASS]	// class = struct super->class
	ldr	r9, [r9, #SUPERCLASS]	// class = class->superclass
	CacheLookup STRET, _objc_msgSendSuper2_stret
	// cache hit, IMP in r12, ne already set for stret forwarding
	ldr	r1, [r1, #RECEIVER]	// load real receiver
	bx	r12			// call imp

	CacheLookup2 STRET, _objc_msgSendSuper2_stret
	// cache miss
	ldr	r9, [r1, #CLASS]	// class = struct super->class
	ldr	r9, [r9, #SUPERCLASS]	// class = class->superclass
	ldr	r1, [r1, #RECEIVER]	// load real receiver
	b	__objc_msgSend_stret_uncached
	
	END_ENTRY _objc_msgSendSuper2_stret

	
	ENTRY _objc_msgLookupSuper2_stret
	
	ldr	r9, [r1, #CLASS]	// class = struct super->class
	ldr	r9, [r9, #SUPERCLASS]	// class = class->superclass
	CacheLookup STRET, _objc_msgLookupSuper2_stret
	// cache hit, IMP in r12, ne already set for stret forwarding
	ldr	r1, [r1, #RECEIVER]	// load real receiver
	bx	lr

	CacheLookup2 STRET, _objc_msgLookupSuper2_stret
	// cache miss
	ldr	r9, [r1, #CLASS]
	ldr	r9, [r9, #SUPERCLASS]	// r9 = class to search
	ldr	r1, [r1, #RECEIVER]	// load real receiver
	b	__objc_msgLookup_stret_uncached
	
	END_ENTRY _objc_msgLookupSuper2_stret

	
/////////////////////////////////////////////////////////////////////
//
// MethodTableLookup	NORMAL|STRET
//
// Locate the implementation for a selector in a class's method lists.
//
// Takes: 
//	  $0 = NORMAL, STRET
//	  r0 or r1 (STRET) = receiver
//	  r1 or r2 (STRET) = selector
//	  r9 = class to search in
//
// On exit: IMP in r12, eq/ne set for forwarding
//
/////////////////////////////////////////////////////////////////////
	
.macro MethodTableLookup
	
	SAVE_REGS

	// lookUpImpOrForward(obj, sel, cls, LOOKUP_INITIALIZE | LOOKUP_RESOLVER)
.if $0 == NORMAL
	// receiver already in r0
	// selector already in r1
.else
	mov 	r0, r1			// receiver
	mov 	r1, r2			// selector
.endif
	mov	r2, r9			// class to search
	mov	r3, #3			// LOOKUP_INITIALIZE | LOOKUP_INITIALIZE
	blx	_lookUpImpOrForward
	mov	r12, r0			// r12 = IMP
	
.if $0 == NORMAL
	cmp	r12, r12		// set eq for nonstret forwarding
.else
	tst	r12, r12		// set ne for stret forwarding
.endif

	RESTORE_REGS

.endmacro


/********************************************************************
 *
 * _objc_msgSend_uncached
 * _objc_msgSend_stret_uncached
 * _objc_msgLookup_uncached
 * _objc_msgLookup_stret_uncached
 * The uncached method lookup.
 *
 ********************************************************************/

	STATIC_ENTRY __objc_msgSend_uncached

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r9 is the class to search
	
	MethodTableLookup NORMAL	// returns IMP in r12
	bx	r12

	END_ENTRY __objc_msgSend_uncached


	STATIC_ENTRY __objc_msgSend_stret_uncached

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r9 is the class to search

	MethodTableLookup STRET		// returns IMP in r12
	bx	r12
	
	END_ENTRY __objc_msgSend_stret_uncached

	
	STATIC_ENTRY __objc_msgLookup_uncached

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r9 is the class to search
	
	MethodTableLookup NORMAL	// returns IMP in r12
	bx	lr

	END_ENTRY __objc_msgLookup_uncached


	STATIC_ENTRY __objc_msgLookup_stret_uncached

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band r9 is the class to search

	MethodTableLookup STRET		// returns IMP in r12
	bx	lr
	
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

	MI_EXTERN(__objc_forward_handler)
	MI_EXTERN(__objc_forward_stret_handler)
	
	STATIC_ENTRY __objc_msgForward_impcache
	// Method cache version

	// THIS IS NOT A CALLABLE C FUNCTION
	// Out-of-band Z is 0 (EQ) for normal, 1 (NE) for stret

	beq	__objc_msgForward
	b	__objc_msgForward_stret
	
	END_ENTRY __objc_msgForward_impcache
	

	ENTRY __objc_msgForward
	// Non-stret version

	MI_GET_EXTERN(r12, __objc_forward_handler)
	ldr	r12, [r12]
	bx	r12

	END_ENTRY __objc_msgForward


	ENTRY __objc_msgForward_stret
	// Struct-return version

	MI_GET_EXTERN(r12, __objc_forward_stret_handler)
	ldr	r12, [r12]
	bx	r12

	END_ENTRY __objc_msgForward_stret


	ENTRY _objc_msgSend_noarg
	b 	_objc_msgSend
	END_ENTRY _objc_msgSend_noarg

	ENTRY _objc_msgSend_debug
	b	_objc_msgSend
	END_ENTRY _objc_msgSend_debug

	ENTRY _objc_msgSendSuper2_debug
	b	_objc_msgSendSuper2
	END_ENTRY _objc_msgSendSuper2_debug

	ENTRY _objc_msgSend_stret_debug
	b	_objc_msgSend_stret
	END_ENTRY _objc_msgSend_stret_debug

	ENTRY _objc_msgSendSuper2_stret_debug
	b	_objc_msgSendSuper2_stret
	END_ENTRY _objc_msgSendSuper2_stret_debug


	ENTRY _method_invoke

	// See if this is a small method.
	lsls	r12, r1, #31
	bne.w	L_method_invoke_small

	// We can directly load the IMP from big methods.
	// r1 is method triplet instead of SEL
	ldr	r12, [r1, #METHOD_IMP]
	ldr	r1, [r1, #METHOD_NAME]
	bx	r12

L_method_invoke_small:
	// Small methods require a call to handle swizzling.
	SAVE_REGS
	mov	r0, r1
	bl	__method_getImplementationAndName
	mov	r12, r0
	mov	r9, r1
	RESTORE_REGS
	mov	r1, r9
	bx	r12


	END_ENTRY _method_invoke


	ENTRY _method_invoke_stret

	// See if this is a small method.
	lsls	r12, r2, #31
	bne.w	L_method_invoke_stret_small

	// We can directly load the IMP from big methods.
	// r2 is method triplet instead of SEL
	ldr	r12, [r2, #METHOD_IMP]
	ldr	r2, [r2, #METHOD_NAME]
	bx	r12

L_method_invoke_stret_small:
	// Small methods require a call to handle swizzling.
	SAVE_REGS
	mov	r0, r2
	bl	__method_getImplementationAndName
	mov	r12, r0
	mov	r9, r1
	RESTORE_REGS
	mov	r2, r9
	bx	r12

	END_ENTRY _method_invoke_stret
	

.section __DATA,__objc_msg_break
.long 0
.long 0
	
#endif
