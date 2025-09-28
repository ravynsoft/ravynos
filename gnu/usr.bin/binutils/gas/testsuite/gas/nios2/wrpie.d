#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 wrpie
#as: -march=r2

# Test the wrpie instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00000020 	wrpie	zero,zero
0+0004 <[^>]*> 00010020 	wrpie	at,zero
0+0008 <[^>]*> 00020020 	wrpie	r2,zero
0+000c <[^>]*> 00040020 	wrpie	r4,zero
0+0010 <[^>]*> 00080020 	wrpie	r8,zero
0+0014 <[^>]*> 00100020 	wrpie	r16,zero
0+0018 <[^>]*> 00000060 	wrpie	zero,at
0+001c <[^>]*> 000000a0 	wrpie	zero,r2
0+0020 <[^>]*> 00000120 	wrpie	zero,r4
0+0024 <[^>]*> 00000220 	wrpie	zero,r8
0+0028 <[^>]*> 00000420 	wrpie	zero,r16
