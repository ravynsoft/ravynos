#source: nodotsym.s
#source: dotsymref.s
#as: -a64
#ld: -melf64ppc -Ttext=0x1000 -shared -z notext
#objdump: -dR

.*:     file format .*

Disassembly of section \.text:

0+1000 <\.foo>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(00 00 00 00|00 10 00 00) 	.*
.*: R_PPC64_RELATIVE	\*ABS\*\+0x1000
.*:	(00 00 10 00|00 00 00 00) 	.*
