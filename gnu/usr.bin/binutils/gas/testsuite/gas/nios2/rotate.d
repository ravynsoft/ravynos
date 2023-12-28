#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 rotate

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2108183a 	rol	r4,r4,r4
0+0004 <[^>]*> 200817fa 	roli	r4,r4,31
0+0008 <[^>]*> 2108583a 	ror	r4,r4,r4
0+000c <[^>]*> 2108983a 	sll	r4,r4,r4
0+0010 <[^>]*> 2008963a 	slli	r4,r4,24
0+0014 <[^>]*> 2109d83a 	sra	r4,r4,r4
0+0018 <[^>]*> 2009d2ba 	srai	r4,r4,10
0+001c <[^>]*> 2108d83a 	srl	r4,r4,r4
0+0020 <[^>]*> 2008d17a 	srli	r4,r4,5
