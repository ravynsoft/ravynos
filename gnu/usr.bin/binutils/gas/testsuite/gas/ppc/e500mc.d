#as: -mppc -me500mc
#objdump: -dr -Me500mc
#name: Power E500MC tests

.*

Disassembly of section \.text:

0+00 <start>:
   0:	(4c 00 00 4e|4e 00 00 4c) 	rfdi
   4:	(4c 00 00 cc|cc 00 00 4c) 	rfgi
   8:	(4c 1f f9 8c|8c f9 1f 4c) 	dnh     0,1023
   c:	(4f e0 01 8c|8c 01 e0 4f) 	dnh     31,0
  10:	(7c 09 57 be|be 57 09 7c) 	icbiep  r9,r10
  14:	(7c 00 69 dc|dc 69 00 7c) 	msgclr  r13
  18:	(7c 00 71 9c|9c 71 00 7c) 	msgsnd  r14
  1c:	(7c 00 00 7c|7c 00 00 7c) 	wait
  20:	(7c 00 00 7c|7c 00 00 7c) 	wait
  24:	(7c 20 00 7c|7c 00 20 7c) 	waitrsv
  28:	(7c 20 00 7c|7c 00 20 7c) 	waitrsv
  2c:	(7c 40 00 7c|7c 00 40 7c) 	waitimpl
  30:	(7c 40 00 7c|7c 00 40 7c) 	waitimpl
  34:	(7f 9c e3 78|78 e3 9c 7f) 	mdors
  38:	(7c 00 02 1c|1c 02 00 7c) 	ehpriv
  3c:	(7c 18 cb c6|c6 cb 18 7c) 	dsn     r24,r25
  40:	(7c 22 18 be|be 18 22 7c) 	lbepx   r1,r2,r3
  44:	(7c 85 32 3e|3e 32 85 7c) 	lhepx   r4,r5,r6
  48:	(7c e8 48 3e|3e 48 e8 7c) 	lwepx   r7,r8,r9
  4c:	(7d 4b 60 3a|3a 60 4b 7d) 	ldepx   r10,r11,r12
  50:	(7d ae 7c be|be 7c ae 7d) 	lfdepx  f13,r14,r15
  54:	(7e 11 91 be|be 91 11 7e) 	stbepx  r16,r17,r18
  58:	(7e 74 ab 3e|3e ab 74 7e) 	sthepx  r19,r20,r21
  5c:	(7e d7 c1 3e|3e c1 d7 7e) 	stwepx  r22,r23,r24
  60:	(7f 3a d9 3a|3a d9 3a 7f) 	stdepx  r25,r26,r27
  64:	(7f 9d f5 be|be f5 9d 7f) 	stfdepx f28,r29,r30
  68:	(7c 01 14 06|06 14 01 7c) 	lbdx    r0,r1,r2
  6c:	(7d 8d 74 46|46 74 8d 7d) 	lhdx    r12,r13,r14
  70:	(7c 64 2c 86|86 2c 64 7c) 	lwdx    r3,r4,r5
  74:	(7f 5b e6 46|46 e6 5b 7f) 	lfddx   f26,r27,r28
  78:	(7d f0 8c c6|c6 8c f0 7d) 	lddx    r15,r16,r17
  7c:	(7c c7 45 06|06 45 c7 7c) 	stbdx   r6,r7,r8
  80:	(7e 53 a5 46|46 a5 53 7e) 	sthdx   r18,r19,r20
  84:	(7d 2a 5d 86|86 5d 2a 7d) 	stwdx   r9,r10,r11
  88:	(7f be ff 46|46 ff be 7f) 	stfddx  f29,r30,r31
  8c:	(7e b6 bd c6|c6 bd b6 7e) 	stddx   r21,r22,r23
  90:	(7c 20 0d ec|ec 0d 20 7c) 	dcbal   0,r1
  94:	(7c 26 3f ec|ec 3f 26 7c) 	dcbzl   r6,r7
  98:	(7c 1f 00 7e|7e 00 1f 7c) 	dcbstep r31,r0
  9c:	(7c 01 10 fe|fe 10 01 7c) 	dcbfep  r1,r2
  a0:	(7c 64 29 fe|fe 29 64 7c) 	dcbtstep r3,r4,r5
  a4:	(7c c7 42 7e|7e 42 c7 7c) 	dcbtep  r6,r7,r8
  a8:	(7c 0b 67 fe|fe 67 0b 7c) 	dcbzep  r11,r12
  ac:	(7c 00 00 24|24 00 00 7c) 	tlbilxlpid
  b0:	(7c 20 00 24|24 00 20 7c) 	tlbilxpid
  b4:	(7c 62 18 24|24 18 62 7c) 	tlbilxva r2,r3
  b8:	(7c 64 28 24|24 28 64 7c) 	tlbilxva r4,r5
#pass
