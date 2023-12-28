	.text
	.weak bar
	.globl	_start
	.type	_start, @function
_start:
	cmp	bar@GOTPCREL(%rip), %rax
	cmp	bar@GOTPCREL(%rip), %ecx
	cmp	bar@GOTPCREL(%rip), %r11
	cmp	bar@GOTPCREL(%rip), %r12d

	mov	bar@GOTPCREL(%rip), %rax
	mov	bar@GOTPCREL(%rip), %ecx
	mov	bar@GOTPCREL(%rip), %r11
	mov	bar@GOTPCREL(%rip), %r12d

	test	%rax, bar@GOTPCREL(%rip)
	test	%ecx, bar@GOTPCREL(%rip)
	test	%r11, bar@GOTPCREL(%rip)
	test	%r12d, bar@GOTPCREL(%rip)
	.size	_start, .-_start
