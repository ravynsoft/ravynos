	.macro insn, mnem:req, opnds:vararg
	.struct
	\mnem	\opnds
	.equiv	\mnem, .
	.endm

	insn	inc	%eax

	.equiv .Lis_64bit, inc > 1

	insn	add	$1, %al

	insn	adc	$1, %cl

	insn	sub	$0x12345678, %eax

	insn	sbb	$0x12345678, %ecx

	insn	and	$1, %eax

	insn	call	.

	insn	jecxz	.

	insn	pextrw	$0, %xmm0, %eax

	.macro pextrw_store opnds:vararg
	{store} pextrw \opnds
	.endm
	insn	pextrw_store $0, %xmm0, %eax

	insn	vpextrw	$0, %xmm0, %eax

	.macro vpextrw_evex opnds:vararg
	{evex} vpextrw \opnds
	.endm
	insn	vpextrw_evex $0, %xmm0, %eax

	.if .Lis_64bit

	insn	mov	$0x876543210, %rcx

	insn	movq	0x876543210, %rax

	.else

	insn	lcall	$0, $0

	.code16

	insn	ljmp	$0, $0

	.endif

	insn	bextr	$0x11223344, %fs:(,%eax,2), %eax
