	.comm	a,4
	.comm	b,4
	.globl	c
	.csect	c[RW],2
c:
	.long	0x11111111
	.weak	d
	.csect	d[RW],2
d:
	.long	0x22222222

	# Same again, with weak common symbols
	.weak	e
	.comm	e,4
	.weak	f
	.comm	f,4
	.globl	g
	.csect	g[RW],2
g:
	.long	0x33333333
	.weak	h
	.csect	h[RW],2
h:
	.long	0x44444444
