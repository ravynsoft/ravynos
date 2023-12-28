
.syntax unified
.text

	nop
	vstr d0, float
	vstr d0, [pc, #4]
	nop
	nop

.align
float:	.double 0.0
