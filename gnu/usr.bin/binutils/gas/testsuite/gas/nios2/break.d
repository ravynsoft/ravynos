#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 break

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 003da03a 	break	0
0+0004 <[^>]*> 003da03a 	break	0
0+0008 <[^>]*> 003da7fa 	break	31
0+000c <[^>]*> 003da3ba 	break	14


