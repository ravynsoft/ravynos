# MPX instructions
	.text
	.code32
mpx32:
	bndcl	(%bx,%si), %bnd0
	bndcn	(%bx,%di), %bnd0
	bndcu	(%bp,%si), %bnd0
	bndldx	(%bp,%di), %bnd0
	bndmk	(%bx), %bnd0
	bndmov	(%bp), %bnd0
	bndmov	%bnd0, (%si)
	bndstx	%bnd0, (%di)

	bndcl	%di, %bnd1
	bndcn	%si, %bnd2
	bndcu	%bp, %bnd3

	.intel_syntax noprefix
	bndcl	bnd0, [bx]
	bndcn	bnd0, [bp]
	bndcu	bnd0, [si]
	bndldx	bnd0, [di]
	bndmk	bnd0, [bx+si]
	bndmov	bnd0, [bx+di]
	bndmov	[bp+si], bnd0
	bndstx	[bp+di], bnd0

	bndcl	bnd3, ax
	bndcn	bnd2, cx
	bndcu	bnd1, dx

	.att_syntax prefix
	.code16
mpx16:
	bndcl	(%bx,%si), %bnd0
	bndcn	(%bx,%di), %bnd0
	bndcu	(%bp,%si), %bnd0
	bndldx	(%bp,%di), %bnd0
	bndmk	(%bx), %bnd0
	bndmov	(%bp), %bnd0
	bndmov	%bnd0, (%si)
	bndstx	%bnd0, (%di)

	bndcl	%di, %bnd1
	bndcn	%si, %bnd2
	bndcu	%bp, %bnd3

	.intel_syntax noprefix
	bndcl	bnd0, [bx]
	bndcn	bnd0, [bp]
	bndcu	bnd0, [si]
	bndldx	bnd0, [di]
	bndmk	bnd0, [bx+si]
	bndmov	bnd0, [bx+di]
	bndmov	[bp+si], bnd0
	bndstx	[bp+di], bnd0

	bndcl	bnd3, ax
	bndcn	bnd2, cx
	bndcu	bnd1, dx
