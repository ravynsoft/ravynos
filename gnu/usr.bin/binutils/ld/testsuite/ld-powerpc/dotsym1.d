#source: nodotsym.s
#source: dotsymref.s
#as: -a64
#ld: -melf64ppc -Ttext=0x10000000 -e foo
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

0+10000000 <\.foo>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(00 00 00 00|00 00 00 10) 	.*
.*:	(10 00 00 00|00 00 00 00) 	.*
