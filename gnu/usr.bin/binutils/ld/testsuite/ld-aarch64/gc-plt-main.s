	.data
	.global var
	.text
	.globl _start
	.type _start, %function
_start:
	bl	foo
	.size _start, . - _start
