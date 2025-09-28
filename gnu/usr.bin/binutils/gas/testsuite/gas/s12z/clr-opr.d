#objdump: -d
#name:    
#source:  clr-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	bc ff       	clr.b \(s\+\)
   2:	bd c0 2d    	clr.w \(45,x\)
   5:	be f9 e2 32 	clr.p 123442
   9:	bf d4 03    	clr.l \[3,y\]
