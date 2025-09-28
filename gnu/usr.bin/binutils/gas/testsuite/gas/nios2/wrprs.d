#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 wrprs

# Test the wrprs instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0000a03a 	wrprs	zero,zero
0+0004 <[^>]*> 0002a03a 	wrprs	at,zero
0+0008 <[^>]*> 0004a03a 	wrprs	r2,zero
0+000c <[^>]*> 0008a03a 	wrprs	r4,zero
0+0010 <[^>]*> 0010a03a 	wrprs	r8,zero
0+0014 <[^>]*> 0020a03a 	wrprs	r16,zero
0+0018 <[^>]*> 0800a03a 	wrprs	zero,at
0+001c <[^>]*> 1000a03a 	wrprs	zero,r2
0+0020 <[^>]*> 2000a03a 	wrprs	zero,r4
0+0024 <[^>]*> 4000a03a 	wrprs	zero,r8
0+0028 <[^>]*> 8000a03a 	wrprs	zero,r16
