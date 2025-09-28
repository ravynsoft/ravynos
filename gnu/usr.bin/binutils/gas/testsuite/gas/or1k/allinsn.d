#as:
#objdump: -dr
#name: allinsn

.*: +file format .*


Disassembly of section \.text:

00000000 <localtext>:
   0:	15 00 00 00 	l\.nop 0x0

00000004 <globaltext>:
   4:	15 00 00 00 	l\.nop 0x0

00000008 <l_j>:
   8:	03 ff ff ff 	l\.j 4 <globaltext>
   c:	00 00 00 01 	l\.j 10 <l_j\+0x8>
  10:	00 00 00 00 	l\.j 10 <l_j\+0x8>
  14:	03 ff ff fb 	l\.j 0 <localtext>
	\.\.\.
			18: R_OR1K_INSN_REL_26	\.data
			1c: R_OR1K_INSN_REL_26	globaltext
			20: R_OR1K_INSN_REL_26	globaldata
  24:	03 ff ff f9 	l\.j 8 <l_j>
  28:	00 00 00 01 	l\.j 2c <l_jal>

0000002c <l_jal>:
  2c:	07 ff ff ff 	l\.jal 28 <l_j\+0x20>
  30:	04 00 00 01 	l\.jal 34 <l_jal\+0x8>
  34:	04 00 00 00 	l\.jal 34 <l_jal\+0x8>
  38:	07 ff ff f2 	l\.jal 0 <localtext>
  3c:	04 00 00 00 	l\.jal 3c <l_jal\+0x10>
			3c: R_OR1K_INSN_REL_26	\.data
  40:	04 00 00 00 	l\.jal 40 <l_jal\+0x14>
			40: R_OR1K_INSN_REL_26	globaltext
  44:	04 00 00 00 	l\.jal 44 <l_jal\+0x18>
			44: R_OR1K_INSN_REL_26	globaldata
  48:	07 ff ff f0 	l\.jal 8 <l_j>
  4c:	07 ff ff f8 	l\.jal 2c <l_jal>

00000050 <l_jr>:
  50:	44 00 00 00 	l\.jr r0
  54:	44 00 f8 00 	l\.jr r31
  58:	44 00 80 00 	l\.jr r16
  5c:	44 00 78 00 	l\.jr r15
  60:	44 00 08 00 	l\.jr r1
  64:	44 00 d8 00 	l\.jr r27
  68:	44 00 70 00 	l\.jr r14
  6c:	44 00 b0 00 	l\.jr r22

00000070 <l_jalr>:
  70:	48 00 00 00 	l\.jalr r0
  74:	48 00 f8 00 	l\.jalr r31
  78:	48 00 80 00 	l\.jalr r16
  7c:	48 00 78 00 	l\.jalr r15
  80:	48 00 08 00 	l\.jalr r1
  84:	48 00 d8 00 	l\.jalr r27
  88:	48 00 70 00 	l\.jalr r14
  8c:	48 00 b0 00 	l\.jalr r22

00000090 <l_bnf>:
  90:	0f ff ff ff 	l\.bnf 8c <l_jalr\+0x1c>
  94:	0c 00 00 01 	l\.bnf 98 <l_bnf\+0x8>
  98:	0c 00 00 00 	l\.bnf 98 <l_bnf\+0x8>
  9c:	0f ff ff d9 	l\.bnf 0 <localtext>
  a0:	0c 00 00 00 	l\.bnf a0 <l_bnf\+0x10>
			a0: R_OR1K_INSN_REL_26	\.data
  a4:	0c 00 00 00 	l\.bnf a4 <l_bnf\+0x14>
			a4: R_OR1K_INSN_REL_26	globaltext
  a8:	0c 00 00 00 	l\.bnf a8 <l_bnf\+0x18>
			a8: R_OR1K_INSN_REL_26	globaldata
  ac:	0f ff ff d7 	l\.bnf 8 <l_j>
  b0:	0f ff ff df 	l\.bnf 2c <l_jal>

000000b4 <l_bf>:
  b4:	13 ff ff ff 	l\.bf b0 <l_bnf\+0x20>
  b8:	10 00 00 01 	l\.bf bc <l_bf\+0x8>
  bc:	10 00 00 00 	l\.bf bc <l_bf\+0x8>
  c0:	13 ff ff d0 	l\.bf 0 <localtext>
  c4:	10 00 00 00 	l\.bf c4 <l_bf\+0x10>
			c4: R_OR1K_INSN_REL_26	\.data
  c8:	10 00 00 00 	l\.bf c8 <l_bf\+0x14>
			c8: R_OR1K_INSN_REL_26	globaltext
  cc:	10 00 00 00 	l\.bf cc <l_bf\+0x18>
			cc: R_OR1K_INSN_REL_26	globaldata
  d0:	13 ff ff ce 	l\.bf 8 <l_j>
  d4:	13 ff ff d6 	l\.bf 2c <l_jal>

000000d8 <l_trap>:
  d8:	21 00 00 00 	l\.trap 0x0
  dc:	21 00 ff ff 	l\.trap 0xffff
  e0:	21 00 80 00 	l\.trap 0x8000
  e4:	21 00 7f ff 	l\.trap 0x7fff
  e8:	21 00 00 01 	l\.trap 0x1
  ec:	21 00 d1 4f 	l\.trap 0xd14f
  f0:	21 00 7f 7c 	l\.trap 0x7f7c
  f4:	21 00 d2 4a 	l\.trap 0xd24a

