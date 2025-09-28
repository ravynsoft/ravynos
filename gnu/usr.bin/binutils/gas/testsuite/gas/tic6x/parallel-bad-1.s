# Test bad syntax for parallel operations.
.text
.globl f
f:
	nop
	|| ; no instruction
	nop
	|| .word 0
	nop
	|| || nop
	nop
	||^ ; no instruction
	nop
	||^ .word 0
	nop
	||^ || nop
	nop
	|| ||^ nop
	nop
	||^ ||^ nop
	nop
	|| label:
	nop
	||^ label2:
	nop
	[A1] || nop
	nop
	[B1] ||^ nop
	nop
# End with this one, to be sure errors detected at new-line are
# detected at end-of-file.
	|| .word 0
