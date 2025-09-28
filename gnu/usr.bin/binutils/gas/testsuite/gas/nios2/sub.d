#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 sub

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2109c83a 	sub	r4,r4,r4

