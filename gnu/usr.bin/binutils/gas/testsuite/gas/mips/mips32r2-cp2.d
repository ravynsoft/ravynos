#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS MIPS32r2 cop2 instructions
#as: -32

# Check MIPS32 Release 2 (mips32r2) cop2 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 48715555 	mfhc2	\$17,0x5555
0+0004 <[^>]*> 48f15555 	mthc2	\$17,0x5555
#pass
