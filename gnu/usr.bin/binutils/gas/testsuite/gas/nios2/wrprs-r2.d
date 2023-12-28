#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 wrprs
#as: -march=r2
#source: wrprs.s

# Test the wrprs instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 50000020 	wrprs	zero,zero
0+0004 <[^>]*> 50010020 	wrprs	at,zero
0+0008 <[^>]*> 50020020 	wrprs	r2,zero
0+000c <[^>]*> 50040020 	wrprs	r4,zero
0+0010 <[^>]*> 50080020 	wrprs	r8,zero
0+0014 <[^>]*> 50100020 	wrprs	r16,zero
0+0018 <[^>]*> 50000060 	wrprs	zero,at
0+001c <[^>]*> 500000a0 	wrprs	zero,r2
0+0020 <[^>]*> 50000120 	wrprs	zero,r4
0+0024 <[^>]*> 50000220 	wrprs	zero,r8
0+0028 <[^>]*> 50000420 	wrprs	zero,r16
