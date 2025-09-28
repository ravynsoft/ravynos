# Check 64bit instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	andq	$foo, %rax
	andq	$((1<<31) - 1), %rax
	andq	$((1<<31) - 1), %rbx
	andq	$((1<<31) - 1), %r14
	andq	$-((1<<31)), %rax
	andq	$-((1<<31)), %rbx
	andq	$-((1<<31)), %r14
	andq	$((1<<7) - 1), %rax
	andq	$((1<<7) - 1), %rbx
	andq	$((1<<7) - 1), %r14
	andq	$-((1<<7)), %rax
	andq	$-((1<<7)), %rbx
	andq	$-((1<<7)), %r14
	testq	$((1<<31) - 1), %rax
	testq	$((1<<31) - 1), %rbx
	testq	$((1<<31) - 1), %r14
	testq	$-((1<<31)), %rax
	testq	$-((1<<31)), %rbx
	testq	$-((1<<31)), %r14
	xorq	(%rsi), %rax
	xorq	%rax, %rax
	xorq	%rbx, %rbx
	xorq	%r14, %r14
	xorq	%rdx, %rax
	xorq	%rdx, %rbx
	xorq	%rdx, %r14
	subq	%rax, %rax
	subq	%rbx, %rbx
	subq	%r14, %r14
	subq	%rdx, %rax
	subq	%rdx, %rbx
	subq	%rdx, %r14
	andq	$((1<<31) - 1), (%rax)
	andq	$-((1<<31)), (%rax)
	testq	$((1<<31) - 1), (%rax)
	testq	$-((1<<31)), (%rax)
	mov	$((1<<31) - 1),%rax
	movq	$((1<<31) - 1),%rax
	mov	$((1<<31) - 1),%r8
	movq	$((1<<31) - 1),%r8
	mov	$0xffffffff,%rax
	movq	$0xffffffff,%rax
	mov	$0xffffffff,%r8
	movq	$0xffffffff,%r8
	mov	$1023,%rax
	movq	$1023,%rax
	mov	$0x100000000,%rax
	movq	$0x100000000,%rax
	clrq	%rax
	clrq	%r14
	bt	$15, %ax
	bt	$16, %ax
	bt	$15, %r8w
	bt	$16, %r8w
	bt	$31, %rax
	bt	$32, %rax
	bt	$31, %r8
	btc	$15, %ax
	btc	$31, %rax
	btr	$15, %ax
	btr	$31, %rax
	bts	$15, %ax
	bts	$31, %rax
