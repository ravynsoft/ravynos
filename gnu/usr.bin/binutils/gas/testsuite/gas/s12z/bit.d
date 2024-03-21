#objdump: -d
#name:    
#source:  bit.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 58 fc 84 	bit d2, #-892
   4:	1b 59 ef 32 	bit d3, #-4302
   8:	1b 5f 00 01 	bit d7, #123456
   c:	e2 40 
   e:	1b 5b 04 d2 	bit d5, #1234
  12:	1b 5c 7b    	bit d0, #123
  15:	1b 5d 22    	bit d1, #34
  18:	1b 5e ff ff 	bit d6, #-56789
  1c:	22 2b 
  1e:	1b 5a 22 3d 	bit d4, #8765
  22:	1b 6c d5 21 	bit d0, \[-223,y\]
  26:	1b 6d f2 00 	bit d1, \(34000,p\)
  2a:	84 d0 
  2c:	1b 68 fb    	bit d2, \(-s\)
  2f:	1b 59 00 04 	bit d3, #4
  33:	1b 6a bc    	bit d4, d0
  36:	1b 6b f9 4c 	bit d5, 85178
  3a:	ba 
  3b:	1b 6e fe 00 	bit d6, \[15256\]
  3f:	3b 98 
  41:	1b 6f 8b    	bit d7, \(d5,x\)
  44:	1b 69 f3    	bit d3, \(\+y\)
