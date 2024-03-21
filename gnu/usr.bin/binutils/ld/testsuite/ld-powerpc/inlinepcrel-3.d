#source: inlinepcrel.s
#source: funv2.s
#as: -a64 -mpower10
#ld: -melf64ppc --hash-style=gnu
#objdump: -dr -Mpower10

.*

Disassembly of section \.text:

.*:
.*:	(07 00 00 00|00 00 00 07) 	pnop
.*:	(00 00 00 00|00 00 00 00) 
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(48 .. .. ..|.. .. .. 48) 	bl      .* <my_func>

.* <my_func>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
