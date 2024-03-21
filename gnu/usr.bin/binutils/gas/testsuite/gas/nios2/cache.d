#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 cache

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 3020003b 	flushd	-32768\(r6\)
0+0004 <[^>]*> 301ffffb 	flushd	32767\(r6\)
0+0008 <[^>]*> 3000003b 	flushd	0\(r6\)
0+000c <[^>]*> 303ffffb 	flushd	-1\(r6\)
0+0010 <[^>]*> 3000003b 	flushd	0\(r6\)
[	]*10: R_NIOS2_S16	.text
0+0014 <[^>]*> 3000003b 	flushd	0\(r6\)
[	]*14: R_NIOS2_S16	external
0+0018 <[^>]*> 1000603a 	flushi	r2
0+001c <[^>]*> 0000203a 	flushp

