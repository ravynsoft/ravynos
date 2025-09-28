#as: -mbooke
#objdump: -dr -Mbooke
#name: BookE tests

.*

Disassembly of section \.text:

0+0000000 <branch_target_1>:
   0:	(7c a8 48 2c|2c 48 a8 7c) 	icbt    5,r8,r9
   4:	(7c a6 02 26|26 02 a6 7c) 	mfapidi r5,r6
   8:	(7c 07 46 24|24 46 07 7c) 	tlbivax r7,r8
   c:	(7c 0b 67 24|24 67 0b 7c) 	tlbsx   r11,r12
  10:	(7c 00 07 a4|a4 07 00 7c) 	tlbwe
  14:	(7c 00 07 a4|a4 07 00 7c) 	tlbwe
  18:	(7c 21 0f a4|a4 0f 21 7c) 	tlbwe   r1,r1,1

0+000001c <branch_target_2>:
  1c:	(4c 00 00 66|66 00 00 4c) 	rfci
  20:	(7c 60 01 06|06 01 60 7c) 	wrtee   r3
  24:	(7c 00 81 46|46 81 00 7c) 	wrteei  1
  28:	(7c 85 02 06|06 02 85 7c) 	mfdcrx  r4,r5
  2c:	(7c aa 3a 86|86 3a aa 7c) 	mfdcr   r5,234
  30:	(7c e6 03 06|06 03 e6 7c) 	mtdcrx  r6,r7
  34:	(7d 10 6b 86|86 6b 10 7d) 	mtdcr   432,r8
  38:	(7c 00 04 ac|ac 04 00 7c) 	msync
  3c:	(7c 09 55 ec|ec 55 09 7c) 	dcba    r9,r10
  40:	(7c 00 06 ac|ac 06 00 7c) 	mbar
  44:	(7c 00 06 ac|ac 06 00 7c) 	mbar
  48:	(7c 20 06 ac|ac 06 20 7c) 	mbar    1
  4c:	(7d 8d 77 24|24 77 8d 7d) 	tlbsx   r12,r13,r14
  50:	(7d 8d 77 25|25 77 8d 7d) 	tlbsx\.  r12,r13,r14
  54:	(7c 12 42 a6|a6 42 12 7c) 	mfsprg  r0,2
  58:	(7c 12 42 a6|a6 42 12 7c) 	mfsprg  r0,2
  5c:	(7c 12 43 a6|a6 43 12 7c) 	mtsprg  2,r0
  60:	(7c 12 43 a6|a6 43 12 7c) 	mtsprg  2,r0
  64:	(7c 07 42 a6|a6 42 07 7c) 	mfsprg  r0,7
  68:	(7c 07 42 a6|a6 42 07 7c) 	mfsprg  r0,7
  6c:	(7c 17 43 a6|a6 43 17 7c) 	mtsprg  7,r0
  70:	(7c 17 43 a6|a6 43 17 7c) 	mtsprg  7,r0
  74:	(7c 05 32 2c|2c 32 05 7c) 	dcbt    r5,r6
