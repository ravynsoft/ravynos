#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX stw.n
#as: -march=r2

# Test the stw.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0875      	stw.n	r4,0\(r17\)
0+0002 <[^>]*> 1875      	stw.n	r4,4\(r17\)
0+0004 <[^>]*> 7875      	stw.n	r4,28\(r17\)
0+0006 <[^>]*> f875      	stw.n	r4,60\(r17\)
0+0008 <[^>]*> 0975      	stw.n	r4,0\(r5\)
0+000a <[^>]*> 1975      	stw.n	r4,4\(r5\)
0+000c <[^>]*> 7975      	stw.n	r4,28\(r5\)
0+000e <[^>]*> f975      	stw.n	r4,60\(r5\)
0+0010 <[^>]*> 001d      	stwz.n	zero,0\(r16\)
0+0012 <[^>]*> 001d      	stwz.n	zero,0\(r16\)
0+0014 <[^>]*> 7e1d      	stwz.n	zero,252\(r16\)
0+0016 <[^>]*> 7fdd      	stwz.n	zero,252\(r7\)
