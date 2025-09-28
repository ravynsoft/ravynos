	.text
	.globl	_start
	.type	_start, @function
_start:
	mov	ifunc@GOT, %eax
	ret
	.type ifunc, @gnu_indirect_function
ifunc:
	mov	$0xbadbeef, %eax
	ret
