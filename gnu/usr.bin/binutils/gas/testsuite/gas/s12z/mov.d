#objdump: -d
#name:    
#source:  mov.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	0c 7b e3    	mov.b #123, \(\+x\)
   3:	0d 11 70 80 	mov.w #4464, \(45,d2\)
   7:	00 2d 
   9:	0e 12 34 56 	mov.p #1193046, \[34,s\]
   d:	e4 22 
   f:	0f 12 34 56 	mov.l #305419896, \(2234,d7\)
  13:	78 87 08 ba 
  17:	1c 84 00 01 	mov.b \(1,d0\), \(2,d1\)
  1b:	85 00 02 
  1e:	1d 82 00 01 	mov.w \(1,d4\), \(-s\)
  22:	fb 
  23:	1e 83 00 03 	mov.p \(3,d5\), \(-x\)
  27:	c3 
  28:	1f 87 00 26 	mov.l \(38,d7\), \(\+x\)
  2c:	e3 