000000f8 <l_sys>:
  f8:	20 00 00 00 	l\.sys 0x0
  fc:	20 00 ff ff 	l\.sys 0xffff
 100:	20 00 80 00 	l\.sys 0x8000
 104:	20 00 7f ff 	l\.sys 0x7fff
 108:	20 00 00 01 	l\.sys 0x1
 10c:	20 00 d2 85 	l\.sys 0xd285
 110:	20 00 e3 15 	l\.sys 0xe315
 114:	20 00 80 fa 	l\.sys 0x80fa

00000118 <l_rfe>:
 118:	24 00 00 00 	l\.rfe

0000011c <l_nop>:
 11c:	15 00 00 00 	l\.nop 0x0

00000120 <l_movhi>:
 120:	18 00 00 00 	l\.movhi r0,0x0
 124:	1b e0 ff ff 	l\.movhi r31,0xffff
 128:	1a 00 80 00 	l\.movhi r16,0x8000
 12c:	19 e0 7f ff 	l\.movhi r15,0x7fff
 130:	18 20 00 01 	l\.movhi r1,0x1
 134:	1b 80 81 ce 	l\.movhi r28,0x81ce
 138:	1a e0 e8 ac 	l\.movhi r23,0xe8ac
 13c:	1a 60 d8 c0 	l\.movhi r19,0xd8c0

00000140 <l_mfspr>:
 140:	b4 00 00 00 	l\.mfspr r0,r0,0x0
 144:	b7 ff ff ff 	l\.mfspr r31,r31,0xffff
 148:	b6 10 80 00 	l\.mfspr r16,r16,0x8000
 14c:	b5 ef 7f ff 	l\.mfspr r15,r15,0x7fff
 150:	b4 21 00 01 	l\.mfspr r1,r1,0x1
 154:	b6 fd d4 98 	l\.mfspr r23,r29,0xd498
 158:	b6 74 11 81 	l\.mfspr r19,r20,0x1181
 15c:	b7 42 f7 d6 	l\.mfspr r26,r2,0xf7d6

00000160 <l_mtspr>:
 160:	c0 00 00 00 	l\.mtspr r0,r0,0x0
 164:	c3 ff ff ff 	l\.mtspr r31,r31,0xffff
 168:	c2 10 80 00 	l\.mtspr r16,r16,0x8000
 16c:	c1 ef 7f ff 	l\.mtspr r15,r15,0x7fff
 170:	c0 01 08 01 	l\.mtspr r1,r1,0x1
 174:	c0 fe 33 77 	l\.mtspr r30,r6,0x3b77
 178:	c2 a9 3c cc 	l\.mtspr r9,r7,0xaccc
 17c:	c3 f9 3d 7b 	l\.mtspr r25,r7,0xfd7b

00000180 <l_lwz>:
 180:	84 00 00 00 	l\.lwz r0,0\(r0\)
 184:	87 ff ff ff 	l\.lwz r31,-1\(r31\)
 188:	86 10 80 00 	l\.lwz r16,-32768\(r16\)
 18c:	85 ef 7f ff 	l\.lwz r15,32767\(r15\)
 190:	84 21 00 01 	l\.lwz r1,1\(r1\)
 194:	85 f9 0b 75 	l\.lwz r15,2933\(r25\)
 198:	86 35 fc e1 	l\.lwz r17,-799\(r21\)
 19c:	84 12 bb 45 	l\.lwz r0,-17595\(r18\)

000001a0 <l_lws>:
 1a0:	88 00 00 00 	l\.lws r0,0\(r0\)
 1a4:	8b ff ff ff 	l\.lws r31,-1\(r31\)
 1a8:	8a 10 80 00 	l\.lws r16,-32768\(r16\)
 1ac:	89 ef 7f ff 	l\.lws r15,32767\(r15\)
 1b0:	88 21 00 01 	l\.lws r1,1\(r1\)
 1b4:	88 35 bb 3a 	l\.lws r1,-17606\(r21\)
 1b8:	89 df 69 0b 	l\.lws r14,26891\(r31\)
 1bc:	89 00 6b a0 	l\.lws r8,27552\(r0\)

000001c0 <l_lbz>:
 1c0:	8c 00 00 00 	l\.lbz r0,0\(r0\)
 1c4:	8f ff ff ff 	l\.lbz r31,-1\(r31\)
 1c8:	8e 10 80 00 	l\.lbz r16,-32768\(r16\)
 1cc:	8d ef 7f ff 	l\.lbz r15,32767\(r15\)
 1d0:	8c 21 00 01 	l\.lbz r1,1\(r1\)
 1d4:	8e 74 64 23 	l\.lbz r19,25635\(r20\)
 1d8:	8d e9 f2 a8 	l\.lbz r15,-3416\(r9\)
 1dc:	8c 61 45 54 	l\.lbz r3,17748\(r1\)

000001e0 <l_lbs>:
 1e0:	90 00 00 00 	l\.lbs r0,0\(r0\)
 1e4:	93 ff ff ff 	l\.lbs r31,-1\(r31\)
 1e8:	92 10 80 00 	l\.lbs r16,-32768\(r16\)
 1ec:	91 ef 7f ff 	l\.lbs r15,32767\(r15\)
 1f0:	90 21 00 01 	l\.lbs r1,1\(r1\)
 1f4:	93 48 44 c6 	l\.lbs r26,17606\(r8\)
 1f8:	92 d0 86 a0 	l\.lbs r22,-31072\(r16\)
 1fc:	90 c9 44 20 	l\.lbs r6,17440\(r9\)

