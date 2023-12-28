	.text
start:
	orw	%al, (%rax)
	orl	%al, (%rax)
	orq	%al, (%rax)

	orb	%ax, (%rax)
	orl	%ax, (%rax)
	orq	%ax, (%rax)

	orb	%eax, (%rax)
	orw	%eax, (%rax)
	orq	%eax, (%rax)

	orb	%rax, (%rax)
	orw	%rax, (%rax)
	orl	%rax, (%rax)

	pushq	%ax
	popq	%ax
	callq	*%ax
	jmpq	*%ax
