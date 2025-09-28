	.text
movs:
	movsb	%al,%ax
	movsb	(%eax),%ax
	movsb	%al,%eax
	movsb	(%eax),%eax
.ifdef x86_64
	movsb	%al,%rax
	movsb	(%rax),%rax
.endif

	movsbw	%al,%ax
	movsbw	(%eax),%ax
	movsbl	%al,%eax
	movsbl	(%eax),%eax
.ifdef x86_64
	movsbq	%al,%rax
	movsbq	(%rax),%rax
.endif

	movsw	%ax,%eax
	movsw	(%eax),%eax
.ifdef x86_64
	movsw	%ax,%rax
	movsw	(%rax),%rax
.endif

	movswl	%ax,%eax
	movswl	(%eax),%eax
.ifdef x86_64
	movswq	%ax,%rax
	movswq	(%rax),%rax

	movsl	%eax,%rax
	movsl	(%rax),%rax

	movslq	%eax,%rax
	movslq	(%rax),%rax
.endif
