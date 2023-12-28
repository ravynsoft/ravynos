#as: -Av7
#objdump: -dr
#name: pc2210

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	13 00 00 00 	sethi  %hi\(0\), %o1
			0: R_SPARC_PC22	.data
   4:	92 12 60 00 	mov  %o1, %o1	! 0 <.text>
			4: R_SPARC_PC10	.data
