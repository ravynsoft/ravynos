#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 cache
#as: -march=r2

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 380011a8 	flushd	-2048\(r6\)
0+0004 <[^>]*> 37ff11a8 	flushd	2047\(r6\)
0+0008 <[^>]*> 300011a8 	flushd	0\(r6\)
0+000c <[^>]*> 3fff11a8 	flushd	-1\(r6\)
0+0010 <[^>]*> 300011a8 	flushd	0\(r6\)
[	]*10: R_NIOS2_R2_S12	.text
0+0014 <[^>]*> 300011a8 	flushd	0\(r6\)
[	]*14: R_NIOS2_R2_S12	external
0+0018 <[^>]*> 300000a0 	flushi	r2
0+001c <[^>]*> 10000020 	flushp

