#objdump: -d
#name:    
#source:  mac.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 4c ec 62 	macs.b d0, d1, #98
   4:	1b 4d c0 b9 	macs.b d1, d2, d3
   8:	1b 48 c9 ba 	macs.w d2, d3, d4
   c:	1b 49 ff 00 	macs.l d3, d7, #9842
  10:	00 26 72 
  13:	1b 4a 68 e0 	macu.b d4, d1, \(32,s\)
  17:	20 
  18:	1b 4b 49 c4 	macu.w d5, d3, \[34,x\]
  1c:	22 
  1d:	1b 4e 7b ff 	macu.l d6, d7, \(s\+\)
  21:	1b 4f 7a d4 	macu.lp d7, \[12,y\], \(7,d1\)
  25:	0c 85 00 07 
