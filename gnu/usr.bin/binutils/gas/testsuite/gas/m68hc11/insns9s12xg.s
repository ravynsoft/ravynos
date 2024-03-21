# XGATE instruction set and all modes

	.sect .text

val1 = 0x1234
val2 = 0x5432
u08_1 = 0x32
u08_2 = 0xa5

label1:
	adc	r1,r2,r3
label2:
	add	r4,r6,r1
label3:
	add	r7,#val1		;splits out to addh, addl
	addl r4,#u08_2
	addh r4,#u08_1
	and	r7,r6,r5
	and	r2,#val2
	andl	r1, #u08_2
	andh	r1, #u08_1
	asr	r2,#3
	asr	r3,r4
	bcc	label1
	bcs	label2
	beq	label3
	bfext	r1,r2,r3
	bffo	r4,r5
	bfins	r6,r7,r1
	bfinsi	r2,r4,r6
	bfinsx	r3,r5,r7
	bge	label1
	bgt	label2
	bhi	label3
	bhs	label1
	bith	r2,#0x55
	bitl	r3,#0xaa
	ble	label2
	blo	label3
	bls	label1
	blt	label2
	bmi	label3
	bne	label1
	bpl	label2
	bra	label3
	brk
	bvc	label1
	bvs	label2
	cmp	r1,r2
	cmp	r3,#val1
	cmpl	r4,#u08_1
	com	r4,r5
	com	r6
	cpc	r7,r5
	cpch	r6,#u08_2
	csem	#2
	csem	r1
	csl	r2,#1
	csl	r3,r4
	csr	r5,#4
	csr	r6,r7
	jal	r1
	ldb	r2,(r3,#4)
	ldb	r3,(r0,r2)
	ldb	r4,(r5,r6+)
	ldb	r5,(r6,-r7)
	ldh	r6,#0x35
	ldl	r7,#0x46
	ldw	r1,(r2,#29)
	ldw	r2,(r3,r0)
	ldw	r3,(r4,r5+)
	ldw	r4,(r5,-r6)
	ldw	r6,#0x1234
	lsl	r7,#2
	lsl	r2,r1
	lsr	r5,#3
	lsl	r6,r3
	mov	r7,r6
	neg	r2,r3
	neg	r4
	nop
	or	r2,r3,r4
	or	r5,#0x1256
	orh	r6,#0x08
	orl	r4,#0xf0
	par	r1	; comma on datasheet a typo?
	rol	r2,#5
	rol	r3,r4
	ror	r3,#6
	ror	r5,r4
	rts
	sbc	r7,r1,r2
	sex	r1
	sif
	sif	r2
	ssem	#5
	ssem	r3
	stb	r2,(r4,#15)
	stb	r3,(r5,r6)
	stb	r0,(r7,r1+)
	stb	r1,(r2,-r3)
	stw	r7,(r6,#30)
	stw	r6,(r5,r0)
	stw	r5,(r4,r3+)
	stw	r4,(r3,-r2)
	sub	r7,r6,r5
	sub	r4,#val1
	subh	r5,#0x44
	subl	r4,#0x55
	tfr	r2,ccr
	tfr	ccr,r3
	tfr	r5,pc
	tst	r2
	xnor	r4,r6,r2
	xnor	r3,#val2
	xnorh	r2,#0x32
	xnorl	r1,#0x54
	
