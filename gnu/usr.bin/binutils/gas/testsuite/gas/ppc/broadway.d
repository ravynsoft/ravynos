#as: -mbroadway
#objdump: -dr -Mbroadway
#name: PPC Broadway instructions

.*

Disassembly of section \.text:

0+0000000 <start>:
   0:	(7c 12 fa a6|a6 fa 12 7c) 	mfiabr  r0
   4:	(7c 32 fb a6|a6 fb 32 7c) 	mtiabr  r1
   8:	(7c 55 fa a6|a6 fa 55 7c) 	mfdabr  r2
   c:	(7c 75 fb a6|a6 fb 75 7c) 	mtdabr  r3
  10:	(7c 90 e2 a6|a6 e2 90 7c) 	mfgqr   r4,0
  14:	(7c b1 e2 a6|a6 e2 b1 7c) 	mfgqr   r5,1
  18:	(7c d2 e2 a6|a6 e2 d2 7c) 	mfgqr   r6,2
  1c:	(7c f3 e2 a6|a6 e2 f3 7c) 	mfgqr   r7,3
  20:	(7d 14 e2 a6|a6 e2 14 7d) 	mfgqr   r8,4
  24:	(7d 35 e2 a6|a6 e2 35 7d) 	mfgqr   r9,5
  28:	(7d 56 e2 a6|a6 e2 56 7d) 	mfgqr   r10,6
  2c:	(7d 77 e2 a6|a6 e2 77 7d) 	mfgqr   r11,7
  30:	(7c 90 e3 a6|a6 e3 90 7c) 	mtgqr   0,r4
  34:	(7c b1 e3 a6|a6 e3 b1 7c) 	mtgqr   1,r5
  38:	(7c d2 e3 a6|a6 e3 d2 7c) 	mtgqr   2,r6
  3c:	(7c f3 e3 a6|a6 e3 f3 7c) 	mtgqr   3,r7
  40:	(7d 14 e3 a6|a6 e3 14 7d) 	mtgqr   4,r8
  44:	(7d 35 e3 a6|a6 e3 35 7d) 	mtgqr   5,r9
  48:	(7d 56 e3 a6|a6 e3 56 7d) 	mtgqr   6,r10
  4c:	(7d 77 e3 a6|a6 e3 77 7d) 	mtgqr   7,r11
  50:	(7d 99 e2 a6|a6 e2 99 7d) 	mfwpar  r12
  54:	(7d b9 e3 a6|a6 e3 b9 7d) 	mtwpar  r13
  58:	(7d db e2 a6|a6 e2 db 7d) 	mfdmal  r14
  5c:	(7d fb e3 a6|a6 e3 fb 7d) 	mtdmal  r15
  60:	(7e 1a e2 a6|a6 e2 1a 7e) 	mfdmau  r16
  64:	(7e 3a e3 a6|a6 e3 3a 7e) 	mtdmau  r17
  68:	(7e 50 fa a6|a6 fa 50 7e) 	mfhid0  r18
  6c:	(7e 70 fb a6|a6 fb 70 7e) 	mthid0  r19
  70:	(7e 91 fa a6|a6 fa 91 7e) 	mfhid1  r20
  74:	(7e b1 fb a6|a6 fb b1 7e) 	mthid1  r21
  78:	(7e d8 e2 a6|a6 e2 d8 7e) 	mfhid2  r22
  7c:	(7e f8 e3 a6|a6 e3 f8 7e) 	mthid2  r23
  80:	(7f 13 fa a6|a6 fa 13 7f) 	mfhid4  r24
  84:	(7f 33 fb a6|a6 fb 33 7f) 	mthid4  r25
  88:	(7c 10 82 a6|a6 82 10 7c) 	mfibatu r0,0
  8c:	(7c 30 83 a6|a6 83 30 7c) 	mtibatu 0,r1
  90:	(7c 52 82 a6|a6 82 52 7c) 	mfibatu r2,1
  94:	(7c 72 83 a6|a6 83 72 7c) 	mtibatu 1,r3
  98:	(7c 94 82 a6|a6 82 94 7c) 	mfibatu r4,2
  9c:	(7c b4 83 a6|a6 83 b4 7c) 	mtibatu 2,r5
  a0:	(7c d6 82 a6|a6 82 d6 7c) 	mfibatu r6,3
  a4:	(7c f6 83 a6|a6 83 f6 7c) 	mtibatu 3,r7
  a8:	(7d 10 8a a6|a6 8a 10 7d) 	mfibatu r8,4
  ac:	(7d 30 8b a6|a6 8b 30 7d) 	mtibatu 4,r9
  b0:	(7d 52 8a a6|a6 8a 52 7d) 	mfibatu r10,5
  b4:	(7d 72 8b a6|a6 8b 72 7d) 	mtibatu 5,r11
  b8:	(7d 94 8a a6|a6 8a 94 7d) 	mfibatu r12,6
  bc:	(7d b4 8b a6|a6 8b b4 7d) 	mtibatu 6,r13
  c0:	(7d d6 8a a6|a6 8a d6 7d) 	mfibatu r14,7
  c4:	(7d f6 8b a6|a6 8b f6 7d) 	mtibatu 7,r15
  c8:	(7e 11 82 a6|a6 82 11 7e) 	mfibatl r16,0
  cc:	(7e 31 83 a6|a6 83 31 7e) 	mtibatl 0,r17
  d0:	(7e 53 82 a6|a6 82 53 7e) 	mfibatl r18,1
  d4:	(7e 73 83 a6|a6 83 73 7e) 	mtibatl 1,r19
  d8:	(7e 95 82 a6|a6 82 95 7e) 	mfibatl r20,2
  dc:	(7e b5 83 a6|a6 83 b5 7e) 	mtibatl 2,r21
  e0:	(7e d7 82 a6|a6 82 d7 7e) 	mfibatl r22,3
  e4:	(7e f7 83 a6|a6 83 f7 7e) 	mtibatl 3,r23
  e8:	(7f 11 8a a6|a6 8a 11 7f) 	mfibatl r24,4
  ec:	(7f 31 8b a6|a6 8b 31 7f) 	mtibatl 4,r25
  f0:	(7f 53 8a a6|a6 8a 53 7f) 	mfibatl r26,5
  f4:	(7f 73 8b a6|a6 8b 73 7f) 	mtibatl 5,r27
  f8:	(7f 95 8a a6|a6 8a 95 7f) 	mfibatl r28,6
  fc:	(7f b5 8b a6|a6 8b b5 7f) 	mtibatl 6,r29
 100:	(7f d7 8a a6|a6 8a d7 7f) 	mfibatl r30,7
 104:	(7f f7 8b a6|a6 8b f7 7f) 	mtibatl 7,r31
 108:	(7c 18 82 a6|a6 82 18 7c) 	mfdbatu r0,0
 10c:	(7c 38 83 a6|a6 83 38 7c) 	mtdbatu 0,r1
 110:	(7c 5a 82 a6|a6 82 5a 7c) 	mfdbatu r2,1
 114:	(7c 7a 83 a6|a6 83 7a 7c) 	mtdbatu 1,r3
 118:	(7c 9c 82 a6|a6 82 9c 7c) 	mfdbatu r4,2
 11c:	(7c bc 83 a6|a6 83 bc 7c) 	mtdbatu 2,r5
 120:	(7c de 82 a6|a6 82 de 7c) 	mfdbatu r6,3
 124:	(7c fe 83 a6|a6 83 fe 7c) 	mtdbatu 3,r7
 128:	(7d 18 8a a6|a6 8a 18 7d) 	mfdbatu r8,4
 12c:	(7d 38 8b a6|a6 8b 38 7d) 	mtdbatu 4,r9
 130:	(7d 5a 8a a6|a6 8a 5a 7d) 	mfdbatu r10,5
 134:	(7d 7a 8b a6|a6 8b 7a 7d) 	mtdbatu 5,r11
 138:	(7d 9c 8a a6|a6 8a 9c 7d) 	mfdbatu r12,6
 13c:	(7d bc 8b a6|a6 8b bc 7d) 	mtdbatu 6,r13
 140:	(7d de 8a a6|a6 8a de 7d) 	mfdbatu r14,7
 144:	(7d fe 8b a6|a6 8b fe 7d) 	mtdbatu 7,r15
 148:	(7e 19 82 a6|a6 82 19 7e) 	mfdbatl r16,0
 14c:	(7e 39 83 a6|a6 83 39 7e) 	mtdbatl 0,r17
 150:	(7e 5b 82 a6|a6 82 5b 7e) 	mfdbatl r18,1
 154:	(7e 7b 83 a6|a6 83 7b 7e) 	mtdbatl 1,r19
 158:	(7e 9d 82 a6|a6 82 9d 7e) 	mfdbatl r20,2
 15c:	(7e bd 83 a6|a6 83 bd 7e) 	mtdbatl 2,r21
 160:	(7e df 82 a6|a6 82 df 7e) 	mfdbatl r22,3
 164:	(7e ff 83 a6|a6 83 ff 7e) 	mtdbatl 3,r23
 168:	(7f 19 8a a6|a6 8a 19 7f) 	mfdbatl r24,4
 16c:	(7f 39 8b a6|a6 8b 39 7f) 	mtdbatl 4,r25
 170:	(7f 5b 8a a6|a6 8a 5b 7f) 	mfdbatl r26,5
 174:	(7f 7b 8b a6|a6 8b 7b 7f) 	mtdbatl 5,r27
 178:	(7f 9d 8a a6|a6 8a 9d 7f) 	mfdbatl r28,6
 17c:	(7f bd 8b a6|a6 8b bd 7f) 	mtdbatl 6,r29
 180:	(7f df 8a a6|a6 8a df 7f) 	mfdbatl r30,7
 184:	(7f ff 8b a6|a6 8b ff 7f) 	mtdbatl 7,r31
#pass
