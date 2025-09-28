# Test bad positions for parallel operations.
.text
.globl f
f:
	|| nop
	nop
	.word 0
	|| nop
	nop
label:
	|| nop
