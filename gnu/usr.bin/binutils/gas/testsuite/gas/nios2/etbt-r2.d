#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 etbt
#as: -march=r2
#source: etbt.s

# Test the et, bt registers

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c4183660 	add	et,bt,r6
0+0004 <[^>]*> c4183660 	add	et,bt,r6
