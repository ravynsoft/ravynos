#objdump: -dr --show-raw-insn
#as: -32
#name: MIPS dot-1
#source: dot-1.s

.*file format.*

Disassembly .*

0+0 <foo>:
.*:	24840000 	addiu	a0,a0,0
	\.\.\.
.*:	2484000c 	addiu	a0,a0,12
.*:	24840008 	addiu	a0,a0,8
.*:	24840014 	addiu	a0,a0,20
.*:	24840000 	addiu	a0,a0,0
.*:	24840010 	addiu	a0,a0,16
.*:	24840018 	addiu	a0,a0,24
.*:	24840024 	addiu	a0,a0,36
#pass
