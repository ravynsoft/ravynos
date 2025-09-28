#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 jmp

# Test the jmp instruction.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c800683a 	jmp	bt