00000200 <l_lhz>:
 200:	94 00 00 00 	l\.lhz r0,0\(r0\)
 204:	97 ff ff ff 	l\.lhz r31,-1\(r31\)
 208:	96 10 80 00 	l\.lhz r16,-32768\(r16\)
 20c:	95 ef 7f ff 	l\.lhz r15,32767\(r15\)
 210:	94 21 00 01 	l\.lhz r1,1\(r1\)
 214:	94 a4 e9 dd 	l\.lhz r5,-5667\(r4\)
 218:	97 04 16 d8 	l\.lhz r24,5848\(r4\)
 21c:	95 47 7b bb 	l\.lhz r10,31675\(r7\)

00000220 <l_lhs>:
 220:	98 00 00 00 	l\.lhs r0,0\(r0\)
 224:	9b ff ff ff 	l\.lhs r31,-1\(r31\)
 228:	9a 10 80 00 	l\.lhs r16,-32768\(r16\)
 22c:	99 ef 7f ff 	l\.lhs r15,32767\(r15\)
 230:	98 21 00 01 	l\.lhs r1,1\(r1\)
 234:	98 cb ff 72 	l\.lhs r6,-142\(r11\)
 238:	9a 9d eb 46 	l\.lhs r20,-5306\(r29\)
 23c:	99 f5 10 52 	l\.lhs r15,4178\(r21\)

00000240 <l_sw>:
 240:	d4 00 00 00 	l\.sw 0\(r0\),r0
 244:	d7 ff ff ff 	l\.sw -1\(r31\),r31
 248:	d6 10 80 00 	l\.sw -32768\(r16\),r16
 24c:	d5 ef 7f ff 	l\.sw 32767\(r15\),r15
 250:	d4 01 08 01 	l\.sw 1\(r1\),r1
 254:	d7 91 50 e1 	l\.sw -7967\(r17\),r10
 258:	d4 1e 57 20 	l\.sw 1824\(r30\),r10
 25c:	d5 ef 23 4e 	l\.sw 31566\(r15\),r4

00000260 <l_sb>:
 260:	d8 00 00 00 	l\.sb 0\(r0\),r0
 264:	db ff ff ff 	l\.sb -1\(r31\),r31
 268:	da 10 80 00 	l\.sb -32768\(r16\),r16
 26c:	d9 ef 7f ff 	l\.sb 32767\(r15\),r15
 270:	d8 01 08 01 	l\.sb 1\(r1\),r1
 274:	d9 4a 06 b8 	l\.sb 22200\(r10\),r0
 278:	d8 90 df 0b 	l\.sb 9995\(r16\),r27
 27c:	da 4e f9 9c 	l\.sb -28260\(r14\),r31

00000280 <l_sh>:
 280:	dc 00 00 00 	l\.sh 0\(r0\),r0
 284:	df ff ff ff 	l\.sh -1\(r31\),r31
 288:	de 10 80 00 	l\.sh -32768\(r16\),r16
 28c:	dd ef 7f ff 	l\.sh 32767\(r15\),r15
 290:	dc 01 08 01 	l\.sh 1\(r1\),r1
 294:	dc b5 c9 bd 	l\.sh 10685\(r21\),r25
 298:	df 3c 2c f6 	l\.sh -13066\(r28\),r5
 29c:	de 49 ef 50 	l\.sh -26800\(r9\),r29

000002a0 <l_sll>:
 2a0:	e0 00 00 08 	l\.sll r0,r0,r0
 2a4:	e3 ff f8 08 	l\.sll r31,r31,r31
 2a8:	e2 10 80 08 	l\.sll r16,r16,r16
 2ac:	e1 ef 78 08 	l\.sll r15,r15,r15
 2b0:	e0 21 08 08 	l\.sll r1,r1,r1
 2b4:	e3 f0 40 08 	l\.sll r31,r16,r8
 2b8:	e3 f1 b0 08 	l\.sll r31,r17,r22
 2bc:	e1 ee 28 08 	l\.sll r15,r14,r5

000002c0 <l_slli>:
 2c0:	b8 00 00 00 	l\.slli r0,r0,0x0
 2c4:	bb ff 00 3f 	l\.slli r31,r31,0x3f
 2c8:	ba 10 00 20 	l\.slli r16,r16,0x20
 2cc:	b9 ef 00 1f 	l\.slli r15,r15,0x1f
 2d0:	b8 21 00 01 	l\.slli r1,r1,0x1
 2d4:	b9 6e 00 31 	l\.slli r11,r14,0x31
 2d8:	b8 fb 00 17 	l\.slli r7,r27,0x17
 2dc:	bb d0 00 0b 	l\.slli r30,r16,0xb

000002e0 <l_srl>:
 2e0:	e0 00 00 48 	l\.srl r0,r0,r0
 2e4:	e3 ff f8 48 	l\.srl r31,r31,r31
 2e8:	e2 10 80 48 	l\.srl r16,r16,r16
 2ec:	e1 ef 78 48 	l\.srl r15,r15,r15
 2f0:	e0 21 08 48 	l\.srl r1,r1,r1
 2f4:	e1 f9 68 48 	l\.srl r15,r25,r13
 2f8:	e2 60 88 48 	l\.srl r19,r0,r17
 2fc:	e1 a0 b8 48 	l\.srl r13,r0,r23

