#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS MIPS32r2 cop2 instructions
#as: -32

# Check MIPS32 Release 2 (mips32r2) cop2 instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section .text:
[0-9a-f]+ <[^>]*> 022f 8d3c 	mfhc2	\$17,\$15
[0-9a-f]+ <[^>]*> 022f 9d3c 	mthc2	\$17,\$15
	\.\.\.
