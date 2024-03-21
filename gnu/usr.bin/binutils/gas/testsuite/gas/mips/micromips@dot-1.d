#objdump: -dr --show-raw-insn
#as: -32
#name: MIPS dot-1
#source: dot-1.s

.*file format.*

Disassembly .*

0+0 <foo>:
.*:	4c80      	addiu	a0,a0,0
.*:	0c00      	nop
.*:	0c00      	nop
.*:	3084 0008 	addiu	a0,a0,8
.*:	4c8c      	addiu	a0,a0,6
.*:	6e46      	addiu	a0,a0,12
.*:	3084 0000 	addiu	a0,a0,0
.*:	3084 000e 	addiu	a0,a0,14
.*:	3084 0012 	addiu	a0,a0,18
.*:	3084 001a 	addiu	a0,a0,26
#pass
