	.text
	.align	3
	.set	micromips

	.ifdef	compact
	.macro	DSNOP
	.endm
	.else
	.macro	DSNOP
	nop
	.endm
	.endif

	.ent	test
	.globl	test
test:
	pref	0, 0
	pref	0, 2047
	pref	0, -2048
	pref	0, 2048
	pref	0, -2049
	pref	0, ($0)
	pref	0, 0($0)
	pref	1, 0($0)
	pref	2, 0($0)
	pref	3, 0($0)
	pref	4, 0($0)
	pref	5, 0($0)
	pref	6, 0($0)
	pref	7, 0($0)
	pref	7, 511($0)
	pref	7, -512($0)
	pref	31, 2047($0)
	pref	31, -2048($0)
	pref	31, 2048($0)
	pref	31, -2049($0)
	pref	3, 32767($0)
	pref	3, -32768($0)

	pref	31, 2047($2)
	pref	31, -2048($2)
	pref	31, 2048($2)
	pref	31, -2049($2)
	pref	3, 32767($2)
	pref	3, -32768($2)

	nop
	.ifndef	insn32
	nop16
	.endif
	nop32
	ssnop
	ehb
	pause

	li	$2, -1
	li	$3, -1
	li	$4, -1
	li	$5, -1
	li	$6, -1
	li	$7, -1
	li	$16, -1
	li	$17, -1
	li	$17, 0
	li	$17, 125
	li	$17, 126
	li	$17, 127

	li32	$2, 0
	li32	$2, 1
	li	$2, 32767
	li	$2, -32768
	li	$2, 65535

	li	$2, 65536
	li	$2, 0xffff8000
	li	$2, 0xffff8001
	li	$2, 0xffffffff
	li	$2, 0x12345678

	move	$0, $22
	move	$2, $22
	move	$3, $22
	move	$4, $22
	move	$5, $22
	move	$6, $22
	move	$7, $22
	move	$8, $22
	move	$9, $22
	move	$10, $22
	move	$30, $22
	move	$31, $22
	move	$0, $0
	move	$0, $2
	move	$0, $3
	move	$0, $4
	move	$0, $5
	move	$0, $6
	move	$0, $7
	move	$0, $8
	move	$0, $9
	move	$0, $10
	move	$0, $30
	move	$0, $31

	move	$22, $2
	.ifndef	insn32
	move16	$2, $22
	move16	$22, $2
	.endif
	move32	$2, $22
	move32	$22, $2

	b	test
	.ifndef	insn32
	b16	test
	.endif
	DSNOP
	b32	test
	DSNOP
	b	1f
	.ifndef	insn32
	b16	1f
	.endif
	DSNOP
	b32	1f
1:
	DSNOP
	b	1b
	.ifndef	insn32
	b16	1b
	.endif
	DSNOP
	b32	1b

	abs	$2, $3
	abs	$2, $4
	abs	$2, $2
	abs	$2

	add	$2, $3, $4
	add	$29, $30, $31
	add	$2, $2, $4
	add	$2, $4
	add	$2, $2, 0
	add	$2, $2, 1
	add	$2, $2, 32767
	add	$2, $2, -32768
	add	$2, $2, 65535

	addi	$3, $4, -32768
	addi	$3, $4, 0
	addi	$3, $4, 32767
	addi	$3, $4, 65535
	addi	$3, $3, 65535
	addi	$3, 65535

	addiu	$0, -8
	addiu	$2, -8
	addiu	$3, -8
	addiu	$4, -8
	addiu	$5, -8
	addiu	$6, -8
	addiu	$7, -8
	addiu	$8, -8
	addiu	$9, -8
	addiu	$10, -8
	addiu	$30, -8
	addiu	$31, -8
	addiu	$31, -7
	addiu	$31, 0
	addiu	$31, 1
	addiu	$31, 6
	addiu	$31, 7
	addiu	$31, 8
	addiu	$29, -258 << 2
	addiu	$29, -257 << 2
	addiu	$29, -256 << 2
	addiu	$29, 255 << 2
	addiu	$29, 256 << 2
	addiu	$29, 257 << 2
	addiu	$29, $29, 257 << 2
	addiu	$29, $29, 258 << 2

	addiu	$2, $2, -1
	addiu	$2, $3, -1
	addiu	$2, $4, -1
	addiu	$2, $5, -1
	addiu	$2, $6, -1
	addiu	$2, $7, -1
	addiu	$2, $16, -1
	addiu	$2, $17, -1
	addiu	$2, $17, 1
	addiu	$2, $17, 4
	addiu	$2, $17, 8
	addiu	$2, $17, 12
	addiu	$2, $17, 16
	addiu	$2, $17, 20
	addiu	$2, $17, 24
	addiu	$3, $17, 24
	addiu	$4, $17, 24
	addiu	$5, $17, 24
	addiu	$6, $17, 24
	addiu	$7, $17, 24
	addiu	$16, $17, 24
	addiu	$17, $17, 24

	addiu	$2, $29, 0 << 2
	addiu	$2, $29, 1 << 2
	addiu	$2, $29, 62 << 2
	addiu	$2, $29, 63 << 2
	addiu	$2, $29, 64 << 2
	addiu	$2, $29, 63 << 2
	addiu	$3, $29, 63 << 2
	addiu	$4, $29, 63 << 2
	addiu	$5, $29, 63 << 2
	addiu	$6, $29, 63 << 2
	addiu	$7, $29, 63 << 2
	addiu	$16, $29, 63 << 2
	addiu	$17, $29, 63 << 2

	addiu	$3, $4, -32768
	addiu	$3, $4, 0
	addiu	$3, $4, 32767
	addiu	$3, $4, 65535
	addiu	$3, $3, 65535
	addiu	$3, 65535

	addu	$2, $22, $0
	addu	$22, $2, $0
	addu	$2, $0, $22
	addu	$22, $0, $2

	addu	$2, $3, $2
	addu	$2, $3, $3
	addu	$2, $3, $4
	addu	$2, $3, $5
	addu	$2, $3, $6
	addu	$2, $3, $7
	addu	$2, $3, $16
	addu	$2, $3, $17

	addu	$2, $2, $17
	addu	$2, $3, $17
	addu	$2, $4, $17
	addu	$2, $5, $17
	addu	$2, $6, $17
	addu	$2, $7, $17
	addu	$2, $16, $17
	addu	$2, $17, $17

	addu	$2, $2, $17
	addu	$3, $2, $17
	addu	$4, $2, $17
	addu	$5, $2, $17
	addu	$6, $2, $17
	addu	$7, $2, $17
	addu	$16, $2, $17
	addu	$17, $2, $17

	addu	$7, $7, $2
	addu	$7, $2
	addu	$7, $2, $7

	addu	$29, $30, $31
	addu	$2, $2, 0
	addu	$2, $2, 1
	addu	$2, $2, 32767
	addu	$2, $2, -32768
	addu	$2, $2, 65535

	and	$2, $2
	and	$2, $3
	and	$2, $4
	and	$2, $5
	and	$2, $6
	and	$2, $7
	and	$2, $16
	and	$2, $17
	and	$3, $2
	and	$4, $2
	and	$5, $2
	and	$6, $2
	and	$7, $2
	and	$16, $2
	and	$17, $2

	and	$2, $3
	and	$2, $2, $3
	and	$2, $3, $2
	.ifndef	insn32
	and16	$2, $2, $3
	.endif
	and32	$2, $2, $3

	andi	$2,$2,1
	andi	$2,$2,2
	andi	$2,$2,3
	andi	$2,$2,4
	andi	$2,$2,7
	andi	$2,$2,8
	andi	$2,$2,15
	andi	$2,$2,16
	andi	$2,$2,31
	andi	$2,$2,32
	andi	$2,$2,63
	andi	$2,$2,64
	andi	$2,$2,128
	andi	$2,$2,255
	andi	$2,$2,32768
	andi	$2,$2,65535
	andi	$2,$3,65535
	andi	$2,$4,65535
	andi	$2,$5,65535
	andi	$2,$6,65535
	andi	$2,$7,65535
	andi	$2,$16,65535
	andi	$2,$17,65535
	andi	$3,$17,65535
	andi	$4,$17,65535
	andi	$5,$17,65535
	andi	$6,$17,65535
	andi	$7,$17,65535
	andi	$16,$17,65535
	andi	$17,$17,65535

	andi	$7,$7,65535
	andi	$7,65535
	.ifndef	insn32
	andi16	$7,65535
	.endif
	andi32	$7,65535

	and32	$2, $3, $4
	and32	$2, $2, $4
	and32	$2, $4
	and	$2, $3, 0
	and	$2, $3, 65535
	and	$2, $3, 65536
	and	$2, $3, 0xffff0001

	bc2f	test
	bc2f	$cc0, test
	bc2f	$cc1, test
	bc2f	$cc2, test
	bc2f	$cc3, test
	bc2f	$cc4, test
	bc2f	$cc5, test
	bc2f	$cc6, test
	bc2f	$cc7, test

	bc2t	test
	bc2t	$cc0, test
	bc2t	$cc1, test
	bc2t	$cc2, test
	bc2t	$cc3, test
	bc2t	$cc4, test
	bc2t	$cc5, test
	bc2t	$cc6, test
	bc2t	$cc7, test

	.set	noreorder
	bc2fl	$cc1, test
	addu	$3, $4, $5
	bc2tl	$cc2, test
	addu	$6, $7, $8
	.set	reorder

	bc2fl	$cc3, test
	addu	$3, $4, $5
	bc2tl	$cc4, test
	addu	$6, $7, $8


test2:
	DSNOP
	beqz	$2, test2
	DSNOP
	beqz	$3, test2
	DSNOP
	beqz	$4, test2
	DSNOP
	beqz	$5, test2
	DSNOP
	beqz	$6, test2
	DSNOP
	beqz	$7, test2
	DSNOP
	beqz	$16, test2
	DSNOP
	beqz	$17, test2
	DSNOP
	beq	$2, $0, test2
	DSNOP
	beq	$3, $0, test2
	DSNOP
	beq	$4, $0, test2
	DSNOP
	beq	$5, $0, test2
	DSNOP
	beq	$6, $0, test2
	DSNOP
	beq	$7, $0, test2
	DSNOP
	beq	$16, $0, test2
	DSNOP
	beq	$17, $0, test2
	DSNOP
	beq	$0, $2, test2
	DSNOP
	beq	$0, $3, test2
	DSNOP
	beq	$0, $4, test2
	DSNOP
	beq	$0, $5, test2
	DSNOP
	beq	$0, $6, test2
	DSNOP
	beq	$0, $7, test2
	DSNOP
	beq	$0, $16, test2
	DSNOP
	beq	$0, $17, test2

	.ifndef	insn32
	beqz16	$16, test2
	.endif
	DSNOP
	beqz32	$16, test2
	DSNOP
	beqz	$17, test2
	DSNOP
	beqz32	$17, test2

	beqzc	$17, test2

	DSNOP
	beq	$16, 0, test2
	beq	$16, 10, test2
	beq	$16, 32767, test2
	beq	$16, 65536, test2

	.set	noreorder
	beql	$16, $17, test2
	addu	$3, $4, $5
	beql	$16, $17, 1f
	addu	$3, $4, $5
	beql	$16, 0, test2
	addu	$3, $4, $5
	beql	$16, 0, 1f
	addu	$3, $4, $5
	beql	$16, 10, test2
	addu	$3, $4, $5
	beql	$16, 10, 1f
	addu	$3, $4, $5
	beql	$16, 32767, test2
	addu	$3, $4, $5
	beql	$16, 32767, 1f
	addu	$3, $4, $5
	beql	$16, 65535, test2
	addu	$3, $4, $5
	beql	$16, 65535, 1f
	addu	$3, $4, $5

	beql	$16, $17, test2
	addu	$3, $4, $29
	beql	$16, $17, 1f
	addu	$3, $4, $29
	beql	$16, 0, test2
	addu	$3, $4, $29
	beql	$16, 0, 1f
	addu	$3, $4, $29
	beql	$16, 10, test2
	addu	$3, $4, $29
	beql	$16, 10, 1f
	addu	$3, $4, $29
	beql	$16, 32767, test2
	addu	$3, $4, $29
	beql	$16, 32767, 1f
	addu	$3, $4, $29
	beql	$16, 65535, test2
	addu	$3, $4, $29
	beql	$16, 65535, 1f
	addu	$3, $4, $29
1:
	.set	reorder

	beql	$16, $17, test2

	beqzl	$17, test2

	DSNOP
	DSNOP
	bnez	$2, test3
	DSNOP
	bnez	$3, test3
	DSNOP
	bnez	$4, test3
	DSNOP
	bnez	$5, test3
	DSNOP
	bnez	$6, test3
	DSNOP
	bnez	$7, test3
	DSNOP
	bnez	$16, test3
	DSNOP
	bnez	$17, test3
	DSNOP
	bne	$2, $0, test3
	DSNOP
	bne	$3, $0, test3
	DSNOP
	bne	$4, $0, test3
	DSNOP
	bne	$5, $0, test3
	DSNOP
	bne	$6, $0, test3
	DSNOP
	bne	$7, $0, test3
	DSNOP
	bne	$16, $0, test3
	DSNOP
	bne	$17, $0, test3
	DSNOP
	bne	$0, $2, test3
	DSNOP
	bne	$0, $3, test3
	DSNOP
	bne	$0, $4, test3
	DSNOP
	bne	$0, $5, test3
	DSNOP
	bne	$0, $6, test3
	DSNOP
	bne	$0, $7, test3
	DSNOP
	bne	$0, $16, test3
	DSNOP
	bne	$0, $17, test3

	.ifndef	insn32
	bnez16	$16, test3
	.endif
	DSNOP
	bnez32	$16, test3
	DSNOP
	bnez	$17, test2
	DSNOP
	bnez32	$17, test2
