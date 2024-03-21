#objdump: -d
#name:    
#source:  and-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <start>:
   0:	58 12 34    	and d2, #4660
   3:	59 12 34    	and d3, #4660
   6:	5a 12 34    	and d4, #4660
   9:	5b 12 34    	and d5, #4660
   c:	5c 12       	and d0, #18
   e:	5d 34       	and d1, #52
  10:	5e 56 78 9a 	and d6, #1450744508
  14:	bc 
  15:	5f 98 76 54 	and d7, #-1737075662
  19:	32 
