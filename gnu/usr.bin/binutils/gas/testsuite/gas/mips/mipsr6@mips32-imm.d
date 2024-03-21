#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 WAIT and SDBBP instructions
#as: -32
#source: mips32-imm.s

# Check MIPS32 WAIT and SDBBP instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
[0-9a-f]+ <[^>]*> 4359e260 	wait	0x56789
[0-9a-f]+ <[^>]*> 0159e24e 	sdbbp	0x56789
	\.\.\.
