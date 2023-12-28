#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DSP ASE Rev2 for MIPS32
#as: -mdspr2 -32
#source: mips32-dspr2.s

# Check MIPS DSP ASE Rev2 for MIPS32 Instruction Assembly (microMIPS)

.*: +file format .*mips.*

Disassembly of section \.text:
0+0000 <[^>]*> 0001 013c 	absq_s\.qb	zero,at
0+0004 <[^>]*> 0062 090d 	addu\.ph	at,v0,v1
0+0008 <[^>]*> 0083 150d 	addu_s\.ph	v0,v1,a0
0+000c <[^>]*> 00a4 194d 	adduh\.qb	v1,a0,a1
0+0010 <[^>]*> 00c5 254d 	adduh_r\.qb	a0,a1,a2
0+0014 <[^>]*> 00a6 0215 	append	a1,a2,0x0
0+0018 <[^>]*> 00a6 fa15 	append	a1,a2,0x1f
0+001c <[^>]*> 0c00      	nop
0+001e <[^>]*> 00c7 48bc 	balign	a2,a3,0x1
0+0022 <[^>]*> 00e6 31ad 	packrl\.ph	a2,a2,a3
0+0026 <[^>]*> 00c7 c8bc 	balign	a2,a3,0x3
0+002a <[^>]*> 0107 3185 	cmpgdu\.eq\.qb	a2,a3,t0
0+002e <[^>]*> 0128 39c5 	cmpgdu\.lt\.qb	a3,t0,t1
0+0032 <[^>]*> 0149 4205 	cmpgdu\.le\.qb	t0,t1,t2
0+0036 <[^>]*> 0149 00bc 	dpa\.w\.ph	\$ac0,t1,t2
0+003a <[^>]*> 016a 44bc 	dps\.w\.ph	\$ac1,t2,t3
0+003e <[^>]*> 018b 8abc 	madd	\$ac2,t3,t4
0+0042 <[^>]*> 01ac dabc 	maddu	\$ac3,t4,t5
0+0046 <[^>]*> 01cd 2abc 	msub	\$ac0,t5,t6
0+004a <[^>]*> 01ee 7abc 	msubu	\$ac1,t6,t7
0+004e <[^>]*> 0230 782d 	mul\.ph	t7,s0,s1
0+0052 <[^>]*> 0251 842d 	mul_s\.ph	s0,s1,s2
0+0056 <[^>]*> 0272 8995 	mulq_rs\.w	s1,s2,s3
0+005a <[^>]*> 0293 9155 	mulq_s\.ph	s2,s3,s4
0+005e <[^>]*> 02b4 99d5 	mulq_s\.w	s3,s4,s5
0+0062 <[^>]*> 02b4 acbc 	mulsa\.w\.ph	\$ac2,s4,s5
0+0066 <[^>]*> 02d5 ccbc 	mult	\$ac3,s5,s6
0+006a <[^>]*> 02f6 1cbc 	multu	\$ac0,s6,s7
0+006e <[^>]*> 0338 b86d 	precr\.qb\.ph	s7,t8,t9
0+0072 <[^>]*> 0319 03cd 	precr_sra\.ph\.w	t8,t9,0x0
0+0076 <[^>]*> 0319 fbcd 	precr_sra\.ph\.w	t8,t9,0x1f
0+007a <[^>]*> 033a 07cd 	precr_sra_r\.ph\.w	t9,k0,0x0
0+007e <[^>]*> 033a ffcd 	precr_sra_r\.ph\.w	t9,k0,0x1f
0+0082 <[^>]*> 035b 0255 	prepend	k0,k1,0x0
0+0086 <[^>]*> 035b fa55 	prepend	k0,k1,0x1f
0+008a <[^>]*> 037c 01fc 	shra\.qb	k1,gp,0x0
0+008e <[^>]*> 037c e1fc 	shra\.qb	k1,gp,0x7
0+0092 <[^>]*> 039d 11fc 	shra_r\.qb	gp,sp,0x0
0+0096 <[^>]*> 039d f1fc 	shra_r\.qb	gp,sp,0x7
0+009a <[^>]*> 03df e9cd 	shrav\.qb	sp,s8,ra
0+009e <[^>]*> 03e0 f5cd 	shrav_r\.qb	s8,ra,zero
0+00a2 <[^>]*> 03e0 03fc 	shrl\.ph	ra,zero,0x0
0+00a6 <[^>]*> 03e0 f3fc 	shrl\.ph	ra,zero,0xf
0+00aa <[^>]*> 0022 0315 	shrlv\.ph	zero,at,v0
0+00ae <[^>]*> 0062 0b0d 	subu\.ph	at,v0,v1
0+00b2 <[^>]*> 0083 170d 	subu_s\.ph	v0,v1,a0
0+00b6 <[^>]*> 00a4 1b4d 	subuh\.qb	v1,a0,a1
0+00ba <[^>]*> 00c5 274d 	subuh_r\.qb	a0,a1,a2
0+00be <[^>]*> 00e6 284d 	addqh\.ph	a1,a2,a3
0+00c2 <[^>]*> 0107 344d 	addqh_r\.ph	a2,a3,t0
0+00c6 <[^>]*> 0128 388d 	addqh\.w	a3,t0,t1
0+00ca <[^>]*> 0149 448d 	addqh_r\.w	t0,t1,t2
0+00ce <[^>]*> 016a 4a4d 	subqh\.ph	t1,t2,t3
0+00d2 <[^>]*> 018b 564d 	subqh_r\.ph	t2,t3,t4
0+00d6 <[^>]*> 01ac 5a8d 	subqh\.w	t3,t4,t5
0+00da <[^>]*> 01cd 668d 	subqh_r\.w	t4,t5,t6
0+00de <[^>]*> 01cd 50bc 	dpax\.w\.ph	\$ac1,t5,t6
0+00e2 <[^>]*> 01ee 94bc 	dpsx\.w\.ph	\$ac2,t6,t7
0+00e6 <[^>]*> 020f e2bc 	dpaqx_s\.w\.ph	\$ac3,t7,s0
0+00ea <[^>]*> 0230 32bc 	dpaqx_sa\.w\.ph	\$ac0,s0,s1
0+00ee <[^>]*> 0251 66bc 	dpsqx_s\.w\.ph	\$ac1,s1,s2
0+00f2 <[^>]*> 0272 b6bc 	dpsqx_sa\.w\.ph	\$ac2,s2,s3
0+00f6 <[^>]*> 0c00      	nop
	\.\.\.
