#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 lineseparator

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0x0+0000 200b883a 	mov	r5,r4
0x0+0004 2809883a 	mov	r4,r5
