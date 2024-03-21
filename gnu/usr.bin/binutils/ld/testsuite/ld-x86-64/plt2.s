	.text
	.globl _start
	.type _start,@function
_start:
	call fn1
	call fn2
	cmpl $fn1, 8(%rsp)
