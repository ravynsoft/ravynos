#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX ldhu.n
#as: -march=r2

# Test the ld instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 084d      	ldhu.n	r4,0\(r17\)
0+0002 <[^>]*> 284d      	ldhu.n	r4,4\(r17\)
0+0004 <[^>]*> 784d      	ldhu.n	r4,14\(r17\)
0+0006 <[^>]*> f84d      	ldhu.n	r4,30\(r17\)
0+0008 <[^>]*> 094d      	ldhu.n	r4,0\(r5\)
0+000a <[^>]*> 294d      	ldhu.n	r4,4\(r5\)
0+000c <[^>]*> 794d      	ldhu.n	r4,14\(r5\)
0+000e <[^>]*> f94d      	ldhu.n	r4,30\(r5\)
