	.arm
	.func ARM
ARM:	nop

	.thumb
	.thumb_func
THUMB:
	blx	ARM
	nop
	blx	ARM
	nop
	.inst	0xf7ffeff6
	.inst	0xf7ffeff5
	nop
	.inst	0xf7ffeff1
	.inst	0xf7ffeff0
