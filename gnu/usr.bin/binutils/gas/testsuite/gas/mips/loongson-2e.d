#as: -march=loongson2e -mabi=o64
#objdump: -M reg-names=numeric -dr
#name: ST Microelectronics Loongson-2E tests

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <movz_insns>:
.*:	0064100a 	movz	\$2,\$3,\$4
.*:	0064100b 	movn	\$2,\$3,\$4
.*:	0064100b 	movn	\$2,\$3,\$4

[0-9a-f]+ <integer_insns>:
.*:	7c641018 	mult.g	\$2,\$3,\$4
.*:	7cc72819 	multu.g	\$5,\$6,\$7
.*:	7d2a401c 	dmult.g	\$8,\$9,\$10
.*:	7d8d581d 	dmultu.g	\$11,\$12,\$13
.*:	7df0701a 	div.g	\$14,\$15,\$16
.*:	7e53881b 	divu.g	\$17,\$18,\$19
.*:	7eb6a01e 	ddiv.g	\$20,\$21,\$22
.*:	7f19b81f 	ddivu.g	\$23,\$24,\$25
.*:	7f7cd022 	mod.g	\$26,\$27,\$28
.*:	7fdfe823 	modu.g	\$29,\$30,\$31
.*:	7c641026 	dmod.g	\$2,\$3,\$4
.*:	7cc72827 	dmodu.g	\$5,\$6,\$7

[0-9a-f]+ <fpu_insns>:
.*:	46020818 	madd.s	\$f0,\$f1,\$f2
.*:	462520d8 	madd.d	\$f3,\$f4,\$f5
.*:	45683998 	madd.ps	\$f6,\$f7,\$f8
.*:	460b5259 	msub.s	\$f9,\$f10,\$f11
.*:	462e6b19 	msub.d	\$f12,\$f13,\$f14
.*:	457183d9 	msub.ps	\$f15,\$f16,\$f17
.*:	46149c9a 	nmadd.s	\$f18,\$f19,\$f20
.*:	4637b55a 	nmadd.d	\$f21,\$f22,\$f23
.*:	457ace1a 	nmadd.ps	\$f24,\$f25,\$f26
.*:	461de6db 	nmsub.s	\$f27,\$f28,\$f29
.*:	4622081b 	nmsub.d	\$f0,\$f1,\$f2
.*:	456520db 	nmsub.ps	\$f3,\$f4,\$f5

[0-9a-f]+ <simd_insns>:
.*:	47420802 	packsshb	\$f0,\$f1,\$f2
.*:	472520c2 	packsswh	\$f3,\$f4,\$f5
.*:	47683982 	packushb	\$f6,\$f7,\$f8
.*:	47cb5240 	paddb	\$f9,\$f10,\$f11
.*:	474e6b00 	paddh	\$f12,\$f13,\$f14
.*:	477183c0 	paddw	\$f15,\$f16,\$f17
.*:	47f49c80 	paddd	\$f18,\$f19,\$f20
.*:	4797b540 	paddsb	\$f21,\$f22,\$f23
.*:	471ace00 	paddsh	\$f24,\$f25,\$f26
.*:	47bde6c0 	paddusb	\$f27,\$f28,\$f29
.*:	47220800 	paddush	\$f0,\$f1,\$f2
.*:	47e520c2 	pandn	\$f3,\$f4,\$f5
.*:	46683980 	pavgb	\$f6,\$f7,\$f8
.*:	464b5240 	pavgh	\$f9,\$f10,\$f11
.*:	46ce6b01 	pcmpeqb	\$f12,\$f13,\$f14
.*:	469183c1 	pcmpeqh	\$f15,\$f16,\$f17
.*:	46549c81 	pcmpeqw	\$f18,\$f19,\$f20
.*:	46f7b541 	pcmpgtb	\$f21,\$f22,\$f23
.*:	46bace01 	pcmpgth	\$f24,\$f25,\$f26
.*:	467de6c1 	pcmpgtw	\$f27,\$f28,\$f29
.*:	45c20802 	pextrh	\$f0,\$f1,\$f2
.*:	478520c3 	pinsrh_0	\$f3,\$f4,\$f5
.*:	47a83983 	pinsrh_1	\$f6,\$f7,\$f8
.*:	47cb5243 	pinsrh_2	\$f9,\$f10,\$f11
.*:	47ee6b03 	pinsrh_3	\$f12,\$f13,\$f14
.*:	45f183c2 	pmaddhw	\$f15,\$f16,\$f17
.*:	46949c80 	pmaxsh	\$f18,\$f19,\$f20
.*:	46d7b540 	pmaxub	\$f21,\$f22,\$f23
.*:	46bace00 	pminsh	\$f24,\$f25,\$f26
.*:	46fde6c0 	pminub	\$f27,\$f28,\$f29
.*:	46a00805 	pmovmskb	\$f0,\$f1
.*:	46e41882 	pmulhuh	\$f2,\$f3,\$f4
.*:	46a73142 	pmulhh	\$f5,\$f6,\$f7
.*:	468a4a02 	pmullh	\$f8,\$f9,\$f10
.*:	46cd62c2 	pmuluw	\$f11,\$f12,\$f13
.*:	45b07b81 	pasubub	\$f14,\$f15,\$f16
.*:	46809445 	biadd	\$f17,\$f18
.*:	4715a4c2 	pshufh	\$f19,\$f20,\$f21
.*:	4678bd82 	psllh	\$f22,\$f23,\$f24
.*:	465bd642 	psllw	\$f25,\$f26,\$f27
.*:	46beef03 	psrah	\$f28,\$f29,\$f30
.*:	46820803 	psraw	\$f0,\$f1,\$f2
.*:	466520c3 	psrlh	\$f3,\$f4,\$f5
.*:	46483983 	psrlw	\$f6,\$f7,\$f8
.*:	47cb5241 	psubb	\$f9,\$f10,\$f11
.*:	474e6b01 	psubh	\$f12,\$f13,\$f14
.*:	477183c1 	psubw	\$f15,\$f16,\$f17
.*:	47f49c81 	psubd	\$f18,\$f19,\$f20
.*:	4797b541 	psubsb	\$f21,\$f22,\$f23
.*:	471ace01 	psubsh	\$f24,\$f25,\$f26
.*:	47bde6c1 	psubusb	\$f27,\$f28,\$f29
.*:	47220801 	psubush	\$f0,\$f1,\$f2
.*:	476520c3 	punpckhbh	\$f3,\$f4,\$f5
.*:	47283983 	punpckhhw	\$f6,\$f7,\$f8
.*:	46eb5243 	punpckhwd	\$f9,\$f10,\$f11
.*:	474e6b03 	punpcklbh	\$f12,\$f13,\$f14
.*:	471183c3 	punpcklhw	\$f15,\$f16,\$f17
.*:	46d49c83 	punpcklwd	\$f18,\$f19,\$f20

