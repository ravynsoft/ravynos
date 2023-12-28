	.text
	.intel_syntax noprefix
	.arch generic32
bnd:
	mov	eax, bnd0
	bndmov	bnd0, bnd0
	.p2align 4,0
