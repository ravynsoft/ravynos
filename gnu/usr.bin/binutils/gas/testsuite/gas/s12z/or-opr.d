#objdump: -d
#name:    
#source:  or-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	8c d5 21    	or d0, \[-223,y\]
   3:	8d f2 00 84 	or d1, \(34000,p\)
   7:	d0 
   8:	88 c3       	or d2, \(-x\)
   a:	89 f7       	or d3, \(y\+\)
   c:	8a ba       	or d4, d4
   e:	8b f8 77 d6 	or d5, 30678
  12:	8e fe 01 64 	or d6, \[91256\]
  16:	78 
  17:	8f 9a       	or d7, \(d4,y\)
