#name: br-isaa.d
#objdump: -dr
#as: -march=isaa -pcrel

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
   0:	4e71           	nop
   2:	60fc           	bras 0 <foo>
   4:	6000 0000      	braw 6 <foo\+0x6>
			6: R_68K_PC16	bar
   8:	61f6           	bsrs 0 <foo>
   a:	6100 0000      	bsrw c <foo\+0xc>
			c: R_68K_PC16	bar
   e:	4e71           	nop
