	.file	"t2.c"
	.text
	.p2align 4,,15
	.globl	foo
	.def	foo;	.scl	2;	.type	32;	.endef
	.seh_proc	foo
foo:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	leal	1(%rcx), %eax
	cltq
	addq	$30, %rax
	andq	$-16, %rax
	call	___chkstk
	leaq	32(%rsp), %rcx
	call	bar
	movq	%rbp, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.def	bar;	.scl	2;	.type	32;	.endef
