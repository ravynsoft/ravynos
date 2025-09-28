#as: -mpower10
#objdump: -d -Mpower10 -Mraw
#name: raw disassembly

.*:     file format .*

Disassembly of section \.text:

0+ <\.text>:
   0:	(f0 64 24 90|90 24 64 f0) 	xxlor   vs3,vs4,vs4
   4:	(f0 64 25 10|10 25 64 f0) 	xxlnor  vs3,vs4,vs4
   8:	(f0 64 26 80|80 26 64 f0) 	xvcpsgnsp vs3,vs4,vs4
   c:	(10 64 24 84|84 24 64 10) 	vor     v3,v4,v4
  10:	(7c 83 23 78|78 23 83 7c) 	or      r3,r4,r4
  14:	(7c 83 20 f8|f8 20 83 7c) 	nor     r3,r4,r4
  18:	(4c a6 30 42|42 30 a6 4c) 	crnor   4\*cr1\+gt,4\*cr1\+eq,4\*cr1\+eq
  1c:	(4c e7 39 82|82 39 e7 4c) 	crxor   4\*cr1\+so,4\*cr1\+so,4\*cr1\+so
  20:	(4c 00 02 42|42 02 00 4c) 	creqv   lt,lt,lt
  24:	(4c 22 13 82|82 13 22 4c) 	cror    gt,eq,eq
  28:	(f0 64 20 50|50 20 64 f0) 	xxpermdi vs3,vs4,vs4,0
  2c:	(f0 64 23 50|50 23 64 f0) 	xxpermdi vs3,vs4,vs4,3
  30:	(f0 64 28 50|50 28 64 f0) 	xxpermdi vs3,vs4,vs5,0
  34:	(f0 64 2b 50|50 2b 64 f0) 	xxpermdi vs3,vs4,vs5,3
  38:	(f0 64 22 50|50 22 64 f0) 	xxpermdi vs3,vs4,vs4,2
  3c:	(10 60 23 ca|ca 23 60 10) 	vctsxs  v3,v4,0
  40:	(38 60 00 7b|7b 00 60 38) 	addi    r3,0,123
  44:	(3c 80 01 c8|c8 01 80 3c) 	addis   r4,0,456
  48:	(43 20 ff f8|f8 ff 20 43) 	bc      25,lt,0x40
  4c:	(41 80 00 04|04 00 80 41) 	bc      12,lt,0x50
  50:	(4e 80 00 20|20 00 80 4e) 	bclr    20,lt,0
  54:	(4c c0 04 20|20 04 c0 4c) 	bcctr   6,lt,0
  58:	(4c 83 04 61|61 04 83 4c) 	bctarl  4,so,0
  5c:	(4c 60 00 04|04 00 60 4c) 	addpcis r3,0
  60:	(28 03 04 d2|d2 04 03 28) 	cmpli   cr0,0,r3,1234
  64:	(28 23 04 d2|d2 04 23 28) 	cmpli   cr0,1,r3,1234
  68:	(7c 03 20 00|00 20 03 7c) 	cmp     cr0,0,r3,r4
  6c:	(7c 23 20 00|00 20 23 7c) 	cmp     cr0,1,r3,r4
  70:	(7c 03 20 40|40 20 03 7c) 	cmpl    cr0,0,r3,r4
  74:	(7c 23 20 40|40 20 23 7c) 	cmpl    cr0,1,r3,r4
  78:	(30 64 ff d6|d6 ff 64 30) 	addic   r3,r4,-42
  7c:	(54 83 80 3e|3e 80 83 54) 	rlwinm  r3,r4,16,0,31
  80:	(78 83 06 a0|a0 06 83 78) 	rldicl  r3,r4,0,58
  84:	(60 00 00 00|00 00 00 60) 	ori     r0,r0,0
  88:	(68 00 00 00|00 00 00 68) 	xori    r0,r0,0
  8c:	(7e 03 20 08|08 20 03 7e) 	tw      16,r3,r4
  90:	(7c 65 20 50|50 20 65 7c) 	subf    r3,r5,r4
  94:	(7c 65 20 11|11 20 65 7c) 	subfc\.  r3,r5,r4
  98:	(7c 83 00 66|66 00 83 7c) 	mfvsrd  r3,vs4
  9c:	(7c 83 00 67|67 00 83 7c) 	mfvsrd  r3,vs36
  a0:	(7c 6f f1 20|20 f1 6f 7c) 	mtcrf   255,r3
  a4:	(7e 03 21 ec|ec 21 03 7e) 	dcbtst  r3,r4,16
  a8:	(7c e3 21 ec|ec 21 e3 7c) 	dcbtst  r3,r4,7
  ac:	(7d 03 21 ec|ec 21 03 7d) 	dcbtst  r3,r4,8
  b0:	(7e 23 22 2c|2c 22 23 7e) 	dcbt    r3,r4,17
  b4:	(7c 68 02 a6|a6 02 68 7c) 	mfspr   r3,8
  b8:	(7c 69 02 a6|a6 02 69 7c) 	mfspr   r3,9
  bc:	(7c 70 43 a6|a6 43 70 7c) 	mtspr   272,r3
  c0:	(7f 7b db 78|78 db 7b 7f) 	or      r27,r27,r27
  c4:	(7f de f3 78|78 f3 de 7f) 	or      r30,r30,r30
  c8:	(7c 20 04 ac|ac 04 20 7c) 	sync    1,0
  cc:	(06 00 00 00|00 00 00 06) 	paddi   r3,0,0,0
  d0:	(38 60 00 00|00 00 60 38) 
  d4:	(06 10 00 00|00 00 10 06) 	paddi   r3,0,0,1	# d4
  d8:	(38 60 00 00|00 00 60 38) 
