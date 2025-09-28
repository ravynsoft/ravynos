#as: -mpower10
#objdump: -dr -Mpower10
#name: byte reverse

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(7c 83 01 76|76 01 83 7c) 	brd     r3,r4
.*:	(7c a4 01 b6|b6 01 a4 7c) 	brh     r4,r5
.*:	(7c c5 01 36|36 01 c5 7c) 	brw     r5,r6
