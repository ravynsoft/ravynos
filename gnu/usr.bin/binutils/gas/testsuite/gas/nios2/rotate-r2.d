#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 rotate
#source: rotate.s
#as: -march=r2

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0c042120 	rol	r4,r4,r4
0+0004 <[^>]*> 0be40120 	roli	r4,r4,31
0+0008 <[^>]*> 2c042120 	ror	r4,r4,r4
0+000c <[^>]*> 4c042120 	sll	r4,r4,r4
0+0010 <[^>]*> 4b040120 	slli	r4,r4,24
0+0014 <[^>]*> ec042120 	sra	r4,r4,r4
0+0018 <[^>]*> e9440120 	srai	r4,r4,10
0+001c <[^>]*> 6c042120 	srl	r4,r4,r4
0+0020 <[^>]*> 68a40120 	srli	r4,r4,5
