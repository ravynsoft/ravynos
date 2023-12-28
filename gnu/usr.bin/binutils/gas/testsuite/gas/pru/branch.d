#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU branch

# Test the branch instructions

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 20ea0000 	jmp	r10
0+0004 <[^>]*> 208a0000 	jmp	r10.w0
0+0008 <[^>]*> 21004000 	jmp	00000100 <[^>]*>
0+000c <[^>]*> 22ca00f6 	jal	r22, r10.w2
0+0010 <[^>]*> 230000f7 	jal	r23, 00000000 <[^>]*>
0+0014 <[^>]*> 23ffffb7 	jal	r23.w1, 0003fffc <[^>]*>
0+0018 <[^>]*> 6100f700 	qbgt	00000018 <[^>]*>, r23, 0
[\t ]*18: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+001c <[^>]*> 71ff5700 	qbge	0000001c <[^>]*>, r23.b2, 255
[\t ]*1c: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+0020 <[^>]*> 4820b600 	qblt	00000020 <[^>]*>, r22.w1, r0.b1
[\t ]*20: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+0024 <[^>]*> 58210000 	qble	00000024 <[^>]*>, r0.b0, r1.b1
[\t ]*24: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+0028 <[^>]*> 50034100 	qbeq	00000028 <[^>]*>, r1.b2, ra.b0
[\t ]*28: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+002c <[^>]*> 68f6f500 	qbne	0000002c <[^>]*>, r21, r22
[\t ]*2c: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+0030 <[^>]*> 78000000 	qba	00000030 <[^>]*>
[\t ]*30: R_PRU_S10_PCREL[\t ]*.text\+0x60
#0+0034 <[^>]*> d0edec00 	qbbs	00000034 <[^>]*>, r12, r13
0+0034 <[^>]*> d0edec00 	wbc	r12, r13
[\t ]*34: R_PRU_S10_PCREL[\t ]*.text\+0x60
#0+0038 <[^>]*> d105ec00 	qbbs	00000038 <[^>]*>, r12, 5
0+0038 <[^>]*> d105ec00 	wbc	r12, 5
[\t ]*38: R_PRU_S10_PCREL[\t ]*.text\+0x60
#0+003c <[^>]*> c8edec00 	qbbc	0000003c <[^>]*>, r12, r13
0+003c <[^>]*> c8edec00 	wbs	r12, r13
[\t ]*3c: R_PRU_S10_PCREL[\t ]*.text\+0x60
#0+0040 <[^>]*> c905ec00 	qbbc	00000040 <[^>]*>, r12, 5
0+0040 <[^>]*> c905ec00 	wbs	r12, 5
[\t ]*40: R_PRU_S10_PCREL[\t ]*.text\+0x60
0+0044 <[^>]*> 6100f700 	qbgt	00000044 <[^>]*>, r23, 0
[\t ]*44: R_PRU_S10_PCREL[\t ]*.text\+0xc
0+0048 <[^>]*> 71ff5700 	qbge	00000048 <[^>]*>, r23.b2, 255
[\t ]*48: R_PRU_S10_PCREL[\t ]*.text\+0xc
0+004c <[^>]*> 4820b600 	qblt	0000004c <[^>]*>, r22.w1, r0.b1
[\t ]*4c: R_PRU_S10_PCREL[\t ]*.text\+0xc
0+0050 <[^>]*> 58210000 	qble	00000050 <[^>]*>, r0.b0, r1.b1
[\t ]*50: R_PRU_S10_PCREL[\t ]*.text\+0xc
0+0054 <[^>]*> 50034100 	qbeq	00000054 <[^>]*>, r1.b2, ra.b0
[\t ]*54: R_PRU_S10_PCREL[\t ]*.text\+0xc
0+0058 <[^>]*> 68f6f500 	qbne	00000058 <[^>]*>, r21, r22
[\t ]*58: R_PRU_S10_PCREL[\t ]*.text\+0xc
0+005c <[^>]*> 78000000 	qba	0000005c <[^>]*>
[\t ]*5c: R_PRU_S10_PCREL[\t ]*.text\+0xc
#0+0060 <[^>]*> d0edec00 	qbbs	00000060 <[^>]*>, r12, r13
0+0060 <[^>]*> d0edec00 	wbc	r12, r13
[\t ]*60: R_PRU_S10_PCREL[\t ]*.text\+0xc
#0+0064 <[^>]*> d105ec00 	qbbs	00000064 <[^>]*>, r12, 5
0+0064 <[^>]*> d105ec00 	wbc	r12, 5
[\t ]*64: R_PRU_S10_PCREL[\t ]*.text\+0xc
#0+0068 <[^>]*> c8edec00 	qbbc	00000068 <[^>]*>, r12, r13
0+0068 <[^>]*> c8edec00 	wbs	r12, r13
[\t ]*68: R_PRU_S10_PCREL[\t ]*.text\+0xc
