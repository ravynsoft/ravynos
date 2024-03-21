	.macro RegReg insn	
	\insn	r31, r0, r0
	\insn	r0, r31, r0
	\insn	r0, r0, r31
	\insn	r1, r2, r3
	.endm

	.macro RegImm insn
	\insn	r0, r0, 0
	\insn	r0, r0, -32768
	\insn	r0, r0, 32767
	\insn	r0, r31, 0
	\insn 	r0, r31, -32768
	\insn	r0, r31, 32767
	\insn	r31, r0, 0
	\insn	r31, r0, -32768
	\insn	r31, r0, 32767
	.endm

	.macro RegUImm insn
	\insn	r0, r0, 0
	\insn	r0, r0, 65535
	\insn	r0, r31, 0
	\insn	r0, r31, 65535
	\insn	r31, r0, 0
	\insn	r31, r0, 65535
	.endm

	RegReg 	add
	RegImm 	addi
	RegReg 	and
	RegUImm andi
	RegUImm	andhi

	andhi	r0, r0, hi(0x0000ffff)
	andhi	r0, r0, hi(0xffff0000)

	bi	label
	b	r0
	b	r31

	be	r0, r0, label
	be	r1, r2, label
	bg	r0, r0, label
	bg	r1, r2, label
	bge	r0, r0, label
	bge	r1, r2, label
	bgeu	r0, r0, label
	bgeu	r1, r2, label
	bgu	r0, r0, label
	bgu	r1, r2, label
	bne	r0, r0, label
	bne	r1, r2, label

	calli	label
	call	r0
	call	r31

	RegReg	cmpe
	RegImm	cmpei
	RegReg	cmpg
	RegImm	cmpgi
	RegReg	cmpge
	RegImm	cmpgei
	RegReg	cmpgeu
	RegUImm	cmpgeui
	RegReg	cmpgu
	RegUImm	cmpgui
	RegReg	cmpne
	RegImm	cmpnei

	RegReg	divu
	RegReg	modu
	RegReg	mul
	RegImm	muli

	RegReg	nor
	RegUImm	nori
	RegReg	or
	RegUImm	ori
	ori	r0, r0, lo(0xffff0000)
	ori	r0, r0, lo(0x0000ffff)
	RegUImm	orhi
	orhi	r0, r0, hi(0x0000ffff)
	orhi	r0, r0, hi(0xffff0000)

	RegReg	sl
	RegReg	sr
	RegReg	sru

	sli	r0, r0, 0
	sli	r0, r0, 31
	sri	r0, r0, 0
	sri	r0, r0, 31
	srui	r0, r0, 0
	srui	r0, r0, 31

	RegReg	sub

	RegReg	xnor
	RegUImm	xnori
	RegReg	xor
	RegUImm	xori

        sextb   r0, r0
        sextb   r31, r0
        sextb   r0, r31
        sextb   r2, r1
        sexth   r0, r0
        sexth   r31, r0
        sexth   r0, r31
        sexth   r2, r1

	lb	r0, (r0+0)
	lb	r1, (r2+3)
	lbu	r0, (r0+0)
 	lbu 	r1, (r2+3)	
	lh	r0, (r0+0)
	lh	r1, (r2+3)
	lhu	r0, (r0+0)
	lhu 	r1, (r2+3)
	lw	r0, (r0+0)
	lw	r1, (r2+3)
	sb	(r0+0), r0
	sb	(r1+2), r3
	sh	(r0+0), r0
	sh	(r1+2), r3
	sw	(r0+0), r0
	sw	(r1+2), r3

	break
        scall

	eret
 	bret

	ret
 	mvi	r0, 0   
	mv	r0, r0
	mvhi   r0, 0
	mvhi   r0, hi(0)
	not	r0, r0
