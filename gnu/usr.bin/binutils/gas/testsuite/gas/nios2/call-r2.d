# objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 call
#as: -march=r2
#source: call.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00000000 	call	00000000 <[^>]*>
[	]*0: R_NIOS2_CALL26	.text\+0xc
0+0004 <[^>]*> 741f02a0 	callr	r10
0+0008 <[^>]*> 00000000 	call	00000000 <[^>]*>
[	]*8: R_NIOS2_CALL26	external
