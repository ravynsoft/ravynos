	.text
movz:
	movzb	%al,%ax
	movzb	(%eax),%ax
	movzb	%al,%eax
	movzb	(%eax),%eax
.ifdef x86_64
	movzb	%al,%rax
	movzb	(%rax),%rax
.endif

	movzbw	%al,%ax
	movzbw	(%eax),%ax
	movzbl	%al,%eax
	movzbl	(%eax),%eax
.ifdef x86_64
	movzbq	%al,%rax
	movzbq	(%rax),%rax
.endif

	movzw	%ax,%eax
	movzw	(%eax),%eax
.ifdef x86_64
	movzw	%ax,%rax
	movzw	(%rax),%rax
.endif

	movzwl	%ax,%eax
	movzwl	(%eax),%eax
.ifdef x86_64
	movzwq	%ax,%rax
	movzwq	(%rax),%rax
.endif