test3:
	bnezc	$17, test2

	break
	break	0
	break	1
	break	2
	break	3
	break	4
	break	5
	break	6
	break	7
	break	8
	break	9
	break	10
	break	11
	break	12
	break	13
	break	14
	break	15
	break	63
	break	64
	break	1023
	break	1023,1023

	break32
	break32	0
	break32	1
	break32	2
	break32	15
	break32	63
	break32	64
	break32	1023
	break32	1023,1023

	cache	0, 0
	cache	0, -2048
	cache	0, 2047
	cache	0, -2049
	cache	0, 2048
	cache	0, 0($2)
	cache	0, -2048($2)
	cache	0, 2047($2)
	cache	0, -2049($2)
	cache	0, 2048($2)

	cache	0, ($0)
	cache	0, 0($0)
	cache	1, 0($0)
	cache	2, 0($0)
	cache	3, 0($0)
	cache	4, 0($0)
	cache	5, 0($0)
	cache	6, 0($0)
	cache	31, 0($0)
	cache	31, 2047($0)
	cache	31, -2048($0)
	cache	0, 2047($0)
	cache	0, -2048($0)

	cache	31, 65536($3)
	cache	31, 2048($3)
	cache	31, -2049($3)
	cache	31, 65537($3)
	cache	31, 0xffffffff($3)
	cache	31, 0xffff0000($3)
	cache	31, 0xffff0001($3)
	cache	31, 0xffff($3)

	cache	31, 65536($0)
	cache	31, 2048($0)
	cache	31, -2049($0)
	cache	31, 65537($0)
	cache	31, 0xffffffff($0)
	cache	31, 0xffff0000($0)
	cache	31, 0xffff0001($0)
	cache	31, 0xffff($0)


	clo	$2, $3
	clo	$3, $2
	clz	$2, $3
	clz	$3, $2

	deret

	di
	di	$0
	di	$2
	di	$3
	di	$30
	di	$31

	div	$0, $2, $3
	div	$0, $30, $31
	div	$0, $3
	div	$0, $31

	div	$2, $3, $0
	div	$2, $3, $4

	div	$3, $4, 0
	div	$3, $4, 1
	div	$3, $4, -1
	div	$3, $4, 2

	divu	$0, $2, $3
	divu	$0, $30, $31
	divu	$0, $3
	divu	$0, $31

	divu	$2, $3, $0
	divu	$2, $3, $4

	divu	$3, $4, 0
	divu	$3, $4, 1
	divu	$3, $4, -1
	divu	$3, $4, 2

	ei
	ei	$0
	ei	$2
	ei	$3
	ei	$30
	ei	$31

	eret

	ext	$2, $3, 5, 15
	ext	$2, $3, 0, 32
	ext	$2, $3, 31, 1
	ext	$31, $30, 31, 1

	ins	$2, $3, 5, 15
	ins	$2, $3, 0, 32
	ins	$2, $3, 31, 1
	ins	$31, $30, 31, 1

	jr	$0
	DSNOP
	jr	$2
	DSNOP
	jr	$3
	DSNOP
	jr	$4
	DSNOP
	jr	$5
	DSNOP
	jr	$6
	DSNOP
	jr	$7
	DSNOP
	jr	$8
	DSNOP
	jr	$30
	DSNOP
	jr	$31

	jr32	$0
	jr32	$2
	jr32	$3
	jr32	$4
	jr32	$5
	jr32	$6
	jr32	$7
	jr32	$8
	jr32	$30
	jr32	$31

	jrc	$0
	jrc	$2
	jrc	$3
	jrc	$4
	jrc	$5
	jrc	$6
	jrc	$7
	jrc	$8
	jrc	$30
	jrc	$31

	jr.hb	$0
	jr.hb	$2
	jr.hb	$3
	jr.hb	$4
	jr.hb	$5
	jr.hb	$6
	jr.hb	$7
	jr.hb	$8
	jr.hb	$30
	jr.hb	$31

	DSNOP
	j	$0
	DSNOP
	j	$2
	DSNOP
	j	$3
	DSNOP
	j	$4
	DSNOP
	j	$5
	DSNOP
	j	$6
	DSNOP
	j	$7
	DSNOP
	j	$8
	DSNOP
	j	$30
	DSNOP
	j	$31

	jalr	$31, $0
	jalr	$2
	jalr	$3
	jalr	$4
	jalr	$5
	jalr	$6
	jalr	$7
	jalr	$8
	jalr	$30

	jalr32	$31, $0
	jalr32	$2
	jalr32	$3
	jalr32	$4
	jalr32	$5
	jalr32	$6
	jalr32	$7
	jalr32	$8
	jalr32	$30

	jalr	$31, $0
	jalr	$31, $2
	jalr	$31, $3
	jalr	$31, $4
	jalr	$31, $5
	jalr	$31, $6
	jalr	$31, $7
	jalr	$31, $8
	jalr	$31, $30
	jalr	$30, $31

	jalr	$2, $0
	jalr	$3, $2
	jalr	$2, $3
	jalr	$2, $4
	jalr	$2, $5
	jalr	$2, $6
	jalr	$2, $7
	jalr	$2, $8
	jalr	$2, $30
	jalr	$2, $31

	jalr.hb	$31, $0
	jalr.hb	$2
	jalr.hb	$3
	jalr.hb	$4
	jalr.hb	$5
	jalr.hb	$6
	jalr.hb	$7
	jalr.hb	$8
	jalr.hb	$30
	#jalr.hb	$31

	jalr.hb	$31, $0
	jalr.hb	$31, $2
	jalr.hb	$31, $3
	jalr.hb	$31, $4
	jalr.hb	$31, $5
	jalr.hb	$31, $6
	jalr.hb	$31, $7
	jalr.hb	$31, $8
	jalr.hb	$31, $30
	jalr.hb	$30, $31

	jalr.hb	$2, $0
	jalr.hb	$3, $2
	jalr.hb	$2, $3
	jalr.hb	$2, $4
	jalr.hb	$2, $5
	jalr.hb	$2, $6
	jalr.hb	$2, $7
	jalr.hb	$2, $8
	jalr.hb	$2, $30
	jalr.hb	$2, $31

	jal	$2, $3
	jal	$30, $31

	jal	$3
	jal	$31

	jal	test
	jal	test2

	jalx	test
	jalx	test4

	la	$2, test
	lca	$2, test

	lb	$3, 0
	lb	$3, 4
	lb	$3, 0($0)
	lb	$3, 4($0)
	lb	$3, 32767($0)
	lb	$3, -32768($0)
	lb	$3, 65535($0)
	lb	$3, 0xffff0000($0)
	lb	$3, 0xffff8000($0)
	lb	$3, 0xffff0001($0)
	lb	$3, 0xffff8001($0)
	lb	$3, 0xf0000000($0)
	lb	$3, 0xffffffff($0)
	lb	$3, 0x12345678($0)
	lb	$3, ($4)
	lb	$3, 0($4)
	lb	$3, 4($4)
	lb	$3, 32767($4)
	lb	$3, -32768($4)
	lb	$3, 65535($4)
	lb	$3, 0xffff0000($4)
	lb	$3, 0xffff8000($4)
	lb	$3, 0xffff0001($4)
	lb	$3, 0xffff8001($4)
	lb	$3, 0xf0000000($4)
	lb	$3, 0xffffffff($4)
	lb	$3, 0x12345678($4)

	lbu	$2, -1($3)
	lbu	$2, 0($3)
	lbu	$2, ($3)
	lbu	$2, 1($3)
	lbu	$2, 2($3)
	lbu	$2, 3($3)
	lbu	$2, 4($3)
	lbu	$2, 5($3)
	lbu	$2, 6($3)
	lbu	$2, 7($3)
	lbu	$2, 8($3)
	lbu	$2, 9($3)
	lbu	$2, 10($3)
	lbu	$2, 11($3)
	lbu	$2, 12($3)
	lbu	$2, 13($3)
	lbu	$2, 14($3)
	lbu	$2, 14($2)
	lbu	$2, 14($4)
	lbu	$2, 14($5)
	lbu	$2, 14($6)
	lbu	$2, 14($7)
	lbu	$2, 14($16)
	lbu	$2, 14($17)
	lbu	$3, 14($17)
	lbu	$4, 14($17)
	lbu	$5, 14($17)
	lbu	$6, 14($17)
	lbu	$7, 14($17)
	lbu	$16, 14($17)
	lbu	$17, 14($17)

	lbu	$3, 0
	lbu	$3, 4
	lbu	$3, 0($0)
	lbu	$3, 4($0)
	lbu	$3, 32767($0)
	lbu	$3, -32768($0)
	lbu	$3, 65535($0)
	lbu	$3, 0xffff0000($0)
	lbu	$3, 0xffff8000($0)
	lbu	$3, 0xffff0001($0)
	lbu	$3, 0xffff8001($0)
	lbu	$3, 0xf0000000($0)
	lbu	$3, 0xffffffff($0)
	lbu	$3, 0x12345678($0)

	lbu	$3, ($4)
	lbu	$3, 0($4)
	lbu	$3, 4($4)
	lbu	$3, 32767($4)
	lbu	$3, -32768($4)
	lbu	$3, 65535($4)
	lbu	$3, 0xffff0000($4)
	lbu	$3, 0xffff8000($4)
	lbu	$3, 0xffff0001($4)
	lbu	$3, 0xffff8001($4)
	lbu	$3, 0xf0000000($4)
	lbu	$3, 0xffffffff($4)
	lbu	$3, 0x12345678($4)

	lh	$3, 0
	lh	$3, 4
	lh	$3, 0($0)
	lh	$3, 4($0)
	lh	$3, 32767($0)
	lh	$3, -32768($0)
	lh	$3, 65535($0)
	lh	$3, 0xffff0000($0)
	lh	$3, 0xffff8000($0)
	lh	$3, 0xffff0001($0)
	lh	$3, 0xffff8001($0)
	lh	$3, 0xf0000000($0)
	lh	$3, 0xffffffff($0)
	lh	$3, 0x12345678($0)
	lh	$3, ($4)
	lh	$3, 0($4)
	lh	$3, 4($4)
	lh	$3, 32767($4)
	lh	$3, -32768($4)
	lh	$3, 65535($4)
	lh	$3, 0xffff0000($4)
	lh	$3, 0xffff8000($4)
	lh	$3, 0xffff0001($4)
	lh	$3, 0xffff8001($4)
	lh	$3, 0xf0000000($4)
	lh	$3, 0xffffffff($4)
	lh	$3, 0x12345678($4)

	lhu	$2, ($3)
	lhu	$2, 0<<1($3)
	lhu	$2, 1<<1($3)
	lhu	$2, 2<<1($3)
	lhu	$2, 3<<1($3)
	lhu	$2, 4<<1($3)
	lhu	$2, 5<<1($3)
	lhu	$2, 6<<1($3)
	lhu	$2, 7<<1($3)
	lhu	$2, 8<<1($3)
	lhu	$2, 9<<1($3)
	lhu	$2, 10<<1($3)
	lhu	$2, 11<<1($3)
	lhu	$2, 12<<1($3)
	lhu	$2, 13<<1($3)
	lhu	$2, 14<<1($3)
	lhu	$2, 15<<1($3)
	lhu	$2, 15<<1($4)
	lhu	$2, 15<<1($5)
	lhu	$2, 15<<1($6)
	lhu	$2, 15<<1($7)
	lhu	$2, 15<<1($2)
	lhu	$2, 15<<1($16)
	lhu	$2, 15<<1($17)
	lhu	$3, 15<<1($17)
	lhu	$4, 15<<1($17)
	lhu	$5, 15<<1($17)
	lhu	$6, 15<<1($17)
	lhu	$7, 15<<1($17)
	lhu	$16, 15<<1($17)
	lhu	$17, 15<<1($17)

	lhu	$3, 0
	lhu	$3, 4
	lhu	$3, 0($0)
	lhu	$3, 4($0)
	lhu	$3, 32767($0)
	lhu	$3, -32768($0)
	lhu	$3, 65535($0)
	lhu	$3, 0xffff0000($0)
	lhu	$3, 0xffff8000($0)
	lhu	$3, 0xffff0001($0)
	lhu	$3, 0xffff8001($0)
	lhu	$3, 0xf0000000($0)
	lhu	$3, 0xffffffff($0)
	lhu	$3, 0x12345678($0)
	lhu	$3, ($4)
	lhu	$3, 0($4)
	lhu	$3, 4($4)
	lhu	$3, 32767($4)
	lhu	$3, -32768($4)
	lhu	$3, 65535($4)
	lhu	$3, 0xffff0000($4)
	lhu	$3, 0xffff8000($4)
	lhu	$3, 0xffff0001($4)
	lhu	$3, 0xffff8001($4)
	lhu	$3, 0xf0000000($4)
	lhu	$3, 0xffffffff($4)
	lhu	$3, 0x12345678($4)

	ll	$3, 0
	ll	$3, 0($0)
	ll	$3, 4
	ll	$3, 4($0)
	ll	$3, 32767($0)
	ll	$3, -32768($0)
	ll	$3, 65535($0)
	ll	$3, 0xffff0000($0)
	ll	$3, 0xffff8000($0)
	ll	$3, 0xffff0001($0)
	ll	$3, 0xffff8001($0)
	ll	$3, 0xf0000000($0)
	ll	$3, 0xffffffff($0)
	ll	$3, 0x12345678($0)
	ll	$3, ($4)
	ll	$3, 0($4)
	ll	$3, 4($4)
	ll	$3, 32767($4)
	ll	$3, -32768($4)
	ll	$3, 65535($4)
	ll	$3, 0xffff0000($4)
	ll	$3, 0xffff8000($4)
	ll	$3, 0xffff0001($4)
	ll	$3, 0xffff8001($4)
	ll	$3, 0xf0000000($4)
	ll	$3, 0xffffffff($4)
	ll	$3, 0x12345678($4)

	lui	$3, 0
	lui	$3, 32767
	lui	$3, 65535

	lw	$2, ($4)
	lw	$2, 0($4)
	lw	$2, 1<<2($4)
	lw	$2, 2<<2($4)
	lw	$2, 3<<2($4)
	lw	$2, 4<<2($4)
	lw	$2, 5<<2($4)
	lw	$2, 6<<2($4)
	lw	$2, 7<<2($4)
	lw	$2, 8<<2($4)
	lw	$2, 9<<2($4)
	lw	$2, 10<<2($4)
	lw	$2, 11<<2($4)
	lw	$2, 12<<2($4)
	lw	$2, 13<<2($4)
	lw	$2, 14<<2($4)
	lw	$2, 15<<2($4)
	lw	$2, 15<<2($5)
	lw	$2, 15<<2($6)
	lw	$2, 15<<2($7)
	lw	$2, 15<<2($2)
	lw	$2, 15<<2($3)
	lw	$2, 15<<2($16)
	lw	$2, 15<<2($17)
	lw	$3, 15<<2($17)
	lw	$4, 15<<2($17)
	lw	$5, 15<<2($17)
	lw	$6, 15<<2($17)
	lw	$7, 15<<2($17)
	lw	$16, 15<<2($17)
	lw	$17, 15<<2($17)

	lw	$4, ($29)
	lw	$4, 0($29)
	lw	$4, 1<<2($29)
	lw	$4, 2<<2($29)
	lw	$4, 3<<2($29)
	lw	$4, 4<<2($29)
	lw	$4, 5<<2($29)
	lw	$4, 31<<2($29)
	lw	$2, 31<<2($29)
	lw	$2, 31<<2($29)
	lw	$3, 31<<2($29)
	lw	$4, 31<<2($29)
	lw	$5, 31<<2($29)
	lw	$6, 31<<2($29)
	lw	$7, 31<<2($29)
	lw	$8, 31<<2($29)
	lw	$9, 31<<2($29)
	lw	$10, 31<<2($29)
	lw	$30, 31<<2($29)
	lw	$31, 31<<2($29)

	lw	$4, 126<<2($29)
	lw	$4, 127<<2($29)
	lw	$16, 127<<2($29)
	lw	$17, 127<<2($29)
	lw	$18, 127<<2($29)
	lw	$19, 127<<2($29)
	lw	$20, 127<<2($29)
	lw	$21, 127<<2($29)
	lw	$31, 127<<2($29)

	lw	$3, 0
	lw	$3, 4
	lw	$3, ($0)
	lw	$3, 0($0)
	lw	$3, 0($0)
	lw	$3, 4($0)
	lw	$3, 32767($0)
	lw	$3, -32768($0)
	lw	$3, 65535($0)
	lw	$3, 0xffff0000($0)
	lw	$3, 0xffff8000($0)
	lw	$3, 0xffff0001($0)
	lw	$3, 0xffff8001($0)
	lw	$3, 0xf0000000($0)
	lw	$3, 0xffffffff($0)
	lw	$3, 0x12345678($0)
	lw	$3, ($4)
	lw	$3, 0($4)
	lw	$3, 4($4)
	lw	$3, 32767($4)
	lw	$3, -32768($4)
	lw	$3, 65535($4)
	lw	$3, 0xffff0000($4)
	lw	$3, 0xffff8000($4)
	lw	$3, 0xffff0001($4)
	lw	$3, 0xffff8001($4)
	lw	$3, 0xf0000000($4)
	lw	$3, 0xffffffff($4)
	lw	$3, 0x12345678($4)

	lwm	$s0, $ra, 12<<2($29)
	lwm	$s0, $s1, $ra, 12<<2($29)
	lwm	$s0-$s1, $ra, 12<<2($29)
	lwm	$s0, $s1, $s2, $ra, 12<<2($29)
	lwm	$s0-$s2, $ra, 12<<2($29)
	lwm	$s0, $s1, $s2, $s3, $ra, 12<<2($29)
	lwm	$s0-$s3, $ra, 12<<2($29)
	lwm	$s0, $ra, ($29)
	lwm	$s0, $ra, 0($29)
	lwm	$s0, $ra, 1<<2($29)
	lwm	$s0, $ra, 2<<2($29)
	lwm	$s0, $ra, 3<<2($29)
	lwm	$s0, $ra, 4<<2($29)
	lwm	$s0, $ra, 5<<2($29)
	lwm	$s0, $ra, 6<<2($29)
	lwm	$s0, $ra, 7<<2($29)
	lwm	$s0, $ra, 8<<2($29)
	lwm	$s0, $ra, 9<<2($29)
	lwm	$s0, $ra, 10<<2($29)
	lwm	$s0, $ra, 11<<2($29)
	lwm	$s0, $ra, 12<<2($29)
	lwm	$s0, $ra, 13<<2($29)
	lwm	$s0, $ra, 14<<2($29)
	lwm	$s0, $ra, 15<<2($29)

	lwm	$s0, 0
	lwm	$s0, 4
	lwm	$s0, ($5)
	lwm	$s0, 2047($5)
	lwm	$s0-$s1, 2047($5)
	lwm	$s0-$s2, 2047($5)
	lwm	$s0-$s3, 2047($5)
	lwm	$s0-$s4, 2047($5)
	lwm	$s0-$s5, 2047($5)
	lwm	$s0-$s6, 2047($5)
	lwm	$s0-$s7, 2047($5)
	lwm	$s0-$s8, 2047($5)
	lwm	$ra, 2047($5)
	lwm	$s0,$ra, ($5)
	lwm	$s0-$s1,$ra, ($5)
	lwm	$s0-$s2,$ra, ($5)
	lwm	$s0-$s3,$ra, ($5)
	lwm	$s0-$s4,$ra, ($5)
	lwm	$s0-$s5,$ra, ($5)
	lwm	$s0-$s6,$ra, ($5)
	lwm	$s0-$s7,$ra, ($5)
	lwm	$s0-$s8,$ra, ($5)
	lwm	$s0, -32768($0)
	lwm	$s0, 32767($0)
	lwm	$s0, 0($0)
	lwm	$s0, 65535($0)
	lwm	$s0, -32768($29)
	lwm	$s0, 32767($29)
	lwm	$s0, 0($29)
	lwm	$s0, 65535($29)

	lwp	$2, 0
	lwp	$2, 4
	lwp	$2, ($29)
	lwp	$2, 0($29)
	lwp	$2, -2048($3)
	lwp	$2, 2047($3)
	lwp	$2, -32768($3)
	lwp	$2, 32767($3)
	lwp	$2, 0($3)
	lwp	$2, 65535($3)
	lwp	$2, -32768($0)
	lwp	$2, 32767($0)
	lwp	$2, 65535($0)

	lwl	$3, 4
	lwl	$3, 4($0)
	lwl	$3, ($0)
	lwl	$3, 0($0)
	lwl	$3, 2047($0)
	lwl	$3, -2048($0)
	lwl	$3, 32767($0)
	lwl	$3, -32768($0)
	lwl	$3, 65535($0)
	lwl	$3, 0xffff0000($0)
	lwl	$3, 0xffff8000($0)
	lwl	$3, 0xffff0001($0)
	lwl	$3, 0xffff8001($0)
	lwl	$3, 0xf0000000($0)
	lwl	$3, 0xffffffff($0)
	lwl	$3, 0x12345678($0)
	lwl	$3, ($4)
	lwl	$3, 0($4)
	lwl	$3, 2047($4)
	lwl	$3, -2048($4)
	lwl	$3, 32767($4)
	lwl	$3, -32768($4)
	lwl	$3, 65535($4)
	lwl	$3, 0xffff0000($4)
	lwl	$3, 0xffff8000($4)
	lwl	$3, 0xffff0001($4)
	lwl	$3, 0xffff8001($4)
	lwl	$3, 0xf0000000($4)
	lwl	$3, 0xffffffff($4)
	lwl	$3, 0x12345678($4)

	lcache	$3, 4
	lcache	$3, 4($0)
	lcache	$3, ($0)
	lcache	$3, 0($0)
	lcache	$3, 2047($0)
	lcache	$3, -2048($0)
	lcache	$3, 32767($0)
	lcache	$3, -32768($0)
	lcache	$3, 65535($0)
	lcache	$3, 0xffff0000($0)
	lcache	$3, 0xffff8000($0)
	lcache	$3, 0xffff0001($0)
	lcache	$3, 0xffff8001($0)
	lcache	$3, 0xf0000000($0)
	lcache	$3, 0xffffffff($0)
	lcache	$3, 0x12345678($0)
	lcache	$3, ($4)
	lcache	$3, 0($4)
	lcache	$3, 2047($4)
	lcache	$3, -2048($4)
	lcache	$3, 32767($4)
	lcache	$3, -32768($4)
	lcache	$3, 65535($4)
	lcache	$3, 0xffff0000($4)
	lcache	$3, 0xffff8000($4)
	lcache	$3, 0xffff0001($4)
	lcache	$3, 0xffff8001($4)
	lcache	$3, 0xf0000000($4)
	lcache	$3, 0xffffffff($4)
	lcache	$3, 0x12345678($4)

	lwr	$3, 4
	lwr	$3, 4($0)
	lwr	$3, ($0)
	lwr	$3, 0($0)
	lwr	$3, 2047($0)
	lwr	$3, -2048($0)
	lwr	$3, 32767($0)
	lwr	$3, -32768($0)
	lwr	$3, 65535($0)
	lwr	$3, 0xffff0000($0)
	lwr	$3, 0xffff8000($0)
	lwr	$3, 0xffff0001($0)
	lwr	$3, 0xffff8001($0)
	lwr	$3, 0xf0000000($0)
	lwr	$3, 0xffffffff($0)
	lwr	$3, 0x12345678($0)
	lwr	$3, ($4)
	lwr	$3, 0($4)
	lwr	$3, 2047($4)
	lwr	$3, -2048($4)
	lwr	$3, 32767($4)
	lwr	$3, -32768($4)
	lwr	$3, 65535($4)
	lwr	$3, 0xffff0000($4)
	lwr	$3, 0xffff8000($4)
	lwr	$3, 0xffff0001($4)
	lwr	$3, 0xffff8001($4)
	lwr	$3, 0xf0000000($4)
	lwr	$3, 0xffffffff($4)
	lwr	$3, 0x12345678($4)

	flush	$3, 4
	flush	$3, 4($0)
	flush	$3, ($0)
	flush	$3, 0($0)
	flush	$3, 2047($0)
	flush	$3, -2048($0)
	flush	$3, 32767($0)
	flush	$3, -32768($0)
	flush	$3, 65535($0)
	flush	$3, 0xffff0000($0)
	flush	$3, 0xffff8000($0)
	flush	$3, 0xffff0001($0)
	flush	$3, 0xffff8001($0)
	flush	$3, 0xf0000000($0)
	flush	$3, 0xffffffff($0)
	flush	$3, 0x12345678($0)
	flush	$3, ($4)
	flush	$3, 0($4)
	flush	$3, 2047($4)
	flush	$3, -2048($4)
	flush	$3, 32767($4)
	flush	$3, -32768($4)
	flush	$3, 65535($4)
	flush	$3, 0xffff0000($4)
	flush	$3, 0xffff8000($4)
	flush	$3, 0xffff0001($4)
	flush	$3, 0xffff8001($4)
	flush	$3, 0xf0000000($4)
	flush	$3, 0xffffffff($4)
	flush	$3, 0x12345678($4)

	lwxs	$3, $4($5)
	madd	$4,$5
	maddu	$4,$5

	mfc0	$2, $0
	mfc0	$2, $1
	mfc0	$2, $2
	mfc0	$2, $3
	mfc0	$2, $4
	mfc0	$2, $5
	mfc0	$2, $6
	mfc0	$2, $7
	mfc0	$2, $8
	mfc0	$2, $9
	mfc0	$2, $10
	mfc0	$2, $11
	mfc0	$2, $12
	mfc0	$2, $13
	mfc0	$2, $14
	mfc0	$2, $15
	mfc0	$2, $16
	mfc0	$2, $17
	mfc0	$2, $18
	mfc0	$2, $19
	mfc0	$2, $20
	mfc0	$2, $21
	mfc0	$2, $22
	mfc0	$2, $23
	mfc0	$2, $24
	mfc0	$2, $25
	mfc0	$2, $26
	mfc0	$2, $27
	mfc0	$2, $28
	mfc0	$2, $29
	mfc0	$2, $30
	mfc0	$2, $31

	mfc0	$2, $0, 0
	mfc0	$2, $0, 1
	mfc0	$2, $0, 2
	mfc0	$2, $0, 3
	mfc0	$2, $0, 4
	mfc0	$2, $0, 5
	mfc0	$2, $0, 6
	mfc0	$2, $0, 7
	mfc0	$2, $1, 0
	mfc0	$2, $1, 1
	mfc0	$2, $1, 2
	mfc0	$2, $1, 3
	mfc0	$2, $1, 4
	mfc0	$2, $1, 5
	mfc0	$2, $1, 6
	mfc0	$2, $1, 7
	mfc0	$2, $2, 0
	mfc0	$2, $2, 1
	mfc0	$2, $2, 2
	mfc0	$2, $2, 3
	mfc0	$2, $2, 4
	mfc0	$2, $2, 5
	mfc0	$2, $2, 6
	mfc0	$2, $2, 7

	mfhi	$0
	mfhi	$2
	mfhi	$3
	mfhi	$4
	mfhi	$29
	mfhi	$30
	mfhi	$31

	mfhi32	$0
	mfhi32	$2
	mfhi32	$3
	mfhi32	$4
	mfhi32	$29
	mfhi32	$30
	mfhi32	$31

	mflo	$0
	mflo	$2
	mflo	$3
	mflo	$4
	mflo	$29
	mflo	$30
	mflo	$31

	mflo32	$0
	mflo32	$2
	mflo32	$3
	mflo32	$4
	mflo32	$29
	mflo32	$30
	mflo32	$31

	movn	$2, $3
	movn	$2, $2, $3
	movn	$2, $3, $4

	movz	$2, $3
	movz	$2, $2, $3
	movz	$2, $3, $4

	msub	$4,$5
	msubu	$4,$5

	mtc0	$2, $0
	mtc0	$2, $1
	mtc0	$2, $2
	mtc0	$2, $3
	mtc0	$2, $4
	mtc0	$2, $5
	mtc0	$2, $6
	mtc0	$2, $7
	mtc0	$2, $8
	mtc0	$2, $9
	mtc0	$2, $10
	mtc0	$2, $11
	mtc0	$2, $12
	mtc0	$2, $13
	mtc0	$2, $14
	mtc0	$2, $15
	mtc0	$2, $16
	mtc0	$2, $17
	mtc0	$2, $18
	mtc0	$2, $19
	mtc0	$2, $20
	mtc0	$2, $21
	mtc0	$2, $22
	mtc0	$2, $23
	mtc0	$2, $24
	mtc0	$2, $25
	mtc0	$2, $26
	mtc0	$2, $27
	mtc0	$2, $28
	mtc0	$2, $29
	mtc0	$2, $30
	mtc0	$2, $31

	mtc0	$2, $0, 0
	mtc0	$2, $0, 1
	mtc0	$2, $0, 2
	mtc0	$2, $0, 3
	mtc0	$2, $0, 4
	mtc0	$2, $0, 5
	mtc0	$2, $0, 6
	mtc0	$2, $0, 7
	mtc0	$2, $1, 0
	mtc0	$2, $1, 1
	mtc0	$2, $1, 2
	mtc0	$2, $1, 3
	mtc0	$2, $1, 4
	mtc0	$2, $1, 5
	mtc0	$2, $1, 6
	mtc0	$2, $1, 7
	mtc0	$2, $2, 0
	mtc0	$2, $2, 1
	mtc0	$2, $2, 2
	mtc0	$2, $2, 3
	mtc0	$2, $2, 4
	mtc0	$2, $2, 5
	mtc0	$2, $2, 6
	mtc0	$2, $2, 7

	mthi	$0
	mthi	$2
	mthi	$3
	mthi	$4
	mthi	$29
	mthi	$30
	mthi	$31

	mtlo	$0
	mtlo	$2
	mtlo	$3
	mtlo	$4
	mtlo	$29
	mtlo	$30
	mtlo	$31

	mul	$2, $3, $4
	mul	$29, $30, $31
	mul	$2, $2, $4
	mul	$2, $4
	mul	$2, $2, 0
	mul	$2, $2, 1
	mul	$2, $2, 32767
	mul	$2, $2, -32768
	mul	$2, $2, 65535

	mulo	$2, $3, $4
	mulo	$2, $3, 4
	mulou	$2, $3, $4
	mulou	$2, $3, 4

	mult	$2, $3
	multu	$2, $3

	neg	$2, $3
	neg	$2, $2
	neg	$2
	negu	$2, $3
	negu	$2, $2
	negu	$2
	negu32	$2, $3
	negu32	$2, $2
	negu32	$2

	not	$2, $2
	not	$2, $2
	not	$2, $3
	not	$2, $4
	not	$2, $5
	not	$2, $6
	not	$2, $7
	not	$2, $16
	not	$2, $17
	not	$3, $17
	not	$4, $17
	not	$5, $17
	not	$6, $17
	not	$7, $17
	not	$16, $17
	not	$17, $17

	nor	$2, $7, $0
	nor	$2, $0, $7

	nor32	$2, $3, $4
	nor32	$29, $30, $31
	nor32	$2, $2, $4
	nor32	$2, $4

	nor	$2, $3, 32768
	nor	$2, $3, 65535
	nor	$2, $3, 65536
	nor	$2, $3, -32768
	nor	$2, $3, -32769

	or	$2, $22, $0
	or	$22, $2, $0
	or	$2, $0, $22
	or	$22, $0, $2

	or	$2, $2
	or	$2, $3
	or	$2, $4
	or	$2, $5
	or	$2, $6
	or	$2, $7
	or	$2, $16
	or	$2, $17
	or	$3, $2
	or	$4, $2
	or	$5, $2
	or	$6, $2
	or	$7, $2
	or	$16, $2
	or	$17, $2
	or	$2, $2
	or	$2, $2, $3
	or	$2, $3, $2

	or32	$2, $3, $4
	or32	$29, $30, $31
	or32	$2, $2, $4
	or32	$2, $4

	or	$2, $3, 32768
	or	$2, $3, 65535
	or	$2, $3, 65536
	or	$2, $3, -32768
	or	$2, $3, -32769

	ori	$3, $4, 0
	ori	$3, $4, 32767
	ori	$3, $4, 65535
	ori	$3, $3, 65535
	ori	$3, 65535

	rdhwr	$2, $0
	rdhwr	$2, $1
	rdhwr	$2, $2
	rdhwr	$2, $3
	rdhwr	$2, $4
	rdhwr	$2, $5
	rdhwr	$2, $6
	rdhwr	$2, $7
	rdhwr	$2, $8
	rdhwr	$2, $9
	rdhwr	$2, $10

	rdpgpr	$2, $3
	rdpgpr	$2, $2
	rdpgpr	$2

	rem	$0, $2, $3
	rem	$0, $30, $31
	rem	$0, $3
	rem	$0, $31

	rem	$2, $3, $0
	rem	$2, $3, $4

	rem	$3, $4, 0
	rem	$3, $4, 1
	rem	$3, $4, -1
	rem	$3, $4, 2

	remu	$0, $2, $3
	remu	$0, $30, $31
	remu	$0, $3
	remu	$0, $31

	remu	$2, $3, $0
	remu	$2, $3, $4

	remu	$3, $4, 0
	remu	$3, $4, 1
	remu	$3, $4, -1
	remu	$3, $4, 2

	rol	$2, $3, $4
	rol	$2, $2, $4
	rol	$2, $3, $3
	rol	$2, $3, $2

	rol	$2, $3, 0
	rol	$2, $3, 1
	rol	$2, $3, 31
	rol	$2, $2, 31
	rol	$2, 31

	ror	$2, $3, 0
	ror	$2, $3, 1
	ror	$2, $3, 31
	ror	$2, $2, 31
	ror	$2, 31

	ror	$2, $3, $4
	ror	$2, $2, $4

	rotr	$2, $3, $4
	rotr	$2, $2, $4

	rorv	$2, $3, $4
	rorv	$2, $2, $4

	rotrv	$2, $3, $4
	rotrv	$2, $2, $4

	sb	$0, ($3)
	sb	$0, 0($3)
	sb	$0, 1($3)
	sb	$0, 2($3)
	sb	$0, 3($3)
	sb	$0, 4($3)
	sb	$0, 5($3)
	sb	$0, 6($3)
	sb	$0, 7($3)
	sb	$0, 8($3)
	sb	$0, 9($3)
	sb	$0, 10($3)
	sb	$0, 11($3)
	sb	$0, 12($3)
	sb	$0, 13($3)
	sb	$0, 14($3)
	sb	$0, 15($3)
	sb	$2, 15($3)
	sb	$3, 15($3)
	sb	$4, 15($3)
	sb	$5, 15($3)
	sb	$6, 15($3)
	sb	$7, 15($3)
	sb	$17, 15($3)
	sb	$17, 15($4)
	sb	$17, 15($5)
	sb	$17, 15($6)
	sb	$17, 15($7)
	sb	$17, 15($2)
	sb	$17, 15($16)
	sb	$17, 15($17)

	sb32	$3, 4
	sb32	$3, 4($0)
	sb32	$3, 32767($0)
	sb32	$3, -32768($0)
	sb	$3, 65535($0)
	sb	$3, 0xffff0000($0)
	sb	$3, 0xffff8000($0)
	sb	$3, 0xffff0001($0)
	sb	$3, 0xffff8001($0)
	sb	$3, 0xf0000000($0)
	sb	$3, 0xffffffff($0)
	sb	$3, 0x12345678($0)
	sb32	$3, ($4)
	sb32	$3, 0($4)
	sb32	$3, 32767($4)
	sb32	$3, -32768($4)
	sb	$3, 65535($4)
	sb	$3, 0xffff0000($4)
	sb	$3, 0xffff8000($4)
	sb	$3, 0xffff0001($4)
	sb	$3, 0xffff8001($4)
	sb	$3, 0xf0000000($4)
	sb	$3, 0xffffffff($4)
	sb	$3, 0x12345678($4)

	sc	$3, 4
	sc	$3, 4($0)
	sc	$3, 2047($0)
	sc	$3, -2048($0)
	sc	$3, 32767($0)
	sc	$3, -32768($0)
	sc	$3, 65535($0)
	sc	$3, 0xffff0000($0)
	sc	$3, 0xffff8000($0)
	sc	$3, 0xffff0001($0)
	sc	$3, 0xffff8001($0)
	sc	$3, 0xf0000000($0)
	sc	$3, 0xffffffff($0)
	sc	$3, 0x12345678($0)
	sc	$3, ($4)
	sc	$3, 0($4)
	sc	$3, 2047($4)
	sc	$3, -2048($4)
	sc	$3, 32767($4)
	sc	$3, -32768($4)
	sc	$3, 65535($4)
	sc	$3, 0xffff0000($4)
	sc	$3, 0xffff8000($4)
	sc	$3, 0xffff0001($4)
	sc	$3, 0xffff8001($4)
	sc	$3, 0xf0000000($4)
	sc	$3, 0xffffffff($4)
	sc	$3, 0x12345678($4)

	sdbbp
	sdbbp	0
	sdbbp	1
	sdbbp	2
	sdbbp	3
	sdbbp	4
	sdbbp	5
	sdbbp	6
	sdbbp	7
	sdbbp	8
	sdbbp	9
	sdbbp	10
	sdbbp	11
	sdbbp	12
	sdbbp	13
	sdbbp	14
	sdbbp	15

	sdbbp32
	sdbbp32	0
	sdbbp32	1
	sdbbp32	2
	sdbbp32	255

	seb	$2, $3
	seb	$2, $2
	seb	$2

	seh	$2, $3
	seh	$2, $2
	seh	$2

	seq	$2, $3, $4
	seq	$2, $3, $0
	seq	$2, $0, $4

	seq	$2, $3, 0
	seq	$2, $3, 1
	seq	$2, $3, -1
	seq	$2, $3, -32769

	sge	$2, $3, $4
	sge	$2, $2, $4
	sge	$2, $4
	sge	$2, $3, 0
	sge	$2, $3, -32768
	sge	$2, $3, 0
	sge	$2, $3, 32767
	sge	$2, $3, 65535
	sge	$2, $3, 65536
	sge	$2, $3, -32769

	sgeu	$2, $3, $4
	sgeu	$2, $2, $4
	sgeu	$2, $4
	sgeu	$2, $3, 0
	sgeu	$2, $3, -32768
	sgeu	$2, $3, 0
	sgeu	$2, $3, 32767
	sgeu	$2, $3, 65535
	sgeu	$2, $3, 65536
	sgeu	$2, $3, -32769

	sgt	$2, $3, $4
	sgt	$2, $2, $4
	sgt	$2, $4
	sgt	$2, $3, 0
	sgt	$2, $3, -32768
	sgt	$2, $3, 0
	sgt	$2, $3, 32767
	sgt	$2, $3, 65535
	sgt	$2, $3, 65536
	sgt	$2, $3, -32769

	sgtu	$2, $3, $4
	sgtu	$2, $2, $4
	sgtu	$2, $4
	sgtu	$2, $3, 0
	sgtu	$2, $3, -32768
	sgtu	$2, $3, 0
	sgtu	$2, $3, 32767
	sgtu	$2, $3, 65535
	sgtu	$2, $3, 65536
	sgtu	$2, $3, -32769

	sh	$2, ($3)
	sh	$2, 0<<1($3)
	sh	$2, 1<<1($3)
	sh	$2, 2<<1($3)
	sh	$2, 3<<1($3)
	sh	$2, 4<<1($3)
	sh	$2, 5<<1($3)
	sh	$2, 6<<1($3)
	sh	$2, 7<<1($3)
	sh	$2, 8<<1($3)
	sh	$2, 9<<1($3)
	sh	$2, 10<<1($3)
	sh	$2, 11<<1($3)
	sh	$2, 12<<1($3)
	sh	$2, 13<<1($3)
	sh	$2, 14<<1($3)
	sh	$2, 15<<1($3)
	sh	$2, 15<<1($4)
	sh	$2, 15<<1($5)
	sh	$2, 15<<1($6)
	sh	$2, 15<<1($7)
	sh	$2, 15<<1($2)
	sh	$2, 15<<1($16)
	sh	$2, 15<<1($17)
	sh	$3, 15<<1($17)
	sh	$4, 15<<1($17)
	sh	$5, 15<<1($17)
	sh	$6, 15<<1($17)
	sh	$7, 15<<1($17)
	sh	$17, 15<<1($17)
	sh	$0, 15<<1($17)

	sh32	$3, 4
	sh32	$3, 4($0)
	sh32	$3, 32767($0)
	sh32	$3, -32768($0)
	sh	$3, 65535($0)
	sh	$3, 0xffff0000($0)
	sh	$3, 0xffff8000($0)
	sh	$3, 0xffff0001($0)
	sh	$3, 0xffff8001($0)
	sh	$3, 0xf0000000($0)
	sh	$3, 0xffffffff($0)
	sh	$3, 0x12345678($0)
	sh32	$3, ($4)
	sh32	$3, 0($4)
	sh32	$3, 32767($4)
	sh32	$3, -32768($4)
	sh	$3, 65535($4)
	sh	$3, 0xffff0000($4)
	sh	$3, 0xffff8000($4)
	sh	$3, 0xffff0001($4)
	sh	$3, 0xffff8001($4)
	sh	$3, 0xf0000000($4)
	sh	$3, 0xffffffff($4)
	sh	$3, 0x12345678($4)

	sle	$2, $3, $4
	sle	$2, $2, $4
	sle	$2, $4
	sle	$2, $3, 0
	sle	$2, $3, -32768
	sle	$2, $3, 0
	sle	$2, $3, 32767
	sle	$2, $3, 65535
	sle	$2, $3, 65536
	sle	$2, $3, -32769

	sleu	$2, $3, $4
	sleu	$2, $2, $4
	sleu	$2, $4
	sleu	$2, $3, 0
	sleu	$2, $3, -32768
	sleu	$2, $3, 0
	sleu	$2, $3, 32767
	sleu	$2, $3, 65535
	sleu	$2, $3, 65536
	sleu	$2, $3, -32769

	sll	$2, $2, 1
	sll	$2, $2, 2
	sll	$2, $2, 3
	sll	$2, $2, 4
	sll	$2, $2, 5
	sll	$2, $2, 6
	sll	$2, $2, 7
	sll	$2, $2, 8
	sll	$2, $3, 8
	sll	$2, $4, 8
	sll	$2, $5, 8
	sll	$2, $6, 8
	sll	$2, $7, 8
	sll	$2, $16, 8
	sll	$2, $17, 8
	sll	$3, $2, 8
	sll	$4, $2, 8
	sll	$5, $2, 8
	sll	$6, $2, 8
	sll	$7, $2, 8
	sll	$16, $2, 8
	sll	$17, $2, 8
	sll	$2, $2, 1
	sll	$3, 1

	sllv	$2, $3, $4
	sllv	$2, $2, $4
	sll	$2, $2, $4
	sll	$2, $4
	sll32	$2, $4, 0
	sll32	$2, $4, 1
	sll32	$2, $4, 31
	sll32	$2, $2, 31
	sll32	$2, 31

	slt	$2, $3, $4
	slt	$2, $2, $4
	slt	$2, $4
	slt	$2, $3, 0
	slt	$2, $3, -32768
	slt	$2, $3, 0
	slt	$2, $3, 32767
	slt	$2, $3, 65535
	slt	$2, $3, 65536
	slt	$2, $3, -32769

	slti	$3, $4, -32768
	slti	$3, $4, 0
	slti	$3, $4, 32767
	slti	$3, $4, 65535
	slti	$3, $3, 65535
	slti	$3, 65535

	sltiu	$3, $4, -32768
	sltiu	$3, $4, 0
	sltiu	$3, $4, 32767
	sltiu	$3, $4, 65535
	sltiu	$3, $3, 65535
	sltiu	$3, 65535

	sltu	$2, $3, $4
	sltu	$2, $2, $4
	sltu	$2, $4
	sltu	$2, $3, 0
	sltu	$2, $3, -32768
	sltu	$2, $3, 0
	sltu	$2, $3, 32767
	sltu	$2, $3, 65535
	sltu	$2, $3, 65536
	sltu	$2, $3, -32769

	sne	$2, $3, $4
	sne	$2, $0, $4
	sne	$2, $3, $0

	sne	$2, $3, 0
	sne	$2, $3, 1
	sne	$2, $3, -1
	sne	$2, $3, -32769

	srav	$2, $3, $4
	srav	$2, $2, $4
	sra	$2, $2, $4
	sra	$2, $4
	sra	$2, $4, 0
	sra	$2, $4, 1
	sra	$2, $4, 31
	sra	$2, $2, 31
	sra	$2, 31

	srlv	$2, $3, $4
	srlv	$2, $2, $4
	srl	$2, $2, $4
	srl	$2, $4
	srl	$2, $4, 0
	srl	$2, $4, 1
	srl	$2, $4, 31
	srl	$2, $2, 31
	srl	$2, 31

	srl	$2, $2, 1
	srl	$2, $2, 2
	srl	$2, $2, 3
	srl	$2, $2, 4
	srl	$2, $2, 5
	srl	$2, $2, 6
	srl	$2, $2, 7
	srl	$2, $2, 8
	srl	$2, $3, 8
	srl	$2, $4, 8
	srl	$2, $5, 8
	srl	$2, $6, 8
	srl	$2, $7, 8
	srl	$2, $16, 8
	srl	$2, $17, 8
	srl	$2, $2, 8
	srl	$3, $2, 8
	srl	$4, $2, 8
	srl	$5, $2, 8
	srl	$6, $2, 8
	srl	$7, $2, 8
	srl	$16, $2, 8
	srl	$17, $2, 8
	srl	$3, $3, 1
	srl	$3, 1

	sub	$2, $3, $4
	sub	$29, $30, $31
	sub	$2, $2, $4
	sub	$2, $4
	sub	$2, $2, 0
	sub	$2, $2, 1
	sub	$2, $2, 32767
	sub	$2, $2, -32768
	sub	$2, $2, 65535

	subu	$2, $3, $2
	subu	$2, $3, $3
	subu	$2, $3, $4
	subu	$2, $3, $5
	subu	$2, $3, $6
	subu	$2, $3, $7
	subu	$2, $3, $16
	subu	$2, $3, $17
	subu	$2, $2, $17
	subu	$2, $4, $17
	subu	$2, $5, $17
	subu	$2, $6, $17
	subu	$2, $7, $17
	subu	$2, $16, $17
	subu	$2, $17, $17
	subu	$2, $2, $17
	subu	$3, $2, $17
	subu	$4, $2, $17
	subu	$5, $2, $17
	subu	$6, $2, $17
	subu	$7, $2, $17
	subu	$16, $2, $17
	subu	$17, $2, $17
	subu	$7, $7, $2
	subu	$7, $2

	subu32	$2, $3, $4
	subu32	$29, $30, $31
	subu32	$2, $2, $4
	subu32	$2, $4
	subu	$2, $2, 0
	subu	$2, $2, 1
	subu	$2, $2, 32767
	subu	$2, $2, -32768
	subu	$2, $2, 65535

	sw	$2, ($4)
	sw	$2, 0($4)
	sw	$2, 1<<2($4)
	sw	$2, 2<<2($4)
	sw	$2, 3<<2($4)
	sw	$2, 4<<2($4)
	sw	$2, 5<<2($4)
	sw	$2, 6<<2($4)
	sw	$2, 7<<2($4)
	sw	$2, 8<<2($4)
	sw	$2, 9<<2($4)
	sw	$2, 10<<2($4)
	sw	$2, 11<<2($4)
	sw	$2, 12<<2($4)
	sw	$2, 13<<2($4)
	sw	$2, 14<<2($4)
	sw	$2, 15<<2($4)
	sw	$2, 15<<2($5)
	sw	$2, 15<<2($6)
	sw	$2, 15<<2($7)
	sw	$2, 15<<2($16)
	sw	$2, 15<<2($17)
	sw	$2, 15<<2($2)
	sw	$2, 15<<2($3)
	sw	$3, 15<<2($3)
	sw	$4, 15<<2($3)
	sw	$5, 15<<2($3)
	sw	$6, 15<<2($3)
	sw	$7, 15<<2($3)
	sw	$17, 15<<2($3)
	sw	$0, 15<<2($3)

	sw	$0, ($29)
	sw	$0, 0($29)
	sw	$0, 1<<2($29)
	sw	$0, 2<<2($29)
	sw	$0, 3<<2($29)
	sw	$0, 4<<2($29)
	sw	$0, 5<<2($29)
	sw	$0, 30<<2($29)
	sw	$0, 31<<2($29)
	sw	$2, 31<<2($29)
	sw	$17, 31<<2($29)
	sw	$3, 31<<2($29)
	sw	$4, 31<<2($29)
	sw	$5, 31<<2($29)
	sw	$6, 31<<2($29)
	sw	$7, 31<<2($29)
	sw	$31, 31<<2($29)

	sw32	$3, 4
	sw32	$3, 4($0)
	sw32	$3, 32767($0)
	sw32	$3, -32768($0)
	sw	$3, 65535($0)
	sw	$3, 0xffff0000($0)
	sw	$3, 0xffff8000($0)
	sw	$3, 0xffff0001($0)
	sw	$3, 0xffff8001($0)
	sw	$3, 0xf0000000($0)
	sw	$3, 0xffffffff($0)
	sw	$3, 0x12345678($0)
	sw32	$3, ($4)
	sw32	$3, 0($4)
	sw32	$3, 32767($4)
	sw32	$3, -32768($4)
	sw	$3, 65535($4)
	sw	$3, 0xffff0000($4)
	sw	$3, 0xffff8000($4)
	sw	$3, 0xffff0001($4)
	sw	$3, 0xffff8001($4)
	sw	$3, 0xf0000000($4)
	sw	$3, 0xffffffff($4)
	sw	$3, 0x12345678($4)

	swl	$3, 4
	swl	$3, 4($0)
	swl	$3, 2047($0)
	swl	$3, -2048($0)
	swl	$3, 32767($0)
	swl	$3, -32768($0)
	swl	$3, 65535($0)
	swl	$3, 0xffff0000($0)
	swl	$3, 0xffff8000($0)
	swl	$3, 0xffff0001($0)
	swl	$3, 0xffff8001($0)
	swl	$3, 0xf0000000($0)
	swl	$3, 0xffffffff($0)
	swl	$3, 0x12345678($0)
	swl	$3, ($4)
	swl	$3, 0($4)
	swl	$3, 2047($4)
	swl	$3, -2048($4)
	swl	$3, 32767($4)
	swl	$3, -32768($4)
	swl	$3, 65535($4)
	swl	$3, 0xffff0000($4)
	swl	$3, 0xffff8000($4)
	swl	$3, 0xffff0001($4)
	swl	$3, 0xffff8001($4)
	swl	$3, 0xf0000000($4)
	swl	$3, 0xffffffff($4)
	swl	$3, 0x12345678($4)

	swr	$3, 4
	swr	$3, 4($0)
	swr	$3, 2047($0)
	swr	$3, -2048($0)
	swr	$3, 32767($0)
	swr	$3, -32768($0)
	swr	$3, 65535($0)
	swr	$3, 0xffff0000($0)
	swr	$3, 0xffff8000($0)
	swr	$3, 0xffff0001($0)
	swr	$3, 0xffff8001($0)
	swr	$3, 0xf0000000($0)
	swr	$3, 0xffffffff($0)
	swr	$3, 0x12345678($0)
	swr	$3, ($4)
	swr	$3, 0($4)
	swr	$3, 2047($4)
	swr	$3, -2048($4)
	swr	$3, 32767($4)
	swr	$3, -32768($4)
	swr	$3, 65535($4)
	swr	$3, 0xffff0000($4)
	swr	$3, 0xffff8000($4)
	swr	$3, 0xffff0001($4)
	swr	$3, 0xffff8001($4)
	swr	$3, 0xf0000000($4)
	swr	$3, 0xffffffff($4)
	swr	$3, 0x12345678($4)

	scache	$3, 4
	scache	$3, 4($0)
	scache	$3, 2047($0)
	scache	$3, -2048($0)
	scache	$3, 32767($0)
	scache	$3, -32768($0)
	scache	$3, 65535($0)
	scache	$3, 0xffff0000($0)
	scache	$3, 0xffff8000($0)
	scache	$3, 0xffff0001($0)
	scache	$3, 0xffff8001($0)
	scache	$3, 0xf0000000($0)
	scache	$3, 0xffffffff($0)
	scache	$3, 0x12345678($0)
	scache	$3, ($4)
	scache	$3, 0($4)
	scache	$3, 2047($4)
	scache	$3, -2048($4)
	scache	$3, 32767($4)
	scache	$3, -32768($4)
	scache	$3, 65535($4)
	scache	$3, 0xffff0000($4)
	scache	$3, 0xffff8000($4)
	scache	$3, 0xffff0001($4)
	scache	$3, 0xffff8001($4)
	scache	$3, 0xf0000000($4)
	scache	$3, 0xffffffff($4)
	scache	$3, 0x12345678($4)

	invalidate	$3, 4
	invalidate	$3, 4($0)
	invalidate	$3, 2047($0)
	invalidate	$3, -2048($0)
	invalidate	$3, 32767($0)
	invalidate	$3, -32768($0)
	invalidate	$3, 65535($0)
	invalidate	$3, 0xffff0000($0)
	invalidate	$3, 0xffff8000($0)
	invalidate	$3, 0xffff0001($0)
	invalidate	$3, 0xffff8001($0)
	invalidate	$3, 0xf0000000($0)
	invalidate	$3, 0xffffffff($0)
	invalidate	$3, 0x12345678($0)
	invalidate	$3, ($4)
	invalidate	$3, 0($4)
	invalidate	$3, 2047($4)
	invalidate	$3, -2048($4)
	invalidate	$3, 32767($4)
	invalidate	$3, -32768($4)
	invalidate	$3, 65535($4)
	invalidate	$3, 0xffff0000($4)
	invalidate	$3, 0xffff8000($4)
	invalidate	$3, 0xffff0001($4)
	invalidate	$3, 0xffff8001($4)
	invalidate	$3, 0xf0000000($4)
	invalidate	$3, 0xffffffff($4)
	invalidate	$3, 0x12345678($4)

	swm	$s0, $ra, 12<<2($29)
	swm	$s0, $s1, $ra, 12<<2($29)
	swm	$s0-$s1, $ra, 12<<2($29)
	swm	$s0, $s1, $s2, $ra, 12<<2($29)
	swm	$s0-$s2, $ra, 12<<2($29)
	swm	$s0, $s1, $s2, $s3, $ra, 12<<2($29)
	swm	$s0-$s3, $ra, 12<<2($29)
	swm	$s0, $ra, ($29)
	swm	$s0, $ra, 0($29)
	swm	$s0, $ra, 1<<2($29)
	swm	$s0, $ra, 2<<2($29)
	swm	$s0, $ra, 3<<2($29)
	swm	$s0, $ra, 4<<2($29)
	swm	$s0, $ra, 5<<2($29)
	swm	$s0, $ra, 6<<2($29)
	swm	$s0, $ra, 7<<2($29)
	swm	$s0, $ra, 8<<2($29)
	swm	$s0, $ra, 9<<2($29)
	swm	$s0, $ra, 10<<2($29)
	swm	$s0, $ra, 11<<2($29)
	swm	$s0, $ra, 12<<2($29)
	swm	$s0, $ra, 13<<2($29)
	swm	$s0, $ra, 14<<2($29)
	swm	$s0, $ra, 15<<2($29)

	swm	$s0, 0
	swm	$s0, 4
	swm	$s0, 2047
	swm	$s0, -2048
	swm	$s0, 2048
	swm	$s0, -2049
	swm	$s0, ($5)
	swm	$s0, 2047($5)
	swm	$s0, -2048($5)
	swm	$s0, 2048($5)
	swm	$s0, -2049($5)
	swm	$s0-$s1, 2047($5)
	swm	$s0-$s2, 2047($5)
	swm	$s0-$s3, 2047($5)
	swm	$s0-$s4, 2047($5)
	swm	$s0-$s5, 2047($5)
	swm	$s0-$s6, 2047($5)
	swm	$s0-$s7, 2047($5)
	swm	$s0-$s8, 2047($5)
	swm	$ra, 2047($5)
	swm	$s0,$ra, ($5)
	swm	$s0-$s1,$ra, ($5)
	swm	$s0-$s2,$ra, ($5)
	swm	$s0-$s3,$ra, ($5)
	swm	$s0-$s4,$ra, ($5)
	swm	$s0-$s5,$ra, ($5)
	swm	$s0-$s6,$ra, ($5)
	swm	$s0-$s7,$ra, ($5)
	swm	$s0-$s8,$ra, ($5)
	swm	$s0, -32768($29)
	swm	$s0, 32767($29)
	swm	$s0, 0($29)
	swm	$s0, 65535($29)

	swp	$2, 0
	swp	$2, 4
	swp	$2, 2047
	swp	$2, -2048
	swp	$2, 2048
	swp	$2, -2049
	swp	$2, ($29)
	swp	$2, 0($29)
	swp	$2, 2047($3)
	swp	$2, -2048($3)
	swp	$2, 2048($3)
	swp	$2, -2049($3)
	swp	$2, 32767($3)
	swp	$2, -32768($3)
	swp	$2, 0($3)
	swp	$2, 65535($3)

	sync
	sync	0
	sync	1
	sync	2
	sync	3
	sync	4
	sync	30
	sync	31

	synci	0
	synci	($0)
	synci	0($0)
	synci	2047($0)
	synci	-2048($0)
	synci	2048($0)
	synci	-2049($0)
	synci	32767($0)
	synci	-32768($0)
	synci	0($2)
	synci	0($3)
	synci	2047($3)
	synci	-2048($3)
	synci	2048($3)
	synci	-2049($3)
	synci	32767($3)
	synci	-32768($3)

	syscall
	syscall	0
	syscall	1
	syscall	2
	syscall	255

	teqi	$2, 0
	teqi	$2, -32768
	teqi	$2, 32767
	teqi	$2, 65535
	teq	$2, $3
	teq	$3, $2
	teq	$2, $3, 0
	teq	$2, $3, 1
	teq	$2, $3, 15
	teq	$2, 0
	teq	$2, -32768
	teq	$2, 32767
	teq	$2, 65535

	tgei	$2, 0
	tgei	$2, -32768
	tgei	$2, 32767
	tgei	$2, 65535
	tge	$2, $3
	tge	$3, $2
	tge	$2, $3, 0
	tge	$2, $3, 1
	tge	$2, $3, 15
	tge	$2, 0
	tge	$2, -32768
	tge	$2, 32767
	tge	$2, 65535

	tgeiu	$2, 0
	tgeiu	$2, -32768
	tgeiu	$2, 32767
	tgeiu	$2, 65535
	tgeu	$2, $3
	tgeu	$3, $2
	tgeu	$2, $3, 0
	tgeu	$2, $3, 1
	tgeu	$2, $3, 15
	tgeu	$2, 0
	tgeu	$2, -32768
	tgeu	$2, 32767
	tgeu	$2, 65535

	tlbp
	tlbr
	tlbwi
	tlbwr

	tlti	$2, 0
	tlti	$2, -32768
	tlti	$2, 32767
	tlti	$2, 65535
	tlt	$2, $3
	tlt	$3, $2
	tlt	$2, $3, 0
	tlt	$2, $3, 1
	tlt	$2, $3, 15
	tlt	$2, 0
	tlt	$2, -32768
	tlt	$2, 32767
	tlt	$2, 65535

	tltiu	$2, 0
	tltiu	$2, -32768
	tltiu	$2, 32767
	tltiu	$2, 65535
	tltu	$2, $3
	tltu	$3, $2
	tltu	$2, $3, 0
	tltu	$2, $3, 1
	tltu	$2, $3, 15
	tltu	$2, 0
	tltu	$2, -32768
	tltu	$2, 32767
	tltu	$2, 65535
	tltu	$2, 65536
	tltu	$2, 0xffffffff

	tnei	$2, 0
	tnei	$2, -32768
	tnei	$2, 32767
	tnei	$2, 65535
	tne	$2, $3
	tne	$3, $2
	tne	$2, $3, 0
	tne	$2, $3, 1
	tne	$2, $3, 15
	tne	$2, 0
	tne	$2, -32768
	tne	$2, 32767
	tne	$2, 65535
	tne	$2, 65536
	tne	$2, 0xffffffff

	ulh	$3, 4
	ulh	$3, 4($0)
	ulh	$3, ($4)
	ulh	$3, 0($4)
	ulh	$3, 32763($4)
	ulh	$3, -32768($4)
	ulh	$3, 65535($4)
	ulh	$3, 0xffff0000($4)
	ulh	$3, 0xffff8000($4)
	ulh	$3, 0xffff0001($4)
	ulh	$3, 0xffff8001($4)
	ulh	$3, 0xf0000000($4)
	ulh	$3, 0xffffffff($4)

	ulhu	$3, 4
	ulhu	$3, 4($0)
	ulhu	$3, ($4)
	ulhu	$3, 0($4)
	ulhu	$3, 32763($4)
	ulhu	$3, -32768($4)
	ulhu	$3, 65535($4)
	ulhu	$3, 0xffff0000($4)
	ulhu	$3, 0xffff8000($4)
	ulhu	$3, 0xffff0001($4)
	ulhu	$3, 0xffff8001($4)
	ulhu	$3, 0xf0000000($4)
	ulhu	$3, 0xffffffff($4)

	ulw	$3, 0
	ulw	$3, ($0)
	ulw	$3, 4
	ulw	$3, 4($0)
	ulw	$3, 2047
	ulw	$3, -2048
	ulw	$3, 2048
	ulw	$3, -2049
	ulw	$3, 32763($0)
	ulw	$3, -32768($0)
	ulw	$3, 65535($0)
	ulw	$3, 0xffff0000($0)
	ulw	$3, 0xffff8000($0)
	ulw	$3, 0xffff0001($0)
	ulw	$3, 0xffff8001($0)
	ulw	$3, 0xf0000000($0)
	ulw	$3, 0xffffffff($0)
	ulw	$3, 0x12345678($0)
	ulw	$3, 0($4)
	ulw	$3, 4($4)
	ulw	$3, 2047($4)
	ulw	$3, -2048($4)
	ulw	$3, 2048($4)
	ulw	$3, -2049($4)
	ulw	$3, 32763($4)
	ulw	$3, -32768($4)
	ulw	$3, 65535($4)
	ulw	$3, 0xffff0000($4)
	ulw	$3, 0xffff8000($4)
	ulw	$3, 0xffff0001($4)
	ulw	$3, 0xffff8001($4)
	ulw	$3, 0xf0000000($4)
	ulw	$3, 0xffffffff($4)
	ulw	$3, 0x12345678($4)

	ush	$3, 4
	ush	$3, 4($0)
	ush	$3, ($4)
	ush	$3, 0($4)
	ush	$3, 32763($4)
	ush	$3, -32768($4)
	ush	$3, 65535($4)
	ush	$3, 0xffff0000($4)
	ush	$3, 0xffff8000($4)
	ush	$3, 0xffff0001($4)
	ush	$3, 0xffff8001($4)
	ush	$3, 0xf0000000($4)
	ush	$3, 0xffffffff($4)

	usw	$3, 0
	usw	$3, ($0)
	usw	$3, 4
	usw	$3, 4($0)
	usw	$3, 2047
	usw	$3, -2048
	usw	$3, 2048
	usw	$3, -2049
	usw	$3, 32763($0)
	usw	$3, -32768($0)
	usw	$3, 65535($0)
	usw	$3, 0xffff0000($0)
	usw	$3, 0xffff8000($0)
	usw	$3, 0xffff0001($0)
	usw	$3, 0xffff8001($0)
	usw	$3, 0xf0000000($0)
	usw	$3, 0xffffffff($0)
	usw	$3, 0x12345678($0)
	usw	$3, 0($4)
	usw	$3, 4($4)
	usw	$3, 2047($4)
	usw	$3, -2048($4)
	usw	$3, 2048($4)
	usw	$3, -2049($4)
	usw	$3, 32763($4)
	usw	$3, -32768($4)
	usw	$3, 65535($4)
	usw	$3, 0xffff0000($4)
	usw	$3, 0xffff8000($4)
	usw	$3, 0xffff0001($4)
	usw	$3, 0xffff8001($4)
	usw	$3, 0xf0000000($4)
	usw	$3, 0xffffffff($4)
	usw	$3, 0x12345678($4)

	wait
	wait	0
	wait	1
	wait	255

	wrpgpr	$2, $3
	wrpgpr	$2, $4
	wrpgpr	$2, $2
	wrpgpr	$2

	wsbh	$2, $3
	wsbh	$2, $4
	wsbh	$2, $2
	wsbh	$2

	xor	$2, $2
	xor	$2, $3
	xor	$2, $4
	xor	$2, $5
	xor	$2, $6
	xor	$2, $7
	xor	$2, $16
	xor	$2, $17
	xor	$3, $17
	xor	$4, $17
	xor	$5, $17
	xor	$6, $17
	xor	$7, $17
	xor	$16, $17
	xor	$17, $17
	xor	$2, $3
	xor	$2, $2, $3
	xor	$2, $3, $2

	xor32	$2, $3, $4
	xor32	$29, $30, $31
	xor32	$2, $2, $4
	xor32	$2, $4

	xor	$2, $3, 32768
	xor	$2, $3, 65535
	xor	$2, $3, 65536
	xor	$2, $3, -32768
	xor	$2, $3, -32769

	xori	$3, $4, 0
	xori	$3, $4, 32767
	xori	$3, $4, 65535
	xori	$3, $3, 65535
	xori	$3, 65535

	.set	noreorder

	beqz	$9, test
	addu	$3, $4, $5

	beq	$9, $10, test
	addu	$3, $4, $5

	beq	$9, 0, test
	addu	$3, $4, $5

	beq	$9, 1, test
	addu	$3, $4, $5

	bge	$10, $0, test
	addu	$3, $4, $5

	bge	$10, $0, test
	addu	$3, $4, $5

	bge	$0, $10, test
	addu	$3, $4, $5

	bge	$10, $11, test
	addu	$3, $4, $5

	bge	$10, 0, test
	addu	$3, $4, $5

	bge	$10, 1, test
	addu	$3, $4, $5

	bge	$10, 2, test
	addu	$3, $4, $5

	bge	$10, 0x80000000, test
	addu	$3, $4, $5

	bgeu	$2, $0, test
	addu	$3, $4, $5

	bgeu	$0, $2, test
	addu	$3, $4, $5

	bgeu	$2, $3, test
	addu	$3, $4, $5

	bgeu	$2, 0, test
	addu	$3, $4, $5

	bgeu	$2, 1, test
	addu	$3, $4, $5

	bgeu	$2, 2, test
	addu	$3, $4, $5

	bgez	$2, test
	addu	$3, $4, $5

	bgezal	$2, test
	addu	$3, $4, $5

	bgt	$2, $0, test
	addu	$3, $4, $5

	bgt	$0, $2, test
	addu	$3, $4, $5

	bgt	$9, $10, test
	addu	$3, $4, $5

	bgt	$9, 0x7fffffff, test
	addu	$3, $4, $5

	bgt	$9, -1, test
	addu	$3, $4, $5

	bgt	$9, 0, test
	addu	$3, $4, $5

	bgt	$9, 1, test
	addu	$3, $4, $5

	bgt	$9, 0x80000000, test
	addu	$3, $4, $5

	bgtu	$9, $0, test
	addu	$3, $4, $5

	bgtu	$0, $9, test
	addu	$3, $4, $5

	bgtu	$9, $10, test
	addu	$3, $4, $5

	bgtu	$0, 0, test
	addu	$3, $4, $5

	bgtu	$9, 0xffffffff, test
	addu	$3, $4, $5

	bgtu	$9, -1, test
	addu	$3, $4, $5

	bgtu	$9, 0, test
	addu	$3, $4, $5

	bgtu	$9, 1, test
	addu	$3, $4, $5

	bgtz	$9, test
	addu	$3, $4, $5

	ble	$9, $0, test
	addu	$3, $4, $5

	ble	$0, $10, test
	addu	$3, $4, $5

	ble	$9, $10, test
	addu	$3, $4, $5

	ble	$9, 0x7fffffff, test
	addu	$3, $4, $5

	ble	$9, -1, test
	addu	$3, $4, $5

	ble	$9, 0, test
	addu	$3, $4, $5

	ble	$9, 1, test
	addu	$3, $4, $5

	bleu	$9, $0, test
	addu	$3, $4, $5

	bleu	$0, $10, test
	addu	$3, $4, $5

	bleu	$9, $10, test
	addu	$3, $4, $5

	bleu	$0, $10, test
	addu	$3, $4, $5

	bleu	$9, 0xffffffff, test
	addu	$3, $4, $5

	bleu	$9, 0, test
	addu	$3, $4, $5

	bleu	$9, 1, test
	addu	$3, $4, $5

	blez	$9, test
	addu	$3, $4, $5

	blt	$9, $0, test
	addu	$3, $4, $5

	blt	$0, $10, test
	addu	$3, $4, $5

	blt	$9, $10, test
	addu	$3, $4, $5

	blt	$9, 0, test
	addu	$3, $4, $5

	blt	$9, 1, test
	addu	$3, $4, $5

	blt	$9, 2, test
	addu	$3, $4, $5

	bltu	$9, $0, test
	addu	$3, $4, $5

	bltu	$0, $10, test
	addu	$3, $4, $5

	bltu	$9, $10, test
	addu	$3, $4, $5

	bltu	$9, 0, test
	addu	$3, $4, $5

	bltu	$9, 1, test
	addu	$3, $4, $5

	bltu	$9, 2, test
	addu	$3, $4, $5

	bltz	$9, test
	addu	$3, $4, $5

	bltzal	$9, test
	addu	$3, $4, $5

	bnez	$9, test
	addu	$3, $4, $5

	bne	$9, $10, test
	addu	$3, $4, $5

	bne	$9, 0, test
	addu	$3, $4, $5

	bne	$9, 1, test
	addu	$3, $4, $5

	beqzl	$9, test
	addu	$3, $4, $5

	beql	$9, $10, test
	addu	$3, $4, $5

	beql	$9, 0, test
	addu	$3, $4, $5

	beql	$9, 1, test
	addu	$3, $4, $5

	bgel	$10, $0, test
	addu	$3, $4, $5

	bgel	$10, $0, test
	addu	$3, $4, $5

	bgel	$0, $10, test
	addu	$3, $4, $5

	bgel	$10, $11, test
	addu	$3, $4, $5

	bgel	$10, 0, test
	addu	$3, $4, $5

	bgel	$10, 1, test
	addu	$3, $4, $5

	bgel	$10, 2, test
	addu	$3, $4, $5

	bgel	$10, 0x80000000, test
	addu	$3, $4, $5

	bgeul	$2, $0, test
	addu	$3, $4, $5

	bgeul	$0, $2, test
	addu	$3, $4, $5

	bgeul	$2, $3, test
	addu	$3, $4, $5

	bgeul	$2, 0, test
	addu	$3, $4, $5

	bgeul	$2, 1, test
	addu	$3, $4, $5

	bgeul	$2, 2, test
	addu	$3, $4, $5

	bgezl	$2, test
	addu	$3, $4, $5

	bgezall	$2, test
	addu	$3, $4, $5

	bgtl	$2, $0, test
	addu	$3, $4, $5

	bgtl	$0, $2, test
	addu	$3, $4, $5

	bgtl	$9, $10, test
	addu	$3, $4, $5

	bgtl	$9, 0x7fffffff, test
	addu	$3, $4, $5

	bgtl	$9, -1, test
	addu	$3, $4, $5

	bgtl	$9, 0, test
	addu	$3, $4, $5

	bgtl	$9, 1, test
	addu	$3, $4, $5

	bgtl	$9, 0x80000000, test
	addu	$3, $4, $5

	bgtul	$9, $0, test
	addu	$3, $4, $5

	bgtul	$0, $9, test
	addu	$3, $4, $5

	bgtul	$9, $10, test
	addu	$3, $4, $5

	bgtul	$0, 0, test
	addu	$3, $4, $5

	bgtul	$9, 0xffffffff, test
	addu	$3, $4, $5

	bgtul	$9, -1, test
	addu	$3, $4, $5

	bgtul	$9, 0, test
	addu	$3, $4, $5

	bgtul	$9, 1, test
	addu	$3, $4, $5

	bgtzl	$9, test
	addu	$3, $4, $5

	blel	$9, $0, test
	addu	$3, $4, $5

	blel	$0, $10, test
	addu	$3, $4, $5

	blel	$9, $10, test
	addu	$3, $4, $5

	blel	$9, 0x7fffffff, test
	addu	$3, $4, $5

	blel	$9, -1, test
	addu	$3, $4, $5

	blel	$9, 0, test
	addu	$3, $4, $5

	blel	$9, 1, test
	addu	$3, $4, $5

	bleul	$9, $0, test
	addu	$3, $4, $5

	bleul	$0, $10, test
	addu	$3, $4, $5

	bleul	$9, $10, test
	addu	$3, $4, $5

	bleul	$0, $10, test
	addu	$3, $4, $5

	bleul	$9, 0xffffffff, test
	addu	$3, $4, $5

	bleul	$9, 0, test
	addu	$3, $4, $5

	bleul	$9, 1, test
	addu	$3, $4, $5

	blezl	$9, test
	addu	$3, $4, $5

	bltl	$9, $0, test
	addu	$3, $4, $5

	bltl	$0, $10, test
	addu	$3, $4, $5

	bltl	$9, $10, test
	addu	$3, $4, $5

	bltl	$9, 0, test
	addu	$3, $4, $5

	bltl	$9, 1, test
	addu	$3, $4, $5

	bltl	$9, 2, test
	addu	$3, $4, $5

	bltul	$9, $0, test
	addu	$3, $4, $5

	bltul	$0, $10, test
	addu	$3, $4, $5

	bltul	$9, $10, test
	addu	$3, $4, $5

	bltul	$9, 0, test
	addu	$3, $4, $5

	bltul	$9, 1, test
	addu	$3, $4, $5

	bltul	$9, 2, test
	addu	$3, $4, $5

	bltzl	$9, test
	addu	$3, $4, $5

	bltzall	$9, test
	addu	$3, $4, $5

	bnezl	$9, test
	addu	$3, $4, $5

	bnel	$9, $10, test
	addu	$3, $4, $5

	bnel	$9, 0, test
	addu	$3, $4, $5

	bnel	$9, 1, test
	addu	$3, $4, $5

	.ifndef	insn32
	addiur1sp	$2, 0
	addiur1sp	$2, 1<<2
	addiur1sp	$2, 2<<2
	addiur1sp	$2, 3<<2
	addiur1sp	$2, 4<<2
	addiur1sp	$2, 63<<2
	addiur1sp	$3, 63<<2
	addiur1sp	$4, 63<<2
	addiur1sp	$5, 63<<2
	addiur1sp	$6, 63<<2
	addiur1sp	$7, 63<<2
	addiur1sp	$16, 63<<2
	addiur1sp	$17, 63<<2

	addiur2	$2, $2, -1
	addiur2	$2, $3, -1
	addiur2	$2, $4, -1
	addiur2	$2, $5, -1
	addiur2	$2, $6, -1
	addiur2	$2, $7, -1
	addiur2	$2, $16, -1
	addiur2	$2, $17, -1
	addiur2	$3, $17, -1
	addiur2	$4, $17, -1
	addiur2	$5, $17, -1
	addiur2	$6, $17, -1
	addiur2	$7, $17, -1
	addiur2	$16, $17, -1
	addiur2	$17, $17, -1
	addiur2	$17, $17, 1
	addiur2	$17, $17, 4
	addiur2	$17, $17, 8
	addiur2	$17, $17, 12
	addiur2	$17, $17, 16
	addiur2	$17, $17, 20
	addiur2	$17, $17, 24

	addiusp	2 << 2
	addiusp	3 << 2
	addiusp	254 << 2
	addiusp	255 << 2
	addiusp	256 << 2
	addiusp	257 << 2
	addiusp	-3 << 2
	addiusp	-4 << 2
	addiusp	-255 << 2
	addiusp	-256 << 2
	addiusp	-257 << 2
	addiusp	-258 << 2

	addius5	$0, 0
	addius5	$2, 0
	addius5	$3, 0
	addius5	$30, 0
	addius5	$31, 0
	addius5	$31, 1
	addius5	$31, 2
	addius5	$31, 3
	addius5	$31, 7
	addius5	$31, -6
	addius5	$31, -7
	addius5	$31, -8
	.endif

	sd	$3, 4
	sd	$3, 4($0)
	sd	$3, 32767($0)
	sd	$3, -32768($0)
	sd	$3, 65535($0)
	sd	$3, 0xffff0000($0)
	sd	$3, 0xffff8000($0)
	sd	$3, 0xffff0001($0)
	sd	$3, 0xffff8001($0)
	sd	$3, 0xf0000000($0)
	sd	$3, 0xffffffff($0)
	sd	$3, 0x12345678($0)
	sd	$3, ($4)
	sd	$3, 0($4)
	sd	$3, 32767($4)
	sd	$3, -32768($4)
	sd	$3, 65535($4)
	sd	$3, 0xffff0000($4)
	sd	$3, 0xffff8000($4)
	sd	$3, 0xffff0001($4)
	sd	$3, 0xffff8001($4)
	sd	$3, 0xf0000000($4)
	sd	$3, 0xffffffff($4)
	sd	$3, 0x12345678($4)

	ld	$3, 4
	ld	$3, 4($0)
	ld	$3, 32767($0)
	ld	$3, -32768($0)
	ld	$3, 65535($0)
	ld	$3, 0xffff0000($0)
	ld	$3, 0xffff8000($0)
	ld	$3, 0xffff0001($0)
	ld	$3, 0xffff8001($0)
	ld	$3, 0xf0000000($0)
	ld	$3, 0xffffffff($0)
	ld	$3, 0x12345678($0)
	ld	$3, ($4)
	ld	$3, 0($4)
	ld	$3, 32767($4)
	ld	$3, -32768($4)
	ld	$3, 65535($4)
	ld	$3, 0xffff0000($4)
	ld	$3, 0xffff8000($4)
	ld	$3, 0xffff0001($4)
	ld	$3, 0xffff8001($4)
	ld	$3, 0xf0000000($4)
	ld	$3, 0xffffffff($4)
	ld	$3, 0x12345678($4)

	jraddiusp	0 << 2
	jraddiusp	1 << 2
	jraddiusp	2 << 2
	jraddiusp	3 << 2
	jraddiusp	4 << 2
	jraddiusp	5 << 2
	jraddiusp	6 << 2
	jraddiusp	7 << 2
	jraddiusp	8 << 2
	jraddiusp	9 << 2
	jraddiusp	10 << 2
	jraddiusp	30 << 2
	jraddiusp	31 << 2

	ldc2	$3, 0
	ldc2	$3, ($0)
	ldc2	$3, 4
	ldc2	$3, 4($0)
	ldc2	$3, ($4)
	ldc2	$3, 0($4)
	ldc2	$3, 32767($4)
	ldc2	$3, -32768($4)
	ldc2	$3, 65535($4)
	ldc2	$3, 0xffff0000($4)
	ldc2	$3, 0xffff8000($4)
	ldc2	$3, 0xffff0001($4)
	ldc2	$3, 0xffff8001($4)
	ldc2	$3, 0xf0000000($4)
	ldc2	$3, 0xffffffff($4)
	ldc2	$3, 0x12345678($4)

	lwc2	$3, 0
	lwc2	$3, ($0)
	lwc2	$3, 4
	lwc2	$3, 4($0)
	lwc2	$3, ($4)
	lwc2	$3, 0($4)
	lwc2	$3, 32767($4)
	lwc2	$3, -32768($4)
	lwc2	$3, 65535($4)
	lwc2	$3, 0xffff0000($4)
	lwc2	$3, 0xffff8000($4)
	lwc2	$3, 0xffff0001($4)
	lwc2	$3, 0xffff8001($4)
	lwc2	$3, 0xf0000000($4)
	lwc2	$3, 0xffffffff($4)
	lwc2	$3, 0x12345678($4)

	mfc2	$5, $0
	mfc2	$5, $1
	mfc2	$5, $2
	mfc2	$5, $3
	mfc2	$5, $4
	mfc2	$5, $5
	mfc2	$5, $6
	mfc2	$5, $7
	mfc2	$5, $8
	mfc2	$5, $9
	mfc2	$5, $10
	mfc2	$5, $11
	mfc2	$5, $12
	mfc2	$5, $13
	mfc2	$5, $14
	mfc2	$5, $15
	mfc2	$5, $16
	mfc2	$5, $17
	mfc2	$5, $18
	mfc2	$5, $19
	mfc2	$5, $20
	mfc2	$5, $21
	mfc2	$5, $22
	mfc2	$5, $23
	mfc2	$5, $24
	mfc2	$5, $25
	mfc2	$5, $26
	mfc2	$5, $27
	mfc2	$5, $28
	mfc2	$5, $29
	mfc2	$5, $30
	mfc2	$5, $31

	mfhc2	$5, $0
	mfhc2	$5, $1
	mfhc2	$5, $2
	mfhc2	$5, $3
	mfhc2	$5, $4
	mfhc2	$5, $5
	mfhc2	$5, $6
	mfhc2	$5, $7
	mfhc2	$5, $8
	mfhc2	$5, $9
	mfhc2	$5, $10
	mfhc2	$5, $11
	mfhc2	$5, $12
	mfhc2	$5, $13
	mfhc2	$5, $14
	mfhc2	$5, $15
	mfhc2	$5, $16
	mfhc2	$5, $17
	mfhc2	$5, $18
	mfhc2	$5, $19
	mfhc2	$5, $20
	mfhc2	$5, $21
	mfhc2	$5, $22
	mfhc2	$5, $23
	mfhc2	$5, $24
	mfhc2	$5, $25
	mfhc2	$5, $26
	mfhc2	$5, $27
	mfhc2	$5, $28
	mfhc2	$5, $29
	mfhc2	$5, $30
	mfhc2	$5, $31

	mtc2	$5, $0
	mtc2	$5, $1
	mtc2	$5, $2
	mtc2	$5, $3
	mtc2	$5, $4
	mtc2	$5, $5
	mtc2	$5, $6
	mtc2	$5, $7
	mtc2	$5, $8
	mtc2	$5, $9
	mtc2	$5, $10
	mtc2	$5, $11
	mtc2	$5, $12
	mtc2	$5, $13
	mtc2	$5, $14
	mtc2	$5, $15
	mtc2	$5, $16
	mtc2	$5, $17
	mtc2	$5, $18
	mtc2	$5, $19
	mtc2	$5, $20
	mtc2	$5, $21
	mtc2	$5, $22
	mtc2	$5, $23
	mtc2	$5, $24
	mtc2	$5, $25
	mtc2	$5, $26
	mtc2	$5, $27
	mtc2	$5, $28
	mtc2	$5, $29
	mtc2	$5, $30
	mtc2	$5, $31

	mthc2	$5, $0
	mthc2	$5, $1
	mthc2	$5, $2
	mthc2	$5, $3
	mthc2	$5, $4
	mthc2	$5, $5
	mthc2	$5, $6
	mthc2	$5, $7
	mthc2	$5, $8
	mthc2	$5, $9
	mthc2	$5, $10
	mthc2	$5, $11
	mthc2	$5, $12
	mthc2	$5, $13
	mthc2	$5, $14
	mthc2	$5, $15
	mthc2	$5, $16
	mthc2	$5, $17
	mthc2	$5, $18
	mthc2	$5, $19
	mthc2	$5, $20
	mthc2	$5, $21
	mthc2	$5, $22
	mthc2	$5, $23
	mthc2	$5, $24
	mthc2	$5, $25
	mthc2	$5, $26
	mthc2	$5, $27
	mthc2	$5, $28
	mthc2	$5, $29
	mthc2	$5, $30
	mthc2	$5, $31

	sdc2	$3, 0
	sdc2	$3, ($0)
	sdc2	$3, 4
	sdc2	$3, 4($0)
	sdc2	$3, ($4)
	sdc2	$3, 0($4)
	sdc2	$3, 32767($4)
	sdc2	$3, -32768($4)
	sdc2	$3, 65535($4)
	sdc2	$3, 0xffff0000($4)
	sdc2	$3, 0xffff8000($4)
	sdc2	$3, 0xffff0001($4)
	sdc2	$3, 0xffff8001($4)
	sdc2	$3, 0xf0000000($4)
	sdc2	$3, 0xffffffff($4)
	sdc2	$3, 0x12345678($4)

	swc2	$3, 0
	swc2	$3, ($0)
	swc2	$3, 4
	swc2	$3, 4($0)
	swc2	$3, ($4)
	swc2	$3, 0($4)
	swc2	$3, 32767($4)
	swc2	$3, -32768($4)
	swc2	$3, 65535($4)
	swc2	$3, 0xffff0000($4)
	swc2	$3, 0xffff8000($4)
	swc2	$3, 0xffff0001($4)
	swc2	$3, 0xffff8001($4)
	swc2	$3, 0xf0000000($4)
	swc2	$3, 0xffffffff($4)
	swc2	$3, 0x12345678($4)

	cache	0, %lo(test)($3)
	lwp	$2, %lo(test)($3)
	swp	$2, %lo(test)($3)
	ll	$2, %lo(test)($3)
	sc	$2, %lo(test)($3)
	lwl	$2, %lo(test)($3)
	lwr	$2, %lo(test)($3)
	swl	$2, %lo(test)($3)
	swr	$2, %lo(test)($3)
	lwm	$16, %lo(test)($3)
	swm	$16, %lo(test)($3)
	lwc2	$16, %lo(test)($3)
	swc2	$16, %lo(test)($3)
	lcache	$2, %lo(test)($3)
	flush	$2, %lo(test)($3)
	scache	$2, %lo(test)($3)
	invalidate	$2, %lo(test)($3)

	sdbbp	1023
	wait	1023
	syscall	1023
	cop2	0x7fffff

	.end	test
	.set	reorder

	.align	3
	.set	micromips
	.ent	fp_test
	.globl	fp_test
