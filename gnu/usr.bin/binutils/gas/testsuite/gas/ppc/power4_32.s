	.text
start:
	lwz	6,0(7)
	lwz	6,16(7)
	lwz	6,-16(7)
	lwz	6,-32768(7)
	lwz	6,32752(7)
	stw	6,0(7)
	stw	6,16(7)
	stw	6,-16(7)
	stw	6,-32768(7)
	stw	6,32752(7)
	attn
	mtcr	3
	mtcrf	0xff,3
	mtcrf	0x81,3
	mtcrf	0x01,3
	mtcrf	0x02,3
	mtcrf	0x04,3
	mtcrf	0x08,3
	mtcrf	0x10,3
	mtcrf	0x20,3
	mtcrf	0x40,3
	mtcrf	0x80,3
	mfcr	3
	mfcr	3,0x01
	mfcr	3,0x02
	mfcr	3,0x04
	mfcr	3,0x08
	mfcr	3,0x10
	mfcr	3,0x20
	mfcr	3,0x40
	mfcr	3,0x80
	dcbz    1, 2
	dcbzl   3, 4
	dcbz    5, 6
	dcbt    5,6
	dcbt    5,6,0
	dcbt    5,6,8
