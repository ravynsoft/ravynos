#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 lineseparator
#as: -march=r2
#source: lineseparator.s

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0x0+0000 c4050120 	mov	r5,r4
0x0+0004 c4040160 	mov	r4,r5
