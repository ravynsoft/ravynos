#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 trap
#as: -march=r2
#source: trap.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> b41d0020 	trap	0
0+0004 <[^>]*> b41d0020 	trap	0
0+0008 <[^>]*> b7fd0020 	trap	31
0+000c <[^>]*> b5dd0020 	trap	14
