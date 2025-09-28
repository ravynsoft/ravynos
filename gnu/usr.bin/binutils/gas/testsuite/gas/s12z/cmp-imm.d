#objdump: -d
#name:    
#source:  cmp-imm.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <here>:
   0:	e4 12       	cmp d0, #18
   2:	e5 34       	cmp d1, #52
   4:	e0 34 56    	cmp d2, #13398
   7:	e1 34 56    	cmp d3, #13398
   a:	e2 34 56    	cmp d4, #13398
   d:	e3 34 56    	cmp d5, #13398
  10:	e6 00 34 56 	cmp d6, #3430008
  14:	78 
  15:	e7 00 34 56 	cmp d7, #3430008
  19:	78 
  1a:	e8 aa bb cc 	cmp x, #-5588020
  1e:	e9 dd ee ff 	cmp y, #-2232577
