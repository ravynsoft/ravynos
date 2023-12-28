#as: -64 -Av9
#objdump: -dr
#name: sparc64 gotop

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <foo>:
   0:	23 00 00 00 	sethi  %hi\(0\), %l1
			0: R_SPARC_GOTDATA_OP_HIX22	.data
   4:	a2 1c 60 00 	xor  %l1, 0, %l1
			4: R_SPARC_GOTDATA_OP_LOX10	.data
   8:	e4 5d c0 11 	ldx  \[ %l7 \+ %l1 \], %l2
			8: R_SPARC_GOTDATA_OP	.data
