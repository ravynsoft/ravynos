#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 sub
#as: -march=r2
#source: sub.s

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> e4042120 	sub	r4,r4,r4

