#as: -mppc -me500
#objdump: -dr -Me500
#name: e500 tests

.*

Disassembly of section \.text:

0+0000000 <start>:
   0:	(7c 43 25 de|de 25 43 7c) 	isel    r2,r3,r4,4\*cr5\+so
   4:	(7c 85 33 0c|0c 33 85 7c) 	dcblc   4,r5,r6
   8:	(7c e8 49 4c|4c 49 e8 7c) 	dcbtls  7,r8,r9
   c:	(7d 4b 61 0c|0c 61 4b 7d) 	dcbtstls 10,r11,r12
  10:	(7d ae 7b cc|cc 7b ae 7d) 	icbtls  13,r14,r15
  14:	(7e 11 91 cc|cc 91 11 7e) 	icblc   16,r17,r18
  18:	(7c 89 33 9c|9c 33 89 7c) 	mtpmr   201,r4
  1c:	(7c ab 32 9c|9c 32 ab 7c) 	mfpmr   r5,203
  20:	(7c 00 04 0c|0c 04 00 7c) 	bblels
  24:	(7c 00 04 4c|4c 04 00 7c) 	bbelr
  28:	(7d 00 83 a6|a6 83 00 7d) 	mtspefscr r8
  2c:	(7d 20 82 a6|a6 82 20 7d) 	mfspefscr r9
  30:	(10 a0 22 cf|cf 22 a0 10) 	efscfd  r5,r4
  34:	(10 a4 02 e4|e4 02 a4 10) 	efdabs  r5,r4
  38:	(10 a4 02 e5|e5 02 a4 10) 	efdnabs r5,r4
  3c:	(10 a4 02 e6|e6 02 a4 10) 	efdneg  r5,r4
  40:	(10 a4 1a e0|e0 1a a4 10) 	efdadd  r5,r4,r3
  44:	(10 a4 1a e1|e1 1a a4 10) 	efdsub  r5,r4,r3
  48:	(10 a4 1a e8|e8 1a a4 10) 	efdmul  r5,r4,r3
  4c:	(10 a4 1a e9|e9 1a a4 10) 	efddiv  r5,r4,r3
  50:	(12 84 1a ec|ec 1a 84 12) 	efdcmpgt cr5,r4,r3
  54:	(12 84 1a ed|ed 1a 84 12) 	efdcmplt cr5,r4,r3
  58:	(12 84 1a ee|ee 1a 84 12) 	efdcmpeq cr5,r4,r3
  5c:	(12 84 1a fc|fc 1a 84 12) 	efdtstgt cr5,r4,r3
  60:	(12 84 1a fc|fc 1a 84 12) 	efdtstgt cr5,r4,r3
  64:	(12 84 1a fd|fd 1a 84 12) 	efdtstlt cr5,r4,r3
  68:	(12 84 1a fe|fe 1a 84 12) 	efdtsteq cr5,r4,r3
  6c:	(10 a0 22 f1|f1 22 a0 10) 	efdcfsi r5,r4
  70:	(10 a0 22 e3|e3 22 a0 10) 	efdcfsid r5,r4
  74:	(10 a0 22 f0|f0 22 a0 10) 	efdcfui r5,r4
  78:	(10 a0 22 e2|e2 22 a0 10) 	efdcfuid r5,r4
  7c:	(10 a0 22 f3|f3 22 a0 10) 	efdcfsf r5,r4
  80:	(10 a0 22 f2|f2 22 a0 10) 	efdcfuf r5,r4
  84:	(10 a0 22 f5|f5 22 a0 10) 	efdctsi r5,r4
  88:	(10 a0 22 eb|eb 22 a0 10) 	efdctsidz r5,r4
  8c:	(10 a0 22 fa|fa 22 a0 10) 	efdctsiz r5,r4
  90:	(10 a0 22 f4|f4 22 a0 10) 	efdctui r5,r4
  94:	(10 a0 22 ea|ea 22 a0 10) 	efdctuidz r5,r4
  98:	(10 a0 22 f8|f8 22 a0 10) 	efdctuiz r5,r4
  9c:	(10 a0 22 f7|f7 22 a0 10) 	efdctsf r5,r4
  a0:	(10 a0 22 f6|f6 22 a0 10) 	efdctuf r5,r4
  a4:	(10 a0 22 ef|ef 22 a0 10) 	efdcfs  r5,r4
  a8:	(7c 20 06 ac|ac 06 20 7c) 	mbar    1
  ac:	(7c 00 06 ac|ac 06 00 7c) 	mbar
  b0:	(7c 20 06 ac|ac 06 20 7c) 	mbar    1
  b4:	(7c 00 04 ac|ac 04 00 7c) 	msync
  b8:	(7c 00 04 ac|ac 04 00 7c) 	msync
#pass
