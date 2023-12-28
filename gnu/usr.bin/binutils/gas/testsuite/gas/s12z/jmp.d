#objdump: -d
#name:    JMP instruction
#source:  jmp.s



.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	aa bb       	jmp d5
   2:	aa 4c       	jmp \(12,x\)
   4:	aa f3       	jmp \(\+y\)
   6:	aa d3       	jmp \(-y\)
   8:	aa f7       	jmp \(y\+\)
   a:	aa d7       	jmp \(y-\)
   c:	aa e3       	jmp \(\+x\)
   e:	aa c3       	jmp \(-x\)
  10:	aa e7       	jmp \(x\+\)
  12:	aa c7       	jmp \(x-\)
  14:	aa fb       	jmp \(-s\)
  16:	aa ff       	jmp \(s\+\)
  18:	aa ab       	jmp \(d5,s\)
  1a:	aa 87 00 5a 	jmp \(90,d7\)
  1e:	aa f0 5a    	jmp \(90,p\)
  21:	aa d4 2d    	jmp \[45,y\]
  24:	aa 00 10    	jmp 16
  27:	aa 37 f1    	jmp 14321
  2a:	aa f9 be 91 	jmp 114321
  2e:	aa fe 06 98 	jmp \[432134\]
  32:	06 
