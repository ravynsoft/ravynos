#source: dotsymref.s
#source: nodotsym.s
#as: -a64
#ld: -melf64ppc -Ttext=0x10000000 -e foo
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

0+10000000 <\.foo-0x8>:
.*:	(00 00 00 00|08 00 00 10) 	.*
.*:	(10 00 00 08|00 00 00 00) 	.*

0+10000008 <\.foo>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
