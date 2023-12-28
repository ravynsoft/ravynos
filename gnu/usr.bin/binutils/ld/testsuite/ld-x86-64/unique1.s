	.text
foo:
	movq	bar@GOTPCREL(%rip), %rbp
	.weak	bar
	.data
	.type	bar, @gnu_unique_object
	.size	bar, 8
bar:
	.quad 8
