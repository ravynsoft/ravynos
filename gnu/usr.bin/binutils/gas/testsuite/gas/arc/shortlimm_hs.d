#objdump: -d

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	70c3 0000 1000      	add_s	r0,r0,0x1000
   6:	71c7 0000 1001      	add_s	0,0x1001,1
   c:	72d3 0000 1000      	cmp_s	r2,0x1000
  12:	71d7 0000 1000      	cmp_s	0x1000,1
  18:	42c3 0000 1000      	mov_s	r2,0x1000
  1e:	46db 0000 1000      	mov_s	0,0x1000
  24:	72df 0000 1000      	mov_s.ne	r2,0x1000
