#as: -march=armv8.6-a+bf16+sve
#objdump: -dr

.* file format .*


Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	64638040 	bfdot	z0\.s, z2\.h, z3\.h
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	64634040 	bfdot	z0\.s, z2\.h, z3\.h\[0\]
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	6463e440 	bfmmla	z0\.s, z2\.h, z3\.h
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	64e38040 	bfmlalb	z0\.s, z2\.h, z3\.h
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	64e38440 	bfmlalt	z0\.s, z2\.h, z3\.h
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	64e34040 	bfmlalb	z0\.s, z2\.h, z3\.h\[0\]
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	64e34440 	bfmlalt	z0\.s, z2\.h, z3\.h\[0\]
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	658aa040 	bfcvt	z0\.h, p0/m, z2\.s
 *[0-9a-f]+:	04912020 	movprfx	z0\.s, p0/m, z1\.s
 *[0-9a-f]+:	658aa040 	bfcvt	z0\.h, p0/m, z2\.s
