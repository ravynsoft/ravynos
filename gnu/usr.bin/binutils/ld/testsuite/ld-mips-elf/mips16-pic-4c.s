	.abicalls
	.option	pic0
	.set	noreorder

	# Create a call stub for f2.  We pretend that f2 takes floating-point
	# arguments but doesn't return a floating-point value.

	.section .mips16.call.f2, "ax", @progbits
	.ent	__call
__call:
	la	$25,f2
	jr	$25
	nop
	.end	__call

	# Create a call stub for f3.  We pretend that f3 returns a
	# floating-point value.

	.section .mips16.call.fp.f3, "ax", @progbits
	.ent	__call_fp
__call_fp:
	la	$25,f3
	jr	$25
	nop
	.end	__call_fp

	# Make sure that f2 and f3 are called from MIPS16 code.
	.set	mips16
	.text
	.global	__start
	.type	__start,@function
	.ent	__start
__start:
	jal	f2
	nop
	jal	f3
	nop
	.end	__start
