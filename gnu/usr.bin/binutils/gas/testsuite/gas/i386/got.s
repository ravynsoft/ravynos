	.text
_start:
	movl	$foo@GOT, %eax
	movl	foo@GOT, %eax
	movl	foo@GOT(%eax), %eax

	addl	$foo@GOT, %eax
	addl	foo@GOT, %eax
	addl	foo@GOT(%eax), %eax

	testl	$foo@GOT, %eax
	testl	foo@GOT, %eax
	testl	foo@GOT(%eax), %eax

	call	*foo@GOT
	call	*foo@GOT(%eax)
	jmp	*foo@GOT
	jmp	*foo@GOT(%eax)

	lsll	foo@GOT, %eax
	lsll	foo@GOT(%eax), %eax

	bndcn	foo@GOT, %bnd0
	bndcn	foo@GOT(%eax), %bnd0

	movlps	%xmm0, foo@GOT
	movlps	%xmm0, foo@GOT(%eax)

	.intel_syntax noprefix

	mov	eax, offset foo@got
	mov	eax, DWORD PTR [foo@GOT]
	mov	eax, DWORD PTR [eax + foo@GOT]

	add	eax, offset foo@got
	add	eax, DWORD PTR [foo@GOT]
	add	eax, DWORD PTR [eax + foo@GOT]

	call	DWORD PTR [eax + foo@GOT]
	call	DWORD PTR [foo@GOT]
	jmp	DWORD PTR [eax + foo@GOT]
	jmp	DWORD PTR [foo@GOT]
