#objdump: -d
#name:    
#source:  cmp-s-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 02 e0 43 	cmp s, \(67,s\)
   4:	1b 02 e3    	cmp s, \(\+x\)
   7:	1b 02 8c    	cmp s, \(d0,x\)
   a:	1b 02 dd    	cmp s, \[d1,y\]
   d:	1b 02 f2 00 	cmp s, \(2134,p\)
  11:	08 56 
  13:	1b 02 f6 00 	cmp s, \[2134,p\]
  17:	08 56 
  19:	1b 02 fa 0f 	cmp s, 987654
  1d:	12 06 
  1f:	1b 02 e6 08 	cmp s, \[565543,s\]
  23:	a1 27 
  25:	1b 02 80 04 	cmp s, \(1233,d2\)
  29:	d1 
