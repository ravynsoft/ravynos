#objdump: -d
#name:    
#source:  mod.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 3c ec 62 	mods.b d0, d1, #98
   4:	1b 3d c0 b9 	mods.b d1, d2, d3
   8:	1b 38 c9 ba 	mods.w d2, d3, d4
   c:	1b 39 ff 00 	mods.l d3, d7, #9842
  10:	00 26 72 
  13:	1b 3a 68 e0 	modu.b d4, d1, \(32,s\)
  17:	20 
  18:	1b 3b 49 c4 	modu.w d5, d3, \[34,x\]
  1c:	22 
  1d:	1b 3e 7b ff 	modu.l d6, d7, \(s\+\)
  21:	1b 3f 7a d4 	modu.lp d7, \[12,y\], \(7,d1\)
  25:	0c 85 00 07 
