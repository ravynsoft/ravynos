# Check Intel64

	.text
	.arch core2
_start:
	lfs	(%rax), %rcx
	lfsq	(%rax), %rcx
	lgs	(%rcx), %rdx
	lgsq	(%rcx), %rdx
	lss	(%rdx), %rbx
	lssq	(%rdx), %rbx

	lcallq	*(%rax)
	ljmpq	*(%rcx)

	syscall
	sysretl
	sysretq

	.intel_syntax noprefix
	lfs	rax, [rcx]
	lfs	rax, tbyte ptr [rcx]
	lgs	rcx, [rdx]
	lgs	rcx, tbyte ptr [rdx]
	lss	rdx, [rbx]
	lss	rdx, tbyte ptr [rbx]

	call	tbyte ptr [rcx]
	jmp	tbyte ptr [rdx]
