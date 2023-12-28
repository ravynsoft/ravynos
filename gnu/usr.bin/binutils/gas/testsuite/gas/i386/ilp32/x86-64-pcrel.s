	.text
_start:
	movb	$(xtrn - .), %al
	movw	$(xtrn - .), %ax
	movl	$(xtrn - .), %eax
	movq	$(xtrn - .), %rax

	movb	$xtrn, %al
	movw	$xtrn, %ax
	movl	$xtrn, %eax
	movq	$xtrn, %rax
	movabs	$xtrn, %rax
	movabsq	xtrn, %rax
