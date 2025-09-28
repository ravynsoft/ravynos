# Invalid Intel MCU instructions / directives
	.text

	fnstsw
	fstsw	%ax

	cmove	%eax,%ebx
	nopw	(%eax)

	movq	%xmm1, (%eax)
	movnti	%eax, (%eax)

	.arch generic32
