#objdump: -d
#name:    
#source:  or-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <start>:
   0:	78 12 34    	or d2, #4660
   3:	79 12 34    	or d3, #4660
   6:	7a fc 7a    	or d4, #-902
   9:	7b 01 59    	or d5, #345
   c:	7c 12       	or d0, #18
   e:	7d 34       	or d1, #52
  10:	7e 56 78 9a 	or d6, #1450744508
  14:	bc 
  15:	7f ff 43 9e 	or d7, #-12345678
  19:	b2 
