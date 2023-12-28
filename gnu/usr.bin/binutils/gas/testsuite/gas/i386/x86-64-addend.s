	.text
_start:
	movq	$foo@GOT, %rax
	movq	$foo@GOT + 4, %rax

	.intel_syntax noprefix

	mov	rax, offset foo@got
	mov	rax, offset foo@got + 4
