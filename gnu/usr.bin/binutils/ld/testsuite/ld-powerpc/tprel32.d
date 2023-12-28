#source: tprel.s
#as: -a32 --defsym REG=2
#ld: -melf32ppc
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

.* <_start>:
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(80 62 90 00|00 90 62 80) 	lwz     r3,-28672\(r2\)
.*:	(4e 80 00 20|20 00 80 4e) 	blr
