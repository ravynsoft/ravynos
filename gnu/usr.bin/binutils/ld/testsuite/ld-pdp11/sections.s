	.globl	_start
	.text
_start:
	mov	_data,_bss
	.globl	_data
	.data
_data:
	.word	1
	.globl	_bss
	.bss
_bss:
	.=.+2
	.end
