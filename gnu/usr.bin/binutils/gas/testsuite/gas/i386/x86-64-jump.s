.psize 0
.text
.extern xxx

1:	jmp	1b
	jmp	xxx
	jmp	*xxx
	jmp	*%rdi
	jmp	*(%rdi)
	ljmp	*xxx(,%rdi,4)
	ljmpw	*xxx(,%rdi,4)
	ljmp	*xxx
	ljmpw	*xxx

	call	1b
	call	xxx
	call	*xxx
	call	*%rdi
	call	*(%rdi)
	lcall	*xxx(,%rdi,4)
	lcallw	*xxx(,%rdi,4)
	lcall	*xxx
	lcallw	*xxx

	jecxz	3+$
	nop
	jrcxz	2+$
	nop

	.intel_syntax noprefix
	call	word ptr [rbx]
	call	dword ptr [rbx]
	call	fword ptr [rbx]
	call	qword ptr [rbx]
	call	near ptr [rbx]
	call	far ptr [rbx]
	jmp	word ptr [rbx]
	jmp	dword ptr [rbx]
	jmp	fword ptr [rbx]
	jmp	qword ptr [rbx]
	jmp	near ptr [rbx]
	jmp	far ptr [rbx]
	jmp	$+2
	nop
	jecxz	3+$
	nop
	jrcxz	2+$
	nop
	jmp	.+2
