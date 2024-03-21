#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 WAIT and SDBBP instructions
#source: micromips@mips32-imm.s
#as: -32

# Check MIPS32 WAIT and SDBBP instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section .text:
[0-9a-f]+ <[^>]*> 03c3 937c 	wait	0x3c3
[0-9a-f]+ <[^>]*> 03c3 db7c 	sdbbp	0x3c3
	\.\.\.