fp_test:
	prefx	0, $0($0)
	prefx	0, $0($2)
	prefx	0, $0($31)
	prefx	0, $2($31)
	prefx	0, $31($31)
	prefx	1, $31($31)
	prefx	2, $31($31)
	prefx	31, $31($31)

	abs.s	$f0, $f1
	abs.s	$f30, $f31
	abs.s	$f2, $f2
	abs.s	$f2
	abs.d	$f0, $f1
	abs.d	$f30, $f31
	abs.d	$f2, $f2
	abs.d	$f2
	abs.ps	$f0, $f1
	abs.ps	$f30, $f31
	abs.ps	$f2, $f2
	abs.ps	$f2

	add.s	$f0, $f1, $f2
	add.s	$f29, $f30, $f31
	add.s	$f29, $f29, $f30
	add.s	$f29, $f30
	add.d	$f0, $f1, $f2
	add.d	$f29, $f30, $f31
	add.d	$f29, $f29, $f30
	add.d	$f29, $f30
	add.ps	$f0, $f1, $f2
	add.ps	$f29, $f30, $f31
	add.ps	$f29, $f29, $f30
	add.ps	$f29, $f30

	alnv.ps	$f0, $f1, $f2, $0
	alnv.ps	$f0, $f1, $f2, $2
	alnv.ps	$f0, $f1, $f2, $31
	alnv.ps	$f29, $f30, $f31, $31
	alnv.ps	$f29, $f29, $f31, $31

	bc1f	fp_test
	bc1f	$fcc0, fp_test
	bc1f	$fcc1, fp_test
	bc1f	$fcc2, fp_test
	bc1f	$fcc3, fp_test
	bc1f	$fcc4, fp_test
	bc1f	$fcc5, fp_test
	bc1f	$fcc6, fp_test
	bc1f	$fcc7, fp_test

	bc1t	fp_test
	bc1t	$fcc0, fp_test
	bc1t	$fcc1, fp_test
	bc1t	$fcc2, fp_test
	bc1t	$fcc3, fp_test
	bc1t	$fcc4, fp_test
	bc1t	$fcc5, fp_test
	bc1t	$fcc6, fp_test
	bc1t	$fcc7, fp_test

	c.f.d	$f0, $f1
	c.f.d	$f30, $f31
	c.f.d	$fcc0, $f30, $f31
	c.f.d	$fcc1, $f30, $f31
	c.f.d	$fcc7, $f30, $f31
	c.f.s	$f0, $f1
	c.f.s	$f30, $f31
	c.f.s	$fcc0, $f30, $f31
	c.f.s	$fcc1, $f30, $f31
	c.f.s	$fcc7, $f30, $f31
	c.f.ps	$f0, $f1
	c.f.ps	$f30, $f31
	c.f.ps	$fcc0, $f30, $f31
	c.f.ps	$fcc2, $f30, $f31
	c.f.ps	$fcc6, $f30, $f31

	c.un.d	$f0, $f1
	c.un.d	$f30, $f31
	c.un.d	$fcc0, $f30, $f31
	c.un.d	$fcc1, $f30, $f31
	c.un.d	$fcc7, $f30, $f31
	c.un.s	$f0, $f1
	c.un.s	$f30, $f31
	c.un.s	$fcc0, $f30, $f31
	c.un.s	$fcc1, $f30, $f31
	c.un.s	$fcc7, $f30, $f31
	c.un.ps	$f0, $f1
	c.un.ps	$f30, $f31
	c.un.ps	$fcc0, $f30, $f31
	c.un.ps	$fcc2, $f30, $f31
	c.un.ps	$fcc6, $f30, $f31

	c.eq.d	$f0, $f1
	c.eq.d	$f30, $f31
	c.eq.d	$fcc0, $f30, $f31
	c.eq.d	$fcc1, $f30, $f31
	c.eq.d	$fcc7, $f30, $f31
	c.eq.s	$f0, $f1
	c.eq.s	$f30, $f31
	c.eq.s	$fcc0, $f30, $f31
	c.eq.s	$fcc1, $f30, $f31
	c.eq.s	$fcc7, $f30, $f31
	c.eq.ps	$f0, $f1
	c.eq.ps	$f30, $f31
	c.eq.ps	$fcc0, $f30, $f31
	c.eq.ps	$fcc2, $f30, $f31
	c.eq.ps	$fcc6, $f30, $f31

	c.ueq.d	$f0, $f1
	c.ueq.d	$f30, $f31
	c.ueq.d	$fcc0, $f30, $f31
	c.ueq.d	$fcc1, $f30, $f31
	c.ueq.d	$fcc7, $f30, $f31
	c.ueq.s	$f0, $f1
	c.ueq.s	$f30, $f31
	c.ueq.s	$fcc0, $f30, $f31
	c.ueq.s	$fcc1, $f30, $f31
	c.ueq.s	$fcc7, $f30, $f31
	c.ueq.ps	$f0, $f1
	c.ueq.ps	$f30, $f31
	c.ueq.ps	$fcc0, $f30, $f31
	c.ueq.ps	$fcc2, $f30, $f31
	c.ueq.ps	$fcc6, $f30, $f31

	c.olt.d	$f0, $f1
	c.olt.d	$f30, $f31
	c.olt.d	$fcc0, $f30, $f31
	c.olt.d	$fcc1, $f30, $f31
	c.olt.d	$fcc7, $f30, $f31
	c.olt.s	$f0, $f1
	c.olt.s	$f30, $f31
	c.olt.s	$fcc0, $f30, $f31
	c.olt.s	$fcc1, $f30, $f31
	c.olt.s	$fcc7, $f30, $f31
	c.olt.ps	$f0, $f1
	c.olt.ps	$f30, $f31
	c.olt.ps	$fcc0, $f30, $f31
	c.olt.ps	$fcc2, $f30, $f31
	c.olt.ps	$fcc6, $f30, $f31

	c.ult.d	$f0, $f1
	c.ult.d	$f30, $f31
	c.ult.d	$fcc0, $f30, $f31
	c.ult.d	$fcc1, $f30, $f31
	c.ult.d	$fcc7, $f30, $f31
	c.ult.s	$f0, $f1
	c.ult.s	$f30, $f31
	c.ult.s	$fcc0, $f30, $f31
	c.ult.s	$fcc1, $f30, $f31
	c.ult.s	$fcc7, $f30, $f31
	c.ult.ps	$f0, $f1
	c.ult.ps	$f30, $f31
	c.ult.ps	$fcc0, $f30, $f31
	c.ult.ps	$fcc2, $f30, $f31
	c.ult.ps	$fcc6, $f30, $f31

	c.ole.d	$f0, $f1
	c.ole.d	$f30, $f31
	c.ole.d	$fcc0, $f30, $f31
	c.ole.d	$fcc1, $f30, $f31
	c.ole.d	$fcc7, $f30, $f31
	c.ole.s	$f0, $f1
	c.ole.s	$f30, $f31
	c.ole.s	$fcc0, $f30, $f31
	c.ole.s	$fcc1, $f30, $f31
	c.ole.s	$fcc7, $f30, $f31
	c.ole.ps	$f0, $f1
	c.ole.ps	$f30, $f31
	c.ole.ps	$fcc0, $f30, $f31
	c.ole.ps	$fcc2, $f30, $f31
	c.ole.ps	$fcc6, $f30, $f31

	c.ule.d	$f0, $f1
	c.ule.d	$f30, $f31
	c.ule.d	$fcc0, $f30, $f31
	c.ule.d	$fcc1, $f30, $f31
	c.ule.d	$fcc7, $f30, $f31
	c.ule.s	$f0, $f1
	c.ule.s	$f30, $f31
	c.ule.s	$fcc0, $f30, $f31
	c.ule.s	$fcc1, $f30, $f31
	c.ule.s	$fcc7, $f30, $f31
	c.ule.ps	$f0, $f1
	c.ule.ps	$f30, $f31
	c.ule.ps	$fcc0, $f30, $f31
	c.ule.ps	$fcc2, $f30, $f31
	c.ule.ps	$fcc6, $f30, $f31

	c.sf.d	$f0, $f1
	c.sf.d	$f30, $f31
	c.sf.d	$fcc0, $f30, $f31
	c.sf.d	$fcc1, $f30, $f31
	c.sf.d	$fcc7, $f30, $f31
	c.sf.s	$f0, $f1
	c.sf.s	$f30, $f31
	c.sf.s	$fcc0, $f30, $f31
	c.sf.s	$fcc1, $f30, $f31
	c.sf.s	$fcc7, $f30, $f31
	c.sf.ps	$f0, $f1
	c.sf.ps	$f30, $f31
	c.sf.ps	$fcc0, $f30, $f31
	c.sf.ps	$fcc2, $f30, $f31
	c.sf.ps	$fcc6, $f30, $f31

	c.ngle.d	$f0, $f1
	c.ngle.d	$f30, $f31
	c.ngle.d	$fcc0, $f30, $f31
	c.ngle.d	$fcc1, $f30, $f31
	c.ngle.d	$fcc7, $f30, $f31
	c.ngle.s	$f0, $f1
	c.ngle.s	$f30, $f31
	c.ngle.s	$fcc0, $f30, $f31
	c.ngle.s	$fcc1, $f30, $f31
	c.ngle.s	$fcc7, $f30, $f31
	c.ngle.ps	$f0, $f1
	c.ngle.ps	$f30, $f31
	c.ngle.ps	$fcc0, $f30, $f31
	c.ngle.ps	$fcc2, $f30, $f31
	c.ngle.ps	$fcc6, $f30, $f31

	c.seq.d	$f0, $f1
	c.seq.d	$f30, $f31
	c.seq.d	$fcc0, $f30, $f31
	c.seq.d	$fcc1, $f30, $f31
	c.seq.d	$fcc7, $f30, $f31
	c.seq.s	$f0, $f1
	c.seq.s	$f30, $f31
	c.seq.s	$fcc0, $f30, $f31
	c.seq.s	$fcc1, $f30, $f31
	c.seq.s	$fcc7, $f30, $f31
	c.seq.ps	$f0, $f1
	c.seq.ps	$f30, $f31
	c.seq.ps	$fcc0, $f30, $f31
	c.seq.ps	$fcc2, $f30, $f31
	c.seq.ps	$fcc6, $f30, $f31

	c.ngl.d	$f0, $f1
	c.ngl.d	$f30, $f31
	c.ngl.d	$fcc0, $f30, $f31
	c.ngl.d	$fcc1, $f30, $f31
	c.ngl.d	$fcc7, $f30, $f31
	c.ngl.s	$f0, $f1
	c.ngl.s	$f30, $f31
	c.ngl.s	$fcc0, $f30, $f31
	c.ngl.s	$fcc1, $f30, $f31
	c.ngl.s	$fcc7, $f30, $f31
	c.ngl.ps	$f0, $f1
	c.ngl.ps	$f30, $f31
	c.ngl.ps	$fcc0, $f30, $f31
	c.ngl.ps	$fcc2, $f30, $f31
	c.ngl.ps	$fcc6, $f30, $f31

	c.lt.d	$f0, $f1
	c.lt.d	$f30, $f31
	c.lt.d	$fcc0, $f30, $f31
	c.lt.d	$fcc1, $f30, $f31
	c.lt.d	$fcc7, $f30, $f31
	c.lt.s	$f0, $f1
	c.lt.s	$f30, $f31
	c.lt.s	$fcc0, $f30, $f31
	c.lt.s	$fcc1, $f30, $f31
	c.lt.s	$fcc7, $f30, $f31
	c.lt.ps	$f0, $f1
	c.lt.ps	$f30, $f31
	c.lt.ps	$fcc0, $f30, $f31
	c.lt.ps	$fcc2, $f30, $f31
	c.lt.ps	$fcc6, $f30, $f31

	c.nge.d	$f0, $f1
	c.nge.d	$f30, $f31
	c.nge.d	$fcc0, $f30, $f31
	c.nge.d	$fcc1, $f30, $f31
	c.nge.d	$fcc7, $f30, $f31
	c.nge.s	$f0, $f1
	c.nge.s	$f30, $f31
	c.nge.s	$fcc0, $f30, $f31
	c.nge.s	$fcc1, $f30, $f31
	c.nge.s	$fcc7, $f30, $f31
	c.nge.ps	$f0, $f1
	c.nge.ps	$f30, $f31
	c.nge.ps	$fcc0, $f30, $f31
	c.nge.ps	$fcc2, $f30, $f31
	c.nge.ps	$fcc6, $f30, $f31

	c.le.d	$f0, $f1
	c.le.d	$f30, $f31
	c.le.d	$fcc0, $f30, $f31
	c.le.d	$fcc1, $f30, $f31
	c.le.d	$fcc7, $f30, $f31
	c.le.s	$f0, $f1
	c.le.s	$f30, $f31
	c.le.s	$fcc0, $f30, $f31
	c.le.s	$fcc1, $f30, $f31
	c.le.s	$fcc7, $f30, $f31
	c.le.ps	$f0, $f1
	c.le.ps	$f30, $f31
	c.le.ps	$fcc0, $f30, $f31
	c.le.ps	$fcc2, $f30, $f31
	c.le.ps	$fcc6, $f30, $f31

	c.ngt.d	$f0, $f1
	c.ngt.d	$f30, $f31
	c.ngt.d	$fcc0, $f30, $f31
	c.ngt.d	$fcc1, $f30, $f31
	c.ngt.d	$fcc7, $f30, $f31
	c.ngt.s	$f0, $f1
	c.ngt.s	$f30, $f31
	c.ngt.s	$fcc0, $f30, $f31
	c.ngt.s	$fcc1, $f30, $f31
	c.ngt.s	$fcc7, $f30, $f31
	c.ngt.ps	$f0, $f1
	c.ngt.ps	$f30, $f31
	c.ngt.ps	$fcc0, $f30, $f31
	c.ngt.ps	$fcc2, $f30, $f31
	c.ngt.ps	$fcc6, $f30, $f31

	ceil.l.d	$f0, $f1
	ceil.l.d	$f30, $f31
	ceil.l.d	$f2, $f2

	ceil.l.s	$f0, $f1
	ceil.l.s	$f30, $f31
	ceil.l.s	$f2, $f2

	ceil.w.d	$f0, $f1
	ceil.w.d	$f30, $f31
	ceil.w.d	$f2, $f2

	ceil.w.s	$f0, $f1
	ceil.w.s	$f30, $f31
	ceil.w.s	$f2, $f2

	cfc1	$5, $0
	cfc1	$5, $1
	cfc1	$5, $2
	cfc1	$5, $3
	cfc1	$5, $4
	cfc1	$5, $5
	cfc1	$5, $6
	cfc1	$5, $7
	cfc1	$5, $8
	cfc1	$5, $9
	cfc1	$5, $10
	cfc1	$5, $11
	cfc1	$5, $12
	cfc1	$5, $13
	cfc1	$5, $14
	cfc1	$5, $15
	cfc1	$5, $16
	cfc1	$5, $17
	cfc1	$5, $18
	cfc1	$5, $19
	cfc1	$5, $20
	cfc1	$5, $21
	cfc1	$5, $22
	cfc1	$5, $23
	cfc1	$5, $24
	cfc1	$5, $25
	cfc1	$5, $26
	cfc1	$5, $27
	cfc1	$5, $28
	cfc1	$5, $29
	cfc1	$5, $30
	cfc1	$5, $31
	cfc1	$5, $f0
	cfc1	$5, $f1
	cfc1	$5, $f2
	cfc1	$5, $f3
	cfc1	$5, $f4
	cfc1	$5, $f5
	cfc1	$5, $f6
	cfc1	$5, $f7
	cfc1	$5, $f8
	cfc1	$5, $f9
	cfc1	$5, $f10
	cfc1	$5, $f11
	cfc1	$5, $f12
	cfc1	$5, $f13
	cfc1	$5, $f14
	cfc1	$5, $f15
	cfc1	$5, $f16
	cfc1	$5, $f17
	cfc1	$5, $f18
	cfc1	$5, $f19
	cfc1	$5, $f20
	cfc1	$5, $f21
	cfc1	$5, $f22
	cfc1	$5, $f23
	cfc1	$5, $f24
	cfc1	$5, $f25
	cfc1	$5, $f26
	cfc1	$5, $f27
	cfc1	$5, $f28
	cfc1	$5, $f29
	cfc1	$5, $f30
	cfc1	$5, $f31

	cfc2	$5, $0
	cfc2	$5, $1
	cfc2	$5, $2
	cfc2	$5, $3
	cfc2	$5, $4
	cfc2	$5, $5
	cfc2	$5, $6
	cfc2	$5, $7
	cfc2	$5, $8
	cfc2	$5, $9
	cfc2	$5, $10
	cfc2	$5, $11
	cfc2	$5, $12
	cfc2	$5, $13
	cfc2	$5, $14
	cfc2	$5, $15
	cfc2	$5, $16
	cfc2	$5, $17
	cfc2	$5, $18
	cfc2	$5, $19
	cfc2	$5, $20
	cfc2	$5, $21
	cfc2	$5, $22
	cfc2	$5, $23
	cfc2	$5, $24
	cfc2	$5, $25
	cfc2	$5, $26
	cfc2	$5, $27
	cfc2	$5, $28
	cfc2	$5, $29
	cfc2	$5, $30
	cfc2	$5, $31

	ctc1	$5, $0
	ctc1	$5, $1
	ctc1	$5, $2
	ctc1	$5, $3
	ctc1	$5, $4
	ctc1	$5, $5
	ctc1	$5, $6
	ctc1	$5, $7
	ctc1	$5, $8
	ctc1	$5, $9
	ctc1	$5, $10
	ctc1	$5, $11
	ctc1	$5, $12
	ctc1	$5, $13
	ctc1	$5, $14
	ctc1	$5, $15
	ctc1	$5, $16
	ctc1	$5, $17
	ctc1	$5, $18
	ctc1	$5, $19
	ctc1	$5, $20
	ctc1	$5, $21
	ctc1	$5, $22
	ctc1	$5, $23
	ctc1	$5, $24
	ctc1	$5, $25
	ctc1	$5, $26
	ctc1	$5, $27
	ctc1	$5, $28
	ctc1	$5, $29
	ctc1	$5, $30
	ctc1	$5, $31
	ctc1	$5, $f0
	ctc1	$5, $f1
	ctc1	$5, $f2
	ctc1	$5, $f3
	ctc1	$5, $f4
	ctc1	$5, $f5
	ctc1	$5, $f6
	ctc1	$5, $f7
	ctc1	$5, $f8
	ctc1	$5, $f9
	ctc1	$5, $f10
	ctc1	$5, $f11
	ctc1	$5, $f12
	ctc1	$5, $f13
	ctc1	$5, $f14
	ctc1	$5, $f15
	ctc1	$5, $f16
	ctc1	$5, $f17
	ctc1	$5, $f18
	ctc1	$5, $f19
	ctc1	$5, $f20
	ctc1	$5, $f21
	ctc1	$5, $f22
	ctc1	$5, $f23
	ctc1	$5, $f24
	ctc1	$5, $f25
	ctc1	$5, $f26
	ctc1	$5, $f27
	ctc1	$5, $f28
	ctc1	$5, $f29
	ctc1	$5, $f30
	ctc1	$5, $f31

	ctc2	$5, $0
	ctc2	$5, $1
	ctc2	$5, $2
	ctc2	$5, $3
	ctc2	$5, $4
	ctc2	$5, $5
	ctc2	$5, $6
	ctc2	$5, $7
	ctc2	$5, $8
	ctc2	$5, $9
	ctc2	$5, $10
	ctc2	$5, $11
	ctc2	$5, $12
	ctc2	$5, $13
	ctc2	$5, $14
	ctc2	$5, $15
	ctc2	$5, $16
	ctc2	$5, $17
	ctc2	$5, $18
	ctc2	$5, $19
	ctc2	$5, $20
	ctc2	$5, $21
	ctc2	$5, $22
	ctc2	$5, $23
	ctc2	$5, $24
	ctc2	$5, $25
	ctc2	$5, $26
	ctc2	$5, $27
	ctc2	$5, $28
	ctc2	$5, $29
	ctc2	$5, $30
	ctc2	$5, $31

	cvt.d.l	$f0, $f1
	cvt.d.l	$f30, $f31
	cvt.d.l	$f2, $f2

	cvt.d.s	$f0, $f1
	cvt.d.s	$f30, $f31
	cvt.d.s	$f2, $f2

	cvt.d.w	$f0, $f1
	cvt.d.w	$f30, $f31
	cvt.d.w	$f2, $f2

	cvt.l.s	$f0, $f1
	cvt.l.s	$f30, $f31
	cvt.l.s	$f2, $f2

	cvt.l.d	$f0, $f1
	cvt.l.d	$f30, $f31
	cvt.l.d	$f2, $f2

	cvt.s.l	$f0, $f1
	cvt.s.l	$f30, $f31
	cvt.s.l	$f2, $f2

	cvt.s.d	$f0, $f1
	cvt.s.d	$f30, $f31
	cvt.s.d	$f2, $f2

	cvt.s.w	$f0, $f1
	cvt.s.w	$f30, $f31
	cvt.s.w	$f2, $f2

	cvt.s.pl	$f0, $f1
	cvt.s.pl	$f30, $f31
	cvt.s.pl	$f2, $f2

	cvt.s.pu	$f0, $f1
	cvt.s.pu	$f30, $f31
	cvt.s.pu	$f2, $f2

	cvt.w.s	$f0, $f1
	cvt.w.s	$f30, $f31
	cvt.w.s	$f2, $f2

	cvt.w.d	$f0, $f1
	cvt.w.d	$f30, $f31
	cvt.w.d	$f2, $f2

	cvt.ps.s	$f0, $f1, $f2
	cvt.ps.s	$f29, $f30, $f31
	cvt.ps.s	$f29, $f29, $f31
	cvt.ps.s	$f29, $f31

	div.d	$f0, $f1, $f2
	div.d	$f29, $f30, $f31
	div.d	$f29, $f29, $f30
	div.d	$f29, $f30

	div.s	$f0, $f1, $f2
	div.s	$f29, $f30, $f31
	div.s	$f29, $f29, $f30
	div.s	$f29, $f30

	floor.l.d	$f0, $f1
	floor.l.d	$f30, $f31
	floor.l.d	$f2, $f2

	floor.l.s	$f0, $f1
	floor.l.s	$f30, $f31
	floor.l.s	$f2, $f2

	floor.w.d	$f0, $f1
	floor.w.d	$f30, $f31
	floor.w.d	$f2, $f2

	floor.w.s	$f0, $f1
	floor.w.s	$f30, $f31
	floor.w.s	$f2, $f2

	ldc1	$3, 0
	ldc1	$3, ($0)
	ldc1	$3, 4
	ldc1	$3, 4($0)
	ldc1	$3, ($4)
	ldc1	$3, 0($4)
	ldc1	$3, 32767($4)
	ldc1	$3, -32768($4)
	ldc1	$3, 65535($4)
	ldc1	$3, 0xffff0000($4)
	ldc1	$3, 0xffff8000($4)
	ldc1	$3, 0xffff0001($4)
	ldc1	$3, 0xffff8001($4)
	ldc1	$3, 0xf0000000($4)
	ldc1	$3, 0xffffffff($4)
	ldc1	$3, 0x12345678($4)
	ldc1	$f3, 0
	ldc1	$f3, ($0)
	ldc1	$f3, 4
	ldc1	$f3, 4($0)
	ldc1	$f3, ($4)
	ldc1	$f3, 0($4)
	ldc1	$f3, 32767($4)
	ldc1	$f3, -32768($4)
	ldc1	$f3, 65535($4)
	ldc1	$f3, 0xffff0000($4)
	ldc1	$f3, 0xffff8000($4)
	ldc1	$f3, 0xffff0001($4)
	ldc1	$f3, 0xffff8001($4)
	ldc1	$f3, 0xf0000000($4)
	ldc1	$f3, 0xffffffff($4)
	ldc1	$f3, 0x12345678($4)

	l.d	$f3, 0
	l.d	$f3, ($0)
	l.d	$f3, 4
	l.d	$f3, 4($0)
	l.d	$f3, ($4)
	l.d	$f3, 0($4)
	l.d	$f3, 32767($4)
	l.d	$f3, -32768($4)

	ldxc1	$f0, $0($0)
	ldxc1	$f0, $0($2)
	ldxc1	$f0, $0($31)
	ldxc1	$f0, $2($31)
	ldxc1	$f0, $31($31)
	ldxc1	$f1, $31($31)
	ldxc1	$f2, $31($31)
	ldxc1	$f31, $31($31)

	luxc1	$f0, $0($0)
	luxc1	$f0, $0($2)
	luxc1	$f0, $0($31)
	luxc1	$f0, $2($31)
	luxc1	$f0, $31($31)
	luxc1	$f1, $31($31)
	luxc1	$f2, $31($31)
	luxc1	$f31, $31($31)

	lwc1	$3, 0
	lwc1	$3, ($0)
	lwc1	$3, 4
	lwc1	$3, 4($0)
	lwc1	$3, ($4)
	lwc1	$3, 0($4)
	lwc1	$3, 32767($4)
	lwc1	$3, -32768($4)
	lwc1	$3, 65535($4)
	lwc1	$3, 0xffff0000($4)
	lwc1	$3, 0xffff8000($4)
	lwc1	$3, 0xffff0001($4)
	lwc1	$3, 0xffff8001($4)
	lwc1	$3, 0xf0000000($4)
	lwc1	$3, 0xffffffff($4)
	lwc1	$3, 0x12345678($4)
	lwc1	$f3, 0
	lwc1	$f3, ($0)
	lwc1	$f3, 4
	lwc1	$f3, 4($0)
	lwc1	$f3, ($4)
	lwc1	$f3, 0($4)
	lwc1	$f3, 32767($4)
	lwc1	$f3, -32768($4)
	lwc1	$f3, 65535($4)
	lwc1	$f3, 0xffff0000($4)
	lwc1	$f3, 0xffff8000($4)
	lwc1	$f3, 0xffff0001($4)
	lwc1	$f3, 0xffff8001($4)
	lwc1	$f3, 0xf0000000($4)
	lwc1	$f3, 0xffffffff($4)
	lwc1	$f3, 0x12345678($4)

	l.s	$f3, 0
	l.s	$f3, ($0)
	l.s	$f3, 4
	l.s	$f3, 4($0)
	l.s	$f3, ($4)
	l.s	$f3, 0($4)
	l.s	$f3, 32767($4)
	l.s	$f3, -32768($4)
	l.s	$f3, 65535($4)
	l.s	$f3, 0xffff0000($4)
	l.s	$f3, 0xffff8000($4)
	l.s	$f3, 0xffff0001($4)
	l.s	$f3, 0xffff8001($4)
	l.s	$f3, 0xf0000000($4)
	l.s	$f3, 0xffffffff($4)
	l.s	$f3, 0x12345678($4)

	lwxc1	$f0, $0($0)
	lwxc1	$f0, $0($2)
	lwxc1	$f0, $0($31)
	lwxc1	$f0, $2($31)
	lwxc1	$f0, $31($31)
	lwxc1	$f1, $31($31)
	lwxc1	$f2, $31($31)
	lwxc1	$f31, $31($31)

	madd.d	$f0, $f1, $f2, $f3
	madd.d	$f28, $f29, $f30, $f31
	madd.s	$f0, $f1, $f2, $f3
	madd.s	$f28, $f29, $f30, $f31
	madd.ps	$f0, $f1, $f2, $f3
	madd.ps	$f28, $f29, $f30, $f31

	mfc1	$5, $0
	mfc1	$5, $1
	mfc1	$5, $2
	mfc1	$5, $3
	mfc1	$5, $4
	mfc1	$5, $5
	mfc1	$5, $6
	mfc1	$5, $7
	mfc1	$5, $8
	mfc1	$5, $9
	mfc1	$5, $10
	mfc1	$5, $11
	mfc1	$5, $12
	mfc1	$5, $13
	mfc1	$5, $14
	mfc1	$5, $15
	mfc1	$5, $16
	mfc1	$5, $17
	mfc1	$5, $18
	mfc1	$5, $19
	mfc1	$5, $20
	mfc1	$5, $21
	mfc1	$5, $22
	mfc1	$5, $23
	mfc1	$5, $24
	mfc1	$5, $25
	mfc1	$5, $26
	mfc1	$5, $27
	mfc1	$5, $28
	mfc1	$5, $29
	mfc1	$5, $30
	mfc1	$5, $31
	mfc1	$5, $f0
	mfc1	$5, $f1
	mfc1	$5, $f2
	mfc1	$5, $f3
	mfc1	$5, $f4
	mfc1	$5, $f5
	mfc1	$5, $f6
	mfc1	$5, $f7
	mfc1	$5, $f8
	mfc1	$5, $f9
	mfc1	$5, $f10
	mfc1	$5, $f11
	mfc1	$5, $f12
	mfc1	$5, $f13
	mfc1	$5, $f14
	mfc1	$5, $f15
	mfc1	$5, $f16
	mfc1	$5, $f17
	mfc1	$5, $f18
	mfc1	$5, $f19
	mfc1	$5, $f20
	mfc1	$5, $f21
	mfc1	$5, $f22
	mfc1	$5, $f23
	mfc1	$5, $f24
	mfc1	$5, $f25
	mfc1	$5, $f26
	mfc1	$5, $f27
	mfc1	$5, $f28
	mfc1	$5, $f29
	mfc1	$5, $f30
	mfc1	$5, $f31

	mfhc1	$5, $0
	mfhc1	$5, $1
	mfhc1	$5, $2
	mfhc1	$5, $3
	mfhc1	$5, $4
	mfhc1	$5, $5
	mfhc1	$5, $6
	mfhc1	$5, $7
	mfhc1	$5, $8
	mfhc1	$5, $9
	mfhc1	$5, $10
	mfhc1	$5, $11
	mfhc1	$5, $12
	mfhc1	$5, $13
	mfhc1	$5, $14
	mfhc1	$5, $15
	mfhc1	$5, $16
	mfhc1	$5, $17
	mfhc1	$5, $18
	mfhc1	$5, $19
	mfhc1	$5, $20
	mfhc1	$5, $21
	mfhc1	$5, $22
	mfhc1	$5, $23
	mfhc1	$5, $24
	mfhc1	$5, $25
	mfhc1	$5, $26
	mfhc1	$5, $27
	mfhc1	$5, $28
	mfhc1	$5, $29
	mfhc1	$5, $30
	mfhc1	$5, $31
	mfhc1	$5, $f0
	mfhc1	$5, $f1
	mfhc1	$5, $f2
	mfhc1	$5, $f3
	mfhc1	$5, $f4
	mfhc1	$5, $f5
	mfhc1	$5, $f6
	mfhc1	$5, $f7
	mfhc1	$5, $f8
	mfhc1	$5, $f9
	mfhc1	$5, $f10
	mfhc1	$5, $f11
	mfhc1	$5, $f12
	mfhc1	$5, $f13
	mfhc1	$5, $f14
	mfhc1	$5, $f15
	mfhc1	$5, $f16
	mfhc1	$5, $f17
	mfhc1	$5, $f18
	mfhc1	$5, $f19
	mfhc1	$5, $f20
	mfhc1	$5, $f21
	mfhc1	$5, $f22
	mfhc1	$5, $f23
	mfhc1	$5, $f24
	mfhc1	$5, $f25
	mfhc1	$5, $f26
	mfhc1	$5, $f27
	mfhc1	$5, $f28
	mfhc1	$5, $f29
	mfhc1	$5, $f30
	mfhc1	$5, $f31

	mov.d	$f0, $f1
	mov.d	$f30, $f31
	mov.s	$f0, $f1
	mov.s	$f30, $f31
	mov.ps	$f0, $f1
	mov.ps	$f30, $f31

	movf.d	$f2, $f3, $fcc0
	movf.d	$f2, $f3, $fcc1
	movf.d	$f2, $f3, $fcc2
	movf.d	$f2, $f3, $fcc3
	movf.d	$f2, $f3, $fcc4
	movf.d	$f2, $f3, $fcc5
	movf.d	$f2, $f3, $fcc6
	movf.d	$f2, $f3, $fcc7
	movf.d	$f30, $f31, $fcc7

	movf.s	$f2, $f3, $fcc0
	movf.s	$f2, $f3, $fcc1
	movf.s	$f2, $f3, $fcc2
	movf.s	$f2, $f3, $fcc3
	movf.s	$f2, $f3, $fcc4
	movf.s	$f2, $f3, $fcc5
	movf.s	$f2, $f3, $fcc6
	movf.s	$f2, $f3, $fcc7
	movf.s	$f30, $f31, $fcc7

	movf.ps	$f2, $f3, $fcc0
	movf.ps	$f2, $f3, $fcc2
	movf.ps	$f2, $f3, $fcc4
	movf.ps	$f2, $f3, $fcc6
	movf.ps	$f2, $f3, $fcc6
	movf.ps	$f30, $f31, $fcc6

	movn.d	$f2, $f3, $0
	movn.d	$f2, $f3, $31
	movn.s	$f2, $f3, $0
	movn.s	$f2, $f3, $31
	movn.ps	$f2, $f3, $0
	movn.ps	$f2, $f3, $31

	movt.ps	$f2, $f3, $fcc0
	movt.ps	$f2, $f3, $fcc2
	movt.ps	$f2, $f3, $fcc4
	movt.ps	$f2, $f3, $fcc6
	movt.ps	$f2, $f3, $fcc6
	movt.ps	$f30, $f31, $fcc6

	movz.d	$f2, $f3, $0
	movz.d	$f2, $f3, $31
	movz.s	$f2, $f3, $0
	movz.s	$f2, $f3, $31
	movz.ps	$f2, $f3, $0
	movz.ps	$f2, $f3, $31

	msub.d	$f0, $f1, $f2, $f3
	msub.d	$f28, $f29, $f30, $f31
	msub.s	$f0, $f1, $f2, $f3
	msub.s	$f28, $f29, $f30, $f31
	msub.ps	$f0, $f1, $f2, $f3
	msub.ps	$f28, $f29, $f30, $f31

	mtc1	$5, $0
	mtc1	$5, $1
	mtc1	$5, $2
	mtc1	$5, $3
	mtc1	$5, $4
	mtc1	$5, $5
	mtc1	$5, $6
	mtc1	$5, $7
	mtc1	$5, $8
	mtc1	$5, $9
	mtc1	$5, $10
	mtc1	$5, $11
	mtc1	$5, $12
	mtc1	$5, $13
	mtc1	$5, $14
	mtc1	$5, $15
	mtc1	$5, $16
	mtc1	$5, $17
	mtc1	$5, $18
	mtc1	$5, $19
	mtc1	$5, $20
	mtc1	$5, $21
	mtc1	$5, $22
	mtc1	$5, $23
	mtc1	$5, $24
	mtc1	$5, $25
	mtc1	$5, $26
	mtc1	$5, $27
	mtc1	$5, $28
	mtc1	$5, $29
	mtc1	$5, $30
	mtc1	$5, $31
	mtc1	$5, $f0
	mtc1	$5, $f1
	mtc1	$5, $f2
	mtc1	$5, $f3
	mtc1	$5, $f4
	mtc1	$5, $f5
	mtc1	$5, $f6
	mtc1	$5, $f7
	mtc1	$5, $f8
	mtc1	$5, $f9
	mtc1	$5, $f10
	mtc1	$5, $f11
	mtc1	$5, $f12
	mtc1	$5, $f13
	mtc1	$5, $f14
	mtc1	$5, $f15
	mtc1	$5, $f16
	mtc1	$5, $f17
	mtc1	$5, $f18
	mtc1	$5, $f19
	mtc1	$5, $f20
	mtc1	$5, $f21
	mtc1	$5, $f22
	mtc1	$5, $f23
	mtc1	$5, $f24
	mtc1	$5, $f25
	mtc1	$5, $f26
	mtc1	$5, $f27
	mtc1	$5, $f28
	mtc1	$5, $f29
	mtc1	$5, $f30
	mtc1	$5, $f31

	mthc1	$5, $0
	mthc1	$5, $1
	mthc1	$5, $2
	mthc1	$5, $3
	mthc1	$5, $4
	mthc1	$5, $5
	mthc1	$5, $6
	mthc1	$5, $7
	mthc1	$5, $8
	mthc1	$5, $9
	mthc1	$5, $10
	mthc1	$5, $11
	mthc1	$5, $12
	mthc1	$5, $13
	mthc1	$5, $14
	mthc1	$5, $15
	mthc1	$5, $16
	mthc1	$5, $17
	mthc1	$5, $18
	mthc1	$5, $19
	mthc1	$5, $20
	mthc1	$5, $21
	mthc1	$5, $22
	mthc1	$5, $23
	mthc1	$5, $24
	mthc1	$5, $25
	mthc1	$5, $26
	mthc1	$5, $27
	mthc1	$5, $28
	mthc1	$5, $29
	mthc1	$5, $30
	mthc1	$5, $31
	mthc1	$5, $f0
	mthc1	$5, $f1
	mthc1	$5, $f2
	mthc1	$5, $f3
	mthc1	$5, $f4
	mthc1	$5, $f5
	mthc1	$5, $f6
	mthc1	$5, $f7
	mthc1	$5, $f8
	mthc1	$5, $f9
	mthc1	$5, $f10
	mthc1	$5, $f11
	mthc1	$5, $f12
	mthc1	$5, $f13
	mthc1	$5, $f14
	mthc1	$5, $f15
	mthc1	$5, $f16
	mthc1	$5, $f17
	mthc1	$5, $f18
	mthc1	$5, $f19
	mthc1	$5, $f20
	mthc1	$5, $f21
	mthc1	$5, $f22
	mthc1	$5, $f23
	mthc1	$5, $f24
	mthc1	$5, $f25
	mthc1	$5, $f26
	mthc1	$5, $f27
	mthc1	$5, $f28
	mthc1	$5, $f29
	mthc1	$5, $f30
	mthc1	$5, $f31

	mul.s	$f0, $f1, $f2
	mul.s	$f29, $f30, $f31
	mul.s	$f29, $f29, $f30
	mul.s	$f29, $f30
	mul.d	$f0, $f1, $f2
	mul.d	$f29, $f30, $f31
	mul.d	$f29, $f29, $f30
	mul.d	$f29, $f30
	mul.ps	$f0, $f1, $f2
	mul.ps	$f29, $f30, $f31
	mul.ps	$f29, $f29, $f30
	mul.ps	$f29, $f30

	neg.s	$f0, $f1
	neg.s	$f30, $f31
	neg.s	$f2, $f2
	neg.s	$f2
	neg.d	$f0, $f1
	neg.d	$f30, $f31
	neg.d	$f2, $f2
	neg.d	$f2
	neg.ps	$f0, $f1
	neg.ps	$f30, $f31
	neg.ps	$f2, $f2
	neg.ps	$f2

	nmadd.d	$f0, $f1, $f2, $f3
	nmadd.d	$f28, $f29, $f30, $f31
	nmadd.s	$f0, $f1, $f2, $f3
	nmadd.s	$f28, $f29, $f30, $f31
	nmadd.ps	$f0, $f1, $f2, $f3
	nmadd.ps	$f28, $f29, $f30, $f31

	nmsub.d	$f0, $f1, $f2, $f3
	nmsub.d	$f28, $f29, $f30, $f31
	nmsub.s	$f0, $f1, $f2, $f3
	nmsub.s	$f28, $f29, $f30, $f31
	nmsub.ps	$f0, $f1, $f2, $f3
	nmsub.ps	$f28, $f29, $f30, $f31

	pll.ps	$f0, $f1, $f2
	pll.ps	$f29, $f30, $f31
	pll.ps	$f29, $f29, $f30
	pll.ps	$f29, $f30
	plu.ps	$f0, $f1, $f2
	plu.ps	$f29, $f30, $f31
	plu.ps	$f29, $f29, $f30
	plu.ps	$f29, $f30
	pul.ps	$f0, $f1, $f2
	pul.ps	$f29, $f30, $f31
	pul.ps	$f29, $f29, $f30
	pul.ps	$f29, $f30
	puu.ps	$f0, $f1, $f2
	puu.ps	$f29, $f30, $f31
	puu.ps	$f29, $f29, $f30
	puu.ps	$f29, $f30

	recip.s	$f0, $f1
	recip.s	$f30, $f31
	recip.s	$f2, $f2
	recip.d	$f0, $f1
	recip.d	$f30, $f31
	recip.d	$f2, $f2

	round.l.s	$f0, $f1
	round.l.s	$f30, $f31
	round.l.s	$f2, $f2
	round.l.d	$f0, $f1
	round.l.d	$f30, $f31
	round.l.d	$f2, $f2

	round.w.s	$f0, $f1
	round.w.s	$f30, $f31
	round.w.s	$f2, $f2
	round.w.d	$f0, $f1
	round.w.d	$f30, $f31
	round.w.d	$f2, $f2

	rsqrt.s	$f0, $f1
	rsqrt.s	$f30, $f31
	rsqrt.s	$f2, $f2
	rsqrt.d	$f0, $f1
	rsqrt.d	$f30, $f31
	rsqrt.d	$f2, $f2

	sdc1	$3, 0
	sdc1	$3, ($0)
	sdc1	$3, 4
	sdc1	$3, 4($0)
	sdc1	$3, ($4)
	sdc1	$3, 0($4)
	sdc1	$3, 32767($4)
	sdc1	$3, -32768($4)
	sdc1	$3, 65535($4)
	sdc1	$3, 0xffff0000($4)
	sdc1	$3, 0xffff8000($4)
	sdc1	$3, 0xffff0001($4)
	sdc1	$3, 0xffff8001($4)
	sdc1	$3, 0xf0000000($4)
	sdc1	$3, 0xffffffff($4)
	sdc1	$3, 0x12345678($4)
	sdc1	$f3, 0
	sdc1	$f3, ($0)
	sdc1	$f3, 4
	sdc1	$f3, 4($0)
	sdc1	$f3, ($4)
	sdc1	$f3, 0($4)
	sdc1	$f3, 32767($4)
	sdc1	$f3, -32768($4)
	sdc1	$f3, 65535($4)
	sdc1	$f3, 0xffff0000($4)
	sdc1	$f3, 0xffff8000($4)
	sdc1	$f3, 0xffff0001($4)
	sdc1	$f3, 0xffff8001($4)
	sdc1	$f3, 0xf0000000($4)
	sdc1	$f3, 0xffffffff($4)
	sdc1	$f3, 0x12345678($4)

	s.d	$f3, 0
	s.d	$f3, ($0)
	s.d	$f3, 4
	s.d	$f3, 4($0)
	s.d	$f3, ($4)
	s.d	$f3, 0($4)
	s.d	$f3, 32767($4)
	s.d	$f3, -32768($4)

	sdxc1	$f0, $0($0)
	sdxc1	$f0, $0($2)
	sdxc1	$f0, $0($31)
	sdxc1	$f0, $2($31)
	sdxc1	$f0, $31($31)
	sdxc1	$f1, $31($31)
	sdxc1	$f2, $31($31)
	sdxc1	$f31, $31($31)

	sqrt.s	$f0, $f1
	sqrt.s	$f30, $f31
	sqrt.s	$f2, $f2
	sqrt.d	$f0, $f1
	sqrt.d	$f30, $f31
	sqrt.d	$f2, $f2

	sub.s	$f0, $f1, $f2
	sub.s	$f29, $f30, $f31
	sub.s	$f29, $f29, $f30
	sub.s	$f29, $f30
	sub.d	$f0, $f1, $f2
	sub.d	$f29, $f30, $f31
	sub.d	$f29, $f29, $f30
	sub.d	$f29, $f30
	sub.ps	$f0, $f1, $f2
	sub.ps	$f29, $f30, $f31
	sub.ps	$f29, $f29, $f30
	sub.ps	$f29, $f30

	suxc1	$f0, $0($0)
	suxc1	$f0, $0($2)
	suxc1	$f0, $0($31)
	suxc1	$f0, $2($31)
	suxc1	$f0, $31($31)
	suxc1	$f1, $31($31)
	suxc1	$f2, $31($31)
	suxc1	$f31, $31($31)

	swc1	$3, 0
	swc1	$3, ($0)
	swc1	$3, 4
	swc1	$3, 4($0)
	swc1	$3, ($4)
	swc1	$3, 0($4)
	swc1	$3, 32767($4)
	swc1	$3, -32768($4)
	swc1	$3, 65535($4)
	swc1	$3, 0xffff0000($4)
	swc1	$3, 0xffff8000($4)
	swc1	$3, 0xffff0001($4)
	swc1	$3, 0xffff8001($4)
	swc1	$3, 0xf0000000($4)
	swc1	$3, 0xffffffff($4)
	swc1	$3, 0x12345678($4)
	swc1	$f3, 0
	swc1	$f3, ($0)
	swc1	$f3, 4
	swc1	$f3, 4($0)
	swc1	$f3, ($4)
	swc1	$f3, 0($4)
	swc1	$f3, 32767($4)
	swc1	$f3, -32768($4)
	swc1	$f3, 65535($4)
	swc1	$f3, 0xffff0000($4)
	swc1	$f3, 0xffff8000($4)
	swc1	$f3, 0xffff0001($4)
	swc1	$f3, 0xffff8001($4)
	swc1	$f3, 0xf0000000($4)
	swc1	$f3, 0xffffffff($4)
	swc1	$f3, 0x12345678($4)

	s.s	$f3, 0
	s.s	$f3, ($0)
	s.s	$f3, 4
	s.s	$f3, 4($0)
	s.s	$f3, ($4)
	s.s	$f3, 0($4)
	s.s	$f3, 32767($4)
	s.s	$f3, -32768($4)
	s.s	$f3, 65535($4)
	s.s	$f3, 0xffff0000($4)
	s.s	$f3, 0xffff8000($4)
	s.s	$f3, 0xffff0001($4)
	s.s	$f3, 0xffff8001($4)
	s.s	$f3, 0xf0000000($4)
	s.s	$f3, 0xffffffff($4)
	s.s	$f3, 0x12345678($4)

	swxc1	$f0, $0($0)
	swxc1	$f0, $0($2)
	swxc1	$f0, $0($31)
	swxc1	$f0, $2($31)
	swxc1	$f0, $31($31)
	swxc1	$f1, $31($31)
	swxc1	$f2, $31($31)
	swxc1	$f31, $31($31)

	trunc.l.s	$f0, $f1
	trunc.l.s	$f30, $f31
	trunc.l.s	$f2, $f2
	trunc.l.d	$f0, $f1
	trunc.l.d	$f30, $f31
	trunc.l.d	$f2, $f2

	trunc.w.s	$f0, $f1
	trunc.w.s	$f30, $f31
	trunc.w.s	$f2, $f2
	trunc.w.d	$f0, $f1
	trunc.w.d	$f30, $f31
	trunc.w.d	$f2, $f2

	movf	$2, $3, $fcc0
	movf	$30, $31, $fcc0
	movf	$30, $31, $fcc1
	movf	$30, $31, $fcc2
	movf	$30, $31, $fcc3
	movf	$30, $31, $fcc4
	movf	$30, $31, $fcc5
	movf	$30, $31, $fcc6
	movf	$30, $31, $fcc7

	movt	$2, $3, $fcc0
	movt	$30, $31, $fcc0
	movt	$30, $31, $fcc1
	movt	$30, $31, $fcc2
	movt	$30, $31, $fcc3
	movt	$30, $31, $fcc4
	movt	$30, $31, $fcc5
	movt	$30, $31, $fcc6
	movt	$30, $31, $fcc7

	.set	noreorder
	bc1fl	$fcc1, test
	addu	$3, $4, $5
	bc1tl	$fcc2, test
	addu	$6, $7, $8
	.set	reorder

	bc1fl	$fcc3, test
	addu	$3, $4, $5
	bc1tl	$fcc4, test
	addu	$6, $7, $8

	.end	fp_test

	.set	mips64r2
	.globl	test_mips64
	.ent	test_mips64

