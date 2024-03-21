#objdump: -dr --show-raw-insn
#as: -32
#name: MIPS dot-1
#source: dot-1.s

.*file format.*

Disassembly .*

0+0 <foo>:
.*:	4c00      	addiu	a0,0
.*:	6500      	nop
.*:	6500      	nop
.*:	4c06      	addiu	a0,6
.*:	4c04      	addiu	a0,4
.*:	4c0a      	addiu	a0,10
.*:	4c00      	addiu	a0,0
.*:	f000 4c0e 	addiu	a0,14
.*:	f000 4c0e 	addiu	a0,14
.*:	f000 4c16 	addiu	a0,22
#pass