00000300 <l_srli>:
 300:	b8 00 00 40 	l\.srli r0,r0,0x0
 304:	bb ff 00 7f 	l\.srli r31,r31,0x3f
 308:	ba 10 00 60 	l\.srli r16,r16,0x20
 30c:	b9 ef 00 5f 	l\.srli r15,r15,0x1f
 310:	b8 21 00 41 	l\.srli r1,r1,0x1
 314:	b9 fe 00 4d 	l\.srli r15,r30,0xd
 318:	b9 a3 00 7f 	l\.srli r13,r3,0x3f
 31c:	b8 52 00 5e 	l\.srli r2,r18,0x1e

00000320 <l_sra>:
 320:	e0 00 00 88 	l\.sra r0,r0,r0
 324:	e3 ff f8 88 	l\.sra r31,r31,r31
 328:	e2 10 80 88 	l\.sra r16,r16,r16
 32c:	e1 ef 78 88 	l\.sra r15,r15,r15
 330:	e0 21 08 88 	l\.sra r1,r1,r1
 334:	e0 7a 00 88 	l\.sra r3,r26,r0
 338:	e3 b2 d8 88 	l\.sra r29,r18,r27
 33c:	e3 7d 18 88 	l\.sra r27,r29,r3

00000340 <l_srai>:
 340:	b8 00 00 80 	l\.srai r0,r0,0x0
 344:	bb ff 00 bf 	l\.srai r31,r31,0x3f
 348:	ba 10 00 a0 	l\.srai r16,r16,0x20
 34c:	b9 ef 00 9f 	l\.srai r15,r15,0x1f
 350:	b8 21 00 81 	l\.srai r1,r1,0x1
 354:	b9 4b 00 9c 	l\.srai r10,r11,0x1c
 358:	ba ef 00 b0 	l\.srai r23,r15,0x30
 35c:	ba 0f 00 a6 	l\.srai r16,r15,0x26

00000360 <l_ror>:
 360:	e0 00 00 c8 	l\.ror r0,r0,r0
 364:	e3 ff f8 c8 	l\.ror r31,r31,r31
 368:	e2 10 80 c8 	l\.ror r16,r16,r16
 36c:	e1 ef 78 c8 	l\.ror r15,r15,r15
 370:	e0 21 08 c8 	l\.ror r1,r1,r1
 374:	e3 ac 28 c8 	l\.ror r29,r12,r5
 378:	e2 46 20 c8 	l\.ror r18,r6,r4
 37c:	e0 50 88 c8 	l\.ror r2,r16,r17

00000380 <l_rori>:
 380:	b8 00 00 c0 	l\.rori r0,r0,0x0
 384:	bb ff 00 ff 	l\.rori r31,r31,0x3f
 388:	ba 10 00 e0 	l\.rori r16,r16,0x20
 38c:	b9 ef 00 df 	l\.rori r15,r15,0x1f
 390:	b8 21 00 c1 	l\.rori r1,r1,0x1
 394:	ba 20 00 d7 	l\.rori r17,r0,0x17
 398:	ba 1f 00 ea 	l\.rori r16,r31,0x2a
 39c:	b9 b5 00 cc 	l\.rori r13,r21,0xc

000003a0 <l_add>:
 3a0:	e0 00 00 00 	l\.add r0,r0,r0
 3a4:	e3 ff f8 00 	l\.add r31,r31,r31
 3a8:	e2 10 80 00 	l\.add r16,r16,r16
 3ac:	e1 ef 78 00 	l\.add r15,r15,r15
 3b0:	e0 21 08 00 	l\.add r1,r1,r1
 3b4:	e3 a7 20 00 	l\.add r29,r7,r4
 3b8:	e3 aa 90 00 	l\.add r29,r10,r18
 3bc:	e2 56 b8 00 	l\.add r18,r22,r23

000003c0 <l_sub>:
 3c0:	e0 00 00 02 	l\.sub r0,r0,r0
 3c4:	e3 ff f8 02 	l\.sub r31,r31,r31
 3c8:	e2 10 80 02 	l\.sub r16,r16,r16
 3cc:	e1 ef 78 02 	l\.sub r15,r15,r15
 3d0:	e0 21 08 02 	l\.sub r1,r1,r1
 3d4:	e2 fa 70 02 	l\.sub r23,r26,r14
 3d8:	e1 58 78 02 	l\.sub r10,r24,r15
 3dc:	e1 64 90 02 	l\.sub r11,r4,r18

000003e0 <l_and>:
 3e0:	e0 00 00 03 	l\.and r0,r0,r0
 3e4:	e3 ff f8 03 	l\.and r31,r31,r31
 3e8:	e2 10 80 03 	l\.and r16,r16,r16
 3ec:	e1 ef 78 03 	l\.and r15,r15,r15
 3f0:	e0 21 08 03 	l\.and r1,r1,r1
 3f4:	e0 1f c8 03 	l\.and r0,r31,r25
 3f8:	e3 c7 98 03 	l\.and r30,r7,r19
 3fc:	e2 62 d0 03 	l\.and r19,r2,r26

00000400 <l_or>:
 400:	e0 00 00 04 	l\.or r0,r0,r0
 404:	e3 ff f8 04 	l\.or r31,r31,r31
 408:	e2 10 80 04 	l\.or r16,r16,r16
 40c:	e1 ef 78 04 	l\.or r15,r15,r15
 410:	e0 21 08 04 	l\.or r1,r1,r1
 414:	e2 2a 10 04 	l\.or r17,r10,r2
 418:	e0 f3 e8 04 	l\.or r7,r19,r29
 41c:	e0 71 88 04 	l\.or r3,r17,r17

