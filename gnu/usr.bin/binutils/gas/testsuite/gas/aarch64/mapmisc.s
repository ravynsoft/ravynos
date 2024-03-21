	.text
	.type foo, %function
foo:
	.align 2
	.fill 0, 0, 0
	nop
	.ascii "abcd"
	nop
	.asciz "abc"
	nop
	.string "efg"
	nop
	.string8 "hij"
	nop
	.string16 "k"
	nop
	.string32 "l"
	nop
	.string64 "m"
	nop
	.float 0e1.5
	nop
	.single 0e2.5
	nop
	.double 0e3.5
	nop
	.dcb.d 1, 4.5
	nop
	.fill 4, 4, 4
	nop
	.space 4
	nop
	.skip 4
	nop
	.zero 4
	nop
	.incbin "mapmisc.dat"
	nop
	.fill 0, 0, 0
	nop
