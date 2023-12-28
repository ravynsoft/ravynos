#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X predicates
#as: -march=c674x -mlittle-endian
#source: insns-predicate.s

.*: *file format elf32-tic6x-le


Disassembly of section \.text:
0+00 <[^>]*> c1880359[ \t]+\[a0\] abs \.L1 a2,a3
0+04 <[^>]*> c290035a[ \t]+\|\| \[a0\] abs \.L2 b4,b5
0+08 <[^>]*> d3980358[ \t]+\[!a0\] abs \.L1 a6,a7
0+0c <[^>]*> d4a0035a[ \t]+\[!a0\] abs \.L2 b8,b9
0+10 <[^>]*> 25a80358[ \t]+\[b0\] abs \.L1 a10,a11
0+14 <[^>]*> 26b0035a[ \t]+\[b0\] abs \.L2 b12,b13
0+18 <[^>]*> 37b80358[ \t]+\[!b0\] abs \.L1 a14,a15
0+1c <[^>]*> 38c0035a[ \t]+\[!b0\] abs \.L2 b16,b17
0+20 <[^>]*> 89c80358[ \t]+\[a1\] abs \.L1 a18,a19
0+24 <[^>]*> 9ad0035a[ \t]+\[!a1\] abs \.L2 b20,b21
0+28 <[^>]*> abd80358[ \t]+\[a2\] abs \.L1 a22,a23
0+2c <[^>]*> bce0035a[ \t]+\[!a2\] abs \.L2 b24,b25
0+30 <[^>]*> 4de80358[ \t]+\[b1\] abs \.L1 a26,a27
0+34 <[^>]*> 5ef0035a[ \t]+\[!b1\] abs \.L2 b28,b29
0+38 <[^>]*> 6ff80358[ \t]+\[b2\] abs \.L1 a30,a31
0+3c <[^>]*> 738c035a[ \t]+\[!b2\] abs \.L2 b3,b7
