#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 nor

# Test the nor instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 428c303a 	nor	r6,r8,r10