[0-9a-f]+ <fixed_point_insns>:
.*:	45c20800 	add	\$f0,\$f1,\$f2
.*:	458520c0 	addu	\$f3,\$f4,\$f5
.*:	45e83980 	dadd	\$f6,\$f7,\$f8
.*:	45cb5241 	sub	\$f9,\$f10,\$f11
.*:	458e6b01 	subu	\$f12,\$f13,\$f14
.*:	45f183c1 	dsub	\$f15,\$f16,\$f17
.*:	45b49c80 	or	\$f18,\$f19,\$f20
.*:	4597b542 	sll	\$f21,\$f22,\$f23
.*:	45bace02 	dsll	\$f24,\$f25,\$f26
.*:	479de6c2 	xor	\$f27,\$f28,\$f29
.*:	47a20802 	nor	\$f0,\$f1,\$f2
.*:	47c520c2 	and	\$f3,\$f4,\$f5
.*:	45883983 	srl	\$f6,\$f7,\$f8
.*:	45ab5243 	dsrl	\$f9,\$f10,\$f11
.*:	45ce6b03 	sra	\$f12,\$f13,\$f14
.*:	45f183c3 	dsra	\$f15,\$f16,\$f17
.*:	46939032 	sequ	\$f18,\$f19
.*:	4695a03c 	sltu	\$f20,\$f21
.*:	4697b03e 	sleu	\$f22,\$f23
.*:	46b9c032 	seq	\$f24,\$f25
.*:	46bbd03c 	slt	\$f26,\$f27
.*:	46bde03e 	sle	\$f28,\$f29

[0-9a-f]+ <mips5_ps_insns>:
.*:	45601005 	abs.ps	\$f0,\$f2
.*:	45662080 	add.ps	\$f2,\$f4,\$f6
.*:	456a4032 	c.eq.ps	\$f8,\$f10
.*:	456a4030 	c.f.ps	\$f8,\$f10
.*:	456a403e 	c.le.ps	\$f8,\$f10
.*:	456a403c 	c.lt.ps	\$f8,\$f10
.*:	456a403d 	c.nge.ps	\$f8,\$f10
.*:	456a403b 	c.ngl.ps	\$f8,\$f10
.*:	456a4039 	c.ngle.ps	\$f8,\$f10
.*:	456a403f 	c.ngt.ps	\$f8,\$f10
.*:	456a4036 	c.ole.ps	\$f8,\$f10
.*:	456a4034 	c.olt.ps	\$f8,\$f10
.*:	456a403a 	c.seq.ps	\$f8,\$f10
.*:	456a4038 	c.sf.ps	\$f8,\$f10
.*:	456a4033 	c.ueq.ps	\$f8,\$f10
.*:	456a4037 	c.ule.ps	\$f8,\$f10
.*:	456a4035 	c.ult.ps	\$f8,\$f10
.*:	456a4031 	c.un.ps	\$f8,\$f10
.*:	4560d606 	mov.ps	\$f24,\$f26
.*:	45662082 	mul.ps	\$f2,\$f4,\$f6
.*:	45604187 	neg.ps	\$f6,\$f8
.*:	457ac581 	sub.ps	\$f22,\$f24,\$f26
#pass
