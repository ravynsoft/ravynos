#objdump: -d
#name:    JSR instruction
#source:  jsr.s



.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	ab b8       	jsr d2
   2:	ab 52       	jsr \(2,y\)
   4:	ab f3       	jsr \(\+y\)
   6:	ab d3       	jsr \(-y\)
   8:	ab f7       	jsr \(y\+\)
   a:	ab d7       	jsr \(y-\)
   c:	ab e3       	jsr \(\+x\)
   e:	ab c3       	jsr \(-x\)
  10:	ab e7       	jsr \(x\+\)
  12:	ab c7       	jsr \(x-\)
  14:	ab fb       	jsr \(-s\)
  16:	ab ff       	jsr \(s\+\)
  18:	ab 89       	jsr \(d3,x\)
  1a:	ab 86 00 1e 	jsr \(30,d6\)
  1e:	ab f0 5d    	jsr \(93,p\)
  21:	ab d4 2d    	jsr \[45,y\]
  24:	ab 00 0c    	jsr 12
  27:	ab 0f b5    	jsr 4021
  2a:	ab f9 be 91 	jsr 114321
  2e:	ab fe 07 82 	jsr \[492134\]
  32:	66 
  33:	ab 40       	jsr \(0,x\)
