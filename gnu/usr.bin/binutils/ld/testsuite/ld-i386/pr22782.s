	.text
	.globl	_start
	.type	_start, @function
_start:
	movl	errno@indntpoff, %eax
	movl	%gs:(%eax), %eax
	ret
	.globl	errno
	.hidden	errno
	.section	.tbss,"awT",@nobits
	.align 4
	.type	errno, @object
	.size	errno, 4
errno:
	.zero	4
