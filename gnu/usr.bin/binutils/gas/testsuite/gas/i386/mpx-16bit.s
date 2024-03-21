	.code16
	.include "mpx.s"

	.att_syntax prefix
	.code32
bad16: # 16-bit addressing mode seen by the disassembler
	bndmk	(%eax), %bnd0
	bndmov	(%eax), %bnd0
	bndcl	(%eax), %bnd0
	bndcn	(%eax), %bnd0
	bndcu	(%eax), %bnd0
	bndstx	%bnd0, (%eax)
	bndldx	(%eax), %bnd0
