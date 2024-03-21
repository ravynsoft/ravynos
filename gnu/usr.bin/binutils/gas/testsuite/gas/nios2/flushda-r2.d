#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 flushda
#as: -march=r2
#source: flushda.s

# Test the jmp instruction.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 300c18a8 	flushda	12\(r2\)

