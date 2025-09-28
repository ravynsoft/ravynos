#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 rdprs
#as: -march=r2

# Test the rdprs instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 70000028 	rdprs	zero,zero,0
0+0004 <[^>]*> 70010028 	rdprs	zero,zero,1
0+0008 <[^>]*> 70020028 	rdprs	zero,zero,2
0+000c <[^>]*> 70040028 	rdprs	zero,zero,4
0+0010 <[^>]*> 70080028 	rdprs	zero,zero,8
0+0014 <[^>]*> 70100028 	rdprs	zero,zero,16
0+0018 <[^>]*> 70200028 	rdprs	zero,zero,32
0+001c <[^>]*> 70400028 	rdprs	zero,zero,64
0+0020 <[^>]*> 70800028 	rdprs	zero,zero,128
0+0024 <[^>]*> 71000028 	rdprs	zero,zero,256
0+0028 <[^>]*> 72000028 	rdprs	zero,zero,512
0+002c <[^>]*> 74000028 	rdprs	zero,zero,1024
0+0030 <[^>]*> 78000028 	rdprs	zero,zero,-2048
0+0034 <[^>]*> 70000828 	rdprs	at,zero,0
0+0038 <[^>]*> 70001028 	rdprs	r2,zero,0
0+003c <[^>]*> 70002028 	rdprs	r4,zero,0
0+0040 <[^>]*> 70004028 	rdprs	r8,zero,0
0+0044 <[^>]*> 70008028 	rdprs	r16,zero,0
0+0048 <[^>]*> 70000068 	rdprs	zero,at,0
0+004c <[^>]*> 700000a8 	rdprs	zero,r2,0
0+0050 <[^>]*> 70000128 	rdprs	zero,r4,0
0+0054 <[^>]*> 70000228 	rdprs	zero,r8,0
0+0058 <[^>]*> 70000428 	rdprs	zero,r16,0
