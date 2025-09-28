#objdump: -d
#name:    
#source:  sbc-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 84 d5 21 	sbc d0, \[-223,y\]
   4:	1b 85 f2 00 	sbc d1, \(34000,p\)
   8:	84 d0 
   a:	1b 80 fb    	sbc d2, \(-s\)
   d:	1b 71 00 22 	sbc d3, #34
  11:	1b 82 bc    	sbc d4, d0
  14:	1b 83 0a 76 	sbc d5, 2678
  18:	1b 86 fe 00 	sbc d6, \[56\]
  1c:	00 38 
  1e:	1b 87 9b    	sbc d7, \(d5,y\)
  21:	1b 81 85 00 	sbc d3, \(34,d1\)
  25:	22 
