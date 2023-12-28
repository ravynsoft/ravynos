#as: -march=armv8.3-a
#objdump: -dr

.*: .*


Disassembly of section \.text:

0+ <.*>:
.*:	f8200c01 	ldraa	x1, \[x0]!
.*:	f8a00c02 	ldrab	x2, \[x0]!
.*:	f8200c01 	ldraa	x1, \[x0]!
.*:	f8a00c02 	ldrab	x2, \[x0]!
