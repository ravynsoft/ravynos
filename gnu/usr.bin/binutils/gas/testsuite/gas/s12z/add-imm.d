#objdump: -d
#name:    
#source:  add-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <start>:
   0:	50 12 34    	add d2, #4660
   3:	51 12 34    	add d3, #4660
   6:	52 12 34    	add d4, #4660
   9:	53 12 34    	add d5, #4660
   c:	54 12       	add d0, #18
   e:	55 34       	add d1, #52
  10:	56 00 56 78 	add d6, #5666970
  14:	9a 
  15:	57 00 98 76 	add d7, #9991764
  19:	54 
