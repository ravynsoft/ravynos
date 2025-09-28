#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS64 instructions
#source: mips64.s
#as: -32

# Check MIPS64 instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 5822 4b3c 	dclo	at,v0
[0-9a-f]+ <[^>]*> 5864 5b3c 	dclz	v1,a0
#pass
