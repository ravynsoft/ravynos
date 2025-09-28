# Test C674x instructions.
.text
.nocmp
.globl f
f:
	abs .L1 a5,a7
	abs .L1X b11,a14
	[a1] abs .L2 b16,b19
	[!b2] abs .L2X a7,b31
	[b1] abs .L1 a11:a10,a19:a18
	abs .L2 b13:b12,b1:b0
	abs2 .L1 a9,a10
	[a2] abs2 .L1X b23,a5
	abs2 .L2 b3,b14
	abs2 .L2X a28,b25
	.word 0x0c180b20
	absdp .S1 a7:a6,a25:a24
	[a0] absdp .S2 b3:b2,b5:b4
	.word 0x0c1feb20
	abssp .S1 a9,a8
	abssp .S1X b18,a16
	[b0] abssp .S2 b0,b7
	[!a1] abssp .S2X a1,b26
	add .L1 a5,a10,a20
	[!a2] add .L1X a3,b11,a4
	[!b1] add .L2 b9,b8,b7
	add .L2X b30,a20,b10
	add .L1 a10,a11,a21:a20
	add .L1X a13,b26,a15:a14
	[!a0] add .L2 b29,b28,b27:b26
	add .L2X b25,a24,b23:b22
	[!b0] add .L1 a1,a3:a2,a5:a4
	add .L1X b20,a17:a16,a15:a14
	add .L2 b24,b23:b22,b21:b20
	[b2] add .L2X a6,b17:b16,b15:b14
	add .L1 -16,a5,a6
	[a0] add .L1X 15,b11,a30
	add .L2 -11,b9,b10
	add .L2X 14,a5,b7
	add .L1 5,a3:a2,a7:a6
	[b0] add .L2 -7,b29:b28,b29:b28
	[!a0] add .S1 a11,a12,a13
	add .S1X a14,b15,a16
	add .S2 b17,b18,b19
	add .S2X b20,a30,b25
	add .S1 -16,a4,a11
	add .S1X 13,b9,a23
	[!b0] add .S2 15,b25,b11
	add .S2X -4,a1,b2
	add .D1 a5,a9,a2
	[a1] add .D2 b16,b17,b18
	[b1] add .D1 a5,31,a6
	add .D2 b22,0,b21
	.word 0x01042840
	[!a1] add .D1X a1,b2,a3
	add .D2X b7,a8,b9
	.word 0x00842af0
	add .D2 b4,-5,b21
	[!b1] add .D1X b5,-16,a4
	add .D2X a2,15,b9
	addab .D1 a5,a10,a15
	[a2] addab .D2 b24,b23,b22
	[b2] addab .D1 a25,31,a28
	addab .D2 b4,0,b7
	addab .D1X b14,32767,a5
	addab .D2 b15,32,b29
	addad .D1 a4,a7,a11
	[!a2] addad .D2 b5,b8,b13
	[!b2] addad .D1 a13,31,a4
	addad .D2 b21,0,b5
	addah .D1 a5,a10,a15
	[a0] addah .D2 b24,b23,b22
	[b0] addah .D1 a25,31,a28
	addah .D2 b4,0,b7
	addah .D1X b14,32767,a5
	addah .D2 b15,32,b29
	addaw .D1 a5,a10,a15
	[!a0] addaw .D2 b24,b23,b22
	[!b0] addaw .D1 a25,31,a28
	addaw .D2 b4,0,b7
	addaw .D1X b14,32767,a5
	addaw .D2 b15,32,b29
	adddp .L1 a3:a2,a15:a14,a19:a18
	[b1] adddp .L1X a9:a8,b7:b6,a21:a20
	adddp .L2 b3:b2,b15:b14,b19:b18
	[b1] adddp .L2X b9:b8,a7:a6,b21:b20
	[a1] adddp .S1 a13:a12,a25:a24,a29:a28
	adddp .S1X a19:a18,b17:b16,a31:a30
	[a1] adddp .S2 b13:b12,b25:b24,b29:b28
	adddp .S2X b19:b18,a17:a16,b31:b30
	addk .S1 -32768,a5
	[!a1] addk .S2 32767,b4
	addsp .L1 a5,a6,a7
	[!b1] addsp .L1X a5,b10,a20
	[a2] addsp .L2 b25,b24,b23
	addsp .L2X b30,a20,b10
	addsp .S1 a5,a6,a7
	[b2] addsp .S1X a5,b10,a20
	[!a2] addsp .S2 b25,b24,b23
	addsp .S2X b30,a20,b10
	addsub .L1 a22,a21,a25:a24
	addsub .L1X a20,b19,a17:a16
	addsub .L2 b4,b7,b17:b16
	addsub .L2X b4,a8,b1:b0
	addsub2 .L1 a22,a21,a25:a24
	addsub2 .L1X a20,b19,a17:a16
	addsub2 .L2 b4,b7,b17:b16
	addsub2 .L2X b4,a8,b1:b0
	[!b2] addu .L1 a4,a5,a7:a6
	addu .L1X a20,b19,a29:a28
	[a0] addu .L2 b11,b10,b9:b8
	addu .L2X b4,a7,b3:b2
	addu .L1 a11,a9:a8,a7:a6
	[b0] addu .L1X b20,a21:a20,a23:a22
	[!a0] addu .L2 b23,b21:b20,b27:b26
	addu .L2X a14,b17:b16,b19:b18
	add2 .S1 a7,a6,a5
	[!b0] add2 .S1X a10,b9,a8
	add2 .S2 b18,b17,b16
	[b1] add2 .S2X b22,a29,b21
	add2 .L1 a7,a6,a5
	[a1] add2 .L1X a10,b9,a8
	add2 .L2 b18,b17,b16
	[!a1] add2 .L2X b22,a29,b21
	add2 .D1 a7,a6,a5
	[!b1] add2 .D1X a10,b9,a8
	add2 .D2 b18,b17,b16
	[a2] add2 .D2X b22,a29,b21
	[b2] add4 .L1 a30,a27,a24
	add4 .L1X a23,b24,a25
	add4 .L2 b24,b26,b27
	[!a2] add4 .L2X b14,a17,b20
	[!b2] and .L1 a1,a2,a3
	and .L1X a10,b3,a11
	[a0] and .L2 b19,b23,b29
	and .L2X b7,a8,b9
	and .L1 -16,a4,a5
	[b0] and .L1X 15,b6,a7
	[!a0] and .L2 -3,b20,b18
	and .L2X 9,a20,b18
	[!b0] and .S1 a1,a2,a3
	and .S1X a10,b3,a11
	[a1] and .S2 b19,b23,b29
	and .S2X b7,a8,b9
	and .S1 -16,a4,a5
	[b1] and .S1X 15,b6,a7
	[!a1] and .S2 12,b20,b18
	and .S2X -8,a20,b18
	[!b1] and .D1 a1,a2,a3
	and .D1X a10,b3,a11
	[a2] and .D2 b19,b23,b29
	and .D2X b7,a8,b9
	and .D1 -16,a4,a5
	[b2] and .D1X 15,b6,a7
	[!a2] and .D2 -14,b20,b18
	and .D2X 13,a20,b18
	andn .L1 a20,a18,a17
	[!b2] andn .L1X a16,b15,a14
	[a0] andn .L2 b23,b25,b27
	andn .L2X b4,a5,b8
	andn .S1 a20,a18,a17
	[b0] andn .S1X a16,b15,a14
	[!a0] andn .S2 b23,b25,b27
	andn .S2X b4,a5,b8
	andn .D1 a20,a18,a17
	[!b0] andn .D1X a16,b15,a14
	[a1] andn .D2 b23,b25,b27
	andn .D2X b4,a5,b8
	avg2 .M1 a8,a11,a14
	[b1] avg2 .M1X a17,b20,a23
	avg2 .M2 b26,b29,b0
	[!a1] avg2 .M2X b3,a6,b9
	avgu4 .M1 a8,a11,a14
	[!b1] avgu4 .M1X a17,b20,a23
	avgu4 .M2 b26,b29,b0
	[a2] avgu4 .M2X b3,a6,b9
	b .S2 b4
	b .S2 b3
	[b2] b .S2X a4
	[!a2] call .S2 b4
	call .S2X a4
	callret .S2 b4
	[!b2] callret .S2X a4
	ret .S2 b4
	[a0] ret .S2X a4
	[b0] b .S2 irp
	[!a0] b .S2 nrp
	call .S2 irp
	[a0] call .S2 nrp
	[b0] callret .S2 irp
	callret .S2 nrp
	[b0] ret .S2 irp
	ret .S2 nrp
	bitc4 .M1 a4,a14
	[!b0] bitc4 .M1X b5,a15
	bitc4 .M2 b16,b26
	[b1] bitc4 .M2X a1,b31
	bitr .M1 a4,a14
	[a1] bitr .M1X b5,a15
	bitr .M2 b16,b26
	[!a1] bitr .M2X a1,b31
	bnop .S2 B5,0
	[!b1] bnop .S2X A20,7
	callnop .S2 B5,0
	[a2] callnop .S2X A20,7
	clr .S1 a5,0,31,a10
	[b2] clr .S2 b10,31,0,b5
	[!a2] clr .S1 a7,a14,a21
	clr .S1X b9,a18,a27
	clr .S2 b20,b18,b16
	[!b2] clr .S2X a4,b16,b31
	cmpeq .L1 a1,a3,a3
	[a0] cmpeq .L1X a1,b4,a7
	[b0] cmpeq .L2 b10,b11,b12
	cmpeq .L2X b13,a14,b15
	[!a0] cmpeq .L1 -16,a16,a17
	cmpeq .L1X 15,b18,a19
	cmpeq .L2 3,b20,b22
	[!b0] cmpeq .L2X 4,a23,b25
	cmpeq .L1 a4,a7:a6,a18
	[a1] cmpeq .L1X b9,a11:a10,a20
	cmpeq .L2 b21,b23:b22,b25
	[b1] cmpeq .L2X a19,b25:b24,b27
	[!a1] cmpeq .L1 -16,a15:a14,a22
	[!b1] cmpeq .L2 15,b19:b18,b17
	cmpeq2 .S1 a11,a9,a10
	[a2] cmpeq2 .S1X a12,b14,a15
	cmpeq2 .S2 b16,b20,b24
	[b2] cmpeq2 .S2X b19,a23,b22
	[!a2] cmpeq4 .S1 a20,a23,a26
	cmpeq4 .S1X a31,b4,a15
	[!b2] cmpeq4 .S2 b9,b26,b5
	cmpeq4 .S2X b3,a5,b8
	cmpeqdp .S1 a9:a8,a7:a6,a5
	[a0] cmpeqdp .S1X a3:a2,b1:b0,a31
	[b0] cmpeqdp .S2 b21:b20,b17:b16,b25
	cmpeqdp .S2X b5:b4,a7:a6,b9
	cmpeqsp .S1 a20,a21,a22
	[!b0] cmpeqsp .S1X a23,b24,a25
	[!a0] cmpeqsp .S2 b26,b27,b28
	cmpeqsp .S2X b29,a30,b31
	cmpgt .L1 a1,a3,a3
	[a1] cmpgt .L1X a1,b4,a7
	[b1] cmpgt .L2 b10,b11,b12
	cmpgt .L2X b13,a14,b15
	[!a1] cmpgt .L1 -16,a16,a17
	cmpgt .L1X 15,b18,a19
	cmpgt .L2 3,b20,b22
	[!b1] cmpgt .L2X 4,a23,b25
	cmpgt .L1 a4,a7:a6,a18
	[a2] cmpgt .L1X b9,a11:a10,a20
	cmpgt .L2 b21,b23:b22,b25
	[b2] cmpgt .L2X a19,b25:b24,b27
	[!a2] cmpgt .L1 -16,a15:a14,a22
	[!b2] cmpgt .L2 15,b19:b18,b17
	[a1] cmpgt .L1X b4,a1,a7
	cmpgt .L2X a14,b13,b15
	[!a1] cmpgt .L1 a16,-16,a17
	cmpgt .L1X b18,15,a19
	cmpgt .L2 b20,3,b22
	[!b1] cmpgt .L2X a23,4,b25
	cmpgt .L1 a7:a6,a4,a18
	[a2] cmpgt .L1X a11:a10,b9,a20
	cmpgt .L2 b23:b22,b21,b25
	[b2] cmpgt .L2X b25:b24,a19,b27
	[!a2] cmpgt .L1 a15:a14,-16,a22
	[!b2] cmpgt .L2 b19:b18,15,b17
	cmpgt2 .S1 a16,a15,a14
	[a0] cmpgt2 .S1X a13,b12,a11
	cmpgt2 .S2 b10,b9,b8
	[b0] cmpgt2 .S2X b7,a6,b5
	[!a0] cmpgtdp .S1 a3:a2,a1:a0,a31
	cmpgtdp .S1X a29:a28,b27:b26,a25
	cmpgtdp .S2 b23:b22,b21:b20,b19
	[!b0] cmpgtdp .S2X b17:b16,a15:a14,b13
	[a1] cmpgtsp .S1 a3,a1,a31
	cmpgtsp .S1X a29,b27,a25
	cmpgtsp .S2 b23,b21,b19
	[b1] cmpgtsp .S2X b17,a15,b13
	cmpgtu .L1 a1,a3,a3
	[a1] cmpgtu .L1X a1,b4,a7
	[b1] cmpgtu .L2 b10,b11,b12
	cmpgtu .L2X b13,a14,b15
	[!a1] cmpgtu .L1 0,a16,a17
	cmpgtu .L1X 31,b18,a19
	cmpgtu .L2 3,b20,b22
	[!b1] cmpgtu .L2X 4,a23,b25
	cmpgtu .L1 a4,a7:a6,a18
	[a2] cmpgtu .L1X b9,a11:a10,a20
	cmpgtu .L2 b21,b23:b22,b25
	[b2] cmpgtu .L2X a19,b25:b24,b27
	[!a2] cmpgtu .L1 0,a15:a14,a22
	[!b2] cmpgtu .L2 31,b19:b18,b17
	cmpgtu4 .S1 a25,a27,a23
	[a0] cmpgtu4 .S1X a21,b20,a17
	cmpgtu4 .S2 b11,b13,b17
	[b0] cmpgtu4 .S2X b19,a23,b29
	cmplt .L1 a1,a3,a3
	[a1] cmplt .L1X a1,b4,a7
	[b1] cmplt .L2 b10,b11,b12
	cmplt .L2X b13,a14,b15
	[!a1] cmplt .L1 -16,a16,a17
	cmplt .L1X 15,b18,a19
	cmplt .L2 3,b20,b22
	[!b1] cmplt .L2X 4,a23,b25
	cmplt .L1 a4,a7:a6,a18
	[a2] cmplt .L1X b9,a11:a10,a20
	cmplt .L2 b21,b23:b22,b25
	[b2] cmplt .L2X a19,b25:b24,b27
	[!a2] cmplt .L1 -16,a15:a14,a22
	[!b2] cmplt .L2 15,b19:b18,b17
	[a1] cmplt .L1X b4,a1,a7
	cmplt .L2X a14,b13,b15
	[!a1] cmplt .L1 a16,-16,a17
	cmplt .L1X b18,15,a19
	cmplt .L2 b20,3,b22
	[!b1] cmplt .L2X a23,4,b25
	cmplt .L1 a7:a6,a4,a18
	[a2] cmplt .L1X a11:a10,b9,a20
	cmplt .L2 b23:b22,b21,b25
	[b2] cmplt .L2X b25:b24,a19,b27
	[!a2] cmplt .L1 a15:a14,-16,a22
	[!b2] cmplt .L2 b19:b18,15,b17
	cmplt2 .S1 a16,a15,a14
	[a0] cmplt2 .S1X b12,a13,a11
	cmplt2 .S2 b10,b9,b8
	[b0] cmplt2 .S2X a6,b7,b5
	[!a0] cmpltdp .S1 a3:a2,a1:a0,a31
	cmpltdp .S1X a29:a28,b27:b26,a25
	cmpltdp .S2 b23:b22,b21:b20,b19
	[!b0] cmpltdp .S2X b17:b16,a15:a14,b13
	[a1] cmpltsp .S1 a3,a1,a31
	cmpltsp .S1X a29,b27,a25
	cmpltsp .S2 b23,b21,b19
	[b1] cmpltsp .S2X b17,a15,b13
	cmpltu .L1 a1,a3,a3
	[a1] cmpltu .L1X a1,b4,a7
	[b1] cmpltu .L2 b10,b11,b12
	cmpltu .L2X b13,a14,b15
	[!a1] cmpltu .L1 0,a16,a17
	cmpltu .L1X 31,b18,a19
	cmpltu .L2 3,b20,b22
	[!b1] cmpltu .L2X 4,a23,b25
	cmpltu .L1 a4,a7:a6,a18
	[a2] cmpltu .L1X b9,a11:a10,a20
	cmpltu .L2 b21,b23:b22,b25
	[b2] cmpltu .L2X a19,b25:b24,b27
	[!a2] cmpltu .L1 0,a15:a14,a22
	[!b2] cmpltu .L2 31,b19:b18,b17
	cmpltu4 .S1 a25,a27,a23
	[a0] cmpltu4 .S1X b20,a21,a17
	cmpltu4 .S2 b11,b13,b17
	[b0] cmpltu4 .S2X a23,b19,b29
	cmpy .M1 a1,a2,a5:a4
	cmpy .M1X a4,b5,a7:a6
	cmpy .M2 b8,b9,b11:b10
	cmpy .M2X b11,a12,b13:b12
	cmpyr .M1 a1,a2,a5
	cmpyr .M1X a4,b5,a7
	cmpyr .M2 b8,b9,b11
	cmpyr .M2X b11,a12,b13
	cmpyr1 .M1 a1,a2,a5
	cmpyr1 .M1X a4,b5,a7
	cmpyr1 .M2 b8,b9,b11
	cmpyr1 .M2X b11,a12,b13
	ddotp4 .M1 a1,a2,a5:a4
	ddotp4 .M1X a4,b5,a7:a6
	ddotp4 .M2 b8,b9,b11:b10
	ddotp4 .M2X b11,a12,b13:b12
	ddotph2 .M1 a1:a0,a2,a5:a4
	ddotph2 .M1X a3:a2,b5,a7:a6
	ddotph2 .M2 b7:b6,b9,b11:b10
	ddotph2 .M2X b11:b10,a12,b13:b12
	ddotph2r .M1 a1:a0,a2,a5
	ddotph2r .M1X a3:a2,b5,a7
	ddotph2r .M2 b7:b6,b9,b11
	ddotph2r .M2X b11:b10,a12,b13
	ddotpl2 .M1 a1:a0,a2,a5:a4
	ddotpl2 .M1X a3:a2,b5,a7:a6
	ddotpl2 .M2 b7:b6,b9,b11:b10
	ddotpl2 .M2X b11:b10,a12,b13:b12
	ddotpl2r .M1 a1:a0,a2,a5
	ddotpl2r .M1X a3:a2,b5,a7
	ddotpl2r .M2 b7:b6,b9,b11
	ddotpl2r .M2X b11:b10,a12,b13
	deal .M1 a8,a9
	[!a0] deal .M1X b10,a11
	[!b0] deal .M2 b12,b13
	deal .M2X a14,b15
	dint
	[a1] dmv .S1 a4,a5,a7:a6
	dmv .S1X a8,b9,a11:a10
	dmv .S2 b12,b13,b15:b14
	[b1] dmv .S2X b16,a17,b19:b18
	dotp2 .M1 a20,a15,a10
	[!a1] dotp2 .M1X a10,b5,a0
	dotp2 .M2 b7,b14,b21
	[!b1] dotp2 .M2X b23,a20,b17
	dotp2 .M1 a20,a15,a11:a10
	[a2] dotp2 .M1X a10,b5,a1:a0
	[b2] dotp2 .M2 b7,b14,b21:b20
	dotp2 .M2X b23,a20,b17:b16
	dotpn2 .M1 a20,a15,a10
	[!a2] dotpn2 .M1X a10,b5,a0
	dotpn2 .M2 b7,b14,b21
	[!b2] dotpn2 .M2X b23,a20,b17
	dotpnrsu2 .M1 a20,a15,a10
	[a0] dotpnrsu2 .M1X a10,b5,a0
	dotpnrsu2 .M2 b7,b14,b21
	[b0] dotpnrsu2 .M2X b23,a20,b17
	[!a0] dotpnrus2 .M1 a20,a15,a10
	dotpnrus2 .M1X b5,a10,a0
	dotpnrus2 .M2 b7,b14,b21
	[!b0] dotpnrus2 .M2X a20,b23,b17
	dotprsu2 .M1 a20,a15,a10
	[a1] dotprsu2 .M1X a10,b5,a0
	dotprsu2 .M2 b7,b14,b21
	[b1] dotprsu2 .M2X b23,a20,b17
	[!a1] dotprus2 .M1 a20,a15,a10
	dotprus2 .M1X b5,a10,a0
	dotprus2 .M2 b7,b14,b21
	[!b1] dotprus2 .M2X a20,b23,b17
	dotpsu4 .M1 a20,a15,a10
	[a2] dotpsu4 .M1X a10,b5,a0
	dotpsu4 .M2 b7,b14,b21
	[b2] dotpsu4 .M2X b23,a20,b17
	dotpus4 .M1 a20,a15,a10
	[!a2] dotpus4 .M1X b5,a10,a0
	dotpus4 .M2 b7,b14,b21
	[!b2] dotpus4 .M2X a20,b23,b17
	dotpu4 .M1 a20,a15,a10
	[a0] dotpu4 .M1X a10,b5,a0
	dotpu4 .M2 b7,b14,b21
	[b0] dotpu4 .M2X b23,a20,b17
	dpack2 .L1 a30,a27,a25:a24
	dpack2 .L1X a21,b18,a15:a14
	dpack2 .L2 b12,b9,b7:b6
	dpack2 .L2X b3,a0,b29:b28
	dpackx2 .L1 a30,a27,a25:a24
	dpackx2 .L1X a21,b18,a15:a14
	dpackx2 .L2 b12,b9,b7:b6
	dpackx2 .L2X b3,a0,b29:b28
	.word 0x01900118
	dpint .L1 a5:a4,a3
	[!a0] dpint .L2 b5:b4,b3
	.word 0x0197e118
	.word 0x01900138
	dpsp .L1 a5:a4,a3
	[!b0] dpsp .L2 b5:b4,b3
	.word 0x0197e138
	.word 0x0190003a
	[a1] dptrunc .L1 a5:a4,a3
	dptrunc .L2 b5:b4,b3
	.word 0x0197e03a
	ext .S1 a5,0,31,a10
	[b1] ext .S2 b10,31,0,b5
	[!a1] ext .S1 a7,a14,a21
	ext .S1X b9,a18,a27
	ext .S2 b20,b18,b16
	[!b1] ext .S2X a4,b16,b31
	extu .S1 a5,0,31,a10
	[a2] extu .S2 b10,31,0,b5
	[b2] extu .S1 a7,a14,a21
	extu .S1X b9,a18,a27
	extu .S2 b20,b18,b16
	[!a2] extu .S2X a4,b16,b31
	gmpy .M1 a25,a16,a9
	gmpy .M2 b5,b12,b13
	[!b2] gmpy4 .M1 a2,a3,a5
	gmpy4 .M1X a7,b11,a13
	gmpy4 .M2 b17,b19,b23
	[a0] gmpy4 .M2X b29,a31,b5
	idle
	intdp .L1 a1,a3:a2
	[b0] intdp .L1X b5,a7:a6
	[!a0] intdp .L2 b9,b11:b10
	intdp .L2X a5,b13:b12
	intdpu .L1 a1,a3:a2
	[!b0] intdpu .L1X b5,a7:a6
	[a1] intdpu .L2 b9,b11:b10
	intdpu .L2X a5,b13:b12
	intsp .L1 a1,a3
	[b1] intsp .L1X b5,a7
	[!a1] intsp .L2 b9,b11
	intsp .L2X a5,b13
	intspu .L1 a1,a3
	[!b1] intspu .L1X b5,a7
	[a2] intspu .L2 b9,b11
	intspu .L2X a5,b13
	ldb .D1T1 *a5,a7
	[b2] ldb .D1T2 *++a9,b11
	ldb .D2T1 *--b13,a15
	[!a2] ldb .D2T2 *b17++,b19
	ldb .D1T1 *a21--,a23
	[!b2] ldb .D2T2 *-b25[31],b27
	ldb .D1T1 *+a29[0],a31
	ldb .D1T1 *-a0(2),a2
	ldb .D1T1 *-a4[a5],a6
	ldb .D1T1 *+a7[a8],a9
	ldb .D1T1 *--a10[11],a12
	ldb .D1T1 *++a13(14),a15
	ldb .D1T1 *a16--(17),a18
	ldb .D1T1 *a19++(20),a21
	ldb .D1T1 *--a22[a23],a24
	ldb .D1T1 *++a25[a26],a27
	ldb .D1T1 *a28--[a29],a30
	ldb .D1T1 *a31++[a0],a1
	[a0] ldb .D2T1 *+b14(32767),a15
	ldb .D2T2 *+b15[32767],b16
	ldbu .D1T1 *a5,a7
	[b2] ldbu .D1T2 *++a9,b11
	ldbu .D2T1 *--b13,a15
	[!a2] ldbu .D2T2 *b17++,b19
	ldbu .D1T1 *a21--,a23
	[!b2] ldbu .D2T2 *-b25[31],b27
	ldbu .D1T1 *+a29[0],a31
	ldbu .D1T1 *-a0(2),a2
	ldbu .D1T1 *-a4[a5],a6
	ldbu .D1T1 *+a7[a8],a9
	ldbu .D1T1 *--a10[11],a12
	ldbu .D1T1 *++a13(14),a15
	ldbu .D1T1 *a16--(17),a18
	ldbu .D1T1 *a19++(20),a21
	ldbu .D1T1 *--a22[a23],a24
	ldbu .D1T1 *++a25[a26],a27
	ldbu .D1T1 *a28--[a29],a30
	ldbu .D1T1 *a31++[a0],a1
	[a0] ldbu .D2T1 *+b14(32767),a15
	ldbu .D2T2 *+b15[32767],b16
	lddw .D1T1 *a5,a7:a6
	[b2] lddw .D1T2 *++a9,b11:b10
	lddw .D2T1 *--b13,a15:a14
	[!a2] lddw .D2T2 *b17++,b19:b18
	lddw .D1T1 *a21--,a23:a22
	[!b2] lddw .D2T2 *-b25[31],b27:b26
	lddw .D1T1 *+a29[0],a31:a30
	lddw .D1T1 *-a0(248),a3:a2
	lddw .D1T1 *-a4[a5],a7:a6
	lddw .D1T1 *+a7[a8],a9:a8
	lddw .D1T1 *--a10[11],a13:a12
	lddw .D1T1 *++a13(16),a15:a14
	lddw .D1T1 *a16--(24),a19:a18
	lddw .D1T1 *a19++(32),a21:a20
	lddw .D1T1 *--a22[a23],a25:a24
	lddw .D1T1 *++a25[a26],a27:a26
	lddw .D1T1 *a28--[a29],a31:a30
	lddw .D1T1 *a31++[a0],a1:a0
	ldh .D1T1 *a5,a7
	[b2] ldh .D1T2 *++a9,b11
	ldh .D2T1 *--b13,a15
	[!a2] ldh .D2T2 *b17++,b19
	ldh .D1T1 *a21--,a23
	[!b2] ldh .D2T2 *-b25[31],b27
	ldh .D1T1 *+a29[0],a31
	ldh .D1T1 *-a0(62),a2
	ldh .D1T1 *-a4[a5],a6
	ldh .D1T1 *+a7[a8],a9
	ldh .D1T1 *--a10[11],a12
	ldh .D1T1 *++a13(14),a15
	ldh .D1T1 *a16--(18),a18
	ldh .D1T1 *a19++(20),a21
	ldh .D1T1 *--a22[a23],a24
	ldh .D1T1 *++a25[a26],a27
	ldh .D1T1 *a28--[a29],a30
	ldh .D1T1 *a31++[a0],a1
	[a0] ldh .D2T1 *+b14(65534),a15
	ldh .D2T2 *+b15[32767],b16
	ldhu .D1T1 *a5,a7
	[b2] ldhu .D1T2 *++a9,b11
	ldhu .D2T1 *--b13,a15
	[!a2] ldhu .D2T2 *b17++,b19
	ldhu .D1T1 *a21--,a23
	[!b2] ldhu .D2T2 *-b25[31],b27
	ldhu .D1T1 *+a29[0],a31
	ldhu .D1T1 *-a0(62),a2
	ldhu .D1T1 *-a4[a5],a6
	ldhu .D1T1 *+a7[a8],a9
	ldhu .D1T1 *--a10[11],a12
	ldhu .D1T1 *++a13(14),a15
	ldhu .D1T1 *a16--(18),a18
	ldhu .D1T1 *a19++(20),a21
	ldhu .D1T1 *--a22[a23],a24
	ldhu .D1T1 *++a25[a26],a27
	ldhu .D1T1 *a28--[a29],a30
	ldhu .D1T1 *a31++[a0],a1
	[a0] ldhu .D2T1 *+b14(65534),a15
	ldhu .D2T2 *+b15[32767],b16
	ldndw .D1T1 *a5,a7:a6
	[b2] ldndw .D1T2 *++a9,b11:b10
	ldndw .D2T1 *--b13,a15:a14
	[!a2] ldndw .D2T2 *b17++,b19:b18
	ldndw .D1T1 *a21--,a23:a22
	[!b2] ldndw .D2T2 *-b25[31],b27:b26
	ldndw .D1T1 *+a29[0],a31:a30
	ldndw .D1T1 *-a0(31),a3:a2
	ldndw .D1T1 *-a4[a5],a7:a6
	ldndw .D1T1 *+a7(a8),a9:a8
	ldndw .D1T1 *--a10[11],a13:a12
	ldndw .D1T1 *++a13(16),a15:a14
	ldndw .D1T1 *a16--(24),a19:a18
	ldndw .D1T1 *a19++(30),a21:a20
	ldndw .D1T1 *--a22[a23],a25:a24
	ldndw .D1T1 *++a25(a26),a27:a26
	ldndw .D1T1 *a28--[a29],a31:a30
	ldndw .D1T1 *a31++(a0),a1:a0
	ldnw .D1T1 *a5,a7
	[b2] ldnw .D1T2 *++a9,b11
	ldnw .D2T1 *--b13,a15
	[!a2] ldnw .D2T2 *b17++,b19
	ldnw .D1T1 *a21--,a23
	[!b2] ldnw .D2T2 *-b25[31],b27
	ldnw .D1T1 *+a29[0],a31
	ldnw .D1T1 *-a0(124),a2
	ldnw .D1T1 *-a4[a5],a6
	ldnw .D1T1 *+a7[a8],a9
	ldnw .D1T1 *--a10[11],a12
	ldnw .D1T1 *++a13(16),a15
	ldnw .D1T1 *a16--(20),a18
	ldnw .D1T1 *a19++(24),a21
	ldnw .D1T1 *--a22[a23],a24
	ldnw .D1T1 *++a25[a26],a27
	ldnw .D1T1 *a28--[a29],a30
	ldnw .D1T1 *a31++[a0],a1
	ldw .D1T1 *a5,a7
	[b2] ldw .D1T2 *++a9,b11
	ldw .D2T1 *--b13,a15
	[!a2] ldw .D2T2 *b17++,b19
	ldw .D1T1 *a21--,a23
	[!b2] ldw .D2T2 *-b25[31],b27
	ldw .D1T1 *+a29[0],a31
	ldw .D1T1 *-a0(124),a2
	ldw .D1T1 *-a4[a5],a6
	ldw .D1T1 *+a7[a8],a9
	ldw .D1T1 *--a10[11],a12
	ldw .D1T1 *++a13(16),a15
	ldw .D1T1 *a16--(20),a18
	ldw .D1T1 *a19++(24),a21
	ldw .D1T1 *--a22[a23],a24
	ldw .D1T1 *++a25[a26],a27
	ldw .D1T1 *a28--[a29],a30
	ldw .D1T1 *a31++[a0],a1
	[a0] ldw .D2T1 *+b14(131068),a15
	ldw .D2T2 *+b15[32767],b16
	lmbd .L1 a5,a8,a13
	[b0] lmbd .L1X a21,b2,a23
	[!a0] lmbd .L2 b25,b16,b9
	lmbd .L2X b1,a2,b3
	lmbd .L1 0,a8,a13
	[!b0] lmbd .L1X 1,b2,a23
	[a1] lmbd .L2 15,b16,b9
	lmbd .L2X -16,a2,b3
	max2 .L1 a1,a2,a3
	[b1] max2 .L1X a4,b5,a6
	[!a1] max2 .L2 b7,b8,b9
	max2 .L2X b10,a11,b12
	max2 .S1 a1,a2,a3
	[!b1] max2 .S1X a4,b5,a6
	max2 .S2 b7,b8,b9
	[a2] max2 .S2X b10,a11,b12
	[b2] maxu4 .L1 a13,a14,a15
	maxu4 .L1X a16,b17,a18
	maxu4 .L2 b19,b20,b21
	[!a2] maxu4 .L2X b22,a23,b24
	min2 .L1 a1,a2,a3
	[!b2] min2 .L1X a4,b5,a6
	[a0] min2 .L2 b7,b8,b9
	min2 .L2X b10,a11,b12
	min2 .S1 a1,a2,a3
	[b0] min2 .S1X a4,b5,a6
	min2 .S2 b7,b8,b9
	[!a0] min2 .S2X b10,a11,b12
	[!b0] minu4 .L1 a13,a14,a15
	minu4 .L1X a16,b17,a18
	minu4 .L2 b19,b20,b21
	[a1] minu4 .L2X b22,a23,b24
	mpy .M1 a25,a26,a27
	[b1] mpy .M1X a28,b29,a30
	[!a1] mpy .M2 b31,b0,b1
	mpy .M2X b2,a3,b4
	[!b1] mpy .M1 -16,a5,a6
	mpy .M1X 15,b7,a8
	mpy .M2 5,b9,b10
	[a2] mpy .M2X -4,a11,b12
	mpydp .M1 a1:a0,a3:a2,a5:a4
	mpydp .M2X b1:b0,a1:a0,b1:b0
	[b2] mpydp .M2 b7:b6,b9:b8,b11:b10
	mpyh .M1 a0,a1,a2
	[!a2] mpyh .M1X a3,b4,a5
	[!b2] mpyh .M2 b6,b7,b8
	mpyh .M2X b9,a10,b11
	mpyhi .M1 a0,a1,a3:a2
	[a0] mpyhi .M1X a3,b4,a5:a4
	[b0] mpyhi .M2 b6,b7,b9:b8
	mpyhi .M2X b9,a10,b11:b10
	mpyhir .M1 a0,a1,a2
	[!a0] mpyhir .M1X a3,b4,a5
	[!b0] mpyhir .M2 b6,b7,b8
	mpyhir .M2X b9,a10,b11
	mpyhl .M1 a0,a1,a2
	[a1] mpyhl .M1X a3,b4,a5
	[b1] mpyhl .M2 b6,b7,b8
	mpyhl .M2X b9,a10,b11
	mpyhlu .M1 a0,a1,a2
	[!a1] mpyhlu .M1X a3,b4,a5
	[!b1] mpyhlu .M2 b6,b7,b8
	mpyhlu .M2X b9,a10,b11
	mpyhslu .M1 a0,a1,a2
	[a2] mpyhslu .M1X a3,b4,a5
	[b2] mpyhslu .M2 b6,b7,b8
	mpyhslu .M2X b9,a10,b11
	mpyhsu .M1 a0,a1,a2
	[!a2] mpyhsu .M1X a3,b4,a5
	[!b2] mpyhsu .M2 b6,b7,b8
	mpyhsu .M2X b9,a10,b11
	mpyhu .M1 a0,a1,a2
	[a0] mpyhu .M1X a3,b4,a5
	[b0] mpyhu .M2 b6,b7,b8
	mpyhu .M2X b9,a10,b11
	mpyhuls .M1 a0,a1,a2
	[!a0] mpyhuls .M1X a3,b4,a5
	[!b0] mpyhuls .M2 b6,b7,b8
	mpyhuls .M2X b9,a10,b11
	mpyhus .M1 a0,a1,a2
	[a1] mpyhus .M1X a3,b4,a5
	[b1] mpyhus .M2 b6,b7,b8
	mpyhus .M2X b9,a10,b11
	[!a1] mpyi .M1 a0,a1,a2
	mpyi .M1X a3,b4,a5
	[!b1] mpyi .M2 b6,b7,b8
	mpyi .M2X b9,a10,b11
	[a2] mpyi .M1 -16,a1,a2
	mpyi .M1X 15,b4,a5
	[b2] mpyi .M2 7,b7,b8
	mpyi .M2X -6,a10,b11
	mpyid .M1 a0,a1,a3:a2
	[!a2] mpyid .M1X a3,b4,a5:a4
	[!b2] mpyid .M2 b6,b7,b9:b8
	mpyid .M2X b9,a10,b11:b10
	mpyid .M1 -16,a1,a3:a2
	[a0] mpyid .M1X 2,b4,a5:a4
	mpyid .M2 15,b7,b9:b8
	[b0] mpyid .M2X -7,a10,b11:b10
	mpyih .M1 a0,a1,a3:a2
	[!a0] mpyih .M1X b4,a3,a5:a4
	[!b0] mpyih .M2 b6,b7,b9:b8
	mpyih .M2X a10,b9,b11:b10
	[a1] mpyihr .M1 a0,a1,a2
	mpyihr .M1X b4,a3,a5
	[b1] mpyihr .M2 b6,b7,b8
	mpyihr .M2X a10,b9,b11
	mpyil .M1 a0,a1,a3:a2
	[!a1] mpyil .M1X b4,a3,a5:a4
	mpyil .M2 b6,b7,b9:b8
	[!b1] mpyil .M2X a10,b9,b11:b10
	[a2] mpyilr .M1 a0,a1,a2
	mpyilr .M1X b4,a3,a5
	mpyilr .M2 b6,b7,b8
	[b2] mpyilr .M2X a10,b9,b11
	mpylh .M1 a0,a1,a2
	[!a2] mpylh .M1X a3,b4,a5
	[!b2] mpylh .M2 b6,b7,b8
	mpylh .M2X b9,a10,b11
	mpylhu .M1 a0,a1,a2
	[a0] mpylhu .M1X a3,b4,a5
	[b0] mpylhu .M2 b6,b7,b8
	mpylhu .M2X b9,a10,b11
	mpyli .M1 a0,a1,a3:a2
	[!a0] mpyli .M1X a3,b4,a5:a4
	[!b0] mpyli .M2 b6,b7,b9:b8
	mpyli .M2X b9,a10,b11:b10
	mpylir .M1 a0,a1,a2
	[a1] mpylir .M1X a3,b4,a5
	mpylir .M2 b6,b7,b8
	[b1] mpylir .M2X b9,a10,b11
	[!a1] mpylshu .M1 a0,a1,a2
	mpylshu .M1X a3,b4,a5
	mpylshu .M2 b6,b7,b8
	[!b1] mpylshu .M2X b9,a10,b11
	mpyluhs .M1 a0,a1,a2
	[a2] mpyluhs .M1X a3,b4,a5
	mpyluhs .M2 b6,b7,b8
	[b2] mpyluhs .M2X b9,a10,b11
	mpysp .M1 a0,a1,a2
	[!a2] mpysp .M1X a3,b4,a5
	mpysp .M2 b6,b7,b8
	[!b2] mpysp .M2X b9,a10,b11
	[a0] mpyspdp .M1 a12,a15:a14,a17:a16
	mpyspdp .M1X a18,b19:b18,a21:a20
	mpyspdp .M2 b22,b25:b24,b27:b26
	[b0] mpyspdp .M2X b29,a31:a30,b1:b0
	mpysp2dp .M1 a0,a1,a3:a2
	[!a0] mpysp2dp .M1X a3,b4,a5:a4
	[!b0] mpysp2dp .M2 b6,b7,b9:b8
	mpysp2dp .M2X b9,a10,b11:b10
	[a1] mpysu .M1 a0,a1,a2
	mpysu .M1X a3,b4,a5
	[b1] mpysu .M2 b6,b7,b8
	mpysu .M2X b9,a10,b11
	[!a1] mpysu .M1 -16,a1,a2
	mpysu .M1X 15,b4,a5
	mpysu .M2 3,b7,b8
	[!b1] mpysu .M2X -9,a10,b11
	mpysu4 .M1 a0,a1,a3:a2
	[!a0] mpysu4 .M1X a3,b4,a5:a4
	[!b0] mpysu4 .M2 b6,b7,b9:b8
	mpysu4 .M2X b9,a10,b11:b10
	[a1] mpyu .M1 a0,a1,a2
	mpyu .M1X a3,b4,a5
	mpyu .M2 b6,b7,b8
	[b1] mpyu .M2X b9,a10,b11
	mpyu4 .M1 a0,a1,a3:a2
	[!a1] mpyu4 .M1X a3,b4,a5:a4
	[!b1] mpyu4 .M2 b6,b7,b9:b8
	mpyu4 .M2X b9,a10,b11:b10
	[a2] mpyus .M1 a0,a1,a2
	mpyus .M1X a3,b4,a5
	mpyus .M2 b6,b7,b8
	[b2] mpyus .M2X b9,a10,b11
	mpyus4 .M1 a0,a1,a3:a2
	[!a2] mpyus4 .M1X b4,a3,a5:a4
	[!b2] mpyus4 .M2 b6,b7,b9:b8
	mpyus4 .M2X a10,b9,b11:b10
	mpy2 .M1 a0,a1,a3:a2
	[a0] mpy2 .M1X a3,b4,a5:a4
	[b0] mpy2 .M2 b6,b7,b9:b8
	mpy2 .M2X b9,a10,b11:b10
	mpy2ir .M1 a0,a1,a3:a2
	mpy2ir .M1X a3,b4,a5:a4
	mpy2ir .M2 b6,b7,b9:b8
	mpy2ir .M2X b9,a10,b11:b10
	[!a0] mpy32 .M1 a0,a1,a2
	mpy32 .M1X a3,b4,a5
	mpy32 .M2 b6,b7,b8
	[!b0] mpy32 .M2X b9,a10,b11
	mpy32 .M1 a0,a1,a3:a2
	[a1] mpy32 .M1X a3,b4,a5:a4
	[b1] mpy32 .M2 b6,b7,b9:b8
	mpy32 .M2X b9,a10,b11:b10
	mpy32su .M1 a0,a1,a3:a2
	[!a1] mpy32su .M1X a3,b4,a5:a4
	[!b1] mpy32su .M2 b6,b7,b9:b8
	mpy32su .M2X b9,a10,b11:b10
	mpy32u .M1 a0,a1,a3:a2
	[a2] mpy32u .M1X a3,b4,a5:a4
	[b2] mpy32u .M2 b6,b7,b9:b8
	mpy32u .M2X b9,a10,b11:b10
	mpy32us .M1 a0,a1,a3:a2
	[!a2] mpy32us .M1X a3,b4,a5:a4
	[!b2] mpy32us .M2 b6,b7,b9:b8
	mpy32us .M2X b9,a10,b11:b10
	[a0] mv .L1 a5,a7
	mv .L1X b8,a13
	[b0] mv .L2 b12,b15
	mv .L2X a17,b19
	[!a0] mv .S1 a5,a7
	mv .S1X b8,a13
	mv .S2 b12,b15
	[!b0] mv .S2X a17,b19
	[a1] mv .D1 a5,a7
	mv .D1X b8,a13
	[b1] mv .D2 b12,b15
	mv .D2X a17,b19
	[a0] mvc .S2 amr,b5
	mvc .S2 b6,amr
	[b0] mvc .S2X a7,amr
	mvc .S2 csr,b8
	mvc .S2 b8,csr
	mvc .S2 dnum,b9
	mvc .S2 b10,ecr
	mvc .S2 efr,b11
	mvc .S2 fadcr,b12
	mvc .S2 b13,fadcr
	mvc .S2 faucr,b14
	mvc .S2 b15,faucr
	mvc .S2 fmcr,b16
	mvc .S2 b17,fmcr
	mvc .S2 gfpgfr,b18
	mvc .S2 b19,gfpgfr
	mvc .S2 gplya,b20
	mvc .S2 b21,gplya
	mvc .S2 gplyb,b22
	mvc .S2 b23,gplyb
	mvc .S2 b24,icr
	mvc .S2 ier,b25
	mvc .S2 b26,ier
	mvc .S2 ierr,b27
	mvc .S2 b28,ierr
	mvc .S2 ifr,b29
	mvc .S2 ilc,b30
	mvc .S2 b31,ilc
	mvc .S2 irp,b0
	mvc .S2 b1,irp
	mvc .S2 b2,isr
	mvc .S2 istp,b3
	mvc .S2 b4,istp
	mvc .S2 itsr,b5
	mvc .S2 b6,itsr
	mvc .S2 nrp,b7
	mvc .S2 b8,nrp
	mvc .S2 ntsr,b9
	mvc .S2 b10,ntsr
	mvc .S2 pce1,b11
	mvc .S2 rep,b12
	mvc .S2 b13,rep
	mvc .S2 rilc,b14
	mvc .S2 b15,rilc
	mvc .S2 ssr,b16
	mvc .S2 b17,ssr
	mvc .S2 tsch,b18
	mvc .S2 tscl,b19
	mvc .S2 b20,tscl
	mvc .S2 tsr,b21
	mvc .S2 b22,tsr
	.word 0x0001e3e2
	.word 0x0005e3e2
	.word 0x0181e3a2
	.word 0x0201e3a2
	.word 0x0301e3a2
	.word 0x0101e3a2
	.word 0x0281e3a2
	.word 0x0381e3a2
	.word 0x004203e2
	mvd .M1 a4,a5
	[!a0] mvd .M1X b6,a7
	[!b0] mvd .M2 b8,b9
	mvd .M2X a10,b11
	[!b1] mvk .S1 -32768,a5
	mvk .S2 32767,b4
	mvk .L1 -16,a4
	[a1] mvk .L2 15,b4
	[b1] mvk .D1 6,a4
	mvk .D2 -9,b12
	mvkh .S1 0x12345678,a6
	[a2] mvkh .S2 0xfedcba98,b7
	[b2] mvklh .S1 0x12345678,a6
	mvklh .S2 0xfedcba98,b7
	mvkl .S1 0x12345678,a6
	[!a2] mvkl .S2 0xfedcba98,b7
	neg .S1 a5,a6
	[a0] neg .S1X b7,a8
	[b0] neg .S2 b9,b10
	neg .S2X a11,b12
	[!a0] neg .L1 a13,a14
	neg .L1X b15,a16
	neg .L2 b17,b18
	[!b0] neg .L2X a19,b20
	[b1] neg .L1 a21:a20,a23:a22
	neg .L2 b25:b24,b27:b26
	nop 1
	nop 2
	nop
	nop 3
	nop 4
	nop 5
	nop 6
	nop 7
	nop 8
	nop 9
	[!b2] norm .L1 a4,a5
	norm .L1X b6,a7
	norm .L2 b8,b9
	[a0] norm .L2X a10,b11
	norm .L1 a5:a4,a6
	[b0] norm .L2 b9:b8,b10
	not .L1 a1,a2
	[b2] not .L1X b3,a4
	[!a2] not .L2 b5,b6
	not .L2X a7,b8
	[!b2] not .S1 a1,a2
	not .S1X b3,a4
	not .S2 b5,b6
	[a0] not .S2X a7,b8
	[b0] not .D1 a1,a2
	not .D1X b3,a4
	not .D2 b5,b6
	[!a0] not .D2X a7,b8
	[!a0] or .D1 a1,a2,a3
	or .D1X a4,b5,a6
	or .D2 b7,b8,b9
	[!b0] or .D2X b10,a11,b12
	or .D1 -16,a2,a3
	[a1] or .D1X 11,b5,a6
	[b1] or .D2 15,b8,b9
	or .D2X -13,a11,b12
	[!a1] or .L1 a1,a2,a3
	or .L1X a4,b5,a6
	or .L2 b7,b8,b9
	[!b1] or .L2X b10,a11,b12
	or .L1 -16,a2,a3
	[a2] or .L1X 11,b5,a6
	[b2] or .L2 15,b8,b9
	or .L2X -13,a11,b12
	[!a2] or .S1 a1,a2,a3
	or .S1X a4,b5,a6
	or .S2 b7,b8,b9
	[!b2] or .S2X b10,a11,b12
	or .S1 -16,a2,a3
	[a0] or .S1X 11,b5,a6
	[b0] or .S2 15,b8,b9
	or .S2X -13,a11,b12
	[!a0] pack2 .L1 a1,a2,a3
	pack2 .L1X a5,b8,a13
	pack2 .L2 b21,b2,b23
	[!b0] pack2 .L2X b25,a16,b9
	[a1] pack2 .S1 a1,a2,a3
	pack2 .S1X a5,b8,a13
	pack2 .S2 b21,b2,b23
	[b1] pack2 .S2X b25,a16,b9
	[!a1] packh2 .L1 a1,a2,a3
	packh2 .L1X a5,b8,a13
	packh2 .L2 b21,b2,b23
	[!b1] packh2 .L2X b25,a16,b9
	[a2] packh2 .S1 a1,a2,a3
	packh2 .S1X a5,b8,a13
	packh2 .S2 b21,b2,b23
	[b2] packh2 .S2X b25,a16,b9
	[!a2] packh4 .L1 a1,a2,a3
	packh4 .L1X a5,b8,a13
	packh4 .L2 b21,b2,b23
	[!b2] packh4 .L2X b25,a16,b9
	[a0] packhl2 .L1 a1,a2,a3
	packhl2 .L1X a5,b8,a13
	packhl2 .L2 b21,b2,b23
	[b0] packhl2 .L2X b25,a16,b9
	packhl2 .S1 a1,a2,a3
	[!a0] packhl2 .S1X a5,b8,a13
	[!b0] packhl2 .S2 b21,b2,b23
	packhl2 .S2X b25,a16,b9
	[a1] packlh2 .L1 a1,a2,a3
	packlh2 .L1X a5,b8,a13
	packlh2 .L2 b21,b2,b23
	[b1] packlh2 .L2X b25,a16,b9
	packlh2 .S1 a1,a2,a3
	[!a1] packlh2 .S1X a5,b8,a13
	[!b1] packlh2 .S2 b21,b2,b23
	packlh2 .S2X b25,a16,b9
	[a2] packl4 .L1 a1,a2,a3
	packl4 .L1X a5,b8,a13
	packl4 .L2 b21,b2,b23
	[b2] packl4 .L2X b25,a16,b9
	.word 0x03100b60
	rcpdp .S1 a5:a4,a7:a6
	[!a2] rcpdp .S2 b9:b8,b11:b10
	.word 0x0317eb60
	rcpsp .S1 a0,a1
	[!b2] rcpsp .S1X b2,a3
	[a0] rcpsp .S2 b4,b5
	rcpsp .S2X a6,b7
	rint
	[b0] rotl .M1 a0,a1,a2
	rotl .M1X b3,a4,a5
	rotl .M2 b6,b7,b8
	[!a0] rotl .M2X a9,b10,b11
	rotl .M1 a12,0,a13
	[!b0] rotl .M1X b14,31,a15
	[a1] rotl .M2 b16,17,b17
	rotl .M2X a18,25,b19
	rpack2 .S1 a1,a2,a3
	rpack2 .S1X a4,b5,a6
	rpack2 .S2 b7,b8,b9
	rpack2 .S2X b10,a11,b12
	.word 0x03100ba0
	rsqrdp .S1 a5:a4,a7:a6
	[b1] rsqrdp .S2 b9:b8,b11:b10
	.word 0x0317eba0
	rsqrsp .S1 a0,a1
	[!a1] rsqrsp .S1X b2,a3
	[!b1] rsqrsp .S2 b4,b5
	rsqrsp .S2X a6,b7
	sadd .L1 a1,a2,a3
	[a2] sadd .L1X a4,b5,a6
	[b2] sadd .L2 b7,b8,b9
	sadd .L2X b10,a11,b12
	[!a2] sadd .L1 a13,a15:a14,a17:a16
	sadd .L1X b18,a21:a20,a23:a22
	sadd .L2 b24,b27:b26,b29:b28
	[!b2] sadd .L2X a30,b1:b0,b3:b2
	sadd .L1 -16,a4,a5
	[a0] sadd .L1X 15,b6,a7
	[b0] sadd .L2 12,b8,b9
	sadd .L2X -11,a10,b11
	sadd .L1 -16,a13:a12,a15:a14
	[!a0] sadd .L2 15,b21:b20,b23:b22
	[!b0] sadd .S1 a28,a29,a30
	sadd .S1X a31,b0,a1
	sadd .S2 b2,b3,b4
	[a1] sadd .S2X b5,a6,b7
	sadd2 .S1 a1,a2,a3
	[b1] sadd2 .S1X a4,b5,a6
	[!a1] sadd2 .S2 b7,b8,b9
	sadd2 .S2X b10,a11,b12
	saddsub .L1 a0,a1,a3:a2
	saddsub .L1X a4,b5,a7:a6
	saddsub .L2 b8,b9,b11:b10
	saddsub .L2X b12,a13,b15:b14
	saddsub2 .L1 a0,a1,a3:a2
	saddsub2 .L1X a4,b5,a7:a6
	saddsub2 .L2 b8,b9,b11:b10
	saddsub2 .L2X b12,a13,b15:b14
	[!b1] saddsu2 .S1 a16,a17,a18
	saddsu2 .S1X b19,a20,a21
	saddsu2 .S2 b22,b23,b24
	[a2] saddsu2 .S2X a25,b26,b27
	saddus2 .S1 a28,a29,a30
	[b2] saddus2 .S1X a31,b0,a1
	[!a2] saddus2 .S2 b2,b3,b4
	saddus2 .S2X b5,a6,b7
	saddu4 .S1 a28,a29,a30
	[!b2] saddu4 .S1X a31,b0,a1
	[a0] saddu4 .S2 b2,b3,b4
	saddu4 .S2X b5,a6,b7
	[b0] sat .L1 a3:a2,a20
	sat .L2 b7:b6,b15
	set .S1 a1,31,0,a2
	[!a0] set .S2 b3,0,31,b4
	set .S1 a5,a6,a7
	[!b0] set .S1X b8,a9,a10
	[a1] set .S2 b11,b12,b13
	set .S2X a14,b15,b16
	shfl .M1 a17,a18
	[b1] shfl .M1X b19,a20
	[!a1] shfl .M2 b21,b22
	shfl .M2X a23,b24
	shfl3 .L1 a0,a1,a3:a2
	shfl3 .L1X a4,b5,a7:a6
	shfl3 .L2 b8,b9,b11:b10
	shfl3 .L2X b12,a13,b15:b14
	shl .S1 a1,a2,a3
	[!b1] shl .S1X b4,a5,a6
	[a2] shl .S2 b7,b8,b9
	shl .S2X a10,b11,b12
	[b2] shl .S1 a15:a14,a16,a19:a18
	shl .S2 b21:b20,b22,b25:b24
	[!a2] shl .S1 a26,a27,a29:a28
	shl .S1X b30,a31,a1:a0
	shl .S2 b2,b3,b5:b4
	[!b2] shl .S2X a6,b7,b9:b8
	shl .S1 a1,0,a3
	[a0] shl .S1X b4,31,a6
	[b0] shl .S2 b7,17,b9
	shl .S2X a10,12,b12
	[!a0] shl .S1 a15:a14,0,a19:a18
	shl .S2 b21:b20,31,b25:b24
	[!b0] shl .S1 a26,31,a29:a28
	shl .S1X b30,0,a1:a0
	shl .S2 b2,5,b5:b4
	[a1] shl .S2X a6,9,b9:b8
	shlmb .L1 a1,a2,a3
	[b1] shlmb .L1X a4,b5,a6
	[!a1] shlmb .L2 b7,b8,b9
	shlmb .L2X b10,a11,b12
	shlmb .S1 a1,a2,a3
	[!b1] shlmb .S1X a4,b5,a6
	[a2] shlmb .S2 b7,b8,b9
	shlmb .S2X b10,a11,b12
	shr .S1 a1,a2,a3
	[b2] shr .S1X b4,a5,a6
	[!a2] shr .S2 b7,b8,b9
	shr .S2X a10,b11,b12
	[!b2] shr .S1 a15:a14,a16,a19:a18
	shr .S2 b21:b20,b22,b25:b24
	shr .S1 a1,0,a3
	[a0] shr .S1X b4,31,a6
	[b0] shr .S2 b7,17,b9
	shr .S2X a10,12,b12
	[!a0] shr .S1 a15:a14,0,a19:a18
	shr .S2 b21:b20,31,b25:b24
	shr2 .S1 a1,a2,a3
	[!b0] shr2 .S1X b4,a5,a6
	[a1] shr2 .S2 b7,b8,b9
	shr2 .S2X a10,b11,b12
	shr2 .S1 a1,31,a3
	[b1] shr2 .S1X b4,0,a6
	[!a1] shr2 .S2 b7,5,b9
	shr2 .S2X a10,25,b12
	shrmb .S1 a1,a2,a3
	[!b1] shrmb .S1X a4,b5,a6
	[a2] shrmb .S2 b7,b8,b9
	shrmb .S2X b10,a11,b12
	shru .S1 a1,a2,a3
	[b2] shru .S1X b4,a5,a6
	[!a2] shru .S2 b7,b8,b9
	shru .S2X a10,b11,b12
	[!b2] shru .S1 a15:a14,a16,a19:a18
	shru .S2 b21:b20,b22,b25:b24
	shru .S1 a1,0,a3
	[a0] shru .S1X b4,31,a6
	[b0] shru .S2 b7,17,b9
	shru .S2X a10,12,b12
	[!a0] shru .S1 a15:a14,0,a19:a18
	shru .S2 b21:b20,31,b25:b24
	shru2 .S1 a1,a2,a3
	[!b0] shru2 .S1X b4,a5,a6
	[a1] shru2 .S2 b7,b8,b9
	shru2 .S2X a10,b11,b12
	shru2 .S1 a1,31,a3
	[b1] shru2 .S1X b4,0,a6
	[!a1] shru2 .S2 b7,5,b9
	shru2 .S2X a10,25,b12
	smpy .M1 a5,a6,a7
	[!b1] smpy .M1X a8,b9,a10
	[a2] smpy .M2 b11,b12,b13
	smpy .M2X b14,a15,b16
	smpyh .M1 a5,a6,a7
	[b2] smpyh .M1X a8,b9,a10
	[!a2] smpyh .M2 b11,b12,b13
	smpyh .M2X b14,a15,b16
	smpyhl .M1 a5,a6,a7
	[!b2] smpyhl .M1X a8,b9,a10
	[a0] smpyhl .M2 b11,b12,b13
	smpyhl .M2X b14,a15,b16
	smpylh .M1 a5,a6,a7
	[b0] smpylh .M1X a8,b9,a10
	[!a0] smpylh .M2 b11,b12,b13
	smpylh .M2X b14,a15,b16
	[!b0] smpy2 .M1 a17,a18,a21:a20
	smpy2 .M1X a22,b23,a25:a24
	smpy2 .M2 b26,b27,b29:b28
	[a1] smpy2 .M2X b30,a31,b1:b0
	smpy32 .M1 a17,a18,a21
	smpy32 .M1X a22,b23,a25
	smpy32 .M2 b26,b27,b29
	smpy32 .M2X b30,a31,b1
	spack2 .S1 a1,a2,a3
	[b1] spack2 .S1X a4,b5,a6
	[!a1] spack2 .S2 b7,b8,b9
	spack2 .S2X b10,a11,b12
	spacku4 .S1 a1,a2,a3
	[!b1] spacku4 .S1X a4,b5,a6
	[a2] spacku4 .S2 b7,b8,b9
	spacku4 .S2X b10,a11,b12
	[b2] spdp .S1 a13,a15:a14
	spdp .S1X b15,a17:a16
	spdp .S2 b18,b21:b20
	[!a2] spdp .S2X a21,b23:b22
	[!b2] spint .L1 a13,a15
	spint .L1X b15,a17
	spint .L2 b18,b21
	[a0] spint .L2X a21,b23
	[b0] sptrunc .L1 a13,a15
	sptrunc .L1X b15,a17
	sptrunc .L2 b18,b21
	[!a0] sptrunc .L2X a21,b23
	sshl .S1 a1,a2,a3
	[!b0] sshl .S1X b4,a5,a6
	[a1] sshl .S2 b7,b8,b9
	sshl .S2X a10,b11,b12
	sshl .S1 a13,31,a14
	[b1] sshl .S1X b15,0,a16
	[!a1] sshl .S2 b17,25,b18
	sshl .S2X a19,7,b20
	sshvl .M1 a1,a2,a3
	[!b1] sshvl .M1X b4,a5,a6
	[a2] sshvl .M2 b7,b8,b9
	sshvl .M2X a10,b11,b12
	sshvr .M1 a1,a2,a3
	[!b1] sshvr .M1X b4,a5,a6
	[a2] sshvr .M2 b7,b8,b9
	sshvr .M2X a10,b11,b12
	[b2] ssub .L1 a1,a2,a3
	ssub .L1X a4,b5,a6
	ssub .L2 b7,b8,b9
	[!a2] ssub .L2X b10,a11,b12
	ssub .L1X b13,a14,a15
	[!b2] ssub .L2X a16,b17,b18
	.word 0x000003f8
	ssub .L1 -16,a19,a20
	[a0] ssub .L1X 15,b21,a22
	[b0] ssub .L2 7,b23,b24
	ssub .L2X -9,a25,b26
	ssub .L1 -16,a29:a28,a31:a30
	[!a0] ssub .L2 15,b1:b0,b3:b2
	ssub2 .L1 a1,a2,a3
	[!b0] ssub2 .L1X a4,b5,a6
	[a1] ssub2 .L2 b7,b8,b9
	ssub2 .L2X b10,a11,b12
	stb .D1T1 a7,*a5
	[b2] stb .D1T2 b11,*++a9
	stb .D2T1 a15,*--b13
	[!a2] stb .D2T2 b19,*b17++
	stb .D1T1 a23,*a21--
	[!b2] stb .D2T2 b27,*-b25[31]
	stb .D1T1 a31,*+a29[0]
	stb .D1T1 a2,*-a0(2)
	stb .D1T1 a6,*-a4[a5]
	stb .D1T1 a9,*+a7[a8]
	stb .D1T1 a12,*--a10[11]
	stb .D1T1 a15,*++a13(14)
	stb .D1T1 a18,*a16--(17)
	stb .D1T1 a21,*a19++(20)
	stb .D1T1 a24,*--a22[a23]
	stb .D1T1 a27,*++a25[a26]
	stb .D1T1 a30,*a28--[a29]
	stb .D1T1 a1,*a31++[a0]
	[a0] stb .D2T1 a15,*+b14(32767)
	stb .D2T2 b16,*+b15[32767]
	stdw .D1T1 a7:a6,*a5
	[b2] stdw .D1T2 b11:b10,*++a9
	stdw .D2T1 a15:a14,*--b13
	[!a2] stdw .D2T2 b19:b18,*b17++
	stdw .D1T1 a23:a22,*a21--
	[!b2] stdw .D2T2 b27:b26,*-b25[31]
	stdw .D1T1 a31:a30,*+a29[0]
	stdw .D1T1 a3:a2,*-a0(248)
	stdw .D1T1 a7:a6,*-a4[a5]
	stdw .D1T1 a9:a8,*+a7[a8]
	stdw .D1T1 a13:a12,*--a10[11]
	stdw .D1T1 a15:a14,*++a13(16)
	stdw .D1T1 a19:a18,*a16--(24)
	stdw .D1T1 a21:a20,*a19++(32)
	stdw .D1T1 a25:a24,*--a22[a23]
	stdw .D1T1 a27:a26,*++a25[a26]
	stdw .D1T1 a31:a30,*a28--[a29]
	stdw .D1T1 a1:a0,*a31++[a0]
	sth .D1T1 a7,*a5
	[b2] sth .D1T2 b11,*++a9
	sth .D2T1 a15,*--b13
	[!a2] sth .D2T2 b19,*b17++
	sth .D1T1 a23,*a21--
	[!b2] sth .D2T2 b27,*-b25[31]
	sth .D1T1 a31,*+a29[0]
	sth .D1T1 a2,*-a0(62)
	sth .D1T1 a6,*-a4[a5]
	sth .D1T1 a9,*+a7[a8]
	sth .D1T1 a12,*--a10[11]
	sth .D1T1 a15,*++a13(14)
	sth .D1T1 a18,*a16--(18)
	sth .D1T1 a21,*a19++(20)
	sth .D1T1 a24,*--a22[a23]
	sth .D1T1 a27,*++a25[a26]
	sth .D1T1 a30,*a28--[a29]
	sth .D1T1 a1,*a31++[a0]
	[a0] sth .D2T1 a15,*+b14(65534)
	sth .D2T2 b16,*+b15[32767]
	stndw .D1T1 a7:a6,*a5
	[b2] stndw .D1T2 b11:b10,*++a9
	stndw .D2T1 a15:a14,*--b13
	[!a2] stndw .D2T2 b19:b18,*b17++
	stndw .D1T1 a23:a22,*a21--
	[!b2] stndw .D2T2 b27:b26,*-b25[31]
	stndw .D1T1 a31:a30,*+a29[0]
	stndw .D1T1 a3:a2,*-a0(31)
	stndw .D1T1 a7:a6,*-a4[a5]
	stndw .D1T1 a9:a8,*+a7(a8)
	stndw .D1T1 a13:a12,*--a10[11]
	stndw .D1T1 a15:a14,*++a13(16)
	stndw .D1T1 a19:a18,*a16--(24)
	stndw .D1T1 a21:a20,*a19++(30)
	stndw .D1T1 a25:a24,*--a22[a23]
	stndw .D1T1 a27:a26,*++a25(a26)
	stndw .D1T1 a31:a30,*a28--[a29]
	stndw .D1T1 a1:a0,*a31++(a0)
	stnw .D1T1 a7,*a5
	[b2] stnw .D1T2 b11,*++a9
	stnw .D2T1 a15,*--b13
	[!a2] stnw .D2T2 b19,*b17++
	stnw .D1T1 a23,*a21--
	[!b2] stnw .D2T2 b27,*-b25[31]
	stnw .D1T1 a31,*+a29[0]
	stnw .D1T1 a2,*-a0(124)
	stnw .D1T1 a6,*-a4[a5]
	stnw .D1T1 a9,*+a7[a8]
	stnw .D1T1 a12,*--a10[11]
	stnw .D1T1 a15,*++a13(16)
	stnw .D1T1 a18,*a16--(20)
	stnw .D1T1 a21,*a19++(24)
	stnw .D1T1 a24,*--a22[a23]
	stnw .D1T1 a27,*++a25[a26]
	stnw .D1T1 a30,*a28--[a29]
	stnw .D1T1 a1,*a31++[a0]
	stw .D1T1 a7,*a5
	[b2] stw .D1T2 b11,*++a9
	stw .D2T1 a15,*--b13
	[!a2] stw .D2T2 b19,*b17++
	stw .D1T1 a23,*a21--
	[!b2] stw .D2T2 b27,*-b25[31]
	stw .D1T1 a31,*+a29[0]
	stw .D1T1 a2,*-a0(124)
	stw .D1T1 a6,*-a4[a5]
	stw .D1T1 a9,*+a7[a8]
	stw .D1T1 a12,*--a10[11]
	stw .D1T1 a15,*++a13(16)
	stw .D1T1 a18,*a16--(20)
	stw .D1T1 a21,*a19++(24)
	stw .D1T1 a24,*--a22[a23]
	stw .D1T1 a27,*++a25[a26]
	stw .D1T1 a30,*a28--[a29]
	stw .D1T1 a1,*a31++[a0]
	[a0] stw .D2T1 a15,*+b14(131068)
	stw .D2T2 b16,*+b15[32767]
	sub .L1 a1,a2,a3
	[b0] sub .L1X a4,b5,a6
	[!a0] sub .L2 b7,b8,b9
	sub .L2X b10,a11,b12
	[!b0] sub .L1X b13,a14,a15
	sub .L2X a16,b17,b18
	.word 0x07b9a2f8
	[a1] sub .L1 a19,a20,a23:a22
	sub .L1X a24,b25,a27:a26
	sub .L2 b28,b29,b31:b30
	[b1] sub .L2X b0,a1,b3:b2
	sub .L1X b4,a5,a7:a6
	[!a1] sub .L2X a8,b9,b11:b10
	.word 0x031486f8
	sub .L1 -16,a12,a13
	[!b1] sub .L1X 15,b14,a15
	[a2] sub .L2 7,b16,b17
	sub .L2X -9,a18,b19
	sub .L1 -16,a21:a20,a23:a22
	[b2] sub .L2 15,b25:b24,b27:b26
	sub .S1 a1,a2,a3
	[!a2] sub .S1X a4,b5,a6
	[!b2] sub .S2 b7,b8,b9
	sub .S2X b10,a11,b12
	[a0] sub .S1X b13,a14,a15
	sub .S2X a16,b17,b18
	.word 0x07b5cd70
	[b0] sub .S1 -16,a19,a20
	sub .S1X 15,b21,a22
	sub .S2 13,b23,b24
	[!a0] sub .S2X -11,a25,b26
	sub .D1 a27,a28,a29
	[!b0] sub .D2 b30,b31,b0
	[a1] sub .D1 a1,0,a2
	sub .D2 b3,31,b4
	sub .D1X a5,b6,a7
	[b1] sub .D2X b8,a9,b10
	.word 0x0398ab30
	sub .L1 a5,16,a6
	[a0] sub .L1X b11,-15,a30
	sub .L2 b9,11,b10
	sub .L2X a5,-14,b7
	sub .L1 a3:a2,-5,a7:a6
	[b0] sub .L2 b29:b28,7,b29:b28
	sub .S1 a4,16,a11
	sub .S1X b9,-13,a23
	[!b0] sub .S2 b25,-15,b11
	sub .S2X a1,4,b2
	subab .D1 a1,a2,a3
	[!a1] subab .D2 b4,b5,b6
	subab .D1 a7,0,a8
	[!b1] subab .D2 b9,31,b10
	subabs4 .L1 a1,a2,a3
	[a2] subabs4 .L1X a4,b5,a6
	[b2] subabs4 .L2 b7,b8,b9
	subabs4 .L2X b10,a11,b12
	subah .D1 a1,a2,a3
	[!a2] subah .D2 b4,b5,b6
	[!b2] subah .D1 a7,0,a8
	subah .D2 b9,31,b10
	subaw .D1 a1,a2,a3
	[a0] subaw .D2 b4,b5,b6
	[b0] subaw .D1 a7,0,a8
	subaw .D2 b9,31,b10
	[!a0] subc .L1 a3,a4,a5
	subc .L1X a6,b7,a8
	subc .L2 b9,b10,b11
	[!b0] subc .L2X b12,a13,b14
	subdp .L1 a3:a2,a5:a4,a7:a6
	[a1] subdp .L1X a9:a8,b11:b10,a13:a12
	[b1] subdp .L2 b15:b14,b17:b16,b19:b18
	subdp .L2X b21:b20,a23:a22,b25:b24
	[!a1] subdp .L1X b27:b26,a29:a28,a31:a30
	subdp .L2X a1:a0,b3:b2,b5:b4
	.word 0x0f7343b8
	subdp .S1 a3:a2,a5:a4,a7:a6
	[a1] subdp .S1X a9:a8,b11:b10,a13:a12
	[b1] subdp .S2 b15:b14,b17:b16,b19:b18
	subdp .S2X b21:b20,a23:a22,b25:b24
	[!a1] subdp .S1X b27:b26,a29:a28,a31:a30
	subdp .S2X a1:a0,b3:b2,b5:b4
	.word 0x0f6b8ef8
	subsp .L1 a3,a5,a7
	[a1] subsp .L1X a9,b11,a13
	[b1] subsp .L2 b15,b17,b19
	subsp .L2X b21,a23,b25
	[!a1] subsp .L1X b27,a29,a31
	subsp .L2X a1,b3,b5
	.word 0x0ff762b8
	subsp .S1 a3,a5,a7
	[!b1] subsp .S1X a9,b11,a13
	[a2] subsp .S2 b15,b17,b19
	subsp .S2X b21,a23,b25
	[b2] subsp .S1X b27,a29,a31
	subsp .S2X a1,b3,b5
	.word 0x0fefaeb8
	subu .L1 a2,a3,a5:a4
	[!a2] subu .L1X a6,b7,a9:a8
	[!b2] subu .L2 b10,b11,b13:b12
	subu .L2X b14,a15,b17:b16
	[a0] subu .L1X b18,a19,a21:a20
	subu .L2X a22,b23,b25:b24
	.word 0x0a4e47f8
	sub2 .L1 a1,a2,a3
	[b0] sub2 .L1X a4,b5,a6
	[!a0] sub2 .L2 b7,b8,b9
	sub2 .L2X b10,a11,b12
	sub2 .S1 a1,a2,a3
	[!b0] sub2 .S1X a4,b5,a6
	[a1] sub2 .S2 b7,b8,b9
	sub2 .S2X b10,a11,b12
	sub2 .D1 a1,a2,a3
	[b1] sub2 .D1X a4,b5,a6
	[!a1] sub2 .D2 b7,b8,b9
	sub2 .D2X b10,a11,b12
	sub4 .L1 a1,a2,a3
	[!b1] sub4 .L1X a4,b5,a6
	[a2] sub4 .L2 b7,b8,b9
	sub4 .L2X b10,a11,b12
	swap2 .L1 a3,a7
	[b2] swap2 .L2 b9,b11
	[!a2] swap2 .S1 a13,a15
	swap2 .S2 b23,b29
	[!b2] swap4 .L1 a1,a2
	swap4 .L1X b3,a4
	swap4 .L2 b5,b6
	[a0] swap4 .L2X a7,b8
	swe
	swenr
	unpkhu4 .L1 a1,a2
	[b0] unpkhu4 .L1X b3,a4
	[!a0] unpkhu4 .L2 b5,b6
	unpkhu4 .L2X a7,b8
	unpkhu4 .S1 a1,a2
	[!b0] unpkhu4 .S1X b3,a4
	[a1] unpkhu4 .S2 b5,b6
	unpkhu4 .S2X a7,b8
	unpklu4 .L1 a1,a2
	[b1] unpklu4 .L1X b3,a4
	[!a1] unpklu4 .L2 b5,b6
	unpklu4 .L2X a7,b8
	unpklu4 .S1 a1,a2
	[!b1] unpklu4 .S1X b3,a4
	[a2] unpklu4 .S2 b5,b6
	unpklu4 .S2X a7,b8
	xor .L1 a1,a2,a3
	[b2] xor .L1X a4,b5,a6
	[!a2] xor .L2 b7,b8,b9
	xor .L2X b10,a11,b12
	[!b2] xor .L1 -16,a13,a14
	xor .L1X 15,b15,a16
	xor .L2 3,b17,b18
	[a0] xor .L2X -12,a19,b20
	xor .S1 a1,a2,a3
	[b0] xor .S1X a4,b5,a6
	[!a0] xor .S2 b7,b8,b9
	xor .S2X b10,a11,b12
	[!b0] xor .S1 -16,a13,a14
	xor .S1X 15,b15,a16
	xor .S2 3,b17,b18
	[a1] xor .S2X -12,a19,b20
	xor .D1 a1,a2,a3
	[b0] xor .D1X a4,b5,a6
	[!a0] xor .D2 b7,b8,b9
	xor .D2X b10,a11,b12
	[!b0] xor .D1 -16,a13,a14
	xor .D1X 15,b15,a16
	xor .D2 3,b17,b18
	[a1] xor .D2X -12,a19,b20
	xormpy .M1 a1,a2,a3
	xormpy .M1X a4,b5,a6
	xormpy .M2 b7,b8,b9
	xormpy .M2X b10,a11,b12
	xpnd2 .M1 a13,a14
	[b1] xpnd2 .M1X b15,a16
	[!a1] xpnd2 .M2 b17,b18
	xpnd2 .M2X a19,b20
	xpnd4 .M1 a13,a14
	[!b1] xpnd4 .M1X b15,a16
	[a2] xpnd4 .M2 b17,b18
	xpnd4 .M2X a19,b20
	zero .L1 a1
	[b2] zero .L2 b2
	[!a2] zero .L1 a5:a4
	zero .L2 b7:b6
	zero .D1 a8
	[!b2] zero .D2 b9
	[a0] zero .S1 a10
	zero .S2 b11
