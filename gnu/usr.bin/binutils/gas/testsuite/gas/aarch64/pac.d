#objdump: -dr
#as: -march=armv8.3-a

.*:     file .*

Disassembly of section \.text:

0+ <.*>:
   0:	dac10083 	pacia	x3, x4
   4:	dac103e5 	pacia	x5, sp
   8:	dac10483 	pacib	x3, x4
   c:	dac107e5 	pacib	x5, sp
  10:	dac10883 	pacda	x3, x4
  14:	dac10be5 	pacda	x5, sp
  18:	dac10c83 	pacdb	x3, x4
  1c:	dac10fe5 	pacdb	x5, sp
  20:	dac11083 	autia	x3, x4
  24:	dac113e5 	autia	x5, sp
  28:	dac11483 	autib	x3, x4
  2c:	dac117e5 	autib	x5, sp
  30:	dac11883 	autda	x3, x4
  34:	dac11be5 	autda	x5, sp
  38:	dac11c83 	autdb	x3, x4
  3c:	dac11fe5 	autdb	x5, sp
  40:	dac123e5 	paciza	x5
  44:	dac127e5 	pacizb	x5
  48:	dac12be5 	pacdza	x5
  4c:	dac12fe5 	pacdzb	x5
  50:	dac133e5 	autiza	x5
  54:	dac137e5 	autizb	x5
  58:	dac13be5 	autdza	x5
  5c:	dac13fe5 	autdzb	x5
  60:	dac143e5 	xpaci	x5
  64:	dac147e5 	xpacd	x5
  68:	9ac33041 	pacga	x1, x2, x3
  6c:	9adf3041 	pacga	x1, x2, sp
  70:	d71f0822 	braa	x1, x2
  74:	d71f087f 	braa	x3, sp
  78:	d71f0c22 	brab	x1, x2
  7c:	d71f0c7f 	brab	x3, sp
  80:	d73f0822 	blraa	x1, x2
  84:	d73f087f 	blraa	x3, sp
  88:	d73f0c22 	blrab	x1, x2
  8c:	d73f0c7f 	blrab	x3, sp
  90:	d61f08bf 	braaz	x5
  94:	d61f0cbf 	brabz	x5
  98:	d63f08bf 	blraaz	x5
  9c:	d63f0cbf 	blrabz	x5
  a0:	d65f0bff 	retaa
  a4:	d65f0fff 	retab
  a8:	d69f0bff 	eretaa
  ac:	d69f0fff 	eretab
  b0:	f8200441 	ldraa	x1, \[x2\]
  b4:	f8200441 	ldraa	x1, \[x2\]
  b8:	f87ff483 	ldraa	x3, \[x4, #-8\]
  bc:	f82014c5 	ldraa	x5, \[x6, #8\]
  c0:	f83ff507 	ldraa	x7, \[x8, #4088\]
  c4:	f8600528 	ldraa	x8, \[x9, #-4096\]
  c8:	f82007e2 	ldraa	x2, \[sp\]
  cc:	f87067e4 	ldraa	x4, \[sp, #-2000\]
  d0:	f8a00441 	ldrab	x1, \[x2\]
  d4:	f8a00441 	ldrab	x1, \[x2\]
  d8:	f8fff483 	ldrab	x3, \[x4, #-8\]
  dc:	f8a014c5 	ldrab	x5, \[x6, #8\]
  e0:	f8bff507 	ldrab	x7, \[x8, #4088\]
  e4:	f8e00528 	ldrab	x8, \[x9, #-4096\]
  e8:	f8a007e2 	ldrab	x2, \[sp\]
  ec:	f8f067e4 	ldrab	x4, \[sp, #-2000\]
  f0:	f8201c62 	ldraa	x2, \[x3, #8\]!
  f4:	f87ffca4 	ldraa	x4, \[x5, #-8\]!
  f8:	f83fffe6 	ldraa	x6, \[sp, #4088\]!
  fc:	f8a01c62 	ldrab	x2, \[x3, #8\]!
 100:	f8fffca4 	ldrab	x4, \[x5, #-8\]!
 104:	f8bfffe6 	ldrab	x6, \[sp, #4088\]!
