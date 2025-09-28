.psize 0
.text
# test segment reg insns with memory operand
	movw	%ds,(%rax)
	mov	%ds,(%rax)
	movw	(%rax),%ds
	mov	(%rax),%ds
# test segment reg insns with avoided REX
	mov	%ds,%rax
	movq	%ds,%rax
	mov	%rax,%ds
	movq	%rax,%ds
# test segment reg insns with REX
	mov	%ds,%r8
	movq	%ds,%r8
	mov	%r8,%ds
	movq	%r8,%ds
	# Force a good alignment.
	.p2align	4,0
