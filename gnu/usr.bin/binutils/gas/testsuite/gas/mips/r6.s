	.text
	.set	reorder
new:	maddf.s	$f0,$f1,$f2
	maddf.d	$f3,$f4,$f5
	msubf.s	$f6,$f7,$f8
	msubf.d	$f9,$f10,$f11
	cmp.af.s	$f0,$f1,$f2
	cmp.af.d	$f0,$f1,$f2
	cmp.un.s	$f0,$f1,$f2
	cmp.un.d	$f0,$f1,$f2
	cmp.eq.s	$f0,$f1,$f2
	cmp.eq.d	$f0,$f1,$f2
	cmp.ueq.s	$f0,$f1,$f2
	cmp.ueq.d	$f0,$f1,$f2
	cmp.lt.s	$f0,$f1,$f2
	cmp.lt.d	$f0,$f1,$f2
	cmp.ult.s	$f0,$f1,$f2
	cmp.ult.d	$f0,$f1,$f2
	cmp.le.s	$f0,$f1,$f2
	cmp.le.d	$f0,$f1,$f2
	cmp.ule.s	$f0,$f1,$f2
	cmp.ule.d	$f0,$f1,$f2
	cmp.saf.s	$f0,$f1,$f2
	cmp.saf.d	$f0,$f1,$f2
	cmp.sun.s	$f0,$f1,$f2
	cmp.sun.d	$f0,$f1,$f2
	cmp.seq.s	$f0,$f1,$f2
	cmp.seq.d	$f0,$f1,$f2
	cmp.sueq.s	$f0,$f1,$f2
	cmp.sueq.d	$f0,$f1,$f2
	cmp.slt.s	$f0,$f1,$f2
	cmp.slt.d	$f0,$f1,$f2
	cmp.sult.s	$f0,$f1,$f2
	cmp.sult.d	$f0,$f1,$f2
	cmp.sle.s	$f0,$f1,$f2
	cmp.sle.d	$f0,$f1,$f2
	cmp.sule.s	$f0,$f1,$f2
	cmp.sule.d	$f0,$f1,$f2
	cmp.or.s	$f0,$f1,$f2
	cmp.or.d	$f0,$f1,$f2
	cmp.une.s	$f0,$f1,$f2
	cmp.une.d	$f0,$f1,$f2
	cmp.ne.s	$f0,$f1,$f2
	cmp.ne.d	$f0,$f1,$f2
	cmp.sor.s	$f0,$f1,$f2
	cmp.sor.d	$f0,$f1,$f2
	cmp.sune.s	$f0,$f1,$f2
	cmp.sune.d	$f0,$f1,$f2
	cmp.sne.s	$f0,$f1,$f2
	cmp.sne.d	$f0,$f1,$f2
	bc1eqz	$f0,1f
	bc1eqz	$f31,1f
	bc1eqz	$f31,new
	bc1eqz	$f31,external_label
	bc1nez	$f0,1f
	bc1nez	$f31,1f
	bc1nez	$f31,new
	bc1nez	$f31,external_label
	bc2eqz	$0,1f
	bc2eqz	$31,1f
	bc2eqz	$31,new
	bc2eqz	$31,external_label
	bc2nez	$0,1f
	bc2nez	$31,1f
	bc2nez	$31,new
	bc2nez	$31,external_label
