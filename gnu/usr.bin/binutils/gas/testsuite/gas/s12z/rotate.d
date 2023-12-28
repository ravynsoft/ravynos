#objdump: -d
#name:    
#source:  rotate.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	10 24 b8    	ror.b d2
   3:	10 64 fb    	rol.b \(-s\)
   6:	10 65 ff    	rol.w \(s\+\)
   9:	10 66 d4 2d 	rol.p \[45,y\]
   d:	10 67 87 00 	rol.l \(78,d7\)
  11:	4e 
  12:	10 24 bc    	ror.b d0
  15:	10 24 c3    	ror.b \(-x\)
  18:	10 25 f7    	ror.w \(y\+\)
  1b:	10 26 f4 29 	ror.p \[41,p\]
  1f:	10 27 8b    	ror.l \(d5,x\)
