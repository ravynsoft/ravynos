# relaxing unconditional jumps

.globl text1
.section text1, "ax", @progbits

	br on_border
	br out_of_range
	nop
	nop


.align 15
#	nop
#	nop
on_border:
	br in_range
	nop
	nop
	nop
out_of_range:
in_range:
	nop
	
.globl text2
.section text2, "ax", @progbits

	br text1
	br out_of_range
	br sym
	nop
	nop
sym:
	nop

	

	