test_mips64:
	dabs	$2, $3
	dabs	$2, $2
	dabs	$2

	dadd	$2, $3, $4
	dadd	$29, $30, $31
	dadd	$2, $2, $3
	dadd	$2, $3

	dadd	$2, $3, 0
	dadd	$2, $3, 1
	dadd	$2, $3, -512
	dadd	$2, $3, 511
	dadd	$2, $3, 32767
	dadd	$2, $3, -32768
	dadd	$2, $3, 65535
	dadd	$2, $3, 0x12345678
	dadd	$2, $3, 0x1234567887654321

	daddi	$2, $3, 0
	daddi	$2, $3, 1
	daddi	$2, $3, -512
	daddi	$2, $3, 511
	daddi	$2, $2, 511
	daddi	$2, 511
	daddi	$2, $3, 32767
	daddi	$2, $3, -32768
	daddi	$2, $3, 65535
	daddi	$2, $3, 0x12345678

	daddiu	$2, $3, 0
	daddiu	$2, $3, -32768
	daddiu	$2, $3, 32767
	daddiu	$2, $2, 32767
	daddiu	$2, 32767

	daddu	$2, $3, $4
	daddu	$29, $30, $31
	daddu	$2, $2, $3
	daddu	$2, $3
	daddu	$2, $3, $0
	daddu	$2, $3, 0
	daddu	$2, $3, 1
	daddu	$2, $3, 32767
	daddu	$2, $3, -32768
	daddu	$2, $3, 65535

	dclo	$2, $3
	dclo	$3, $2
	dclz	$2, $3
	dclz	$3, $2

	ddiv	$0, $2, $3
	ddiv	$0, $30, $31
	ddiv	$0, $3
	ddiv	$0, $31

	ddiv	$2, $3, $0
	ddiv	$2, $3, $4

	ddiv	$3, $4, 0
	ddiv	$3, $4, 1
	ddiv	$3, $4, -1
	ddiv	$3, $4, 2

	ddivu	$0, $2, $3
	ddivu	$0, $30, $31
	ddivu	$0, $3
	ddivu	$0, $31

	ddivu	$2, $3, $0
	ddivu	$2, $3, $4

	ddivu	$3, $4, 0
	ddivu	$3, $4, 1
	ddivu	$3, $4, -1
	ddivu	$3, $4, 2

	dext	$2, $3, 31, 1
	dext	$2, $3, 0, 32

	dext	$2, $3, 31, 33
	dextm	$2, $3, 31, 33

	dext	$2, $3, 33, 10
	dextu	$2, $3, 33, 10

	dins	$2, $3, 31, 1
	dins	$2, $3, 0, 32

	dins	$2, $3, 31, 33
	dinsm	$2, $3, 31, 33

	dins	$2, $3, 33, 10
	dinsu	$2, $3, 33, 10

	dla	$2, test
	dlca	$2, test

	dli	$2, -32768
	dli	$2, 32767
	dli	$2, 65535
	dli	$2, 0x12345678

	dmfc0	$2, $0
	dmfc0	$2, $1
	dmfc0	$2, $2
	dmfc0	$2, $3
	dmfc0	$2, $4
	dmfc0	$2, $5
	dmfc0	$2, $6
	dmfc0	$2, $7
	dmfc0	$2, $8
	dmfc0	$2, $9
	dmfc0	$2, $10
	dmfc0	$2, $11
	dmfc0	$2, $12
	dmfc0	$2, $13
	dmfc0	$2, $14
	dmfc0	$2, $15
	dmfc0	$2, $16
	dmfc0	$2, $17
	dmfc0	$2, $18
	dmfc0	$2, $19
	dmfc0	$2, $20
	dmfc0	$2, $21
	dmfc0	$2, $22
	dmfc0	$2, $23
	dmfc0	$2, $24
	dmfc0	$2, $25
	dmfc0	$2, $26
	dmfc0	$2, $27
	dmfc0	$2, $28
	dmfc0	$2, $29
	dmfc0	$2, $30
	dmfc0	$2, $31
	dmfc0	$2, $0, 0
	dmfc0	$2, $0, 1
	dmfc0	$2, $0, 2
	dmfc0	$2, $0, 3
	dmfc0	$2, $0, 4
	dmfc0	$2, $0, 5
	dmfc0	$2, $0, 6
	dmfc0	$2, $0, 7
	dmfc0	$2, $1, 0
	dmfc0	$2, $1, 1
	dmfc0	$2, $1, 2
	dmfc0	$2, $1, 3
	dmfc0	$2, $1, 4
	dmfc0	$2, $1, 5
	dmfc0	$2, $1, 6
	dmfc0	$2, $1, 7
	dmfc0	$2, $2, 0
	dmfc0	$2, $2, 1
	dmfc0	$2, $2, 2
	dmfc0	$2, $2, 3
	dmfc0	$2, $2, 4
	dmfc0	$2, $2, 5
	dmfc0	$2, $2, 6
	dmfc0	$2, $2, 7

	dmtc0	$2, $0
	dmtc0	$2, $1
	dmtc0	$2, $2
	dmtc0	$2, $3
	dmtc0	$2, $4
	dmtc0	$2, $5
	dmtc0	$2, $6
	dmtc0	$2, $7
	dmtc0	$2, $8
	dmtc0	$2, $9
	dmtc0	$2, $10
	dmtc0	$2, $11
	dmtc0	$2, $12
	dmtc0	$2, $13
	dmtc0	$2, $14
	dmtc0	$2, $15
	dmtc0	$2, $16
	dmtc0	$2, $17
	dmtc0	$2, $18
	dmtc0	$2, $19
	dmtc0	$2, $20
	dmtc0	$2, $21
	dmtc0	$2, $22
	dmtc0	$2, $23
	dmtc0	$2, $24
	dmtc0	$2, $25
	dmtc0	$2, $26
	dmtc0	$2, $27
	dmtc0	$2, $28
	dmtc0	$2, $29
	dmtc0	$2, $30
	dmtc0	$2, $31
	dmtc0	$2, $0, 0
	dmtc0	$2, $0, 1
	dmtc0	$2, $0, 2
	dmtc0	$2, $0, 3
	dmtc0	$2, $0, 4
	dmtc0	$2, $0, 5
	dmtc0	$2, $0, 6
	dmtc0	$2, $0, 7
	dmtc0	$2, $1, 0
	dmtc0	$2, $1, 1
	dmtc0	$2, $1, 2
	dmtc0	$2, $1, 3
	dmtc0	$2, $1, 4
	dmtc0	$2, $1, 5
	dmtc0	$2, $1, 6
	dmtc0	$2, $1, 7
	dmtc0	$2, $2, 0
	dmtc0	$2, $2, 1
	dmtc0	$2, $2, 2
	dmtc0	$2, $2, 3
	dmtc0	$2, $2, 4
	dmtc0	$2, $2, 5
	dmtc0	$2, $2, 6
	dmtc0	$2, $2, 7

	dmfc1	$5, $0
	dmfc1	$5, $1
	dmfc1	$5, $2
	dmfc1	$5, $3
	dmfc1	$5, $4
	dmfc1	$5, $5
	dmfc1	$5, $6
	dmfc1	$5, $7
	dmfc1	$5, $8
	dmfc1	$5, $9
	dmfc1	$5, $10
	dmfc1	$5, $11
	dmfc1	$5, $12
	dmfc1	$5, $13
	dmfc1	$5, $14
	dmfc1	$5, $15
	dmfc1	$5, $16
	dmfc1	$5, $17
	dmfc1	$5, $18
	dmfc1	$5, $19
	dmfc1	$5, $20
	dmfc1	$5, $21
	dmfc1	$5, $22
	dmfc1	$5, $23
	dmfc1	$5, $24
	dmfc1	$5, $25
	dmfc1	$5, $26
	dmfc1	$5, $27
	dmfc1	$5, $28
	dmfc1	$5, $29
	dmfc1	$5, $30
	dmfc1	$5, $31
	dmfc1	$5, $f0
	dmfc1	$5, $f1
	dmfc1	$5, $f2
	dmfc1	$5, $f3
	dmfc1	$5, $f4
	dmfc1	$5, $f5
	dmfc1	$5, $f6
	dmfc1	$5, $f7
	dmfc1	$5, $f8
	dmfc1	$5, $f9
	dmfc1	$5, $f10
	dmfc1	$5, $f11
	dmfc1	$5, $f12
	dmfc1	$5, $f13
	dmfc1	$5, $f14
	dmfc1	$5, $f15
	dmfc1	$5, $f16
	dmfc1	$5, $f17
	dmfc1	$5, $f18
	dmfc1	$5, $f19
	dmfc1	$5, $f20
	dmfc1	$5, $f21
	dmfc1	$5, $f22
	dmfc1	$5, $f23
	dmfc1	$5, $f24
	dmfc1	$5, $f25
	dmfc1	$5, $f26
	dmfc1	$5, $f27
	dmfc1	$5, $f28
	dmfc1	$5, $f29
	dmfc1	$5, $f30
	dmfc1	$5, $f31

	dmtc1	$5, $0
	dmtc1	$5, $1
	dmtc1	$5, $2
	dmtc1	$5, $3
	dmtc1	$5, $4
	dmtc1	$5, $5
	dmtc1	$5, $6
	dmtc1	$5, $7
	dmtc1	$5, $8
	dmtc1	$5, $9
	dmtc1	$5, $10
	dmtc1	$5, $11
	dmtc1	$5, $12
	dmtc1	$5, $13
	dmtc1	$5, $14
	dmtc1	$5, $15
	dmtc1	$5, $16
	dmtc1	$5, $17
	dmtc1	$5, $18
	dmtc1	$5, $19
	dmtc1	$5, $20
	dmtc1	$5, $21
	dmtc1	$5, $22
	dmtc1	$5, $23
	dmtc1	$5, $24
	dmtc1	$5, $25
	dmtc1	$5, $26
	dmtc1	$5, $27
	dmtc1	$5, $28
	dmtc1	$5, $29
	dmtc1	$5, $30
	dmtc1	$5, $31
	dmtc1	$5, $f0
	dmtc1	$5, $f1
	dmtc1	$5, $f2
	dmtc1	$5, $f3
	dmtc1	$5, $f4
	dmtc1	$5, $f5
	dmtc1	$5, $f6
	dmtc1	$5, $f7
	dmtc1	$5, $f8
	dmtc1	$5, $f9
	dmtc1	$5, $f10
	dmtc1	$5, $f11
	dmtc1	$5, $f12
	dmtc1	$5, $f13
	dmtc1	$5, $f14
	dmtc1	$5, $f15
	dmtc1	$5, $f16
	dmtc1	$5, $f17
	dmtc1	$5, $f18
	dmtc1	$5, $f19
	dmtc1	$5, $f20
	dmtc1	$5, $f21
	dmtc1	$5, $f22
	dmtc1	$5, $f23
	dmtc1	$5, $f24
	dmtc1	$5, $f25
	dmtc1	$5, $f26
	dmtc1	$5, $f27
	dmtc1	$5, $f28
	dmtc1	$5, $f29
	dmtc1	$5, $f30
	dmtc1	$5, $f31

	dmfc2	$2, $0
	dmfc2	$2, $1
	dmfc2	$2, $2
	dmfc2	$2, $3
	dmfc2	$2, $4
	dmfc2	$2, $5
	dmfc2	$2, $6
	dmfc2	$2, $7
	dmfc2	$2, $8
	dmfc2	$2, $9
	dmfc2	$2, $10
	dmfc2	$2, $11
	dmfc2	$2, $12
	dmfc2	$2, $13
	dmfc2	$2, $14
	dmfc2	$2, $15
	dmfc2	$2, $16
	dmfc2	$2, $17
	dmfc2	$2, $18
	dmfc2	$2, $19
	dmfc2	$2, $20
	dmfc2	$2, $21
	dmfc2	$2, $22
	dmfc2	$2, $23
	dmfc2	$2, $24
	dmfc2	$2, $25
	dmfc2	$2, $26
	dmfc2	$2, $27
	dmfc2	$2, $28
	dmfc2	$2, $29
	dmfc2	$2, $30
	dmfc2	$2, $31
