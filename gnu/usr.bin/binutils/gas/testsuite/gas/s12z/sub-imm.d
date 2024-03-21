#objdump: -d
#name:    
#source:  sub-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <start>:
   0:	70 12 34    	sub d2, #4660
   3:	71 12 34    	sub d3, #4660
   6:	72 12 34    	sub d4, #4660
   9:	73 12 34    	sub d5, #4660
   c:	74 ef       	sub d0, #-17
   e:	75 34       	sub d1, #52
  10:	76 56 78 9a 	sub d6, #1450744508
  14:	bc 
  15:	77 98 77 17 	sub d7, #-1737025662
  19:	82 
