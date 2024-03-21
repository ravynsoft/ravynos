#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS64 instructions
#as: -32
#source: mips64.s

# Check MIPS64 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 00400853 	dclo	at,v0
0+0004 <[^>]*> 00801852 	dclz	v1,a0
#pass
