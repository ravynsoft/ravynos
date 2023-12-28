#objdump: -d
#name:    
#source:  mul.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	4c ec 62    	muls.b d0, d1, #98
   3:	49 ff 00 00 	muls.l d3, d7, #9842
   7:	26 72 
   9:	4a 68 e0 20 	mulu.b d4, d1, \(32,s\)
   d:	4b 49 c4 22 	mulu.w d5, d3, \[34,x\]
  11:	4e 7b ff    	mulu.l d6, d7, \(s\+\)
  14:	4f 7a d4 0c 	mulu.lp d7, \[12,y\], \(7,d1\)
  18:	85 00 07 
