	.text
	.code64
equ:
	.set R18, %xmm18
	.set lDI, %dil
	.set xDI, %rdi
	.set x8, %r8
	.set x9, %r9d

	.code32
	vmovaps %xmm0, R18

	inc	lDI
	incl	(xDI)
	inc	x8
	inc	x9

	shlx	x8, x8, x8
	shlx	x9, x9, x9
