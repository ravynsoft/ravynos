#as: -32
#objdump: -M reg-names=numeric -dr
#name: MIPS1 FP instructions

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <.*>:
.*:	46041000 	add.s	\$f0,\$f2,\$f4
.*:	44020000 	mfc1	\$2,\$f0
#pass