00000420 <l_xor>:
 420:	e0 00 00 05 	l\.xor r0,r0,r0
 424:	e3 ff f8 05 	l\.xor r31,r31,r31
 428:	e2 10 80 05 	l\.xor r16,r16,r16
 42c:	e1 ef 78 05 	l\.xor r15,r15,r15
 430:	e0 21 08 05 	l\.xor r1,r1,r1
 434:	e3 e5 88 05 	l\.xor r31,r5,r17
 438:	e2 c4 28 05 	l\.xor r22,r4,r5
 43c:	e3 d4 d0 05 	l\.xor r30,r20,r26

00000440 <l_addc>:
 440:	e0 00 00 01 	l\.addc r0,r0,r0
 444:	e3 ff f8 01 	l\.addc r31,r31,r31
 448:	e2 10 80 01 	l\.addc r16,r16,r16
 44c:	e1 ef 78 01 	l\.addc r15,r15,r15
 450:	e0 21 08 01 	l\.addc r1,r1,r1
 454:	e1 1a c0 01 	l\.addc r8,r26,r24
 458:	e2 46 20 01 	l\.addc r18,r6,r4
 45c:	e3 a0 90 01 	l\.addc r29,r0,r18

00000460 <l_mul>:
 460:	e0 00 03 06 	l\.mul r0,r0,r0
 464:	e3 ff fb 06 	l\.mul r31,r31,r31
 468:	e2 10 83 06 	l\.mul r16,r16,r16
 46c:	e1 ef 7b 06 	l\.mul r15,r15,r15
 470:	e0 21 0b 06 	l\.mul r1,r1,r1
 474:	e1 19 6b 06 	l\.mul r8,r25,r13
 478:	e1 15 eb 06 	l\.mul r8,r21,r29
 47c:	e3 63 8b 06 	l\.mul r27,r3,r17

00000480 <l_mulu>:
 480:	e0 00 03 0b 	l\.mulu r0,r0,r0
 484:	e3 ff fb 0b 	l\.mulu r31,r31,r31
 488:	e2 10 83 0b 	l\.mulu r16,r16,r16
 48c:	e1 ef 7b 0b 	l\.mulu r15,r15,r15
 490:	e0 21 0b 0b 	l\.mulu r1,r1,r1
 494:	e3 4e 83 0b 	l\.mulu r26,r14,r16
 498:	e0 32 5b 0b 	l\.mulu r1,r18,r11
 49c:	e1 d2 8b 0b 	l\.mulu r14,r18,r17

000004a0 <l_div>:
 4a0:	e0 00 03 09 	l\.div r0,r0,r0
 4a4:	e3 ff fb 09 	l\.div r31,r31,r31
 4a8:	e2 10 83 09 	l\.div r16,r16,r16
 4ac:	e1 ef 7b 09 	l\.div r15,r15,r15
 4b0:	e0 21 0b 09 	l\.div r1,r1,r1
 4b4:	e0 02 e3 09 	l\.div r0,r2,r28
 4b8:	e3 47 fb 09 	l\.div r26,r7,r31
 4bc:	e0 52 a3 09 	l\.div r2,r18,r20

000004c0 <l_divu>:
 4c0:	e0 00 03 0a 	l\.divu r0,r0,r0
 4c4:	e3 ff fb 0a 	l\.divu r31,r31,r31
 4c8:	e2 10 83 0a 	l\.divu r16,r16,r16
 4cc:	e1 ef 7b 0a 	l\.divu r15,r15,r15
 4d0:	e0 21 0b 0a 	l\.divu r1,r1,r1
 4d4:	e0 a4 cb 0a 	l\.divu r5,r4,r25
 4d8:	e1 0b eb 0a 	l\.divu r8,r11,r29
 4dc:	e1 73 13 0a 	l\.divu r11,r19,r2

000004e0 <l_addi>:
 4e0:	9c 00 00 00 	l\.addi r0,r0,0
 4e4:	9f ff ff ff 	l\.addi r31,r31,-1
 4e8:	9e 10 80 00 	l\.addi r16,r16,-32768
 4ec:	9d ef 7f ff 	l\.addi r15,r15,32767
 4f0:	9c 21 00 01 	l\.addi r1,r1,1
 4f4:	9d c0 1b 6c 	l\.addi r14,r0,7020
 4f8:	9d ae 37 33 	l\.addi r13,r14,14131
 4fc:	9d d0 97 3b 	l\.addi r14,r16,-26821

00000500 <l_andi>:
 500:	a4 00 00 00 	l\.andi r0,r0,0x0
 504:	a7 ff ff ff 	l\.andi r31,r31,0xffff
 508:	a6 10 80 00 	l\.andi r16,r16,0x8000
 50c:	a5 ef 7f ff 	l\.andi r15,r15,0x7fff
 510:	a4 21 00 01 	l\.andi r1,r1,0x1
 514:	a7 75 2e 97 	l\.andi r27,r21,0x2e97
 518:	a6 b7 2f 1b 	l\.andi r21,r23,0x2f1b
 51c:	a7 de 83 c4 	l\.andi r30,r30,0x83c4

00000520 <l_ori>:
 520:	a8 00 00 00 	l\.ori r0,r0,0x0
 524:	ab ff ff ff 	l\.ori r31,r31,0xffff
 528:	aa 10 80 00 	l\.ori r16,r16,0x8000
 52c:	a9 ef 7f ff 	l\.ori r15,r15,0x7fff
 530:	a8 21 00 01 	l\.ori r1,r1,0x1
 534:	aa db d8 81 	l\.ori r22,r27,0xd881
 538:	aa 3f 00 80 	l\.ori r17,r31,0x80
 53c:	a9 b4 cf 6d 	l\.ori r13,r20,0xcf6d