/*
	dmfc2	$2, $0, 0
	dmfc2	$2, $0, 1
	dmfc2	$2, $0, 2
	dmfc2	$2, $0, 3
	dmfc2	$2, $0, 4
	dmfc2	$2, $0, 5
	dmfc2	$2, $0, 6
	dmfc2	$2, $0, 7
	dmfc2	$2, $1, 0
	dmfc2	$2, $1, 1
	dmfc2	$2, $1, 2
	dmfc2	$2, $1, 3
	dmfc2	$2, $1, 4
	dmfc2	$2, $1, 5
	dmfc2	$2, $1, 6
	dmfc2	$2, $1, 7
	dmfc2	$2, $2, 0
	dmfc2	$2, $2, 1
	dmfc2	$2, $2, 2
	dmfc2	$2, $2, 3
	dmfc2	$2, $2, 4
	dmfc2	$2, $2, 5
	dmfc2	$2, $2, 6
	dmfc2	$2, $2, 7
*/

	dmtc2	$2, $0
	dmtc2	$2, $1
	dmtc2	$2, $2
	dmtc2	$2, $3
	dmtc2	$2, $4
	dmtc2	$2, $5
	dmtc2	$2, $6
	dmtc2	$2, $7
	dmtc2	$2, $8
	dmtc2	$2, $9
	dmtc2	$2, $10
	dmtc2	$2, $11
	dmtc2	$2, $12
	dmtc2	$2, $13
	dmtc2	$2, $14
	dmtc2	$2, $15
	dmtc2	$2, $16
	dmtc2	$2, $17
	dmtc2	$2, $18
	dmtc2	$2, $19
	dmtc2	$2, $20
	dmtc2	$2, $21
	dmtc2	$2, $22
	dmtc2	$2, $23
	dmtc2	$2, $24
	dmtc2	$2, $25
	dmtc2	$2, $26
	dmtc2	$2, $27
	dmtc2	$2, $28
	dmtc2	$2, $29
	dmtc2	$2, $30
	dmtc2	$2, $31
