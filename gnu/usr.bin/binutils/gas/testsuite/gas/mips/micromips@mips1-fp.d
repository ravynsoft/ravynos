#objdump: -dr --show-raw-insn -M reg-names=numeric
#name: MIPS1 FP instructions
#source: mips1-fp.s
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <foo>:
[0-9a-f ]+:	5482 0030 	add\.s	\$f0,\$f2,\$f4
[0-9a-f ]+:	5440 203b 	mfc1	\$2,\$f0
#pass