1:	sel.s	$f0,$f1,$f2
	sel.d	$f0,$f1,$f2
	seleqz.s	$f0,$f1,$f2
	seleqz.d	$f0,$f1,$f2
	selnez.s	$f0,$f1,$f2
	selnez.d	$f0,$f1,$f2
	seleqz	$2,$3,$4
	selnez	$2,$3,$4
	mul	$2,$3,$4
	muh	$2,$3,$4
	mulu	$2,$3,$4
	muhu	$2,$3,$4
	div	$2,$3,$4
	mod	$2,$3,$4
	divu	$2,$3,$4
	modu	$2,$3,$4
	lwc2	$2,0($4)
	lwc2	$2,-1024($4)
	lwc2	$2,1023($4)
	swc2	$2,0($4)
	swc2	$2,-1024($4)
	swc2	$2,1023($4)
	ldc2	$2,0($4)
	ldc2	$2,-1024($4)
	ldc2	$2,1023($4)
	sdc2	$2,0($4)
	sdc2	$2,-1024($4)
	sdc2	$2,1023($4)
	lsa	$2,$3,$4,1
	lsa	$2,$3,$4,4
	clz	$2,$3
	clo	$2,$3
	sdbbp
	sdbbp	0
	sdbbp	1
	sdbbp	1048575
	lui	$2,0xffff
	pref	0, -256($0)
	pref	31, 255($31)
	ll	$2,-256($3)
	ll	$2,255($3)
	sc	$2,-256($3)
	sc	$2,255($3)
	cache	0,-256($3)
	cache	31,255($3)


        align   $4, $2, $3, 0
        align   $4, $2, $3, 1
        align   $4, $2, $3, 2
        align   $4, $2, $3, 3


        bitswap  $4, $2

        bovc     $0, $0, ext
        bovc     $2, $0, ext
        bovc     $0, $2, ext
        bovc     $2, $4, ext
        bovc     $4, $2, ext
        bovc     $2, $4, . + 4 + (-32768 << 2)
        bovc     $2, $4, . + 4 + (32767 << 2)
        bovc     $2, $4, 1f
        bovc     $2, $2, ext
        bovc     $2, $2, . + 4 + (-32768 << 2)
        beqzalc $2, ext
        beqzalc $2, . + 4 + (-32768 << 2)
        beqzalc $2, . + 4 + (32767 << 2)
        beqzalc $2, 1f
        beqc    $3, $2, ext
        beqc    $2, $3, ext
        beqc    $3, $2, . + 4 + (-32768 << 2)
        beqc    $3, $2, . + 4 + (32767 << 2)
        beqc    $3, $2, 1f

        bnvc     $0, $0, ext
        bnvc     $2, $0, ext
        bnvc     $0, $2, ext
        bnvc     $2, $4, ext
        bnvc     $4, $2, ext
        bnvc     $2, $4, . + 4 + (-32768 << 2)
        bnvc     $2, $4, . + 4 + (32767 << 2)
        bnvc     $2, $4, 1f
        bnvc     $2, $2, ext
        bnvc     $2, $2, . + 4 + (-32768 << 2)
        bnezalc $2, ext
        bnezalc $2, . + 4 + (-32768 << 2)
        bnezalc $2, . + 4 + (32767 << 2)
        bnezalc $2, 1f
        bnec    $3, $2, ext
        bnec    $2, $3, ext
        bnec    $3, $2, . + 4 + (-32768 << 2)
        bnec    $3, $2, . + 4 + (32767 << 2)
        bnec    $3, $2, 1f

        blezc   $2, ext
        blezc   $2, . + 4 + (-32768 << 2)
        blezc   $2, . + 4 + (32767 << 2)
        blezc   $2, 1f
        bgezc   $2, ext
        bgezc   $2, . + 4 + (-32768 << 2)
        bgezc   $2, . + 4 + (32767 << 2)
        bgezc   $2, 1f
        bgec    $2, $3, ext
        bgec    $2, $3, . + 4 + (-32768 << 2)
        bgec    $2, $3, . + 4 + (32767 << 2)
        bgec    $2, $3, 1f
        bgec    $3, $2, 1f

        bgtzc   $2, ext
        bgtzc   $2, . + 4 + (-32768 << 2)
        bgtzc   $2, . + 4 + (32767 << 2)
        bgtzc   $2, 1f
        bltzc   $2, ext
        bltzc   $2, . + 4 + (-32768 << 2)
        bltzc   $2, . + 4 + (32767 << 2)
        bltzc   $2, 1f
        bltc    $2, $3, ext
        bltc    $2, $3, . + 4 + (-32768 << 2)
        bltc    $2, $3, . + 4 + (32767 << 2)
        bltc    $2, $3, 1f
        bltc    $3, $2, 1f

        blezalc $2, ext
        blezalc $2, . + 4 + (-32768 << 2)
        blezalc $2, . + 4 + (32767 << 2)
        blezalc $2, 1f
        bgezalc $2, ext
        bgezalc $2, . + 4 + (-32768 << 2)
        bgezalc $2, . + 4 + (32767 << 2)
        bgezalc $2, 1f
        bgeuc    $2, $3, ext
        bgeuc    $2, $3, . + 4 + (-32768 << 2)
        bgeuc    $2, $3, . + 4 + (32767 << 2)
        bgeuc    $2, $3, 1f
        bgeuc    $3, $2, 1f

        bgtzalc $2, ext
        bgtzalc $2, . + 4 + (-32768 << 2)
        bgtzalc $2, . + 4 + (32767 << 2)
        bgtzalc $2, 1f
        bltzalc $2, ext
        bltzalc $2, . + 4 + (-32768 << 2)
        bltzalc $2, . + 4 + (32767 << 2)
        bltzalc $2, 1f
        bltuc   $2, $3, ext
        bltuc   $2, $3, . + 4 + (-32768 << 2)
        bltuc   $2, $3, . + 4 + (32767 << 2)
        bltuc   $2, $3, 1f
        bltuc   $3, $2, 1f

        bc      ext
        bc      . + 4 + (-33554432 << 2)
        bc      . + 4 + (33554431 << 2)
        bc      1f
        balc    ext
        balc    . + 4 + (-33554432 << 2)
        balc    . + 4 + (33554431 << 2)
        balc    1f

        beqzc   $2, ext
        beqzc   $2, . + 4 + (-1048576 << 2)
        beqzc   $2, . + 4 + (1048575 << 2)
        beqzc   $2, 1f
	jic	$3,-32768
	jic	$3,32767
	jrc	$31

        bnezc   $2, ext
        bnezc   $2, . + 4 + (-1048576 << 2)
        bnezc   $2, . + 4 + (1048575 << 2)
        bnezc   $2, 1f
	jialc	$3,-32768
	jialc	$3,32767


        aui      $3, $2, 0xffff

        lapc        $3, 1f
        lapc   $4, .+(-262144 << 2)
        lapc   $4, .+(262143 << 2)
        addiupc   $4, (-262144 << 2)
        addiupc   $4, (262143 << 2)
        auipc      $3, 0xffff
        aluipc     $3, 0xffff
        lwpc      $4, 1f
        lwpc      $4, .+(-262144 << 2)
        lwpc      $4, .+(262143 << 2)
        lw      $4, (-262144 << 2)($pc)
        lw      $4, (262143 << 2)($pc)
1:
        nop
	addiu	$4, $pc, (262143 << 2)

	jalrc	$4
	nal

	evp
	dvp
	evp	$2
	dvp	$2

	sigrie	0
	sigrie	0xffff

	llwp	$5, $4, $6
	scwp	$5, $4, $6

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
