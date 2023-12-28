#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX sth.n
#as: -march=r2

# Test the sth.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 086d      	sth.n	r4,0\(r17\)
0+0002 <[^>]*> 286d      	sth.n	r4,4\(r17\)
0+0004 <[^>]*> 786d      	sth.n	r4,14\(r17\)
0+0006 <[^>]*> f86d      	sth.n	r4,30\(r17\)
0+0008 <[^>]*> 096d      	sth.n	r4,0\(r5\)
0+000a <[^>]*> 296d      	sth.n	r4,4\(r5\)
0+000c <[^>]*> 796d      	sth.n	r4,14\(r5\)
0+000e <[^>]*> f96d      	sth.n	r4,30\(r5\)
