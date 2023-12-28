#objdump: -d
#name:    
#source:  ld-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <here>:
   0:	90 01 23    	ld d2, #291
   3:	91 01 23    	ld d3, #291
   6:	92 01 23    	ld d4, #291
   9:	93 01 23    	ld d5, #291
   c:	94 56       	ld d0, #86
   e:	95 78       	ld d1, #120
  10:	96 12 34 56 	ld d6, #305419896
  14:	78 
  15:	97 12 34 56 	ld d7, #305419896
  19:	78 
  1a:	98 ab cd ef 	ld x, #-5517841
  1e:	99 fe dc ba 	ld y, #-74566
  22:	98 00 cd ef 	ld x, #52719
  26:	99 00 dc ba 	ld y, #56506
