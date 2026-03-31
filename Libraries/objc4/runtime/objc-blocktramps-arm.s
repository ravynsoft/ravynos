#if __arm__
	
#include <arm/arch.h>
#include <mach/vm_param.h>

.syntax unified

.text
.globl __objc_blockTrampolineImpl
.globl __objc_blockTrampolineStart
.globl __objc_blockTrampolineLast

// Trampoline machinery assumes the trampolines are Thumb function pointers
#if !__thumb2__
#   error sorry
#endif

.thumb

// Exported symbols are not marked as functions.
// The trampoline construction code assumes that the Thumb bit is not set.
.thumb_func L__objc_blockTrampolineImpl_func

.align PAGE_MAX_SHIFT
__objc_blockTrampolineImpl:
L__objc_blockTrampolineImpl_func:
	/*
	 r0 == self
	 r12 == pc of trampoline's first instruction + PC bias
	 lr == original return address
	 */

	mov  r1, r0                   // _cmd = self

	// Trampoline's data is two pages before the trampoline text.
	// Also correct PC bias of 4 bytes.
	sub  r12, # 2*PAGE_MAX_SIZE
	ldr  r0, [r12, #-4]          // self = block object
	ldr  pc, [r0, #12]           // tail call block->invoke
	// not reached

.macro TrampolineEntry
	mov r12, pc
	b L__objc_blockTrampolineImpl_func
.align 3
.endmacro

.macro TrampolineEntryX16
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
.endmacro

.macro TrampolineEntryX256
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
.endmacro

.align 5
__objc_blockTrampolineStart:
	// 2048-4 trampolines to fill 16K page
	TrampolineEntryX256
	TrampolineEntryX256
	TrampolineEntryX256
	TrampolineEntryX256

	TrampolineEntryX256
	TrampolineEntryX256
	TrampolineEntryX256

	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16

	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16

	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16

	TrampolineEntryX16
	TrampolineEntryX16
	TrampolineEntryX16

	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry

	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
	TrampolineEntry

	TrampolineEntry
	TrampolineEntry
	TrampolineEntry
__objc_blockTrampolineLast:
	TrampolineEntry

	// TrampolineEntry
	// TrampolineEntry
	// TrampolineEntry
	// TrampolineEntry



.text
.globl __objc_blockTrampolineImpl_stret
.globl __objc_blockTrampolineStart_stret
.globl __objc_blockTrampolineLast_stret

// Trampoline machinery assumes the trampolines are Thumb function pointers
#if !__thumb2__
#   error sorry
#endif

.thumb

// Exported symbols are not marked as functions.
// The trampoline construction code assumes that the Thumb bit is not set.
.thumb_func L__objc_blockTrampolineImpl_stret_func

.align PAGE_MAX_SHIFT
__objc_blockTrampolineImpl_stret:
L__objc_blockTrampolineImpl_stret_func:
	/*
	 r1 == self
	 r12 == pc of trampoline's first instruction + PC bias
	 lr == original return address
	 */

	mov  r2, r1                   // _cmd = self

	// Trampoline's data is three pages before the trampoline text.
	// Also correct PC bias of 4 bytes.
	sub  r12, # 3*PAGE_MAX_SIZE
	ldr  r1, [r12, #-4]          // self = block object
	ldr  pc, [r1, #12]           // tail call block->invoke
	// not reached
	
.macro TrampolineEntry_stret
	mov r12, pc
	b L__objc_blockTrampolineImpl_stret_func
.align 3
.endmacro

.macro TrampolineEntryX16_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
.endmacro

.macro TrampolineEntryX256_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
.endmacro

.align 5
__objc_blockTrampolineStart_stret:
	// 2048-4 trampolines to fill 16K page
	TrampolineEntryX256_stret
	TrampolineEntryX256_stret
	TrampolineEntryX256_stret
	TrampolineEntryX256_stret

	TrampolineEntryX256_stret
	TrampolineEntryX256_stret
	TrampolineEntryX256_stret

	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret

	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret

	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret

	TrampolineEntryX16_stret
	TrampolineEntryX16_stret
	TrampolineEntryX16_stret

	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret

	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret

	TrampolineEntry_stret
	TrampolineEntry_stret
	TrampolineEntry_stret
__objc_blockTrampolineLast_stret:
	TrampolineEntry_stret

	// TrampolineEntry_stret
	// TrampolineEntry_stret
	// TrampolineEntry_stret
	// TrampolineEntry_stret

#endif
