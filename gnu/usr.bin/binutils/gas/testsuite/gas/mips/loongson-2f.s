	.text
	.set noreorder

movz_insns:
	movz		$2, $3, $4
	movnz		$2, $3, $4
	movn		$2, $3, $4

integer_insns:
	mult.g		$2, $3, $4
	multu.g		$5, $6, $7
	dmult.g		$8, $9, $10
	dmultu.g	$11, $12, $13
	div.g		$14, $15, $16
	divu.g		$17, $18, $19
	ddiv.g		$20, $21, $22
	ddivu.g		$23, $24, $25
	mod.g		$26, $27, $28
	modu.g		$29, $30, $31
	dmod.g		$2, $3, $4
	dmodu.g		$5, $6, $7

fpu_insns:
	madd.s		$f0, $f1, $f2
	madd.d		$f3, $f4, $f5
	madd.ps		$f6, $f7, $f8
	msub.s		$f9, $f10, $f11
	msub.d		$f12, $f13, $f14
	msub.ps		$f15, $f16, $f17
	nmadd.s		$f18, $f19, $f20
	nmadd.d		$f21, $f22, $f23
	nmadd.ps	$f24, $f25, $f26
	nmsub.s		$f27, $f28, $f29
	nmsub.d		$f0, $f1, $f2
	nmsub.ps	$f3, $f4, $f5

mips5_ps_insns:
	abs.ps		$f0, $f2
	add.ps		$f2, $f4, $f6
	c.eq.ps		$f8, $f10
	c.f.ps		$f8, $f10
	c.le.ps		$f8, $f10
	c.lt.ps		$f8, $f10
	c.nge.ps	$f8, $f10
	c.ngl.ps	$f8, $f10
	c.ngle.ps	$f8, $f10
	c.ngt.ps	$f8, $f10
	c.ole.ps	$f8, $f10
	c.olt.ps	$f8, $f10
	c.seq.ps	$f8, $f10
	c.sf.ps		$f8, $f10
	c.ueq.ps	$f8, $f10
	c.ule.ps	$f8, $f10
	c.ult.ps	$f8, $f10
	c.un.ps		$f8, $f10
	mov.ps		$f24, $f26
	mul.ps		$f2, $f4, $f6
	neg.ps		$f6, $f8
	sub.ps		$f22, $f24, $f26
