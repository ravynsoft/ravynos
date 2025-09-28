#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X parallel instructions, multiple sections
#as: -march=c674x -mlittle-endian
#source: insns-parallel-multi.s

.*: *file format elf32-tic6x-le


Disassembly of section \.text\.f1:
0+00 <[^>]*> 00008001[ \t]+nop 5
0+04 <[^>]*> 00000001[ \t]+\|\| nop 1
0+08 <[^>]*> 00000001[ \t]+\|\| nop 1
0+0c <[^>]*> 00000001[ \t]+\|\| nop 1
0+10 <[^>]*> 00000001[ \t]+\|\| nop 1
0+14 <[^>]*> 00000001[ \t]+\|\| nop 1
0+18 <[^>]*> 00000001[ \t]+\|\| nop 1
0+1c <[^>]*> 00000000[ \t]+\|\| nop 1

Disassembly of section \.text\.f2:
0+00 <[^>]*> 00006001[ \t]+nop 4
0+04 <[^>]*> 00000001[ \t]+\|\| nop 1
0+08 <[^>]*> 00000001[ \t]+\|\| nop 1
0+0c <[^>]*> 00000001[ \t]+\|\| nop 1
0+10 <[^>]*> 00000001[ \t]+\|\| nop 1
0+14 <[^>]*> 00000001[ \t]+\|\| nop 1
0+18 <[^>]*> 00000001[ \t]+\|\| nop 1
0+1c <[^>]*> 00000000[ \t]+\|\| nop 1
