#objdump: -d
#name:    
#source:  cmp-opr-sxe4.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	e0 ff ff    	cmp d2, #-1
   3:	e1 00 01    	cmp d3, #1
   6:	e2 00 02    	cmp d4, #2
   9:	e3 00 03    	cmp d5, #3
   c:	e4 0e       	cmp d0, #14
   e:	e5 0f       	cmp d1, #15
  10:	e6 00 00 00 	cmp d6, #4
  14:	04 
  15:	e7 00 00 00 	cmp d7, #10
  19:	0a 
