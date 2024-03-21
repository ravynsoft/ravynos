foo:
	jmp foo
	call foo
.L1:
	brsh .L1
.p2align	1
	nop
