.set noreorder
# Some macros for n32 / n64 compatibility
#ifdef _ABI64
#define LP             ld
#define SP             sd
#else
#warning N32 is untested, O32 is unsupported.
#define LP             lw
#define SP             sw
#endif

.macro dump_and_crash reg
nop
move   $a0, \reg
ld     $25, %got_disp(logInt)($t8) 
jalr   $25
nop
lw     $zero, ($zero)
.endm

// FIXME: CHERI needs (or, at least, strongly encourages) 32-byte aligned
// stacks.  
#ifndef __mips_soft_float
#define SAVE_SIZE      136
#else
#define SAVE_SIZE      72
#endif

.macro MSGSEND receiver, sel
0:
	.cfi_startproc                          # Start emitting unwind data.  We
	                                        # don't actually care about any of
	                                        # the stuff except the slow call,
	                                        # because that's the only one that
	                                        # can throw.
	beq    \receiver, $0, 4f                # If the receiver is nil, return nil
	nop

	lui    $t8, %hi(%neg(%gp_rel(0b)))      # Load the GOT address that we use for relocations into $t8
	daddu  $t8, $t8, $t9
	daddiu $t8, $t8, %lo(%neg(%gp_rel(0b)))


	andi   $t0, \receiver, SMALLOBJ_MASK    # Check if the receiver is a small object
	bne    $t0, $0, 6f                      # Get the small object class
	nop

	LP     $t1, (\sel)

	                                        # By this point, we have a non-nil
	                                        # receiver that is a real pointer
	LP     $t0, (\receiver)                 # Load the class

1:                                          # class loaded, stored in $t0
	LP     $t0, DTABLE_OFFSET($t0)          # Load the dtable from the class
	lw     $t2, SHIFT_OFFSET($t0)           # Load the shift (dtable size)
	                                        # $t0 = dtable, $t1 = sel index
	daddi  $t3, $t0, DATA_OFFSET            # Compute the address of the start of the array


	beq    $0, $t2, 3f                      # If this is a small dtable, jump to the small dtable handlers
	daddi  $v0, $t2, -8

	beq    $0, $v0, 2f
	lui    $t2, 0x00ff                      # The mask for a big dtable won't fit in an and immediate
	and    $t2, $t2, $t1                    # mask the selector
#ifdef _ABI64         
	dsrl   $t2, $t2, 13                     # Right shift 16, but then left shift by pointer size
#else
	srl    $t2, $t2, 14
#endif
	dadd   $t2, $t2, $t3
	LP     $t3, ($t2)
	daddi  $t3, $t3, DATA_OFFSET            # Compute the address of the start of the array
2:                                          # dtable16:
	andi   $t2, $t1, 0xff00                 # mask the selector
#ifdef _ABI64        
	dsrl   $t2, $t2, 5                      # Right shift 8, but then left shift by pointer size
#else
	srl    $t2, $t2, 6
#endif
	dadd   $t2, $t2, $t3
	LP     $t3, ($t2)
	daddi  $t3, $t3, DATA_OFFSET            # Compute the address of the start of the array
3:                                          # dtable8:
	andi   $t2, $t1, 0xff                   # mask the selector
#ifdef _ABI64         
	dsll   $t2, $t2, 3                      # Left shift by pointer size
#else
	sll    $t2, $t2, 2
#endif
	dadd   $t2, $t2, $t3
	LP     $t3, ($t2)

	
	beq    $0, $t3, 5f                      # Nil slot - invoke some kind of forwarding mechanism
	nop

	LP     $25, SLOT_OFFSET($t3)
	jr     $25
	nop
4:                                          # returnNil:
	                                        # All of the return registers are
	                                        # callee-save, so we can
	                                        # return 0 in both in the same code:
#ifndef __mips_soft_float
	dmtc1  $0, $f0                          # Return 0 as a floating point value (only if we're not a soft-float target)
	dmtc1  $0, $f2
#endif
	daddi  $v0, $0, 0                        # Return 0 as an integer
	jr     $ra
	daddi  $v1, $0, 0
	
5:                                          # slowSend:
	# Load the address of the slow lookup function now, so that we don't get
	# pipeline stalls on the jump.  This is more important on CHERI than proper
	# MIPS implementations.
	# Note: A better linker ought to be able to turn this into a single
	# jump-immediate, so revisit this decision later...
	LP     $25, %got_disp(CDECL(slowMsgLookup))($t8) 

	daddiu $sp, $sp, -SAVE_SIZE             # We need to preserve all registers that may contain arguments:

	SP     $a0, ($sp)
	SP     $a1, 8($sp)
	SP     $a2, 16($sp)
	SP     $a3, 24($sp)
	SP     $a4, 32($sp)
	SP     $a5, 40($sp)
	SP     $a6, 48($sp)
	SP     $a7, 56($sp)
	SP     $ra, 64($sp)
#ifndef __mips_soft_float
	sdc1   $f12, 72($sp)
	sdc1   $f13, 80($sp)
	sdc1   $f14, 88($sp)
	sdc1   $f15, 96($sp)
	sdc1   $f16, 104($sp)
	sdc1   $f17, 112($sp)
	sdc1   $f18, 120($sp)
	sdc1   $f19, 128($sp)
#endif     

	# We're (potentially) modifying the self argument with the lookup.  Use the
	# address of the stack save slot for the address so that when we reload it
	# we get the old or new version automatically.  Note that we must reload it
	# anyway, because argument registers are not guaranteed to be preserved
	# across calls.
.ifc "\receiver", "$a0"
	daddiu $a0, $sp, 0                      # replace self with &self in $a0
.else
	daddiu $a0, $sp, 8                      # replace sret pointer with &self in $a0
	daddiu $a1, $a2, 0                      # replace self with _cmd in $a1
.endif

	.cfi_def_cfa_offset SAVE_SIZE
	.cfi_offset 31, (64 - SAVE_SIZE)
	jalr   $25                              # Call the slow lookup function
	nop

	move   $25, $v0                         # Move the return value to $25 for use with the call

	LP     $a0, ($sp)                       # Restore all of the arguments.  Note
	LP     $a1, 8($sp)                      # that the receiver may have been
	LP     $a2, 16($sp)                     # modified during the call
	LP     $a3, 24($sp)
	LP     $a4, 32($sp)
	LP     $a5, 40($sp)
	LP     $a6, 48($sp)
	LP     $a7, 56($sp)
	LP     $ra, 64($sp)
#ifndef __mips_soft_float
	ldc1   $f12, 72($sp)
	ldc1   $f13, 80($sp)
	ldc1   $f14, 88($sp)
	ldc1   $f15, 96($sp)
	ldc1   $f16, 104($sp)
	ldc1   $f17, 112($sp)
	ldc1   $f18, 120($sp)
	ldc1   $f19, 128($sp)
#endif     
	jr     $25
	daddiu $sp, $sp, SAVE_SIZE                 
6:                                          # smallObject:
#if _ABI64
	dsll   $t0, $t0, 3                      # Convert tag to pointer offset
	LP     $t2, %got_disp(CDECL(SmallObjectClasses))($t8) # Load small object classes array address
	daddu  $t0, $t0, $t2                    # Add the base address to the offset
	b      1b                               # Return to the normal path
	LP     $t0, ($t0)                       # Load the class (in delay slot)
#else
	b      1b
	LP     $t0, %got_disp(CDECL(SmallIntClass))($t8)
#endif
	.cfi_endproc
.endm
.globl CDECL(objc_msgSend)
TYPE_DIRECTIVE(CDECL(objc_msgSend), @function)
.globl CDECL(objc_msgSend_fpret)
TYPE_DIRECTIVE(CDECL(objc_msgSend_fpret), @function)
CDECL(objc_msgSend_fpret):
CDECL(objc_msgSend):
	MSGSEND $a0, $a1
.globl CDECL(objc_msgSend_stret)
TYPE_DIRECTIVE(CDECL(objc_msgSend_stret), @function)
CDECL(objc_msgSend_stret):
	MSGSEND $a1, $a2
