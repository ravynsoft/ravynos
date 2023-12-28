#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS MIPS32r2 fp instructions
#source: mips32r2-fp32.s
#as: -32

# Check MIPS32 Release 2 (mips32r2) FP instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 5620 303b 	mfhc1	\$17,\$f0
[0-9a-f]+ <[^>]*> 5620 383b 	mthc1	\$17,\$f0
#pass
