# Source file used to test the loop instructions.

foo:
L1:
	loop	L2, r10.b2
	iloop	L2, r11
	add	r0, r0, r0
	add	r0, r0, r0
	add	r0, r0, r0
L2:
