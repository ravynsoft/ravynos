#as:
#objdump:  -dr
#name:  pic2

.*: +file format .*

Disassembly of section .text:

00000000 <_main>:
   0:	12 00 0c 90 	loadd	0x0:l\(r12\),\(r1,r0\)
   4:	00 00 
			0: R_CR16_GOT_REGREL20	_text_pointer
   6:	12 00 0c 90 	loadd	0x0:l\(r12\),\(r1,r0\)
   a:	00 00 
			6: R_CR16_GOTC_REGREL20	_text_address_1

0000000c <_text_address_1>:
   c:	ee 0a       	jump	\(ra\)
