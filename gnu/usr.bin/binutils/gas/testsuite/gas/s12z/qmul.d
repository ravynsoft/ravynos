#objdump: -d
#name:    
#source:  qmul.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b b4 ec 62 	qmuls.b d0, d1, #98
   4:	1b b5 c0 b9 	qmuls.b d1, d2, d3
   8:	1b b0 c9 ba 	qmuls.w d2, d3, d4
   c:	1b b1 ff 00 	qmuls.l d3, d7, #9842
  10:	00 26 72 
  13:	1b b2 68 e0 	qmulu.b d4, d1, \(32,s\)
  17:	20 
  18:	1b b3 49 c4 	qmulu.w d5, d3, \[34,x\]
  1c:	22 
  1d:	1b b6 7b ff 	qmulu.l d6, d7, \(s\+\)
  21:	1b b7 7a d4 	qmulu.lp d7, \[12,y\], \(7,d1\)
  25:	0c 85 00 07 
