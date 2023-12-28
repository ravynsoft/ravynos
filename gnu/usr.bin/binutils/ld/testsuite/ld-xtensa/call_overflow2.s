	.text
	.global a
	.align	4
a:
	j	a

	.align	4
x:
	call8	b
#29
	.rep	131070
	nop.n
	nop.n
	.endr
