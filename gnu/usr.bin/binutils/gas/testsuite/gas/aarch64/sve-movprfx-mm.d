#as: -march=armv8-a+i8mm+f32mm+f64mm+sve
#objdump: -dr

.* file format .*

Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	451b9ab1 	smmla	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	45db9ab1 	ummla	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	459b9ab1 	usmmla	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	449b7ab1 	usdot	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	44bf1ab1 	usdot	z17\.s, z21\.b, z7\.b\[3\]
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	44bf1eb1 	sudot	z17\.s, z21\.b, z7\.b\[3\]
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	64bbe6b1 	fmmla	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	0420bc11 	movprfx	z17, z0
 *[0-9a-f]+:	64fbe6b1 	fmmla	z17\.d, z21\.d, z27\.d
