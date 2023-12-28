	.data
	.type	bar, @object
bar:
	.byte	1
	.size	bar, .-bar
	.globl	foo
	.type	foo, @object
foo:
	.byte	1
	.size	foo, .-foo
	.text
	.globl	_start
	.type	_start, @function
_start:
	adcw	bar@GOT(%ecx), %ax
	addw	bar@GOT(%ecx), %bx
	andw	bar@GOT(%ecx), %cx
	cmpw	bar@GOT(%ecx), %dx
	orw	bar@GOT(%ecx), %di
	sbbw	bar@GOT(%ecx), %si
	subw	bar@GOT(%ecx), %bp
	xorw	bar@GOT(%ecx), %sp
	testw	%cx, bar@GOT(%ecx)
	adcw	foo@GOT(%ecx), %ax
	addw	foo@GOT(%ecx), %bx
	andw	foo@GOT(%ecx), %cx
	cmpw	foo@GOT(%ecx), %dx
	orw	foo@GOT(%ecx), %di
	sbbw	foo@GOT(%ecx), %si
	subw	foo@GOT(%ecx), %bp
	xorw	foo@GOT(%ecx), %sp
	testw	%cx, foo@GOT(%ecx)
	.size	_start, .-_start
