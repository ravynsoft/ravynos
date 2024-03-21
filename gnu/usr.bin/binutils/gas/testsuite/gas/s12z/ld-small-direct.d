#objdump: -d
#name:    LD reg - small constants left in OPR mode
#source:  ld-small-direct.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1>:
   0:	a0 fa 00 00 	ld d2, L1
   4:	00 
   5:	a1 fa 00 00 	ld d3, 25
   9:	19 
   a:	a2 fa 00 00 	ld d4, L1
   e:	00 

0000000f <L3>:
   f:	a4 fa 00 00 	ld d0, 25
  13:	19 
  14:	a5 fa 00 00 	ld d1, L3
  18:	0f 
  19:	a3 fa 00 00 	ld d5, L1
  1d:	00 
  1e:	a6 fa 00 00 	ld d6, L3
  22:	0f 
  23:	a7 fa 00 00 	ld d7, 25
  27:	19 
  28:	a8 30 39    	ld x, 12345
  2b:	a9 26 94    	ld y, 9876
