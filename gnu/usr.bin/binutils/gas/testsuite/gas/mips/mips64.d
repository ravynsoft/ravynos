#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS64 instructions
#as: -32

# Check MIPS64 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 70410825 	dclo	at,v0
0+0004 <[^>]*> 70831824 	dclz	v1,a0
#pass
