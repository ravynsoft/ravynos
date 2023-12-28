# Test too many instructions in execute packet.
.text
.globl f
f:
	nop
	|| nop
	|| nop
	|| nop
	|| nop
	|| nop
	|| nop
	|| nop
	|| nop