00000540 <l_xori>:
 540:	ac 00 00 00 	l\.xori r0,r0,0
 544:	af ff ff ff 	l\.xori r31,r31,-1
 548:	ae 10 80 00 	l\.xori r16,r16,-32768
 54c:	ad ef 7f ff 	l\.xori r15,r15,32767
 550:	ac 21 00 01 	l\.xori r1,r1,1
 554:	ae 50 ff ff 	l\.xori r18,r16,-1
 558:	af 2d c0 35 	l\.xori r25,r13,-16331
 55c:	ad 9d 80 29 	l\.xori r12,r29,-32727

00000560 <l_muli>:
 560:	b0 00 00 00 	l\.muli r0,r0,0
 564:	b3 ff ff ff 	l\.muli r31,r31,-1
 568:	b2 10 80 00 	l\.muli r16,r16,-32768
 56c:	b1 ef 7f ff 	l\.muli r15,r15,32767
 570:	b0 21 00 01 	l\.muli r1,r1,1
 574:	b3 67 ed 85 	l\.muli r27,r7,-4731
 578:	b0 f4 ff ff 	l\.muli r7,r20,-1
 57c:	b3 15 5a b3 	l\.muli r24,r21,23219

00000580 <l_addic>:
 580:	a0 00 00 00 	l\.addic r0,r0,0
 584:	a3 ff ff ff 	l\.addic r31,r31,-1
 588:	a2 10 80 00 	l\.addic r16,r16,-32768
 58c:	a1 ef 7f ff 	l\.addic r15,r15,32767
 590:	a0 21 00 01 	l\.addic r1,r1,1
 594:	a0 d6 80 44 	l\.addic r6,r22,-32700
 598:	a2 69 ff ff 	l\.addic r19,r9,-1
 59c:	a3 7c 1a eb 	l\.addic r27,r28,6891

000005a0 <l_sfgtu>:
 5a0:	e4 40 00 00 	l\.sfgtu r0,r0
 5a4:	e4 5f f8 00 	l\.sfgtu r31,r31
 5a8:	e4 50 80 00 	l\.sfgtu r16,r16
 5ac:	e4 4f 78 00 	l\.sfgtu r15,r15
 5b0:	e4 41 08 00 	l\.sfgtu r1,r1
 5b4:	e4 48 20 00 	l\.sfgtu r8,r4
 5b8:	e4 51 a8 00 	l\.sfgtu r17,r21
 5bc:	e4 46 28 00 	l\.sfgtu r6,r5

000005c0 <l_sfgeu>:
 5c0:	e4 60 00 00 	l\.sfgeu r0,r0
 5c4:	e4 7f f8 00 	l\.sfgeu r31,r31
 5c8:	e4 70 80 00 	l\.sfgeu r16,r16
 5cc:	e4 6f 78 00 	l\.sfgeu r15,r15
 5d0:	e4 61 08 00 	l\.sfgeu r1,r1
 5d4:	e4 6e 60 00 	l\.sfgeu r14,r12
 5d8:	e4 76 38 00 	l\.sfgeu r22,r7
 5dc:	e4 6d 08 00 	l\.sfgeu r13,r1

000005e0 <l_sfltu>:
 5e0:	e4 80 00 00 	l\.sfltu r0,r0
 5e4:	e4 9f f8 00 	l\.sfltu r31,r31
 5e8:	e4 90 80 00 	l\.sfltu r16,r16
 5ec:	e4 8f 78 00 	l\.sfltu r15,r15
 5f0:	e4 81 08 00 	l\.sfltu r1,r1
 5f4:	e4 81 68 00 	l\.sfltu r1,r13
 5f8:	e4 96 f0 00 	l\.sfltu r22,r30
 5fc:	e4 94 30 00 	l\.sfltu r20,r6

00000600 <l_sfleu>:
 600:	e4 a0 00 00 	l\.sfleu r0,r0
 604:	e4 bf f8 00 	l\.sfleu r31,r31
 608:	e4 b0 80 00 	l\.sfleu r16,r16
 60c:	e4 af 78 00 	l\.sfleu r15,r15
 610:	e4 a1 08 00 	l\.sfleu r1,r1
 614:	e4 b3 40 00 	l\.sfleu r19,r8
 618:	e4 bb 78 00 	l\.sfleu r27,r15
 61c:	e4 bb 18 00 	l\.sfleu r27,r3

00000620 <l_sfgts>:
 620:	e5 40 00 00 	l\.sfgts r0,r0
 624:	e5 5f f8 00 	l\.sfgts r31,r31
 628:	e5 50 80 00 	l\.sfgts r16,r16
 62c:	e5 4f 78 00 	l\.sfgts r15,r15
 630:	e5 41 08 00 	l\.sfgts r1,r1
 634:	e5 45 28 00 	l\.sfgts r5,r5
 638:	e5 5f 28 00 	l\.sfgts r31,r5
 63c:	e5 5e 90 00 	l\.sfgts r30,r18

00000640 <l_sfges>:
 640:	e5 60 00 00 	l\.sfges r0,r0
 644:	e5 7f f8 00 	l\.sfges r31,r31
 648:	e5 70 80 00 	l\.sfges r16,r16
 64c:	e5 6f 78 00 	l\.sfges r15,r15
 650:	e5 61 08 00 	l\.sfges r1,r1
 654:	e5 71 90 00 	l\.sfges r17,r18
 658:	e5 60 48 00 	l\.sfges r0,r9
 65c:	e5 76 c8 00 	l\.sfges r22,r25

