#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 trap

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 003b683a 	trap	0
0+0004 <[^>]*> 003b683a 	trap	0
0+0008 <[^>]*> 003b6ffa 	trap	31
0+000c <[^>]*> 003b6bba 	trap	14
