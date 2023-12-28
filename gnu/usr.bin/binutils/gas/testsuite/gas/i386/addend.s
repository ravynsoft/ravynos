	.text
_start:
	movl	$foo@GOT, %eax
	movl	$foo@GOT + 4, %eax

	.intel_syntax noprefix

	mov	eax, offset foo@got
	mov	eax, offset foo@got + 4
