	.data
	.p2align 16

	.text
	.globl	_start
_start:
	l.addi	r1, r1, -4
	l.sw	0(r1), r9

	l.jal	8
	 l.movhi	r19, gotpchi(_GLOBAL_OFFSET_TABLE_-4)
	l.ori	r19, r19, gotpclo(_GLOBAL_OFFSET_TABLE_+0)
	l.add	r19, r19, r9

	l.movhi	r17, gotha(x)
	l.add	r17, r17, r19
	l.lwz	r17, got(x)(r17)
	l.lwz	r3, 0(r17)

	l.jal	plt(func)
	 l.nop
	l.lwz	r9, 0(r1)
	l.jr	r9
	 l.addi	r1, r1, 4
