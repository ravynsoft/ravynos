#objdump: -dr

.*: +file format .*arc.*



Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	3e2f 703f           	myinsn	r0
   4:	3e6f 7ebf           	myinsn	0x3a
   8:	3e2f 7fbf dead beef 	myinsn	0xdeadbeef
  10:	3e2f 7fbf 0000 0000 	myinsn	0
			14: R_ARC_32_ME	label
  18:	3e2f 7fbf 0000 0000 	myinsn	0
			1c: R_ARC_PC32	label
  20:	386f 203f           	noop
