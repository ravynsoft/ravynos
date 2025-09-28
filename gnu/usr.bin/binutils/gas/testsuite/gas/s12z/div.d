#objdump: -d
#name:    
#source:  div.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 34 ec 62 	divs.b d0, d1, #98
   4:	1b 35 c0 b9 	divs.b d1, d2, d3
   8:	1b 30 c9 ba 	divs.w d2, d3, d4
   c:	1b 31 ff 00 	divs.l d3, d7, #9842
  10:	00 26 72 
  13:	1b 32 68 e0 	divu.b d4, d1, \(32,s\)
  17:	20 
  18:	1b 33 49 c4 	divu.w d5, d3, \[34,x\]
  1c:	22 
  1d:	1b 36 7b ff 	divu.l d6, d7, \(s\+\)
  21:	1b 37 7a d4 	divu.lp d7, \[12,y\], \(7,d1\)
  25:	0c 85 00 07 
