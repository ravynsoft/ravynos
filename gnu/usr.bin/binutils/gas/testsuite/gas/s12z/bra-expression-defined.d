#objdump: -d
#name:    pc_relative expressions defined at assembly time
#source:  bra-expression-defined.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <loop-0x11>:
   0:	01          	nop
   1:	01          	nop
   2:	20 80 19    	bra \*\+25
   5:	01          	nop
   6:	02 c0 bc 80 	brclr.b d0, #4, \*\+31
   a:	1f 
   b:	01          	nop
   c:	0b 06 80 23 	tbne d6, \*\+35
  10:	01          	nop

00000011 <loop>:
  11:	01          	nop
