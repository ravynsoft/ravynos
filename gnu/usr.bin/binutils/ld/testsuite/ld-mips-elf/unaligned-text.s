	.macro	sym n:req
	.if	\n
	.globl	bar\@
	.type	bar\@, @object
bar\@ :
	.byte	0
	.size	bar\@, . - bar\@
	sym	\n - 1
	.endif
	.endm

	.text
	.align	4
	.space	32
	sym	16
