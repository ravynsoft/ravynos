#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 flushda

# Test the jmp instruction.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 1000031b 	flushda	12\(r2\)

