#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 xor
#as: -march=r2
#source: xor.s

# Test the nor instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 78065220 	xor	r6,r8,r10
0+0004 <[^>]*> ffff31fc 	xorhi	r6,r7,65535
0+0008 <[^>]*> ffff31dc 	xori	r6,r7,65535
