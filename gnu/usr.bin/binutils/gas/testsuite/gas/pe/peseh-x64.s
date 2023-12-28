	.file	"t1.c"
	.text
	.p2align 4,,15
	.globl	foo
	.def	foo;	.scl	2;	.type	32;	.endef
	.seh_proc	foo
foo:
	subq	$8, %rsp
	.seh_stackalloc	8
	.seh_endprologue
	movl	$1, %eax
	addq	$8, %rsp
	ret
	.seh_endproc
