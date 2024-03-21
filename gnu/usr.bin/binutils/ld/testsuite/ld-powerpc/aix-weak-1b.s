	.globl	a
	.csect	a[RW],2
a:
	.long	0x55555555
	.weak	b
	.csect	b[RW],2
b:
	.long	0x66666666
	.comm	c,4
	.comm	d,4

	# Same again, with weak common symbols
	.globl	e
	.csect	e[RW],2
e:
	.long	0x77777777
	.weak	f
	.csect	f[RW],2
f:
	.long	0x88888888
	.weak	g
	.comm	g,4
	.weak	h
	.comm	h,4