00000660 <l_sflts>:
 660:	e5 80 00 00 	l\.sflts r0,r0
 664:	e5 9f f8 00 	l\.sflts r31,r31
 668:	e5 90 80 00 	l\.sflts r16,r16
 66c:	e5 8f 78 00 	l\.sflts r15,r15
 670:	e5 81 08 00 	l\.sflts r1,r1
 674:	e5 99 c0 00 	l\.sflts r25,r24
 678:	e5 97 68 00 	l\.sflts r23,r13
 67c:	e5 8f 40 00 	l\.sflts r15,r8

00000680 <l_sfles>:
 680:	e5 a0 00 00 	l\.sfles r0,r0
 684:	e5 bf f8 00 	l\.sfles r31,r31
 688:	e5 b0 80 00 	l\.sfles r16,r16
 68c:	e5 af 78 00 	l\.sfles r15,r15
 690:	e5 a1 08 00 	l\.sfles r1,r1
 694:	e5 b1 68 00 	l\.sfles r17,r13
 698:	e5 be c8 00 	l\.sfles r30,r25
 69c:	e5 a0 60 00 	l\.sfles r0,r12

000006a0 <l_sfgtui>:
 6a0:	bc 40 00 00 	l\.sfgtui r0,0
 6a4:	bc 5f ff ff 	l\.sfgtui r31,-1
 6a8:	bc 50 80 00 	l\.sfgtui r16,-32768
 6ac:	bc 4f 7f ff 	l\.sfgtui r15,32767
 6b0:	bc 41 00 01 	l\.sfgtui r1,1
 6b4:	bc 45 4b 21 	l\.sfgtui r5,19233
 6b8:	bc 57 91 22 	l\.sfgtui r23,-28382
 6bc:	bc 51 25 dd 	l\.sfgtui r17,9693

000006c0 <l_sfgeui>:
 6c0:	bc 60 00 00 	l\.sfgeui r0,0
 6c4:	bc 7f ff ff 	l\.sfgeui r31,-1
 6c8:	bc 70 80 00 	l\.sfgeui r16,-32768
 6cc:	bc 6f 7f ff 	l\.sfgeui r15,32767
 6d0:	bc 61 00 01 	l\.sfgeui r1,1
 6d4:	bc 71 ec b6 	l\.sfgeui r17,-4938
 6d8:	bc 6f 40 13 	l\.sfgeui r15,16403
 6dc:	bc 66 f1 a4 	l\.sfgeui r6,-3676

000006e0 <l_sfltui>:
 6e0:	bc 80 00 00 	l\.sfltui r0,0
 6e4:	bc 9f ff ff 	l\.sfltui r31,-1
 6e8:	bc 90 80 00 	l\.sfltui r16,-32768
 6ec:	bc 8f 7f ff 	l\.sfltui r15,32767
 6f0:	bc 81 00 01 	l\.sfltui r1,1
 6f4:	bc 83 cc af 	l\.sfltui r3,-13137
 6f8:	bc 98 4c fd 	l\.sfltui r24,19709
 6fc:	bc 8a 03 3e 	l\.sfltui r10,830

00000700 <l_sfleui>:
 700:	bc a0 00 00 	l\.sfleui r0,0
 704:	bc bf ff ff 	l\.sfleui r31,-1
 708:	bc b0 80 00 	l\.sfleui r16,-32768
 70c:	bc af 7f ff 	l\.sfleui r15,32767
 710:	bc a1 00 01 	l\.sfleui r1,1
 714:	bc b7 9b 66 	l\.sfleui r23,-25754
 718:	bc b1 b6 d7 	l\.sfleui r17,-18729
 71c:	bc a9 a8 81 	l\.sfleui r9,-22399

00000720 <l_sfgtsi>:
 720:	bd 40 00 00 	l\.sfgtsi r0,0
 724:	bd 5f ff ff 	l\.sfgtsi r31,-1
 728:	bd 50 80 00 	l\.sfgtsi r16,-32768
 72c:	bd 4f 7f ff 	l\.sfgtsi r15,32767
 730:	bd 41 00 01 	l\.sfgtsi r1,1
 734:	bd 4d b6 82 	l\.sfgtsi r13,-18814
 738:	bd 4d d6 5f 	l\.sfgtsi r13,-10657
 73c:	bd 5c 97 d5 	l\.sfgtsi r28,-26667

00000740 <l_sfgesi>:
 740:	bd 60 00 00 	l\.sfgesi r0,0
 744:	bd 7f ff ff 	l\.sfgesi r31,-1
 748:	bd 70 80 00 	l\.sfgesi r16,-32768
 74c:	bd 6f 7f ff 	l\.sfgesi r15,32767
 750:	bd 61 00 01 	l\.sfgesi r1,1
 754:	bd 6c 09 48 	l\.sfgesi r12,2376
 758:	bd 69 7d 3b 	l\.sfgesi r9,32059
 75c:	bd 6d 50 d8 	l\.sfgesi r13,20696

00000760 <l_sfltsi>:
 760:	bd 80 00 00 	l\.sfltsi r0,0
 764:	bd 9f ff ff 	l\.sfltsi r31,-1
 768:	bd 90 80 00 	l\.sfltsi r16,-32768
 76c:	bd 8f 7f ff 	l\.sfltsi r15,32767
 770:	bd 81 00 01 	l\.sfltsi r1,1
 774:	bd 9e 0b cd 	l\.sfltsi r30,3021
 778:	bd 85 93 5b 	l\.sfltsi r5,-27813
 77c:	bd 9c dd 90 	l\.sfltsi r28,-8816

