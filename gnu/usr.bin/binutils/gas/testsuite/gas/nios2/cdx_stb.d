#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX stb.n
#as: -march=r2

# Test the stb.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0865      	stb.n	r4,0\(r17\)
0+0002 <[^>]*> 4865      	stb.n	r4,4\(r17\)
0+0004 <[^>]*> 7865      	stb.n	r4,7\(r17\)
0+0006 <[^>]*> f865      	stb.n	r4,15\(r17\)
0+0008 <[^>]*> 0965      	stb.n	r4,0\(r5\)
0+000a <[^>]*> 4965      	stb.n	r4,4\(r5\)
0+000c <[^>]*> 7965      	stb.n	r4,7\(r5\)
0+000e <[^>]*> f965      	stb.n	r4,15\(r5\)
0+0010 <[^>]*> 801d      	stbz.n	zero,0\(r16\)
0+0012 <[^>]*> 801d      	stbz.n	zero,0\(r16\)
0+0014 <[^>]*> fe1d      	stbz.n	zero,63\(r16\)
0+0016 <[^>]*> ffdd      	stbz.n	zero,63\(r7\)
