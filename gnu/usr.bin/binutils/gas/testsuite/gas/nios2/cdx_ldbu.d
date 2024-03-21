#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX ldbu.n
#as: -march=r2

# Test the ld instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0845      	ldbu.n	r4,0\(r17\)
0+0002 <[^>]*> 4845      	ldbu.n	r4,4\(r17\)
0+0004 <[^>]*> 7845      	ldbu.n	r4,7\(r17\)
0+0006 <[^>]*> f845      	ldbu.n	r4,15\(r17\)
0+0008 <[^>]*> 0945      	ldbu.n	r4,0\(r5\)
0+000a <[^>]*> 4945      	ldbu.n	r4,4\(r5\)
0+000c <[^>]*> 7945      	ldbu.n	r4,7\(r5\)
0+000e <[^>]*> f945      	ldbu.n	r4,15\(r5\)
