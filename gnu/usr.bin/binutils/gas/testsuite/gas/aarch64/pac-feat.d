#name: PAUTH (Pointer authentication) feature
#objdump: -dr

.*:     file .*

Disassembly of section \.text:

0+ <.*>:
.*:	dac10083 	pacia	x3, x4
.*:	dac103e5 	pacia	x5, sp
.*:	dac10483 	pacib	x3, x4
.*:	dac107e5 	pacib	x5, sp
.*:	dac10883 	pacda	x3, x4
.*:	dac10be5 	pacda	x5, sp
.*:	dac10c83 	pacdb	x3, x4
.*:	dac10fe5 	pacdb	x5, sp
.*:	dac11083 	autia	x3, x4
.*:	dac113e5 	autia	x5, sp
.*:	dac11483 	autib	x3, x4
.*:	dac117e5 	autib	x5, sp
.*:	dac11883 	autda	x3, x4
.*:	dac11be5 	autda	x5, sp
.*:	dac11c83 	autdb	x3, x4
.*:	dac11fe5 	autdb	x5, sp
.*:	dac123e5 	paciza	x5
.*:	dac127e5 	pacizb	x5
.*:	dac12be5 	pacdza	x5
.*:	dac12fe5 	pacdzb	x5
.*:	dac133e5 	autiza	x5
.*:	dac137e5 	autizb	x5
.*:	dac13be5 	autdza	x5
.*:	dac13fe5 	autdzb	x5
.*:	dac143e5 	xpaci	x5
.*:	dac147e5 	xpacd	x5
.*:	9ac33041 	pacga	x1, x2, x3
.*:	9adf3041 	pacga	x1, x2, sp
.*:	d71f0822 	braa	x1, x2
.*:	d71f087f 	braa	x3, sp
.*:	d71f0c22 	brab	x1, x2
.*:	d71f0c7f 	brab	x3, sp
.*:	d73f0822 	blraa	x1, x2
.*:	d73f087f 	blraa	x3, sp
.*:	d73f0c22 	blrab	x1, x2
.*:	d73f0c7f 	blrab	x3, sp
.*:	d61f08bf 	braaz	x5
.*:	d61f0cbf 	brabz	x5
.*:	d63f08bf 	blraaz	x5
.*:	d63f0cbf 	blrabz	x5
.*:	d65f0bff 	retaa
.*:	d65f0fff 	retab
.*:	d69f0bff 	eretaa
.*:	d69f0fff 	eretab
.*:	f8200441 	ldraa	x1, \[x2\]
.*:	f8200441 	ldraa	x1, \[x2\]
.*:	f87ff483 	ldraa	x3, \[x4, #-8\]
.*:	f82014c5 	ldraa	x5, \[x6, #8\]
.*:	f83ff507 	ldraa	x7, \[x8, #4088\]
.*:	f8600528 	ldraa	x8, \[x9, #-4096\]
.*:	f82007e2 	ldraa	x2, \[sp\]
.*:	f87067e4 	ldraa	x4, \[sp, #-2000\]
.*:	f8a00441 	ldrab	x1, \[x2\]
.*:	f8a00441 	ldrab	x1, \[x2\]
.*:	f8fff483 	ldrab	x3, \[x4, #-8\]
.*:	f8a014c5 	ldrab	x5, \[x6, #8\]
.*:	f8bff507 	ldrab	x7, \[x8, #4088\]
.*:	f8e00528 	ldrab	x8, \[x9, #-4096\]
.*:	f8a007e2 	ldrab	x2, \[sp\]
.*:	f8f067e4 	ldrab	x4, \[sp, #-2000\]
.*:	f8201c62 	ldraa	x2, \[x3, #8\]!
.*:	f87ffca4 	ldraa	x4, \[x5, #-8\]!
.*:	f83fffe6 	ldraa	x6, \[sp, #4088\]!
.*:	f8a01c62 	ldrab	x2, \[x3, #8\]!
.*:	f8fffca4 	ldrab	x4, \[x5, #-8\]!
.*:	f8bfffe6 	ldrab	x6, \[sp, #4088\]!
