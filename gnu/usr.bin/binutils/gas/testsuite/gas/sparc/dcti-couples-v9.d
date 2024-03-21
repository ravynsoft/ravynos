#as: -Av9 --dcti-couples-detect
#objdump: -dr
#name: dcti couples (v9)
#source: dcti-couples.s

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	1a 80 00 03 	bcc  0xc
   4:	10 80 00 02 	b  0xc
   8:	01 00 00 00 	nop 
   c:	10 80 00 00 	b  0xc
  10:	10 bf ff ff 	b  0xc
  14:	01 00 00 00 	nop 
