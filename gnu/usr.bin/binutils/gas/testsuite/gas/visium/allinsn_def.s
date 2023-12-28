begin:
	write.l	(r2),r1
	write.l	0(r2),r1
	write.w	1(r1),r2
	write.b	31(r3),r7
	write.b	(r4),r7

	eamwrite 0,r4,r5
	eamwrite 31,r7,r10

	writemd	r14,r15

	writemdc r9

	divs	r5
	divu	r6
	divds	r10
	divdu	r11

	asrd	r12
	lsrd	r13
	asld	r14

	dsi

	mults	r7,r8
	multu	r9,r10

	eni
	dsi
	rfi


nsrel:
	brr	fa,nsrel
	rflag	r0
	brr	eq,nsrel
	rflag	r0
	brr	cs,nsrel
	rflag	r0
	brr	os,nsrel
	rflag	r0
	brr	ns,sreg
	rflag	r0
	brr	ne,sreg
	rflag	r0
	brr	cc,sreg
	rflag	r0
	brr	oc,sreg
	rflag	r0
	brr	nc,sreg
	rflag	r0
	brr	ge,sreg
	rflag	r0
	brr	gt,sreg
	rflag	r0
	brr	hi,sreg
	rflag	r0
	brr	le,sreg
	rflag	r0
	brr	ls,sreg
	rflag	r0
	brr	lt,sreg
	rflag	r0
	brr	tr,sreg
	rflag	r0
	brr	eq,nsrel
	nop
	brr	fa,.
	nop


sreg:
	adc.l	r0,r0,r1
	adc.w	r2,r0,r3
	adc.b	r4,r0,r5

	add.l	r2,r0,r1
	add.w	r5,r4,r3
	add.b	r7,r7,r6

	and.l	r2,r0,r1
	and.w	r5,r4,r3
	and.b	r7,r7,r6

	asl.l	r4,r3,r4
	asl.w	r6,r5,0
	asl.w	r6,r5,1
	asl.b	r8,r7,31

	asr.l	r4,r3,r4
	asr.w	r6,r5,0
	asr.w	r6,r5,1
	asr.b	r8,r7,31

	bra	eq,r9,r10
	rflag	r0
	bra	ne,r7,r1

	eamread	r11,0
	eamread	r12,31

	extb.l	r12,r13
	extb.w	r14,r15
	extb.b	r0,r1

	extw.l	r2,r3
	extw.w	r4,r5

	lsr.l	r6,r7,r8
	lsr.w	r9,r10,0
	lsr.w	r9,r10,1
	lsr.b	r9,r10,31

	not.l	r11,r12
	not.w	r13,r14
	not.b 	r15,r10

	or.l	r5,r6,r7
	or.w	r8,r9,r10
	or.b	r1,r2,r3

	read.l	r4,(r5)
	read.l	r4,0(r5)
	read.w	r6,1(r7)
	read.b	r8,31(r9)
	read.b	r6,1(r9)

	readmda	r10
	readmdb	r11
	readmdc	r17

	rflag	r4
	rflag	r7

	sub.l	r4,r5,r6
	sub.w	r7,r8,r9
	sub.b	r0,r1,r2

	subc.l	r4,r5,r6
	subc.w	r7,r8,r9
	subc.b	r0,r1,r2

	xor.l	r4,r3,r2
	xor.w	r5,r6,r7
	xor.b	r1,r9,r8

	addi	r7,65535
	movil	r7,32768
	moviu	r7,32767
	moviq	r6,1
	subi	r7,65535

	bra	tr,r6,r0
	add.l	r0,r0,r0

	.end
