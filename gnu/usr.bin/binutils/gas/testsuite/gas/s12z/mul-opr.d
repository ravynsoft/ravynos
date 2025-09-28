#objdump: -d
#name:    
#source:  mul-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	4c e8 70    	muls.b d0, d1, #-1
   3:	49 f9 fb    	muls.w d3, d7, \(-s\)
   6:	49 fb e3    	muls.l d3, d7, \(\+x\)
   9:	4a 68 e0 20 	mulu.b d4, d1, \(32,s\)
   d:	4b 49 c4 22 	mulu.w d5, d3, \[34,x\]
  11:	4e 7b ff    	mulu.l d6, d7, \(s\+\)
