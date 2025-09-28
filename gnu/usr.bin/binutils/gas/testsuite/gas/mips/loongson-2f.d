#as: -march=loongson2f -mabi=o64
#objdump: -M reg-names=numeric -dr
#name: ST Microelectronics Loongson-2F tests

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <movz_insns>:
.*:	0064100a 	movz	\$2,\$3,\$4
.*:	0064100b 	movn	\$2,\$3,\$4
.*:	0064100b 	movn	\$2,\$3,\$4

[0-9a-f]+ <integer_insns>:
.*:	70641010 	mult.g	\$2,\$3,\$4
.*:	70c72812 	multu.g	\$5,\$6,\$7
.*:	712a4011 	dmult.g	\$8,\$9,\$10
.*:	718d5813 	dmultu.g	\$11,\$12,\$13
.*:	71f07014 	div.g	\$14,\$15,\$16
.*:	72538816 	divu.g	\$17,\$18,\$19
.*:	72b6a015 	ddiv.g	\$20,\$21,\$22
.*:	7319b817 	ddivu.g	\$23,\$24,\$25
.*:	737cd01c 	mod.g	\$26,\$27,\$28
.*:	73dfe81e 	modu.g	\$29,\$30,\$31
.*:	7064101d 	dmod.g	\$2,\$3,\$4
.*:	70c7281f 	dmodu.g	\$5,\$6,\$7

[0-9a-f]+ <fpu_insns>:
.*:	72020818 	madd.s	\$f0,\$f1,\$f2
.*:	722520d8 	madd.d	\$f3,\$f4,\$f5
.*:	72c83998 	madd.ps	\$f6,\$f7,\$f8
.*:	720b5259 	msub.s	\$f9,\$f10,\$f11
.*:	722e6b19 	msub.d	\$f12,\$f13,\$f14
.*:	72d183d9 	msub.ps	\$f15,\$f16,\$f17
.*:	72149c9a 	nmadd.s	\$f18,\$f19,\$f20
.*:	7237b55a 	nmadd.d	\$f21,\$f22,\$f23
.*:	72dace1a 	nmadd.ps	\$f24,\$f25,\$f26
.*:	721de6db 	nmsub.s	\$f27,\$f28,\$f29
.*:	7222081b 	nmsub.d	\$f0,\$f1,\$f2
.*:	72c520db 	nmsub.ps	\$f3,\$f4,\$f5

[0-9a-f]+ <mips5_ps_insns>:
.*:	46c01005 	abs.ps	\$f0,\$f2
.*:	46c62080 	add.ps	\$f2,\$f4,\$f6
.*:	46ca4032 	c.eq.ps	\$f8,\$f10
.*:	46ca4030 	c.f.ps	\$f8,\$f10
.*:	46ca403e 	c.le.ps	\$f8,\$f10
.*:	46ca403c 	c.lt.ps	\$f8,\$f10
.*:	46ca403d 	c.nge.ps	\$f8,\$f10
.*:	46ca403b 	c.ngl.ps	\$f8,\$f10
.*:	46ca4039 	c.ngle.ps	\$f8,\$f10
.*:	46ca403f 	c.ngt.ps	\$f8,\$f10
.*:	46ca4036 	c.ole.ps	\$f8,\$f10
.*:	46ca4034 	c.olt.ps	\$f8,\$f10
.*:	46ca403a 	c.seq.ps	\$f8,\$f10
.*:	46ca4038 	c.sf.ps	\$f8,\$f10
.*:	46ca4033 	c.ueq.ps	\$f8,\$f10
.*:	46ca4037 	c.ule.ps	\$f8,\$f10
.*:	46ca4035 	c.ult.ps	\$f8,\$f10
.*:	46ca4031 	c.un.ps	\$f8,\$f10
.*:	46c0d606 	mov.ps	\$f24,\$f26
.*:	46c62082 	mul.ps	\$f2,\$f4,\$f6
.*:	46c04187 	neg.ps	\$f6,\$f8
.*:	46dac581 	sub.ps	\$f22,\$f24,\$f26
#pass
