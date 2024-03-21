	.text
_start:
	movq	$foo@GOTPCREL, %rax
	movq	foo@GOTPCREL, %rax
	movq	foo@GOTPCREL(%rip), %rax
	movq	foo@GOTPCREL(%rcx), %rax

	call	*foo@GOTPCREL(%rip)
	call	*foo@GOTPCREL(%rax)
	jmp	*foo@GOTPCREL(%rip)
	jmp	*foo@GOTPCREL(%rcx)

	.intel_syntax noprefix

	mov	rax, offset foo@gotpcrel
	mov	rax, QWORD PTR [foo@GOTPCREL]
	mov	rax, QWORD PTR [rip + foo@GOTPCREL]
	mov	rax, QWORD PTR [rcx + foo@GOTPCREL]

	call	QWORD PTR [rip + foo@GOTPCREL]
	call	QWORD PTR [rax + foo@GOTPCREL]
	jmp	QWORD PTR [rip + foo@GOTPCREL]
	jmp	QWORD PTR [rcx + foo@GOTPCREL]
