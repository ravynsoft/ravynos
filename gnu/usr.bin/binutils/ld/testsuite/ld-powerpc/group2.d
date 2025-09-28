#source: group2.s
#source: group1.s
#source: group3.s
#as: -a64 -e foo
#ld: -melf64ppc
#objdump: -d

.*

Disassembly of section \.text:

.* <foo>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
