#as:
#objdump:  -dr
#name:  pic1

.*: +file format .*

Disassembly of section .text:

00000000 <_main>:
   0:	70 00 00 00 	movd	\$0x0:l,\(r1,r0\)
   4:	00 00 
			0: R_CR16_IMM32	_text_pointer
   6:	70 00 00 00 	movd	\$0x0:l,\(r1,r0\)
   a:	00 00 
			6: R_CR16_IMM32a	_text_address_1

0000000c <_text_address_1>:
   c:	ee 0a       	jump	\(ra\)
