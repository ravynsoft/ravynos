#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 or
#as: -march=r2
#source: or.s

# Test the nor instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 58065220 	or	r6,r8,r10
0+0004 <[^>]*> ffff31f4 	orhi	r6,r7,65535
0+0008 <[^>]*> ffff31d4 	ori	r6,r7,65535
