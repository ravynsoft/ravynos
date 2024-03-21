#as: 
#ld: 
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

.* <_start>:
.*:	(3c 60 00 00|00 00 60 3c) 	lis     r3,0
.*:	(38 63 90 00|00 90 63 38) 	addi    r3,r3,-28672
.*:	(4e 80 00 20|20 00 80 4e) 	blr
