	.text
	.set noreorder

foo:
        bbit032 $23,51,foo
        nop
        bbit0   $23,71,foo
        nop

        bbit132 $23,49,foo
        nop
        bbit1   $23,74,foo
        nop

        cins    $2,0,37

        cins32  $19,$31,39,12
        cins32  $17,$20,7,25

        cins    $24,$10,64,8
        cins    $21,$30,50,14

        c2      1
        bc2f    foo
        bc2fl   foo
        bc2t    foo
        bc2tl   foo
        cfc2    $25,$12
        ctc2    $12,$2
        ldc2    $10,0($25)
        lwc2    $11,12($31)
        mfc2    $24,$1
        mfhc2   $17,$20
        mtc2    $2,$21
        mthc2   $13,$25
        sdc2    $22,8($4)
        swc2    $2,24($2)

        cop2    23
        ldc2    $8,foo
        lwc2    $16,foo+4
        sdc2    $10,0x12345678
        swc2    $16,0x12345($15)

        dmfc2   $2,0x10000
        dmtc2   $2,0x12345
        dmfc2   $9,$12
        dmfc2   $4,$15,4
        dmtc2   $16,$8
        dmtc2   $22,$7,$4

        exts    $26,26,32

        exts32  $7,$21,32,10
        exts32  $31,$13,3,29

        exts    $14,$29,70,14
        exts    $20,$16,39,25

        seqi    $14,$13,512
        seqi    $19,-771
        snei    $18,$30,615
        snei    $17,-513
