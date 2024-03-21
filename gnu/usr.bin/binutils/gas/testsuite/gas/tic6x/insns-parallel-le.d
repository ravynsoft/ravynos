#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X parallel instructions, little-endian
#as: -march=c674x -mlittle-endian
#source: insns-parallel.s

.*: *file format elf32-tic6x-le


Disassembly of section \.text:
0+00 <[^>]*> 00008001[ \t]+nop 5
0+04 <[^>]*> 00000000[ \t]+\|\| nop 1
0+08 <[^>]*> 00006001[ \t]+nop 4
0+0c <[^>]*> 00006001[ \t]+\|\| nop 4
0+10 <[^>]*> 00006000[ \t]+\|\| nop 4
[ \t]*\.\.\.
