#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS MIPS32r2 fp instructions
#as: -32

# Check MIPS32 Release 2 (mips32r2) FP instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 44710000 	mfhc1	\$17,\$f0
0+0004 <[^>]*> 44f10000 	mthc1	\$17,\$f0
#pass
