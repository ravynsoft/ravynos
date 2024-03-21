	.text
_start:
	rep; stosb
	rep ret
	rep
	ret
	rep
	ret
	call *%rax
	.byte 0xf3, 0xc3
	.word 0x6666
	.byte 0xc3
	rep
	ret
	fwait
	rep ret
	.byte 0xf3
	.byte 0xc3
	ret
	.byte 0xf3
	call *%rax
	.data
	.byte 0
