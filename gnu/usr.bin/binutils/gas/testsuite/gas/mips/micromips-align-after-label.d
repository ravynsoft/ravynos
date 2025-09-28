#objdump: -dr --show-raw-insn
#name: microMIPSr3 (align after label)
#as: -mips32r3 -32 -mmicromips -EB
#source: align-after-label.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <seg1>:
[ 0-9a-f]+:	4c02      	addiu	zero,zero,1
[ 0-9a-f]+:	0c00      	nop
#pass
