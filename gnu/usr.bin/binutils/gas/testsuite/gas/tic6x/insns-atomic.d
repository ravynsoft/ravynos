#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X atomic instructions
#as: -march=c674x -mlittle-endian

.*: *file format elf32-tic6x-le


Disassembly of section \.text:
[0-9a-f]+[048c] <[^>]*> c0800742[ \t]+\[a0\] cmtl \.D2T2 \*b0,b1
[0-9a-f]+[048c] <[^>]*> 51880642[ \t]+\[!b1\] ll \.D2T2 \*b2,b3
[0-9a-f]+[048c] <[^>]*> af7406c2[ \t]+\[a2\] sl \.D2T2 b30,\*b29
[ \t]*\.\.\.
