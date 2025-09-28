# relaxing conditional jumps -- absolute

.globl text1
.section text1, "ax", @progbits
	beq r2, r3, on_border
	beq r2, r3, out_of_range
	nop
	nop

.align 15
on_border:
	bne r2, r3, in_range
	nop
	nop
	nop
	nop
	nop
out_of_range:
in_range:
	nop
	
.globl text2
.section text2, "ax", @progbits

	bge r2, r3, text1
	blt r2, r3, out_of_range
	ble r2, r3, sym
	nop
	nop
sym:
	nop

