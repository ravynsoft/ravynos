	.text
	.globl	_start
	.type	_start, @function
_start:
	call	*func1@GOTPCREL(%rip)
	cmp	$func1,%eax
	.globl func1
	.type	func1, @gnu_indirect_function
func1:
	ret
