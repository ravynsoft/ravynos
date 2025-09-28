	.text
	.align	2
	.globl	_func1
	.type	_func1, @function
_func1:
	.cfi_startproc
	lla	a1,_func2
	add	sp,sp,-16
	.cfi_def_cfa_offset 16
	.cfi_endproc
	.size	_func1, .-_func1
