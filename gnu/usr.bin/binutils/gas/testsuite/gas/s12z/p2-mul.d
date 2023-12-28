#objdump: -d
#name:    Multiply instructions from page2 in all reg-reg-reg mode
#source:  p2-mul.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b b4 a9    	qmuls d0, d1, d3
   3:	1b b2 1e    	qmulu d4, d5, d6
   6:	1b 37 a5    	divs d7, d0, d1
   9:	1b 30 0a    	divu d2, d3, d4
   c:	1b 3b b7    	mods d5, d6, d7
   f:	1b 3c 28    	modu d0, d1, d2
