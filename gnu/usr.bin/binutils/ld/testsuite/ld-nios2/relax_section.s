# relaxing conditional and unconditional jumps -- pc-relative

.globl text1
.section text1, "ax", @progbits
	beq r2, r3, just_out_of_range
	blt r2, r3, farther_out_of_range
	bne r2, r3, in_range
	nop
	nop
in_range:
	nop
.align 15
	br in_range
just_out_of_range:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
farther_out_of_range:
	nop
