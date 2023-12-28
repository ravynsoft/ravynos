#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 or

# Test the nor instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 428cb03a 	or	r6,r8,r10
0+0004 <[^>]*> 39bffff4 	orhi	r6,r7,65535
0+0008 <[^>]*> 39bfffd4 	ori	r6,r7,65535
