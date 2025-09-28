	.data
	.p2align	4
dsym0:	.llong	0xdeadbeef
	.llong	0xc0ffee
dsym1:

	.section	".toc"
	.p2align	4
.L_tsym0:
	.tc	ignored0[TC],dsym0
	.tc	ignored1[TC],dsym1
.L_tsym1:
	.tc	ignored2[TC],usym0
	.tc	ignored3[TC],usym1

	.text
	.p2align	4
	lq	4,dsym0@l(3)
	lq	4,dsym1@l(3)
	lq	4,usym0@l(3)
	lq	4,usym1@l(3)
	lq	4,esym0@l(3)
	lq	4,esym1@l(3)
	lq	4,.L_tsym0@toc(2)
	lq	4,.L_tsym1@toc(2)
	lq	4,.text@l(0)	
	lq	6,dsym0@got(3)
	lq	6,dsym0@got@l(3)
	lq	6,dsym0@plt@l(3)
	lq	6,dsym1@sectoff(3)
	lq	6,dsym1@sectoff@l(3)
	lq	6,usym1-dsym0@l(4)
	stq	6,0(7)
	stq	6,16(7)
	stq	6,-16(7)
	stq	6,-32768(7)
	stq	6,32752(7)

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
#	mfcr	3,0xff	#Error, invalid mask
#	mfcr	3,0x81	#Error, invalid mask
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

	lq 2,16(0)
	lq 0,16(5)
	lq 2,16(5)
	stq 2,16(0)
	stq 0,16(5)
	stq 2,16(5)
	slbia
	hwsync
	sync
	sync    0
	lwsync
	sync    1
	ptesync
	sync    2
	lwarx   20,0,6
	lwarx   20,1,6
	ldarx   21,0,7
	ldarx   21,1,7
	stwcx.  22,0,8
	stwcx.  22,1,8
	stdcx.  23,0,9
	stdcx.  23,1,9

	.section	".data"
usym0:	.llong	0xcafebabe
	.llong	0xc0ffee
usym1:

