#as: -march=loongson2f -mabi=o64
#objdump: -M reg-names=numeric -dp
#name: Loongson-2F MMI tests

.*:     file format .*

private flags = .*

MIPS ABI Flags Version: 0
ISA: .*
GPR size: .*
CPR1 size: .*
CPR2 size: .*
FP ABI: .*
ISA Extension: ST Microelectronics Loongson 2F
ASEs:
	Loongson MMI ASE
FLAGS 1: .*
FLAGS 2: .*

Disassembly of section .text:

[0-9a-f]+ <simd_insns>:
.*:	4b420802 	packsshb	\$f0,\$f1,\$f2
.*:	4b2520c2 	packsswh	\$f3,\$f4,\$f5
.*:	4b683982 	packushb	\$f6,\$f7,\$f8
.*:	4bcb5240 	paddb	\$f9,\$f10,\$f11
.*:	4b4e6b00 	paddh	\$f12,\$f13,\$f14
.*:	4b7183c0 	paddw	\$f15,\$f16,\$f17
.*:	4bf49c80 	paddd	\$f18,\$f19,\$f20
.*:	4b97b540 	paddsb	\$f21,\$f22,\$f23
.*:	4b1ace00 	paddsh	\$f24,\$f25,\$f26
.*:	4bbde6c0 	paddusb	\$f27,\$f28,\$f29
.*:	4b220800 	paddush	\$f0,\$f1,\$f2
.*:	4be520c2 	pandn	\$f3,\$f4,\$f5
.*:	4b283988 	pavgb	\$f6,\$f7,\$f8
.*:	4b0b5248 	pavgh	\$f9,\$f10,\$f11
.*:	4b8e6b09 	pcmpeqb	\$f12,\$f13,\$f14
.*:	4b5183c9 	pcmpeqh	\$f15,\$f16,\$f17
.*:	4b149c89 	pcmpeqw	\$f18,\$f19,\$f20
.*:	4bb7b549 	pcmpgtb	\$f21,\$f22,\$f23
.*:	4b7ace09 	pcmpgth	\$f24,\$f25,\$f26
.*:	4b3de6c9 	pcmpgtw	\$f27,\$f28,\$f29
.*:	4b42080e 	pextrh	\$f0,\$f1,\$f2
.*:	4b8520c3 	pinsrh_0	\$f3,\$f4,\$f5
.*:	4ba83983 	pinsrh_1	\$f6,\$f7,\$f8
.*:	4bcb5243 	pinsrh_2	\$f9,\$f10,\$f11
.*:	4bee6b03 	pinsrh_3	\$f12,\$f13,\$f14
.*:	4b7183ce 	pmaddhw	\$f15,\$f16,\$f17
.*:	4b549c88 	pmaxsh	\$f18,\$f19,\$f20
.*:	4b97b548 	pmaxub	\$f21,\$f22,\$f23
.*:	4b7ace08 	pminsh	\$f24,\$f25,\$f26
.*:	4bbde6c8 	pminub	\$f27,\$f28,\$f29
.*:	4ba0080f 	pmovmskb	\$f0,\$f1
.*:	4ba4188a 	pmulhuh	\$f2,\$f3,\$f4
.*:	4b67314a 	pmulhh	\$f5,\$f6,\$f7
.*:	4b4a4a0a 	pmullh	\$f8,\$f9,\$f10
.*:	4b8d62ca 	pmuluw	\$f11,\$f12,\$f13
.*:	4b307b8d 	pasubub	\$f14,\$f15,\$f16
.*:	4b80944f 	biadd	\$f17,\$f18
.*:	4b15a4c2 	pshufh	\$f19,\$f20,\$f21
.*:	4b38bd8a 	psllh	\$f22,\$f23,\$f24
.*:	4b1bd64a 	psllw	\$f25,\$f26,\$f27
.*:	4b7eef0b 	psrah	\$f28,\$f29,\$f30
.*:	4b42080b 	psraw	\$f0,\$f1,\$f2
.*:	4b2520cb 	psrlh	\$f3,\$f4,\$f5
.*:	4b08398b 	psrlw	\$f6,\$f7,\$f8
.*:	4bcb5241 	psubb	\$f9,\$f10,\$f11
.*:	4b4e6b01 	psubh	\$f12,\$f13,\$f14
.*:	4b7183c1 	psubw	\$f15,\$f16,\$f17
.*:	4bf49c81 	psubd	\$f18,\$f19,\$f20
.*:	4b97b541 	psubsb	\$f21,\$f22,\$f23
.*:	4b1ace01 	psubsh	\$f24,\$f25,\$f26
.*:	4bbde6c1 	psubusb	\$f27,\$f28,\$f29
.*:	4b220801 	psubush	\$f0,\$f1,\$f2
.*:	4b6520c3 	punpckhbh	\$f3,\$f4,\$f5
.*:	4b283983 	punpckhhw	\$f6,\$f7,\$f8
.*:	4bab524b 	punpckhwd	\$f9,\$f10,\$f11
.*:	4b4e6b03 	punpcklbh	\$f12,\$f13,\$f14
.*:	4b1183c3 	punpcklhw	\$f15,\$f16,\$f17
.*:	4b949c8b 	punpcklwd	\$f18,\$f19,\$f20

[0-9a-f]+ <fixed_point_insns>:
.*:	4b42080c 	add	\$f0,\$f1,\$f2
.*:	4b0520cc 	addu	\$f3,\$f4,\$f5
.*:	4b68398c 	dadd	\$f6,\$f7,\$f8
.*:	4b4b524d 	sub	\$f9,\$f10,\$f11
.*:	4b0e6b0d 	subu	\$f12,\$f13,\$f14
.*:	4b7183cd 	dsub	\$f15,\$f16,\$f17
.*:	4b349c8c 	or	\$f18,\$f19,\$f20
.*:	4b17b54e 	sll	\$f21,\$f22,\$f23
.*:	4b3ace0e 	dsll	\$f24,\$f25,\$f26
.*:	4b9de6c2 	xor	\$f27,\$f28,\$f29
.*:	4ba20802 	nor	\$f0,\$f1,\$f2
.*:	4bc520c2 	and	\$f3,\$f4,\$f5
.*:	4b08398f 	srl	\$f6,\$f7,\$f8
.*:	4b2b524f 	dsrl	\$f9,\$f10,\$f11
.*:	4b4e6b0f 	sra	\$f12,\$f13,\$f14
.*:	4b7183cf 	dsra	\$f15,\$f16,\$f17
.*:	4b93900c 	sequ	\$f18,\$f19
.*:	4b95a00d 	sltu	\$f20,\$f21
.*:	4b97b00e 	sleu	\$f22,\$f23
.*:	4bb9c00c 	seq	\$f24,\$f25
.*:	4bbbd00d 	slt	\$f26,\$f27
.*:	4bbde00e 	sle	\$f28,\$f29
#pass
