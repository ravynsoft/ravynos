#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 movia
#as: -march=r2
#source: movia.s

# Test the movia instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 80811034 	movhi	r2,32897
0+0004 <[^>]*> 80801084 	addi	r2,r2,-32640
0+0008 <[^>]*> 00001834 	movhi	r3,0
			8: R_NIOS2_HIADJ16	sym-0x80000000
0+000c <[^>]*> 000018c4 	addi	r3,r3,0
			c: R_NIOS2_LO16	sym-0x80000000
0+0010 <[^>]*> 00002034 	movhi	r4,0
			10: R_NIOS2_HIADJ16	sym-0x7fffffff
0+0014 <[^>]*> 00002104 	addi	r4,r4,0
			14: R_NIOS2_LO16	sym-0x7fffffff
0+0018 <[^>]*> 00001034 	movhi	r2,0
0+001c <[^>]*> fff01084 	addi	r2,r2,-16
