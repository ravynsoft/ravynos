#objdump: -d
#name:    
#source:  bfext.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 0c 14    	bfext d0, d1, d2
   3:	1b 0d 21 17 	bfext d1, d2, #8:23
   7:	1b 08 41 e3 	bfext.b d2, \(\+x\), d3
   b:	1b 09 46 c4 	bfext.w d3, \[123,x\], d4
   f:	7b 
  10:	1b 0a 4b ac 	bfext.p d4, \(d0,s\), d5
  14:	1b 0b 4c 84 	bfext.l d5, \(45,d0\), d2
  18:	00 2d 
  1a:	1b 0e 51 84 	bfext.b \(45,d0\), d6, d3
  1e:	00 2d 
  20:	1b 0f 54 c4 	bfext.w \[45,x\], d7, d2
  24:	2d 
  25:	1b 0c 61 a2 	bfext.b d0, \(45,d1\), #13:2
  29:	85 00 2d 
  2c:	1b 0f 75 a3 	bfext.w \(45,d1\), d7, #13:3
  30:	85 00 2d 
  33:	1b 0f 58 c6 	bfext.p \[451,x\], d7, d2
  37:	00 01 c3 
  3a:	1b 0c 94    	bfins d0, d1, d2
  3d:	1b 0d a1 17 	bfins d1, d2, #8:23
  41:	1b 08 c1 e3 	bfins.b d2, \(\+x\), d3
  45:	1b 09 c6 c4 	bfins.w d3, \[123,x\], d4
  49:	7b 
  4a:	1b 0a cb ac 	bfins.p d4, \(d0,s\), d5
  4e:	1b 0b cc 84 	bfins.l d5, \(45,d0\), d2
  52:	00 2d 
  54:	1b 0e d1 84 	bfins.b \(45,d0\), d6, d3
  58:	00 2d 
  5a:	1b 0f d4 c4 	bfins.w \[45,x\], d7, d2
  5e:	2d 
  5f:	1b 0c e1 a2 	bfins.b d0, \(45,d1\), #13:2
  63:	85 00 2d 
  66:	1b 0f f5 a3 	bfins.w \(45,d1\), d7, #13:3
  6a:	85 00 2d 
  6d:	1b 0f d8 c6 	bfins.p \[451,x\], d7, d2
  71:	00 01 c3 
