	.macro	sym n:req
	.if	\n
	.globl	bar\@
	.type	bar\@, @function
bar\@ :
	.insn
	.hword	0
	.size	bar\@, . - bar\@
	sym	\n - 1
	.endif
	.endm

	.text
	.align	4
	sym	8
