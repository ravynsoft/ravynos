#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 jmp
#as: -march=r2
#source: jmp.s

# Test the jmp instruction.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 34000660 	jmp	bt

