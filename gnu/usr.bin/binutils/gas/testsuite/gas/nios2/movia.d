#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 movia

# Test the movia instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00a02074 	movhi	r2,32897
0+0004 <[^>]*> 10a02004 	addi	r2,r2,-32640
0+0008 <[^>]*> 00c00034 	movhi	r3,0
			8: R_NIOS2_HIADJ16	sym-0x80000000
0+000c <[^>]*> 18c00004 	addi	r3,r3,0
			c: R_NIOS2_LO16	sym-0x80000000
0+0010 <[^>]*> 01000034 	movhi	r4,0
			10: R_NIOS2_HIADJ16	sym-0x7fffffff
0+0014 <[^>]*> 21000004 	addi	r4,r4,0
			14: R_NIOS2_LO16	sym-0x7fffffff
0+0018 <[^>]*> 00800034 	movhi	r2,0
0+001c <[^>]*> 10bffc04 	addi	r2,r2,-16
