#as: -mcell
#objdump: -dr -Mcell
#name: Cell tests (includes Altivec)


.*


Disassembly of section \.text:

0+00 <.text>:
   0:	(7c 01 14 0e|0e 14 01 7c) 	lvlx    v0,r1,r2
   4:	(7c 00 14 0e|0e 14 00 7c) 	lvlx    v0,0,r2
   8:	(7c 01 16 0e|0e 16 01 7c) 	lvlxl   v0,r1,r2
   c:	(7c 00 16 0e|0e 16 00 7c) 	lvlxl   v0,0,r2
  10:	(7c 01 14 4e|4e 14 01 7c) 	lvrx    v0,r1,r2
  14:	(7c 00 14 4e|4e 14 00 7c) 	lvrx    v0,0,r2
  18:	(7c 01 16 4e|4e 16 01 7c) 	lvrxl   v0,r1,r2
  1c:	(7c 00 16 4e|4e 16 00 7c) 	lvrxl   v0,0,r2
  20:	(7c 01 15 0e|0e 15 01 7c) 	stvlx   v0,r1,r2
  24:	(7c 00 15 0e|0e 15 00 7c) 	stvlx   v0,0,r2
  28:	(7c 01 17 0e|0e 17 01 7c) 	stvlxl  v0,r1,r2
  2c:	(7c 00 17 0e|0e 17 00 7c) 	stvlxl  v0,0,r2
  30:	(7c 01 15 4e|4e 15 01 7c) 	stvrx   v0,r1,r2
  34:	(7c 00 15 4e|4e 15 00 7c) 	stvrx   v0,0,r2
  38:	(7c 01 17 4e|4e 17 01 7c) 	stvrxl  v0,r1,r2
  3c:	(7c 00 17 4e|4e 17 00 7c) 	stvrxl  v0,0,r2
  40:	(7c 00 0c 28|28 0c 00 7c) 	ldbrx   r0,0,r1
  44:	(7c 01 14 28|28 14 01 7c) 	ldbrx   r0,r1,r2
  48:	(7c 00 0d 28|28 0d 00 7c) 	stdbrx  r0,0,r1
  4c:	(7c 01 15 28|28 15 01 7c) 	stdbrx  r0,r1,r2
  50:	(7c 60 06 6c|6c 06 60 7c) 	dss     3
  54:	(7e 00 06 6c|6c 06 00 7e) 	dssall
  58:	(7c 25 22 ac|ac 22 25 7c) 	dst     r5,r4,1
  5c:	(7e 08 3a ac|ac 3a 08 7e) 	dstt    r8,r7,0
  60:	(7c 65 32 ec|ec 32 65 7c) 	dstst   r5,r6,3
  64:	(7e 44 2a ec|ec 2a 44 7e) 	dststt  r4,r5,2
