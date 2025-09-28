#as: -Av8
#objdump: -dr
#name: save args

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	81 e0 00 00 	save 
   4:	9d e3 bf a0 	save  %sp, -96, %sp
   8:	9d e3 bf a0 	save  %sp, -96, %sp
