#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DSP ASE for MIPS32
#as: -mdsp -32
#source: mips32-dsp.s

# Check MIPS DSP ASE for MIPS32 Instruction Assembly (microMIPS)

.*: +file format .*mips.*

Disassembly of section \.text:
0+0000 <[^>]*> 0041 000d 	addq\.ph	zero,at,v0
0+0004 <[^>]*> 0062 0c0d 	addq_s\.ph	at,v0,v1
0+0008 <[^>]*> 0083 1305 	addq_s\.w	v0,v1,a0
0+000c <[^>]*> 00a4 18cd 	addu\.qb	v1,a0,a1
0+0010 <[^>]*> 00c5 24cd 	addu_s\.qb	a0,a1,a2
0+0014 <[^>]*> 00e6 2a0d 	subq\.ph	a1,a2,a3
0+0018 <[^>]*> 0107 360d 	subq_s\.ph	a2,a3,t0
0+001c <[^>]*> 0128 3b45 	subq_s\.w	a3,t0,t1
0+0020 <[^>]*> 0149 42cd 	subu\.qb	t0,t1,t2
0+0024 <[^>]*> 016a 4ecd 	subu_s\.qb	t1,t2,t3
0+0028 <[^>]*> 018b 5385 	addsc	t2,t3,t4
0+002c <[^>]*> 01ac 5bc5 	addwc	t3,t4,t5
0+0030 <[^>]*> 01cd 6295 	modsub	t4,t5,t6
0+0034 <[^>]*> 01ae f13c 	raddu\.w\.qb	t5,t6
0+0038 <[^>]*> 01cf 113c 	absq_s\.ph	t6,t7
0+003c <[^>]*> 01f0 213c 	absq_s\.w	t7,s0
0+0040 <[^>]*> 0251 80ad 	precrq\.qb\.ph	s0,s1,s2
0+0044 <[^>]*> 0272 88ed 	precrq\.ph\.w	s1,s2,s3
0+0048 <[^>]*> 0293 912d 	precrq_rs\.ph\.w	s2,s3,s4
0+004c <[^>]*> 02b4 996d 	precrqu_s\.qb\.ph	s3,s4,s5
0+0050 <[^>]*> 0295 513c 	preceq\.w\.phl	s4,s5
0+0054 <[^>]*> 02b6 613c 	preceq\.w\.phr	s5,s6
0+0058 <[^>]*> 02d7 713c 	precequ\.ph\.qbl	s6,s7
0+005c <[^>]*> 02f8 913c 	precequ\.ph\.qbr	s7,t8
0+0060 <[^>]*> 0319 733c 	precequ\.ph\.qbla	t8,t9
0+0064 <[^>]*> 033a 933c 	precequ\.ph\.qbra	t9,k0
0+0068 <[^>]*> 035b b13c 	preceu\.ph\.qbl	k0,k1
0+006c <[^>]*> 037c d13c 	preceu\.ph\.qbr	k1,gp
0+0070 <[^>]*> 039d b33c 	preceu\.ph\.qbla	gp,sp
0+0074 <[^>]*> 03be d33c 	preceu\.ph\.qbra	sp,s8
0+0078 <[^>]*> 03df 087c 	shll\.qb	s8,ra,0x0
0+007c <[^>]*> 03df e87c 	shll\.qb	s8,ra,0x7
0+0080 <[^>]*> 0001 fb95 	shllv\.qb	ra,zero,at
0+0084 <[^>]*> 0001 03b5 	shll\.ph	zero,at,0x0
0+0088 <[^>]*> 0001 f3b5 	shll\.ph	zero,at,0xf
0+008c <[^>]*> 0043 0b8d 	shllv\.ph	at,v0,v1
0+0090 <[^>]*> 0043 0bb5 	shll_s\.ph	v0,v1,0x0
0+0094 <[^>]*> 0043 fbb5 	shll_s\.ph	v0,v1,0xf
0+0098 <[^>]*> 0085 1f8d 	shllv_s\.ph	v1,a0,a1
0+009c <[^>]*> 0085 03f5 	shll_s\.w	a0,a1,0x0
0+00a0 <[^>]*> 0085 fbf5 	shll_s\.w	a0,a1,0x1f
0+00a4 <[^>]*> 00c7 2bd5 	shllv_s\.w	a1,a2,a3
0+00a8 <[^>]*> 00c7 187c 	shrl\.qb	a2,a3,0x0
0+00ac <[^>]*> 00c7 f87c 	shrl\.qb	a2,a3,0x7
0+00b0 <[^>]*> 0109 3b55 	shrlv\.qb	a3,t0,t1
0+00b4 <[^>]*> 0109 0335 	shra\.ph	t0,t1,0x0
0+00b8 <[^>]*> 0109 f335 	shra\.ph	t0,t1,0xf
0+00bc <[^>]*> 014b 498d 	shrav\.ph	t1,t2,t3
0+00c0 <[^>]*> 014b 0735 	shra_r\.ph	t2,t3,0x0
0+00c4 <[^>]*> 014b f735 	shra_r\.ph	t2,t3,0xf
0+00c8 <[^>]*> 018d 5d8d 	shrav_r\.ph	t3,t4,t5
0+00cc <[^>]*> 018d 02f5 	shra_r\.w	t4,t5,0x0
0+00d0 <[^>]*> 018d faf5 	shra_r\.w	t4,t5,0x1f
0+00d4 <[^>]*> 01cf 6ad5 	shrav_r\.w	t5,t6,t7
0+00d8 <[^>]*> 020f 7095 	muleu_s\.ph\.qbl	t6,t7,s0
0+00dc <[^>]*> 0230 78d5 	muleu_s\.ph\.qbr	t7,s0,s1
0+00e0 <[^>]*> 0251 8115 	mulq_rs\.ph	s0,s1,s2
0+00e4 <[^>]*> 0272 8825 	muleq_s\.w\.phl	s1,s2,s3
0+00e8 <[^>]*> 0293 9065 	muleq_s\.w\.phr	s2,s3,s4
0+00ec <[^>]*> 0293 20bc 	dpau\.h\.qbl	\$ac0,s3,s4
0+00f0 <[^>]*> 02b4 70bc 	dpau\.h\.qbr	\$ac1,s4,s5
0+00f4 <[^>]*> 02d5 a4bc 	dpsu\.h\.qbl	\$ac2,s5,s6
0+00f8 <[^>]*> 02f6 f4bc 	dpsu\.h\.qbr	\$ac3,s6,s7
0+00fc <[^>]*> 0317 02bc 	dpaq_s\.w\.ph	\$ac0,s7,t8
0+0100 <[^>]*> 0338 46bc 	dpsq_s\.w\.ph	\$ac1,t8,t9
0+0104 <[^>]*> 0359 bcbc 	mulsaq_s\.w\.ph	\$ac2,t9,k0
0+0108 <[^>]*> 037a d2bc 	dpaq_sa.l\.w	\$ac3,k0,k1
0+010c <[^>]*> 039b 16bc 	dpsq_sa.l\.w	\$ac0,k1,gp
0+0110 <[^>]*> 03bc 5a7c 	maq_s\.w\.phl	\$ac1,gp,sp
0+0114 <[^>]*> 03dd 8a7c 	maq_s\.w\.phr	\$ac2,sp,s8
0+0118 <[^>]*> 03fe fa7c 	maq_sa\.w\.phl	\$ac3,s8,ra
0+011c <[^>]*> 001f 2a7c 	maq_sa\.w\.phr	\$ac0,ra,zero
0+0120 <[^>]*> 0001 313c 	bitrev	zero,at
0+0124 <[^>]*> 0022 413c 	insv	at,v0
0+0128 <[^>]*> 0040 05fc 	repl\.qb	v0,0x0
0+012c <[^>]*> 005f e5fc 	repl\.qb	v0,0xff
0+0130 <[^>]*> 0064 133c 	replv\.qb	v1,a0
0+0134 <[^>]*> 0200 203d 	repl\.ph	a0,-512
0+0138 <[^>]*> 01ff 203d 	repl\.ph	a0,511
0+013c <[^>]*> 00a6 033c 	replv\.ph	a1,a2
0+0140 <[^>]*> 00e6 0245 	cmpu\.eq\.qb	a2,a3
0+0144 <[^>]*> 0107 0285 	cmpu\.lt\.qb	a3,t0
0+0148 <[^>]*> 0128 02c5 	cmpu\.le\.qb	t0,t1
0+014c <[^>]*> 016a 48c5 	cmpgu\.eq\.qb	t1,t2,t3
0+0150 <[^>]*> 018b 5105 	cmpgu\.lt\.qb	t2,t3,t4
0+0154 <[^>]*> 01ac 5945 	cmpgu\.le\.qb	t3,t4,t5
0+0158 <[^>]*> 01ac 0005 	cmp\.eq\.ph	t4,t5
0+015c <[^>]*> 01cd 0045 	cmp\.lt\.ph	t5,t6
0+0160 <[^>]*> 01ee 0085 	cmp\.le\.ph	t6,t7
0+0164 <[^>]*> 0230 79ed 	pick\.qb	t7,s0,s1
0+0168 <[^>]*> 0251 822d 	pick\.ph	s0,s1,s2
0+016c <[^>]*> 0272 89ad 	packrl\.ph	s1,s2,s3
0+0170 <[^>]*> 0240 4e7c 	extr\.w	s2,\$ac1,0x0
0+0174 <[^>]*> 025f 4e7c 	extr\.w	s2,\$ac1,0x1f
0+0178 <[^>]*> 0260 9e7c 	extr_r\.w	s3,\$ac2,0x0
0+017c <[^>]*> 027f 9e7c 	extr_r\.w	s3,\$ac2,0x1f
0+0180 <[^>]*> 0280 ee7c 	extr_rs\.w	s4,\$ac3,0x0
0+0184 <[^>]*> 029f ee7c 	extr_rs\.w	s4,\$ac3,0x1f
0+0188 <[^>]*> 02a0 3e7c 	extr_s\.h	s5,\$ac0,0x0
0+018c <[^>]*> 02bf 3e7c 	extr_s\.h	s5,\$ac0,0x1f
0+0190 <[^>]*> 02d7 7ebc 	extrv_s\.h	s6,\$ac1,s7
0+0194 <[^>]*> 02f8 8ebc 	extrv\.w	s7,\$ac2,t8
0+0198 <[^>]*> 0319 debc 	extrv_r\.w	t8,\$ac3,t9
0+019c <[^>]*> 033a 2ebc 	extrv_rs\.w	t9,\$ac0,k0
0+01a0 <[^>]*> 0340 667c 	extp	k0,\$ac1,0x0
0+01a4 <[^>]*> 035f 667c 	extp	k0,\$ac1,0x1f
0+01a8 <[^>]*> 037c a8bc 	extpv	k1,\$ac2,gp
0+01ac <[^>]*> 0380 f67c 	extpdp	gp,\$ac3,0x0
0+01b0 <[^>]*> 039f f67c 	extpdp	gp,\$ac3,0x1f
0+01b4 <[^>]*> 03be 38bc 	extpdpv	sp,\$ac0,s8
0+01b8 <[^>]*> 0020 401d 	shilo	\$ac1,-32
0+01bc <[^>]*> 001f 401d 	shilo	\$ac1,31
0+01c0 <[^>]*> 001e 927c 	shilov	\$ac2,s8
0+01c4 <[^>]*> 001f c27c 	mthlip	ra,\$ac3
0+01c8 <[^>]*> 0000 007c 	mfhi	zero,\$ac0
0+01cc <[^>]*> 0001 507c 	mflo	at,\$ac1
0+01d0 <[^>]*> 0002 a07c 	mthi	v0,\$ac2
0+01d4 <[^>]*> 0003 f07c 	mtlo	v1,\$ac3
0+01d8 <[^>]*> 0080 167c 	wrdsp	a0,0x0
0+01dc <[^>]*> 008f d67c 	wrdsp	a0
0+01e0 <[^>]*> 00af d67c 	wrdsp	a1
0+01e4 <[^>]*> 00c0 067c 	rddsp	a2,0x0
0+01e8 <[^>]*> 00cf c67c 	rddsp	a2
0+01ec <[^>]*> 00ef c67c 	rddsp	a3
0+01f0 <[^>]*> 012a 4225 	lbux	t0,t1\(t2\)
0+01f4 <[^>]*> 014b 4965 	lhx	t1,t2\(t3\)
0+01f8 <[^>]*> 016c 51a5 	lwx	t2,t3\(t4\)
0+01fc <[^>]*> 4360 fffe 	bposge32	000001fc <text_label\+0x1fc>
			1fc: R_MICROMIPS_PC16_S1	text_label
0+0200 <[^>]*> 0c00      	nop
0+0202 <[^>]*> 018b 8abc 	madd	\$ac2,t3,t4
0+0206 <[^>]*> 01ac dabc 	maddu	\$ac3,t4,t5
0+020a <[^>]*> 01cd 2abc 	msub	\$ac0,t5,t6
0+020e <[^>]*> 01ee 7abc 	msubu	\$ac1,t6,t7
0+0212 <[^>]*> 02d5 ccbc 	mult	\$ac3,s5,s6
0+0216 <[^>]*> 02f6 1cbc 	multu	\$ac0,s6,s7
0+021a <[^>]*> 0c00      	nop
	\.\.\.
