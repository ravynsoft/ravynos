#if __arm64__

#include <mach/vm_param.h>
#include "arm64-asm.h"

// Offset of block->invoke field.
#if __LP64__
    // true arm64
#   define BLOCK_INVOKE 16
#else
    // arm64_32
#   define BLOCK_INVOKE 12
#endif

.text
.globl __objc_blockTrampolineImpl
.globl __objc_blockTrampolineStart
.globl __objc_blockTrampolineLast
	
.align PAGE_MAX_SHIFT
__objc_blockTrampolineImpl:
L_objc_blockTrampolineImpl:
	/*
	 x0  == self
	 x17 == address of called trampoline's data (2 pages before its code)
	 lr  == original return address
	 */

	mov  x1, x0                  // _cmd = self
	ldr  p0, [x17]               // self = block object
	add  p15, p0, #BLOCK_INVOKE  // x15 = &block->invoke
	ldr  p16, [x15]              // x16 = block->invoke
	TailCallBlockInvoke x16, x15

	// pad up to TrampolineBlockPagePair header size
	nop
	nop
	nop
	
.macro TrampolineEntry
	// load address of trampoline data (two pages before this instruction)
	adr  x17, -2*PAGE_MAX_SIZE
	b    L_objc_blockTrampolineImpl
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
	
.align 3
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

#endif