00000780 <l_sflesi>:
 780:	bd a0 00 00 	l\.sflesi r0,0
 784:	bd bf ff ff 	l\.sflesi r31,-1
 788:	bd b0 80 00 	l\.sflesi r16,-32768
 78c:	bd af 7f ff 	l\.sflesi r15,32767
 790:	bd a1 00 01 	l\.sflesi r1,1
 794:	bd b2 2c 4a 	l\.sflesi r18,11338
 798:	bd bd 49 b9 	l\.sflesi r29,18873
 79c:	bd bc 65 c2 	l\.sflesi r28,26050

000007a0 <l_sfeq>:
 7a0:	e4 00 00 00 	l\.sfeq r0,r0
 7a4:	e4 1f f8 00 	l\.sfeq r31,r31
 7a8:	e4 10 80 00 	l\.sfeq r16,r16
 7ac:	e4 0f 78 00 	l\.sfeq r15,r15
 7b0:	e4 01 08 00 	l\.sfeq r1,r1
 7b4:	e4 1c d0 00 	l\.sfeq r28,r26
 7b8:	e4 0d 30 00 	l\.sfeq r13,r6
 7bc:	e4 1a 48 00 	l\.sfeq r26,r9

000007c0 <l_sfeqi>:
 7c0:	bc 00 00 00 	l\.sfeqi r0,0
 7c4:	bc 1f ff ff 	l\.sfeqi r31,-1
 7c8:	bc 10 80 00 	l\.sfeqi r16,-32768
 7cc:	bc 0f 7f ff 	l\.sfeqi r15,32767
 7d0:	bc 01 00 01 	l\.sfeqi r1,1
 7d4:	bc 0a 65 1f 	l\.sfeqi r10,25887
 7d8:	bc 15 4d b6 	l\.sfeqi r21,19894
 7dc:	bc 12 cb 95 	l\.sfeqi r18,-13419

000007e0 <l_sfne>:
 7e0:	e4 20 00 00 	l\.sfne r0,r0
 7e4:	e4 3f f8 00 	l\.sfne r31,r31
 7e8:	e4 30 80 00 	l\.sfne r16,r16
 7ec:	e4 2f 78 00 	l\.sfne r15,r15
 7f0:	e4 21 08 00 	l\.sfne r1,r1
 7f4:	e4 32 d8 00 	l\.sfne r18,r27
 7f8:	e4 26 90 00 	l\.sfne r6,r18
 7fc:	e4 20 f0 00 	l\.sfne r0,r30

00000800 <l_sfnei>:
 800:	bc 20 00 00 	l\.sfnei r0,0
 804:	bc 3f ff ff 	l\.sfnei r31,-1
 808:	bc 30 80 00 	l\.sfnei r16,-32768
 80c:	bc 2f 7f ff 	l\.sfnei r15,32767
 810:	bc 21 00 01 	l\.sfnei r1,1
 814:	bc 28 2c 92 	l\.sfnei r8,11410
 818:	bc 26 b4 d9 	l\.sfnei r6,-19239
 81c:	bc 34 a7 01 	l\.sfnei r20,-22783

00000820 <l_lo>:
 820:	9c 21 be ef 	l\.addi r1,r1,-16657

00000824 <l_hi>:
 824:	18 20 de ad 	l\.movhi r1,0xdead

00000828 <l_ha>:
 828:	18 20 de ae 	l\.movhi r1,0xdeae

0000082c <l_mac>:
 82c:	c4 01 10 01 	l.mac r1,r2

00000830 <l_maci>:
 830:	4c 01 00 00 	l\.maci r1,0
 834:	4c 02 ff ff 	l\.maci r2,-1
 838:	4c 02 7f ff 	l\.maci r2,32767
 83c:	4c 02 80 00 	l\.maci r2,-32768

00000840 <l_adrp>:
 840:	08 60 00 00 	l\.adrp r3,0 <localtext>
			840: R_OR1K_PCREL_PG21	globaldata
 844:	08 60 00 00 	l\.adrp r3,0 <localtext>
			844: R_OR1K_PCREL_PG21	\.data

00000848 <l_muld>:
 848:	e0 00 03 07 	l\.muld r0,r0
 84c:	e0 1f fb 07 	l\.muld r31,r31
 850:	e0 03 23 07 	l\.muld r3,r4

00000854 <l_muldu>:
 854:	e0 00 03 0d 	l\.muldu r0,r0
 858:	e0 1f fb 0d 	l\.muldu r31,r31
 85c:	e0 03 23 0d 	l\.muldu r3,r4

00000860 <l_macu>:
 860:	c4 00 00 03 	l\.macu r0,r0
 864:	c4 1f f8 03 	l\.macu r31,r31
 868:	c4 03 20 03 	l\.macu r3,r4

0000086c <l_msb>:
 86c:	c4 00 00 02 	l\.msb r0,r0
 870:	c4 1f f8 02 	l\.msb r31,r31
 874:	c4 03 20 02 	l\.msb r3,r4

00000878 <l_msbu>:
 878:	c4 00 00 04 	l\.msbu r0,r0
 87c:	c4 1f f8 04 	l\.msbu r31,r31
 880:	c4 03 20 04 	l\.msbu r3,r4
