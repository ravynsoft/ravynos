#as: -march=armv8-a+sve+f32mm
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	64bbe6b1 	fmmla	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	64a0e400 	fmmla	z0\.s, z0\.s, z0\.s
