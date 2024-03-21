#objdump: -d
#name:    
#source:  eor.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 78 fc 84 	eor d2, #-892
   4:	1b 79 ef 32 	eor d3, #-4302
   8:	1b 7f 00 01 	eor d7, #123456
   c:	e2 40 
   e:	1b 7b 04 d2 	eor d5, #1234
  12:	1b 7c 7b    	eor d0, #123
  15:	1b 7d 22    	eor d1, #34
  18:	1b 7e ff ff 	eor d6, #-56789
  1c:	22 2b 
  1e:	1b 7a 22 3d 	eor d4, #8765
  22:	1b 8c d5 21 	eor d0, \[-223,y\]
  26:	1b 8d f2 00 	eor d1, \(34000,p\)
  2a:	84 d0 
  2c:	1b 88 fb    	eor d2, \(-s\)
  2f:	1b 79 00 04 	eor d3, #4
  33:	1b 8a bc    	eor d4, d0
  36:	1b 8b f9 4c 	eor d5, 85178
  3a:	ba 
  3b:	1b 8e fe 00 	eor d6, \[15256\]
  3f:	3b 98 
  41:	1b 8f 8b    	eor d7, \(d5,x\)
  44:	1b 89 f7    	eor d3, \(y\+\)
