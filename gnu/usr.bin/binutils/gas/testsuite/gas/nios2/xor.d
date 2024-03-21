#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 xor

# Test the nor instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 428cf03a 	xor	r6,r8,r10
0+0004 <[^>]*> 39bffffc 	xorhi	r6,r7,65535
0+0008 <[^>]*> 39bfffdc 	xori	r6,r7,65535
