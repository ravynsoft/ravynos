#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 etbt

# Test the et, bt registers

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c9b1883a 	add	et,bt,r6
0+0004 <[^>]*> c9b1883a 	add	et,bt,r6
