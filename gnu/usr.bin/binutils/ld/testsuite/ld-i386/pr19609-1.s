	.text
	.weak bar
	.globl	_start
	.type	_start, @function
_start:
	cmp	bar@GOT(%edx), %eax
	cmp	bar@GOT(%edx), %ecx
	mov	bar@GOT(%edx), %eax
	mov	bar@GOT(%edx), %ecx
	test	bar@GOT(%edx), %eax
	test	bar@GOT(%edx), %ecx
	.size	_start, .-_start
