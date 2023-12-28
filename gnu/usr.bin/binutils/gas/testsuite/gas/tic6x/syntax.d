#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X syntax variants
#as: -march=c674x -mlittle-endian

.*: *file format elf32-tic6x-le


Disassembly of section \.text:
0+00 <[^>]*> 00000000[ \t]+nop 1
0+04 <[^>]*> 00008000[ \t]+nop 5
0+08 <[^>]*> 00002000[ \t]+nop 2
0+0c <[^>]*> 00004000[ \t]+nop 3
0+10 <[^>]*> 00008000[ \t]+nop 5
0+14 <[^>]*> 00002000[ \t]+nop 2
0+18 <[^>]*> 00006000[ \t]+nop 4
0+1c <[^>]*> 00002000[ \t]+nop 2
0+20 <[^>]*> 05900358[ \t]+abs \.L1 a4,a11
0+24 <[^>]*> 0694135a[ \t]+abs \.L2X a5,b13
0+28 <[^>]*> 0593e1a0[ \t]+add \.S1 -1,a4,a11
[ \t]*\.\.\.
