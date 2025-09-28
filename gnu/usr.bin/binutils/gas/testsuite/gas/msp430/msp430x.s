	.text
	.global foo
foo:
	adc	r4		; MSP430 instruction for comparison purposes.

	adcx	r4
	adcx.a	bar
	adcx.b	r6
	adcx.w	r7

	addcx	r8, r9
	addcx.a	#0x12345, r10
	addcx.b	r11, r12
	addcx.w	r13, r14
	
	ADDX    @R9, PC
	ADDX    R9, PC
	ADDX.A  #FE000h, PC
	ADDX.A  &EDE, PC
	ADDX.A  @R9+, PC
	ADDX.A  EDE, PC
	addx.b	r1, r2
	addx.w	r3, r4
	ADDX    K(R4), R5
	
	ANDX    #1234, 4(R6)
	ANDX    4(R7), 4(R6)
	ANDX    @R5+, 4(R6)
	ANDX    EDE, 4(R6)
	ANDX    EDE, TONI
	ANDX.A  @R5, 4(R6)
	ANDX.A  R5, 4(R6)
	ANDX.B  &EDE, 4(R6)
	andx.w	r1, r2
	
	bicx	#0xa0, r14
	bicx.a	#0xa0, r14
	bicx.b	#0xa0, r14
	bicx.w	#0xa0, r14
	
	bisx	#8, r11
	bisx.a	#8, r11
	bisx.b	#8, r11
	bisx.w	#8, r11
	
	BITX    #20, R8
	BITX    &EDE, &TONI
	BITX    &EDE, R8
	BITX    2(R5), R8
	BITX    8(SP), &EDE
	BITX    @R5+, &EDE
	BITX    @R5+, R8
	BITX    @R5, R8
	BITX    EDE, &TONI
	BITX.B  #12, &EDE
	BITX.B  @R5, &EDE
	BITX.B  EDE, R8
	BITX.B  R5, R8
	BITX.W  R5, &EDE
	
	clrx	TONI
	clrx.a	fooz
	clrx.b	bar
	clrx.w	baz

	cmpx	#0, r15
	cmpx.a	#01800h, ede
	cmpx.b	@r1, r15
	cmpx.w	@r2+, &pin
	
	dadcx	fooz
	dadcx.a 0(r12)
	dadcx.b	bar
	dadcx.w r12

	daddx	@r5, r7
	daddx.a	#10h, &decdr
	daddx.b 2(r6), r4
	daddx.w bcd, r4

	decx	toni
	decx.a	fooz
	decx.b	bar
	decx.w	fred
	
	decdx	toni
	decdx.a	fooz
	decdx.b	bar
	decdx.w	fred

	incx	r4
	incx.a	r5
	incx.b	r6
	incx.w	r7
	
	incdx	r8
	incdx.a	r9
	incdx.b	r10
	incdx.w	r11

	invx	r12
	invx.a	LEO
	invx.b	r14
	invx.w	r15

	movx	#foo, r4
	movx.a	#foo, r5
	movx.b	#foo, r6
	movx.w	#foo, r7
	MOVX    &X, R5
	MOVX    #X, R5
	MOVX    R5, &Y
	MOVX    #0xabcde, &Y
	MOVX    &X, &Y
	MOVX    #X, &Y
	MOVX    X, R5
	MOVX    R5, Y
	MOVX    #0xabcde, Y
	MOVX    X, Y

	sbcx	r15
	sbcx.a	012345h
	sbcx.b	r15
	sbcx.w	0(r7)
	
	subcx	r15, r15
	subcx.a	#012345h, r15
	subcx.b	r15, r15
	subcx.w	@r5+, 0(r7)

	SUBX    2(R6), PC
	SUBX.A  #4455, ede
	SUBX.B  2(R6), PC
	SUBX.W  2(R6), PC

	tstx	LEO
	tstx.a	foo
	tstx.b	bar
	tstx.w	baz

	XORX    #5A5Ah, EDE
	XORX    &EDE, TONI
	XORX    @R8, EDE
	XORX    R8, EDE
	XORX.B  2(R6), EDE
	XORX.B  @R8+, EDE
	xorx.a	toni, &cntr
	xorx.w	@r5, r6
	xorx.a	#12345, 0x45678h(r15)

	adda	#0x12345, r7
	adda	r6, r14

	bra	#bar
	bra	#011044H
	bra	r5
	bra	&ede
	bra	@r5
	bra	@r5+
	bra	0x9876(r5)

	calla	r5
	calla	0x1234(r6)
	calla	@r7
	calla	@r8+
	calla	&foo
	calla	bar
	calla	#011004h

	clra	r6

	cmpa	r1, r2
	cmpa	#0xfedcb, r3

	decda	r5
	incda	r5

	mova 	R9,R8
	MOVA 	#12345h,R12
	MOVA	100h(R9),R8
	MOVA	&EDE,R12
	MOVA	@R9,R8
	MOVA	@R9+,R8
	MOVA	R8,100h(R9)
	MOVA	R13,&EDE

	reta
	reti

	suba	r5, r6
	suba	#0xfffff, r6

	tsta	fooz

	popm	#1, r5
	popm.a	#3, r15
	popm.w	#8, r12

	popx	r10
	popx.a	r10
	popx.b	r10
	popx.w	r10

	pushm	#1, r9
	pushm.a	#2, r9
	pushm.w	#3, r9

	pushx	r8
	pushx.a	r8
	pushx.b	&ede
	pushx.w	r8

	rlam	#1, r15
	rlam.a	#2, r15
	rlam.w	#3, r15

	rlax	r6
	rlax.a	r6
	rlax.w	r6

	rlcx	r6
	rlcx.a	r6
	rlcx.w	r6
	
	rram	#1, r6
	rram.a	#4, r6
	rram.w	#2, r6

	rrax	r11
	rrax.a	r11
	rrax.w	r11
	
	rrcm	#4, r5
	rrcm.a	#1, r5
	rrcm.w	#3, r5

	rrcx	r13
	rrcx.a	r13
	rrcx.w	r13

	rrum	#3, r4
	rrum.a	#2, r4
	rrum.w	#1, r4

	rrux	r4
	rrux.a	r7
	rrux.b	r5
	rrux.w	r6

	swpbx	r1
	swpbx.a	ede
	swpbx.w	r12

	sxtx	r2
	sxtx.a	&ede
	sxtx.w	r2

	rpt	#5
	rrax.a	r5
	rpt	r5
	rrax.a	r5

	;; The following are all aliases for similarly named instructions
	;; without the period.  Eg: add.a -> adda
	add.a	r1, r2
	br.a	r1
	call.a	r1
	clr.a	r1
	cmp.a	r1, r2
	decd.a	r1
	incd.a	r1
	mov.a	r1, r2
	ret.a
	sub.a	r1, r2
	tst.a	fooz
	
	;; Check that repeat counts can be used with shift instructions.
	rpt r1 { rrux.w r1
	rpt #2 { rrcx.w r2
	rpt #3 { rrax.b r7
	rpt r4 { rrax.a r4
	rpt #5 { rlax.b r5
	rpt #6 { rlcx.a r6
