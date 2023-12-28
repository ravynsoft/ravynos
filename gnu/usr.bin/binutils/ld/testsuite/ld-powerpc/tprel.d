#as: -a64 --defsym REG=13
#ld: -melf64ppc
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

.* <_start>:
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(80 6d 90 00|00 90 6d 80) 	lwz     r3,-28672\(r13\)
.*:	(4e 80 00 20|20 00 80 4e) 	blr
