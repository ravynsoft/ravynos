#objdump: -dr --show-raw-insn
#name: MIPS32r3 (align after label)
#as: -mips32r3 -32 -mno-micromips -EB
#source: align-after-label.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <seg1>:
[ 0-9a-f]+:	24000001 	li	zero,1
#pass
