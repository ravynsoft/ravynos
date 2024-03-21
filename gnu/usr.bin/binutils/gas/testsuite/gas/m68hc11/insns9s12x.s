# Test for correct generation of 9s12x specific insns.

	.sect .text

    addx    #0x5678
    addy    2,x+
    aded    0,x
    adex    2,-y
    adey    [d,x]
    andx    #0x9988
    andy    0x55aa
    aslw    0x2004
    aslx
    asly
    asrw    0x3000,y
    asrx
    asry
    bitx    [0x3456,sp]
    bity    [d,sp]
    btas    0x2345, #0x04
    clrw    0x2008,y
    clrx
    clry
    comw    0,x
    comx
    comy
    cped    #0xfdeb
    cpes    0xfedc
    cpex    2,sp
    cpey    2,sp+
    decw    0,x
    decx
    decy
    eorx    0x34
    eory    0x1234
; exg in own test
    gldaa   0x5678
    gldab   0,x
    gldd    2,y+
    glds    0,y
    gldx    [d,y]
    gldy    [d,x]
    gstaa   0x5001
    gstab   0x5189
    gstd    0x5000,x
    gsts    0x7008
    gstx    0x6001,y
    gsty    [d,x]
    incw    [0x100,sp]
    incx
    incy
    lslw    0x3005
    lslx
    lsly
    lsrw    0x3890
    lsrx
    lsry
; mov in own test
    negw    2,-y
    negx
    negy
    orx     #0x9876
    ory     0x9876
    pshcw
    pulcw
    rolw    0x5544
    rolx
    roly
    rorw    0,x
    rorx
    rory
    sbed    2,y
    sbex    0x3458
    sbey    0x8543
;sex with exg    
    subx    [d,y]
    suby    [d,x]
    sys
;tfr with exg
    tstw    3,x
    tstx
    tsty