/*
	dmtc2	$2, $0, 0
	dmtc2	$2, $0, 1
	dmtc2	$2, $0, 2
	dmtc2	$2, $0, 3
	dmtc2	$2, $0, 4
	dmtc2	$2, $0, 5
	dmtc2	$2, $0, 6
	dmtc2	$2, $0, 7
	dmtc2	$2, $1, 0
	dmtc2	$2, $1, 1
	dmtc2	$2, $1, 2
	dmtc2	$2, $1, 3
	dmtc2	$2, $1, 4
	dmtc2	$2, $1, 5
	dmtc2	$2, $1, 6
	dmtc2	$2, $1, 7
	dmtc2	$2, $2, 0
	dmtc2	$2, $2, 1
	dmtc2	$2, $2, 2
	dmtc2	$2, $2, 3
	dmtc2	$2, $2, 4
	dmtc2	$2, $2, 5
	dmtc2	$2, $2, 6
	dmtc2	$2, $2, 7
*/

	dmult	$2, $3
	dmultu	$2, $3

	dmul	$2, $3, $4
	dmul	$2, $3, 0x12345678

	dmulo	$2, $3, $4
	dmulo	$2, $3, 4

	dmulou	$2, $3, $4
	dmulou	$2, $3, 4

	drem	$3, $4, 0
	drem	$3, $4, 1
	drem	$3, $4, -1
	drem	$3, $4, 2

	drem	$0, $2, $3
	drem	$0, $30, $31
	drem	$0, $3
	drem	$0, $31

	drem	$3, $4, 0
	drem	$3, $4, 1
	drem	$3, $4, -1
	drem	$3, $4, 2

	dremu	$0, $2, $3
	dremu	$0, $30, $31
	dremu	$0, $3
	dremu	$0, $31

	dremu	$3, $4, 0
	dremu	$3, $4, 1
	dremu	$3, $4, -1
	dremu	$3, $4, 2

	drol	$2, $3, $4
	drol	$2, $2, $4
	drol	$2, $3, 4

	dror	$2, $3, $4
	dror	$2, $3, 4
	dror	$2, $3, 36

	drorv	$2, $3, $4
	dror32	$2, $3, 4

	drotl	$2, $3, $4
	drotl	$2, $2, $4
	drotl	$2, $3, 4

	drotr	$2, $3, $4
	drotr	$2, $3, 4
	drotr	$2, $3, 36

	drotrv	$2, $3, $4
	drotr32	$2, $3, 4

	dsbh	$2, $3
	dsbh	$2, $2
	dsbh	$2

	dshd	$2, $3
	dshd	$2, $2
	dshd	$2

	dsllv	$2, $3, $4
	dsll32	$2, $3, 31
	dsll	$2, $3, $4
	dsll	$2, $3, 63
	dsll	$2, $3, 31

	dsrav	$2, $3, $4
	dsra32	$2, $3, 4
	dsra	$2, $3, $4
	dsra	$2, $3, 36
	dsra	$2, $3, 4

	dsrlv	$2, $3, $4
	dsrl32	$2, $3, 31
	dsrl	$2, $3, $4
	dsrl	$2, $3, 36
	dsrl	$2, $3, 4

	dsub	$2, $3, $4
	dsub	$29, $30, $31
	dsub	$2, $2, $3
	dsub	$2, $3

	dsubu	$2, $3, $4
	dsubu	$29, $30, $31
	dsubu	$2, $2, $3
	dsubu	$2, $3

	dsubu	$2, $3, 0x1234
	dsubu	$2, $3, 0x12345678

	dsub	$2, $3, 0
	dsub	$2, $3, 1
	dsub	$2, $3, 512
	dsub	$2, $3, -511
	dsub	$2, $3, -32768
	dsub	$2, $3, 32767
	dsub	$2, $3, 65535
	dsub	$2, $3, 0x12345678
	dsub	$2, $3, 0x8888111112345678

	.set	push
	.set	noreorder
	.set	nomacro
	ld	$2, 0
	ld	$2, 4
	ld	$2, ($0)
	ld	$2, 0($0)
	ld	$2, 4($0)
	ld	$2, 4($3)
	ld	$2, -32768($3)
	ld	$2, 32767($3)
	.set	pop

	ldl	$2, 0
	ldl	$2, 4
	ldl	$2, ($0)
	ldl	$2, 0($0)
	ldl	$2, 4($0)
	ldl	$2, 4($3)
	ldl	$2, -512($3)
	ldl	$2, 511($3)
	ldl	$2, -32768($3)
	ldl	$2, 0x12345678($3)

	ldr	$2, 0
	ldr	$2, 4
	ldr	$2, ($0)
	ldr	$2, 0($0)
	ldr	$2, 4($0)
	ldr	$2, 4($3)
	ldr	$2, -512($3)
	ldr	$2, 511($3)
	ldr	$2, -32768($3)
	ldr	$2, 0x12345678($3)

	lld	$2, 0
	lld	$2, 4
	lld	$2, ($0)
	lld	$2, 0($0)
	lld	$2, 4($0)
	lld	$2, 4($3)
	lld	$2, -512($3)
	lld	$2, 511($3)
	lld	$2, -32768($3)
	lld	$2, 0x12345678($3)

	lwu	$2, 0
	lwu	$2, 4
	lwu	$2, ($0)
	lwu	$2, 0($0)
	lwu	$2, 4($0)
	lwu	$2, 4($3)
	lwu	$2, -512($3)
	lwu	$2, 511($3)
	lwu	$2, -32768($3)
	lwu	$2, 0x12345678($3)

	scd	$2, 0
	scd	$2, 4
	scd	$2, ($0)
	scd	$2, 0($0)
	scd	$2, 4($0)
	scd	$2, 4($3)
	scd	$2, -512($3)
	scd	$2, 511($3)
	scd	$2, -32768($3)
	scd	$2, 0x12345678($3)

	.set	push
	.set	noreorder
	.set	nomacro
	sd	$2, 0
	sd	$2, 4
	sd	$2, ($0)
	sd	$2, 0($0)
	sd	$2, 4($0)
	sd	$2, 4($3)
	sd	$2, -32768($3)
	sd	$2, 32767($3)
	.set	pop

	sdl	$2, 0
	sdl	$2, 4
	sdl	$2, ($0)
	sdl	$2, 0($0)
	sdl	$2, 4($0)
	sdl	$2, 4($3)
	sdl	$2, -32768($3)
	sdl	$2, 32767($3)
	sdl	$2, 0x12345678($3)

	sdr	$2, 0
	sdr	$2, 4
	sdr	$2, ($0)
	sdr	$2, 0($0)
	sdr	$2, 4($0)
	sdr	$2, 4($3)
	sdr	$2, -32768($3)
	sdr	$2, 32767($3)
	sdr	$2, 0x12345678($3)

	ldm	$s0, 0
	ldm	$s0, 4
	ldm	$s0, ($5)
	ldm	$s0, 2047($5)
	ldm	$s0-$s1, 2047($5)
	ldm	$s0-$s2, 2047($5)
	ldm	$s0-$s3, 2047($5)
	ldm	$s0-$s4, 2047($5)
	ldm	$s0-$s5, 2047($5)
	ldm	$s0-$s6, 2047($5)
	ldm	$s0-$s7, 2047($5)
	ldm	$s0-$s8, 2047($5)
	ldm	$ra, 2047($5)
	ldm	$s0,$ra, ($5)
	ldm	$s0-$s1,$ra, ($5)
	ldm	$s0-$s2,$ra, ($5)
	ldm	$s0-$s3,$ra, ($5)
	ldm	$s0-$s4,$ra, ($5)
	ldm	$s0-$s5,$ra, ($5)
	ldm	$s0-$s6,$ra, ($5)
	ldm	$s0-$s7,$ra, ($5)
	ldm	$s0-$s8,$ra, ($5)
	ldm	$s0, -32768($0)
	ldm	$s0, 32767($0)
	ldm	$s0, 0($0)
	ldm	$s0, 65535($0)
	ldm	$s0, -32768($29)
	ldm	$s0, 32767($29)
	ldm	$s0, 0($29)
	ldm	$s0, 65535($29)
	ldm	$s0, 0x12345678($29)

	ldp	$2, 0
	ldp	$2, 4
	ldp	$2, ($29)
	ldp	$2, 0($29)
	ldp	$2, -2048($3)
	ldp	$2, 2047($3)
	ldp	$2, -32768($3)
	ldp	$2, 32767($3)
	ldp	$2, 0($3)
	ldp	$2, 65535($3)
	ldp	$2, -32768($0)
	ldp	$2, 32767($0)
	ldp	$2, 65535($0)
	ldp	$2, 0x12345678($0)

	sdm	$s0, 0
	sdm	$s0, 4
	sdm	$s0, ($5)
	sdm	$s0, 2047($5)
	sdm	$s0-$s1, 2047($5)
	sdm	$s0-$s2, 2047($5)
	sdm	$s0-$s3, 2047($5)
	sdm	$s0-$s4, 2047($5)
	sdm	$s0-$s5, 2047($5)
	sdm	$s0-$s6, 2047($5)
	sdm	$s0-$s7, 2047($5)
	sdm	$s0-$s8, 2047($5)
	sdm	$ra, 2047($5)
	sdm	$s0,$ra, ($5)
	sdm	$s0-$s1,$ra, ($5)
	sdm	$s0-$s2,$ra, ($5)
	sdm	$s0-$s3,$ra, ($5)
	sdm	$s0-$s4,$ra, ($5)
	sdm	$s0-$s5,$ra, ($5)
	sdm	$s0-$s6,$ra, ($5)
	sdm	$s0-$s7,$ra, ($5)
	sdm	$s0-$s8,$ra, ($5)
	sdm	$s0, -32768($0)
	sdm	$s0, 32767($0)
	sdm	$s0, 0($0)
	sdm	$s0, 65535($0)
	sdm	$s0, -32768($29)
	sdm	$s0, 32767($29)
	sdm	$s0, 0($29)
	sdm	$s0, 65535($29)
	sdm	$s0, 0x12345678($29)

	sdp	$2, 0
	sdp	$2, 4
	sdp	$2, ($29)
	sdp	$2, 0($29)
	sdp	$2, -2048($3)
	sdp	$2, 2047($3)
	sdp	$2, -32768($3)
	sdp	$2, 32767($3)
	sdp	$2, 0($3)
	sdp	$2, 65535($3)
	sdp	$2, -32768($0)
	sdp	$2, 32767($0)
	sdp	$2, 65535($0)
	sdp	$2, 0x12345678($0)

	uld	$3, 0
	uld	$3, ($0)
	uld	$3, 4
	uld	$3, 4($0)
	uld	$3, 2047
	uld	$3, -2048
	uld	$3, 2048
	uld	$3, -2049
	uld	$3, 32753($0)
	uld	$3, -32768($0)
	uld	$3, 65535($0)
	uld	$3, 0xffff0000($0)
	uld	$3, 0xffff8000($0)
	uld	$3, 0xffff0001($0)
	uld	$3, 0xffff8001($0)
	uld	$3, 0xf0000000($0)
	uld	$3, 0xffffffff($0)
	uld	$3, 0x12345678($0)
	uld	$3, 0($4)
	uld	$3, 4($4)
	uld	$3, 2047($4)
	uld	$3, -2048($4)
	uld	$3, 2048($4)
	uld	$3, -2049($4)
	uld	$3, 32753($4)
	uld	$3, -32768($4)
	uld	$3, 65535($4)
	uld	$3, 0xffff0000($4)
	uld	$3, 0xffff8000($4)
	uld	$3, 0xffff0001($4)
	uld	$3, 0xffff8001($4)
	uld	$3, 0xf0000000($4)
	uld	$3, 0xffffffff($4)
	uld	$3, 0x12345678($4)

	usd	$3, 0
	usd	$3, ($0)
	usd	$3, 4
	usd	$3, 4($0)
	usd	$3, 2047
	usd	$3, -2048
	usd	$3, 2048
	usd	$3, -2049
	usd	$3, 32753($0)
	usd	$3, -32768($0)
	usd	$3, 65535($0)
	usd	$3, 0xffff0000($0)
	usd	$3, 0xffff8000($0)
	usd	$3, 0xffff0001($0)
	usd	$3, 0xffff8001($0)
	usd	$3, 0xf0000000($0)
	usd	$3, 0xffffffff($0)
	usd	$3, 0x12345678($0)
	usd	$3, 0($4)
	usd	$3, 4($4)
	usd	$3, 2047($4)
	usd	$3, -2048($4)
	usd	$3, 2048($4)
	usd	$3, -2049($4)
	usd	$3, 32753($4)
	usd	$3, -32768($4)
	usd	$3, 65535($4)
	usd	$3, 0xffff0000($4)
	usd	$3, 0xffff8000($4)
	usd	$3, 0xffff0001($4)
	usd	$3, 0xffff8001($4)
	usd	$3, 0xf0000000($4)
	usd	$3, 0xffffffff($4)
	usd	$3, 0x12345678($4)

	ldl	$16, %lo(test)($3)
	ldr	$16, %lo(test)($3)
	lld	$16, %lo(test)($3)
	lwu	$16, %lo(test)($3)
	scd	$16, %lo(test)($3)
	sdl	$16, %lo(test)($3)
	sdr	$16, %lo(test)($3)
	ldm	$16, %lo(test)($3)
	ldp	$16, %lo(test)($3)
	sdm	$16, %lo(test)($3)
	sdp	$16, %lo(test)($3)
	ldc2	$16, %lo(test)($3)
	sdc2	$16, %lo(test)($3)

	.end	test_mips64

	.set	reorder
	.ent	test_delay_slot
