#name: br-isac.d
#objdump: -dr
#as: -march=isac -pcrel

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
   0:	4e71           	nop
   2:	61ff ffff fffc 	bsrl 0 <foo>
   8:	60f6           	bras 0 <foo>
   a:	66ff 0000 0000 	bnel c <foo\+0xc>
			c: R_68K_PC32	bar
  10:	67ff 0000 0000 	beql 12 <foo\+0x12>
			12: R_68K_PC32	bar
  16:	61e8           	bsrs 0 <foo>
  18:	61ff 0000 0000 	bsrl 1a <foo\+0x1a>
			1a: R_68K_PC32	bar
  1e:	4e71           	nop
  20:	40e7 46fc 04d2 	stldsr #1234
