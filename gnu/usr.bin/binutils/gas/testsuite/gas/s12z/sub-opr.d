#objdump: -d
#name:    
#source:  sub-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	84 d5 21    	sub d0, \[-223,y\]
   3:	85 f2 00 84 	sub d1, \(34000,p\)
   7:	d0 
   8:	80 fb       	sub d2, \(-s\)
   a:	71 00 04    	sub d3, #4
   d:	82 bc       	sub d4, d0
   f:	83 f9 4e ae 	sub d5, 85678
  13:	86 fe 00 4b 	sub d6, \[19256\]
  17:	38 
  18:	87 8a       	sub d7, \(d4,x\)
  1a:	81 81 00 0b 	sub d3, \(11,d3\)