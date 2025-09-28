#objdump: -d
#name:    
#source:  st-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	c4 d8       	st d0, \[d2,y\]
   2:	c5 c9       	st d1, \[d3,x\]
   4:	c0 ca       	st d2, \[d4,x\]
   6:	c1 87 3b 82 	st d3, \(15234,d7\)
   a:	c2 e1 16    	st d4, \(-234,s\)
   d:	c3 bc       	st d5, d0
   f:	c6 88       	st d6, \(d2,x\)
  11:	c7 fe 00 04 	st d7, \[1234\]
  15:	d2 
  16:	c8 88       	st x, \(d2,x\)
  18:	c9 db       	st y, \[d5,y\]
