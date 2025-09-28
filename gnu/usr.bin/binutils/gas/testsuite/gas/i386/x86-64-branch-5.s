	.text
	lcallq	*(%rax)
	lfs	(%rax), %rax
	lfsq	(%rax), %rax
	lgs	(%rax), %rax
	lgsq	(%rax), %rax
	ljmpq	*(%rax)
	lss	(%rax), %rax
	lssq	(%rax), %rax

	.intel_syntax noprefix
	call	TBYTE PTR [rax]
	lfs	rax, [rax]
	lfs	rax, TBYTE PTR [rax]
	lgs	rax, [rax]
	lgs	rax, TBYTE PTR [rax]
	jmp	TBYTE PTR [rax]
	lss	rax, [rax]
	lss	rax, TBYTE PTR [rax]
