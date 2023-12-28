	.text
_start:
	rep; stosb
	rep ret
	rep
	ret
	rep
	ret
	call *%eax
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
	call *%eax
	.data
	.byte 0
