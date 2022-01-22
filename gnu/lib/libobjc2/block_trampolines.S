#include "common.S"

#
# This file defines some trampolines for calling blocks.  A block function
# looks like this:
# 
# retType blockFn(block*, ...)
#
# An IMP looks like this:
#
# retType imp(id, SEL,...)
#
# The trampoline must find the block pointer and then call the block function
# with the correct first argument, the self pointer moved to the second real
# argument (the first block argument) and the _cmd parameter excised

.file	"block_trampolines.S"


#if __x86_64
////////////////////////////////////////////////////////////////////////////////
// x86-64 trampoline
////////////////////////////////////////////////////////////////////////////////
.macro trampoline arg0, arg1
	mov   -0x1007(%rip), \arg1   # Load the block pointer into the second argument
	xchg  \arg1, \arg0           # Swap the first and second arguments
	jmp   *-0x1008(%rip)         # Call the block function
.endm
// The Win64 and SysV x86-64 ABIs use different registers
#	ifdef _WIN64
#		define ARG0 %rcx
#		define ARG1 %rdx
#		define SARG1 %r8
#	else
#		define ARG0 %rdi
#		define ARG1 %rsi
#		define SARG1 %rdx
#	endif
#	define SARG0 ARG1

#elif __i386
////////////////////////////////////////////////////////////////////////////////
// x86-32 trampoline
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
// Mark this compilation unit as SEH-safe
.text
.def     @feat.00;
.scl    3;
.type   0;
.endef
.globl  @feat.00
.set @feat.00, 1
.data
#endif

.macro trampoline arg0, arg1
	call  1f                   # Store the instruction pointer on the stack
1:
	pop   %eax                 # Load the old instruction pointer
	mov   \arg0(%esp), %ebx    # Load the self parameter
	mov   %ebx, \arg1(%esp)    # Store self as the second argument
	mov   -0x1005(%eax), %ebx  # Load the block pointer to %ebx
	mov   %ebx, \arg0(%esp)    # Store the block pointer in the first argument
	jmp   *-0x1001(%eax)       # Call the block function
.endm
// All arguments on i386 are passed on the stack.  These values are stack
// offsets - on other platforms they're register values.
#	define ARG0 4
#	define ARG1 8
#	define SARG0 8
#	define SARG1 12

#elif __mips__
////////////////////////////////////////////////////////////////////////////////
// MIPS trampoline
////////////////////////////////////////////////////////////////////////////////
#	ifdef _ABI64
.macro trampoline arg0, arg1
	move     \arg1, \arg0
	ld       \arg0, -4096($25)
	ld       $25, -4088($25)
	jr       $25
.endm
#	else
// 32-bit variant.  This ought to work with both n32 and o32, because they both
// use 32-bit pointers and both use the same registers for the first four
// arguments (and we only care about the first three).
.macro trampoline arg0, arg1
	move   \arg1, \arg0
	lw     \arg0, -4096($25)
	lw     $25, -4092($25)
	jr     $25
.endm
#	endif
#define ARG0 $a0
#define ARG1 $a1
#define ARG2 $a2

#elif defined(__ARM_ARCH_ISA_A64)
////////////////////////////////////////////////////////////////////////////////
// AArch64 (ARM64) trampoline
////////////////////////////////////////////////////////////////////////////////
.macro trampoline arg0, arg1
	adr x17, #-4096
	mov \arg1, \arg0
	ldp \arg0, x17, [x17]
	br x17
.endm
#define ARG0 x0
#define ARG1 x1
#define SARG0 x0
#define SARG1 x1

#elif __arm__
////////////////////////////////////////////////////////////////////////////////
// AArch32 (ARM) trampoline
////////////////////////////////////////////////////////////////////////////////

#	if (__ARM_ARCH_ISA_THUMB == 2)
// If we're on a target that supports Thumb 2, then we need slightly more
// instructions to support Thumb/ARM code for the IMP and so we need to make
// the trampolines thumb to be able to fit them in 16 bytes (they fit exactly
// when assembled as Thumb-2).
.thumb
.macro trampoline arg0, arg1
	sub r12, pc, #4095
	mov \arg1, \arg0            // Move self over _cmd
	ldr \arg0, [r12, #-5]       // Load the block pointer over self
	ldr r12, [r12, #-1]         // Jump to the block function
	bx  r12
.endm
#	else
.macro trampoline arg0, arg1
	sub r12, pc, #4096
	mov \arg1, \arg0            // Move self over _cmd
	ldr \arg0, [r12, #-8]       // Load the block pointer over self
	ldr pc, [r12, #-4]          // Jump to the block function
.endm
#	endif // (__ARM_ARCH_ISA_THUMB == 2)
#define ARG0 r0
#define ARG1 r1
#define SARG0 r1
#define SARG1 r2

#else

#warning imp_implementationWithBlock() not implemented for your architecture
.macro trampoline arg0, arg1
.endm
#define ARG0 0
#define ARG1 0
#define SARG0 0
#define SARG1 0

#endif


.globl CDECL(__objc_block_trampoline)
CDECL(__objc_block_trampoline):
	trampoline ARG0, ARG1
.globl CDECL(__objc_block_trampoline_end)
CDECL(__objc_block_trampoline_end):
.globl CDECL(__objc_block_trampoline_sret)
CDECL(__objc_block_trampoline_sret):
	trampoline SARG0, SARG1
.globl CDECL(__objc_block_trampoline_end_sret)
CDECL(__objc_block_trampoline_end_sret):


#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
