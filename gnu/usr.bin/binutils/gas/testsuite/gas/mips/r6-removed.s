	.set	reorder
	.set	eva
	abs.ps	$f0,$f2
	add.ps	$f0,$f2,$f2
	addi	$15,$16,256
	alnv.ps	$f0,$f2,$f2,$3
	bc0f	1f
	bc0fl	1f
	bc0t	1f
	bc0tl	1f
	bc1f	1f
	bc1fl	1f
	bc1t	1f
	bc1tl	1f
	bc2f	1f
	bc2fl	1f
	bc2t	1f
	bc2tl	1f
	bc3f	1f
	bc3fl	1f
	bc3t	1f
	bc3tl	1f
	beql	$28,$29,1f
	bgezal	$4,1f
	bgezall	$28,1f
	bgezl	$28,1f
	bgtzl	$28,1f
	blezl	$28,1f
	bltzal	$4,1f
	bltzall	$28,1f
	bltzl	$28,1f
	bnel	$28,$29,1f
	c.f.s	$f0,$f2
	c.un.s	$f0,$f2
	c.eq.s	$f0,$f2
	c.ueq.s	$f0,$f2
	c.olt.s	$f0,$f2
	c.ult.s	$f0,$f2
	c.ole.s	$f0,$f2
	c.ule.s	$f0,$f2
	c.sf.s	$f0,$f2
	c.ngle.s	$f0,$f2
	c.seq.s	$f0,$f2
	c.ngl.s	$f0,$f2
	c.lt.s	$f0,$f2
	c.nge.s	$f0,$f2
	c.le.s	$f0,$f2
	c.ngt.s	$f0,$f2
	c.f.ps	$f0,$f2
	c.un.ps	$f0,$f2
	c.eq.ps	$f0,$f2
	c.ueq.ps	$f0,$f2
	c.olt.ps	$f0,$f2
	c.ult.ps	$f0,$f2
	c.ole.ps	$f0,$f2
	c.ule.ps	$f0,$f2
	c.sf.ps	$f0,$f2
	c.ngle.ps	$f0,$f2
	c.seq.ps	$f0,$f2
	c.ngl.ps	$f0,$f2
	c.lt.ps	$f0,$f2
	c.nge.ps	$f0,$f2
	c.le.ps	$f0,$f2
	c.ngt.ps	$f0,$f2
	c.f.d	$f0,$f2
	c.un.d	$f0,$f2
	c.eq.d	$f0,$f2
	c.ueq.d	$f0,$f2
	c.olt.d	$f0,$f2
	c.ult.d	$f0,$f2
	c.ole.d	$f0,$f2
	c.ule.d	$f0,$f2
	c.sf.d	$f0,$f2
	c.ngle.d	$f0,$f2
	c.seq.d		$f0,$f2
	c.ngl.d		$f0,$f2
	c.lt.d	$f0,$f2
	c.nge.d	$f0,$f2
	c.le.d	$f0,$f2
	c.ngt.d	$f0,$f2
	c.f.s	$fcc2, $f0,$f2
	c.un.s	$fcc2, $f0,$f2
	c.eq.s	$fcc2, $f0,$f2
	c.ueq.s	$fcc2, $f0,$f2
	c.olt.s	$fcc2, $f0,$f2
	c.ult.s	$fcc2, $f0,$f2
	c.ole.s	$fcc2, $f0,$f2
	c.ule.s	$fcc2, $f0,$f2
	c.sf.s	$fcc2, $f0,$f2
	c.ngle.s	$fcc2, $f0,$f2
	c.seq.s	$fcc2, $f0,$f2
	c.ngl.s	$fcc2, $f0,$f2
	c.lt.s	$fcc2, $f0,$f2
	c.nge.s	$fcc2, $f0,$f2
	c.le.s	$fcc2, $f0,$f2
	c.ngt.s	$fcc2, $f0,$f2
	c.f.ps	$fcc2, $f0,$f2
	c.un.ps	$fcc2, $f0,$f2
	c.eq.ps	$fcc2, $f0,$f2
	c.ueq.ps	$fcc2, $f0,$f2
	c.olt.ps	$fcc2, $f0,$f2
	c.ult.ps	$fcc2, $f0,$f2
	c.ole.ps	$fcc2, $f0,$f2
	c.ule.ps	$fcc2, $f0,$f2
	c.sf.ps	$fcc2, $f0,$f2
	c.ngle.ps	$fcc2, $f0,$f2
	c.seq.ps	$fcc2, $f0,$f2
	c.ngl.ps	$fcc2, $f0,$f2
	c.lt.ps	$fcc2, $f0,$f2
	c.nge.ps	$fcc2, $f0,$f2
	c.le.ps	$fcc2, $f0,$f2
	c.ngt.ps	$fcc2, $f0,$f2
	c.f.d	$fcc2, $f0,$f2
	c.un.d	$fcc2, $f0,$f2
	c.eq.d	$fcc2, $f0,$f2
	c.ueq.d	$fcc2, $f0,$f2
	c.olt.d	$fcc2, $f0,$f2
	c.ult.d	$fcc2, $f0,$f2
	c.ole.d	$fcc2, $f0,$f2
	c.ule.d	$fcc2, $f0,$f2
	c.sf.d	$fcc2, $f0,$f2
	c.ngle.d	$fcc2, $f0,$f2
	c.seq.d		$fcc2, $f0,$f2
	c.ngl.d		$fcc2, $f0,$f2
	c.lt.d	$fcc2, $f0,$f2
	c.nge.d	$fcc2, $f0,$f2
	c.le.d	$fcc2, $f0,$f2
	c.ngt.d	$fcc2, $f0,$f2
	cvt.ps.s	$f2,$f3,$f4
	jalx	1f
	ldxc1	$f0,$0($2)
	luxc1	$f0,$0($2)
	lwl	$2, 1($3)
	lwle	$4,0($6)
	lwr	$2, 1($3)
	lwre	$4,0($6)
	lwxc1	$f0,$0($2)
	madd	$2,$3
	maddu	$2,$3
	madd.s	$f5,$f6,$f7,$f8
	madd.d	$f6,$f8,$f10,$f12
	madd.ps	$f6,$f8,$f10,$f12
	mfhi	$2
	mflo	$2
	mov.ps	$f10,$f20
	movf	$8,$9,$fcc0
	movf.s	$f8,$f9,$fcc0
	movf.d	$f8,$f10,$fcc0
	movf.ps	$f8,$f10,$fcc0
	movn	$2,$3,$4
	movn.s	$f0,$f2,$10
	movn.d	$f0,$f2,$10
	movn.ps	$f0,$f2,$10
	movt	$10,$11,$fcc2
	movt.s	$f20,$f21,$fcc2
	movt.d	$f20,$f22,$fcc2
	movt.ps	$f20,$f22,$fcc2
	movz	$5,$6,$7
	movz.s	$f0,$f2,$10
	movz.d	$f0,$f2,$10
	movz.ps	$f0,$f2,$10
	msub	$2,$3
	msubu	$2,$3
	msub.s	$f5,$f6,$f7,$f8
	msub.d	$f6,$f8,$f10,$f12
	msub.ps	$f6,$f8,$f10,$f12
	mthi	$2
	mtlo	$2
	mul.ps	$f10,$f20,$f22
	mult	$2,$3
	multu	$2,$3
	neg.ps	$f22,$f24
	nmadd.s	$f5,$f6,$f7,$f8
	nmadd.d	$f6,$f8,$f10,$f12
	nmadd.ps	$f6,$f8,$f10,$f12
	nmsub.s	$f5,$f6,$f7,$f8
	nmsub.d	$f6,$f8,$f10,$f12
	nmsub.ps	$f6,$f8,$f10,$f12
	pll.ps	$f24,$f20,$f26
	plu.ps	$f24,$f20,$f26
	pul.ps	$f24,$f20,$f26
	puu.ps	$f24,$f20,$f26
	prefx   5, $3($5)
	sdxc1	$f0,$0($2)
	sub.ps	$f20,$f28,$f26
	suxc1	$f0,$0($2)
	swl	$2, 1($3)
	swle	$4,0($6)
	swr	$2, 1($3)
	swre	$4,0($6)
	swxc1	$f0,$0($2)
	teqi	$11,1024
	tgei	$11,1024
	tgeiu	$11,1024
	tlti	$11,1024
	tltiu	$11,1024
	tnei	$11,1024
1:
