#objdump: -d
#name:    
#source:  min-max.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 2c fe 01 	maxs d0, \[123456\]
   4:	e2 40 
   6:	1b 2d c4 04 	maxs d1, \[4,x\]
   a:	1b 28 b8    	maxs d2, d2
   d:	1b 29 dc    	maxs d3, \[d0,y\]
  10:	1b 2a e4 0c 	maxs d4, \[12,s\]
  14:	1b 2b 63    	maxs d5, \(3,s\)
  17:	1b 2e e1 85 	maxs d6, \(-123,s\)
  1b:	1b 2f f2 01 	maxs d7, \(123987,p\)
  1f:	e4 53 
  21:	1b 24 c7    	mins d0, \(x-\)
  24:	1b 25 d7    	mins d1, \(y-\)
  27:	1b 20 e7    	mins d2, \(x\+\)
  2a:	1b 21 f7    	mins d3, \(y\+\)
  2d:	1b 22 ff    	mins d4, \(s\+\)
  30:	1b 23 e3    	mins d5, \(\+x\)
  33:	1b 26 f3    	mins d6, \(\+y\)
  36:	1b 27 c3    	mins d7, \(-x\)
  39:	1b 1c d3    	maxu d0, \(-y\)
  3c:	1b 1d fb    	maxu d1, \(-s\)
  3f:	1b 18 8b    	maxu d2, \(d5,x\)
  42:	1b 19 9e    	maxu d3, \(d6,y\)
  45:	1b 1a af    	maxu d4, \(d7,s\)
  48:	1b 1b e2 ff 	maxu d5, \(-1023,s\)
  4c:	fc 01 
  4e:	1b 1e f6 00 	maxu d6, \[1087,p\]
  52:	04 3f 
  54:	1b 1f d3    	maxu d7, \(-y\)
  57:	1b 14 c3    	minu d0, \(-x\)
  5a:	1b 15 ff    	minu d1, \(s\+\)
  5d:	1b 10 8d    	minu d2, \(d1,x\)
  60:	1b 11 98    	minu d3, \(d2,y\)
  63:	1b 12 a9    	minu d4, \(d3,s\)
  66:	1b 13 e1 85 	minu d5, \(-123,s\)
  6a:	1b 16 f6 01 	minu d6, \[123987,p\]
  6e:	e4 53 
  70:	1b 17 d7    	minu d7, \(y-\)
