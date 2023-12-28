#objdump: -d

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	70c7 0000 1000      	add_s	r0,r0,0x1000
   6:	72d7 0000 1000      	cmp_s	r2,0x1000
   c:	72cf 0000 1000      	mov_s	r2,0x1000
