#objdump: -d
#name:    ST reg - small constants left in OPR mode
#source:  st-small-direct.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1>:
   0:	c0 fa 00 00 	st d2, L1
   4:	00 
   5:	c1 fa 00 00 	st d3, L2
   9:	1a 
   a:	c2 fa 00 00 	st d4, L1
   e:	00 

0000000f <L3>:
   f:	01          	nop
  10:	c4 fa 00 00 	st d0, L2
  14:	1a 
  15:	c5 fa 00 00 	st d1, L3
  19:	0f 

0000001a <L2>:
  1a:	c3 fa 00 00 	st d5, L1
  1e:	00 
  1f:	c6 fa 00 00 	st d6, L3
  23:	0f 
  24:	c7 fa 00 00 	st d7, L2
  28:	1a 
  29:	c8 30 39    	st x, 12345
  2c:	c9 26 94    	st y, 9876
