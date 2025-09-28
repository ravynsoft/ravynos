	.section	.rodata
	.align 4
.LC2:
	.word	0x01
	.word	0x02
	.word	0x03

	.section	.text
	.align 4
	nop_s
	mov r4,@.LC2
