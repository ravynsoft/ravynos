#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX ldw.n
#as: -march=r2

# Test the ld instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0855      	ldw.n	r4,0\(r17\)
0+0002 <[^>]*> 1855      	ldw.n	r4,4\(r17\)
0+0004 <[^>]*> 7855      	ldw.n	r4,28\(r17\)
0+0006 <[^>]*> f855      	ldw.n	r4,60\(r17\)
0+0008 <[^>]*> 0955      	ldw.n	r4,0\(r5\)
0+000a <[^>]*> 1955      	ldw.n	r4,4\(r5\)
0+000c <[^>]*> 7955      	ldw.n	r4,28\(r5\)
0+000e <[^>]*> f955      	ldw.n	r4,60\(r5\)
