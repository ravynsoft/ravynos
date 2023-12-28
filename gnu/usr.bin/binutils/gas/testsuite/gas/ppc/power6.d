#as: -a32 -mpower6
#objdump: -dr -Mpower6
#name: POWER6 tests (includes DFP and Altivec)

.*

Disassembly of section \.text:

0+00 <start>:
   0:	(4c 00 03 24|24 03 00 4c) 	doze
   4:	(4c 00 03 64|64 03 00 4c) 	nap
   8:	(4c 00 03 a4|a4 03 00 4c) 	sleep
   c:	(4c 00 03 e4|e4 03 00 4c) 	rvwinkle
  10:	(7c 83 01 34|34 01 83 7c) 	prtyw   r3,r4
  14:	(7d cd 01 74|74 01 cd 7d) 	prtyd   r13,r14
  18:	(7d 5c 02 a6|a6 02 5c 7d) 	mfcfar  r10
  1c:	(7d 7c 03 a6|a6 03 7c 7d) 	mtcfar  r11
  20:	(7c 83 2b f8|f8 2b 83 7c) 	cmpb    r3,r4,r5
  24:	(7c c0 3c be|be 3c c0 7c) 	mffgpr  f6,r7
  28:	(7d 00 4d be|be 4d 00 7d) 	mftgpr  r8,f9
  2c:	(7d 4b 66 2a|2a 66 4b 7d) 	lwzcix  r10,r11,r12
  30:	(7d 8e 7e 2e|2e 7e 8e 7d) 	lfdpx   f12,r14,r15
  34:	(ee 11 90 04|04 90 11 ee) 	dadd    f16,f17,f18
  38:	(fe 96 c0 04|04 c0 96 fe) 	daddq   f20,f22,f24
  3c:	(7c 60 06 6c|6c 06 60 7c) 	dss     3
  40:	(7e 00 06 6c|6c 06 00 7e) 	dssall
  44:	(7c 25 22 ac|ac 22 25 7c) 	dst     r5,r4,1
  48:	(7e 08 3a ac|ac 3a 08 7e) 	dstt    r8,r7,0
  4c:	(7c 65 32 ec|ec 32 65 7c) 	dstst   r5,r6,3
  50:	(7e 44 2a ec|ec 2a 44 7e) 	dststt  r4,r5,2
  54:	(00 00 02 00|00 02 00 00) 	attn
  58:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
  5c:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
  60:	(7c 68 11 20|20 11 68 7c) 	mtcrf   129,r3
  64:	(7c 70 11 20|20 11 70 7c) 	mtocrf  1,r3
  68:	(7c 70 21 20|20 21 70 7c) 	mtocrf  2,r3
  6c:	(7c 70 41 20|20 41 70 7c) 	mtocrf  4,r3
  70:	(7c 70 81 20|20 81 70 7c) 	mtocrf  8,r3
  74:	(7c 71 01 20|20 01 71 7c) 	mtocrf  16,r3
  78:	(7c 72 01 20|20 01 72 7c) 	mtocrf  32,r3
  7c:	(7c 74 01 20|20 01 74 7c) 	mtocrf  64,r3
  80:	(7c 78 01 20|20 01 78 7c) 	mtocrf  128,r3
  84:	(7c 60 00 26|26 00 60 7c) 	mfcr    r3
  88:	(7c 70 10 26|26 10 70 7c) 	mfocrf  r3,1
  8c:	(7c 70 20 26|26 20 70 7c) 	mfocrf  r3,2
  90:	(7c 70 40 26|26 40 70 7c) 	mfocrf  r3,4
  94:	(7c 70 80 26|26 80 70 7c) 	mfocrf  r3,8
  98:	(7c 71 00 26|26 00 71 7c) 	mfocrf  r3,16
  9c:	(7c 72 00 26|26 00 72 7c) 	mfocrf  r3,32
  a0:	(7c 74 00 26|26 00 74 7c) 	mfocrf  r3,64
  a4:	(7c 78 00 26|26 00 78 7c) 	mfocrf  r3,128
  a8:	(7c 01 17 ec|ec 17 01 7c) 	dcbz    r1,r2
  ac:	(7c 23 27 ec|ec 27 23 7c) 	dcbzl   r3,r4
  b0:	(7c 05 37 ec|ec 37 05 7c) 	dcbz    r5,r6
  b4:	(fc 0c 55 8e|8e 55 0c fc) 	mtfsf   6,f10
  b8:	(fc 0c 5d 8f|8f 5d 0c fc) 	mtfsf.  6,f11
  bc:	(fc 0c 55 8e|8e 55 0c fc) 	mtfsf   6,f10
  c0:	(fc 0c 5d 8f|8f 5d 0c fc) 	mtfsf.  6,f11
  c4:	(fc 0d 55 8e|8e 55 0d fc) 	mtfsf   6,f10,0,1
  c8:	(fc 0d 5d 8f|8f 5d 0d fc) 	mtfsf.  6,f11,0,1
  cc:	(fe 0c 55 8e|8e 55 0c fe) 	mtfsf   6,f10,1
  d0:	(fe 0c 5d 8f|8f 5d 0c fe) 	mtfsf.  6,f11,1
  d4:	(ff 00 01 0c|0c 01 00 ff) 	mtfsfi  6,0
  d8:	(ff 00 f1 0d|0d f1 00 ff) 	mtfsfi. 6,15
  dc:	(ff 00 01 0c|0c 01 00 ff) 	mtfsfi  6,0
  e0:	(ff 00 f1 0d|0d f1 00 ff) 	mtfsfi. 6,15
  e4:	(ff 01 01 0c|0c 01 01 ff) 	mtfsfi  6,0,1
  e8:	(ff 01 f1 0d|0d f1 01 ff) 	mtfsfi. 6,15,1
  ec:	(7d 6a 02 74|74 02 6a 7d) 	cbcdtd  r10,r11
  f0:	(7d 6a 02 34|34 02 6a 7d) 	cdtbcd  r10,r11
  f4:	(7d 4b 60 94|94 60 4b 7d) 	addg6s  r10,r11,r12
  f8:	(60 21 00 00|00 00 21 60) 	ori     r1,r1,0
  fc:	(60 21 00 00|00 00 21 60) 	ori     r1,r1,0
.*:	(7c 00 03 e4|e4 03 00 7c) 	slbia
.*:	(7c 00 03 e4|e4 03 00 7c) 	slbia
.*:	(7c e0 03 e4|e4 03 e0 7c) 	slbia   7
.*:	(7c 00 52 64|64 52 00 7c) 	tlbie   r10
.*:	(7c 00 52 64|64 52 00 7c) 	tlbie   r10
.*:	(7c 20 52 64|64 52 20 7c) 	tlbie   r10,1
#pass
