#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS64 cop2 instructions
#source: micromips@mips64-cp2.s
#as: -32

# Check MIPS64 cop2 instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section .text:
[0-9a-f]+ <[^>]*> 0064 6d3c 	dmfc2	v1,\$4
[0-9a-f]+ <[^>]*> 00c7 7d3c 	dmtc2	a2,\$7
	\.\.\.
