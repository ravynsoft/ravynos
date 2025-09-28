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
	adcw	bar@GOTPCREL(%rip), %ax
	addw	bar@GOTPCREL(%rip), %bx
	andw	bar@GOTPCREL(%rip), %cx
	cmpw	bar@GOTPCREL(%rip), %dx
	orw	bar@GOTPCREL(%rip), %di
	sbbw	bar@GOTPCREL(%rip), %si
	subw	bar@GOTPCREL(%rip), %bp
	xorw	bar@GOTPCREL(%rip), %r8w
	testw	%cx, bar@GOTPCREL(%rip)
	adcw	foo@GOTPCREL(%rip), %ax
	addw	foo@GOTPCREL(%rip), %bx
	andw	foo@GOTPCREL(%rip), %cx
	cmpw	foo@GOTPCREL(%rip), %dx
	orw	foo@GOTPCREL(%rip), %di
	sbbw	foo@GOTPCREL(%rip), %si
	subw	foo@GOTPCREL(%rip), %bp
	xorw	foo@GOTPCREL(%rip), %r8w
	testw	%cx, foo@GOTPCREL(%rip)
	.size	_start, .-_start
