	.text
;	.org	0

	.globl	label1
	.globl	label2
	.globl	label3
	.globl	label4
	.globl	label5
	.globl	label6
	.globl	label7
	.globl	label8
	.globl	label9

	.globl	value8
	.globl	value8_1
	.globl	value8_2
	.globl	value8_3
	.globl	value16
	.globl	value24
	.globl	value32

	.globl	field_0
	.globl	field_1

	call	label1
	call	nz,label2
	call	z,label3
	call	nc,label4
	call	c,label5
	call	po,label6
	call	pe,label7
	call	p,label8
	call	m,label9

	jp	label1
	jp	nz,label2
	jp	z,label3
	jp	nc,label4
	jp	c,label5
	jp	po,label6
	jp	pe,label7
	jp	p,label8
	jp	m,label9

	ld	l,(ix+5)
	ld	a,(ix+field_0)
	ld	c,(ix+field_1-10)
	ld	b,(ix+field_1-11)

field_0_1	.equ	field_0+90

	ld	(iy-5),l
	ld	(iy+field_0),a
	ld	(iy+field_1+10),c
	ld	(iy+field_1+11),b
	ld	h,(iy+field_0_1)

	.ifdef ADLMODE
	ld.is	de,value32 >> 16
	ld.is	hl,value32 & 0xffff
	ld.is	de,(value32 + 0x12345678) >> 16
	ld.is	hl,(value32 + 0x12345678) & 0xffff
	.else
	ld	de,value32 >> 16
	ld	hl,value32 & 0xffff
	ld	de,(value32 + 0x12345678) >> 16
	ld	hl,(value32 + 0x12345678) & 0xffff
	.endif

	ld	d,value32 >> 24
	ld	e,value32 >> 16
	ld	h,value32 >> 8
	ld	l,value32 >> 0

	ld	d,(value32 + 0x12345678) >> 24
	ld	e,(value32 + 0x12345678) >> 16
	ld	h,(value32 + 0x12345678) >> 8
	ld	l,(value32 + 0x12345678) >> 0

	.ifdef Z80N
	push	label1
	push	value16
	nextreg	value8_1,value8_2
	nextreg	value8_3,a
	ld	a,a
	.endif

	.data
	.db	value8
	.dw	value16
	.d24	value24
	.d32	value32
