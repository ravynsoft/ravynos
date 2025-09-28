# Test parallel instructions on same functional unit.
.text
.globl f
f:
	add .L1 a1,a2,a3
	nop
	|| [b1] add .L1 a1,a2,a3
	|| nop
	|| [!b1] add .L1 a4,a5,a6