test_delay_slot:
	bal	test_delay_slot
	bgezal	$3, test_delay_slot
	bltzal	$3, test_delay_slot
	bgezall	$3, test_delay_slot
	bltzall	$3, test_delay_slot
	jal	test_delay_slot
	jalx	test_delay_slot_ext
	.ifndef	insn32
	jalr16	$2
	.endif
	jalr32	$2
	.ifndef	insn32
	DSNOP
	jr16	$2
	.endif
	jr32	$2
	jalr.hb	$2
	jr.hb	$2

	.ifndef	insn32
	jals	test_delay_slot
	jalrs16	$2
	jalrs32	$2
	jrs	$2
	jalrs.hb	$2
	jrs.hb	$2
	.endif

	.end	test_delay_slot

	.set	noreorder
	.ent	test_spec102
test_spec102:
	lw	$2, -64<<2 ($28)
	lw	$3, -64<<2 ($28)
	lw	$4, -64<<2 ($28)
	lw	$5, -64<<2 ($28)
	lw	$6, -64<<2 ($28)
	lw	$7, -64<<2 ($28)
	lw	$16, -64<<2 ($28)
	lw	$17, -64<<2 ($28)
	lw	$17, -63<<2 ($28)
	lw	$17, -1<<2 ($28)
	lw	$17, 0<<2 ($28)
	lw	$17, 1<<2 ($28)
	lw	$17, 62<<2 ($28)
	lw	$17, 63<<2 ($28)
	lw	$17, 64<<2 ($28)
	lw	$17, -65<<2 ($28)
	lw	$17, 1 ($28)
	lw	$17, 2 ($28)
	lw	$17, 3 ($28)
	lw	$17, -1 ($28)
	lw	$17, -2 ($28)
	lw	$17, -3 ($28)
	lw	$17, 0 ($27)

	addiu	$2, $pc, 0
	addiu	$3, $pc, 0
	addiu	$4, $pc, 0
	addiu	$5, $pc, 0
	addiu	$6, $pc, 0
	addiu	$7, $pc, 0
	addiu	$16, $pc, 0
	addiu	$17, $pc, 0
	addiu	$17, $pc, 4194303 << 2
	addiu	$17, $pc, -4194304 << 2
	addiupc	$2, 0
	addiupc	$3, 0
	addiupc	$4, 0
	addiupc	$5, 0
	addiupc	$6, 0
	addiupc	$7, 0
	addiupc	$16, 0
	addiupc	$17, 0
	addiupc	$17, 4194303 << 2
	addiupc	$17, -4194304 << 2

	.end	test_spec102

	.set	noreorder
	.ent	test_spec107
test_spec107:
	movep	$5, $6, $0, $0
	movep	$5, $7, $0, $0
	movep	$6, $7, $0, $0
	movep	$4, $21, $0, $0
	movep	$4, $22, $0, $0
	movep	$4, $5, $0, $0
	movep	$4, $6, $0, $0
	movep	$4, $7, $0, $0
	movep	$4, $7, $17, $0
	movep	$4, $7, $2, $0
	movep	$4, $7, $3, $0
	movep	$4, $7, $16, $0
	movep	$4, $7, $18, $0
	movep	$4, $7, $19, $0
	movep	$4, $7, $20, $0
	movep	$4, $7, $20, $17
	movep	$4, $7, $20, $2
	movep	$4, $7, $20, $3
	movep	$4, $7, $20, $16
	movep	$4, $7, $20, $18
	movep	$4, $7, $20, $19
	movep	$4, $7, $20, $20
	.ifndef	insn32
	bals	test_spec107
	nop
	bgezals	$2, test_spec107
	nop
	bltzals	$2, test_spec107
	nop
	.endif
	bal	test_spec107
	nop
	bgezal	$2, test_spec107
	nop
	bltzal	$2, test_spec107
	nop

	.end	test_spec107
