#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 complex
#as: -march=r2
#source: complex.s

# Test complex expression parsing

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> ffff10d7 	ldw	r2,-1\(r3\)
0+0004 <[^>]*> 000110d7 	ldw	r2,1\(r3\)
0+0008 <[^>]*> 000010d7 	ldw	r2,0\(r3\)
			8: R_NIOS2_S16	stack_top-0x1
