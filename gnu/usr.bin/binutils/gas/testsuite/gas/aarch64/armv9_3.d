#objdump: -dr

.*


Disassembly of section \.text:

0+ <\.text>:
[^:]*:	d50330ff 	sb
[^:]*:	453f1800 	rshrnb	z0\.h, z0\.s, #1
[^:]*:	658aa000 	bfcvt	z0\.h, p0/m, z0\.s
[^:]*:	45029820 	smmla	z0\.s, z1.b, z2\.b
[^:]*:	f83fd100 	ld64b	x0, \[x8\]
[^:]*:	190107c0 	cpyfp	\[x0\]!, \[x1\]!, x30!
[^:]*:	194107c0 	cpyfm	\[x0\]!, \[x1\]!, x30!
[^:]*:	198107c0 	cpyfe	\[x0\]!, \[x1\]!, x30!
[^:]*:	54000010 	bc\.eq	.*
