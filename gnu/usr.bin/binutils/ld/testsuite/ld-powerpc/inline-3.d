#source: inline.s
#source: funv2.s
#as: -a64
#ld: -melf64ppc --hash-style=gnu
#objdump: -dr

.*

Disassembly of section \.text:

.*:
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(48 .. .. ..|.. .. .. 48) 	bl      .* <my_func>
.*:	(60 00 00 00|00 00 00 60) 	nop

.* <my_func>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
