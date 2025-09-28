	call foo
	nop
	.p2align	1
        nop
.L618:
	ldi r24,lo8(6)
	brsh .L618
foo:    nop
