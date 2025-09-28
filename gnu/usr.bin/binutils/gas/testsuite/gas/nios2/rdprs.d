#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 rdprs

# Test the rdprs instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00000038 	rdprs	zero,zero,0
0+0004 <[^>]*> 00000078 	rdprs	zero,zero,1
0+0008 <[^>]*> 000000b8 	rdprs	zero,zero,2
0+000c <[^>]*> 00000138 	rdprs	zero,zero,4
0+0010 <[^>]*> 00000238 	rdprs	zero,zero,8
0+0014 <[^>]*> 00000438 	rdprs	zero,zero,16
0+0018 <[^>]*> 00000838 	rdprs	zero,zero,32
0+001c <[^>]*> 00001038 	rdprs	zero,zero,64
0+0020 <[^>]*> 00002038 	rdprs	zero,zero,128
0+0024 <[^>]*> 00004038 	rdprs	zero,zero,256
0+0028 <[^>]*> 00008038 	rdprs	zero,zero,512
0+002c <[^>]*> 00010038 	rdprs	zero,zero,1024
0+0030 <[^>]*> 00020038 	rdprs	zero,zero,2048
0+0034 <[^>]*> 00040038 	rdprs	zero,zero,4096
0+0038 <[^>]*> 00080038 	rdprs	zero,zero,8192
0+003c <[^>]*> 00100038 	rdprs	zero,zero,16384
0+0040 <[^>]*> 00200038 	rdprs	zero,zero,-32768
0+0044 <[^>]*> 00400038 	rdprs	at,zero,0
0+0048 <[^>]*> 00800038 	rdprs	r2,zero,0
0+004c <[^>]*> 01000038 	rdprs	r4,zero,0
0+0050 <[^>]*> 02000038 	rdprs	r8,zero,0
0+0054 <[^>]*> 04000038 	rdprs	r16,zero,0
0+0058 <[^>]*> 08000038 	rdprs	zero,at,0
0+005c <[^>]*> 10000038 	rdprs	zero,r2,0
0+0060 <[^>]*> 20000038 	rdprs	zero,r4,0
0+0064 <[^>]*> 40000038 	rdprs	zero,r8,0
0+0068 <[^>]*> 80000038 	rdprs	zero,r16,0
