#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 nor
#as: -march=r2
#source: nor.s

# Test the nor instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 18065220 	nor	r6,r8,r10
