#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 mul
#as: -march=r2
#source: mul.s

# Test the mul macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 9c043160 	mul	r4,r5,r6
0+0004 <[^>]*> 00002164 	muli	r4,r5,0
0+0008 <[^>]*> 00012164 	muli	r4,r5,1
0+000c <[^>]*> 80002164 	muli	r4,r5,-32768
0+0010 <[^>]*> 7fff2164 	muli	r4,r5,32767
0+0014 <[^>]*> 00002164 	muli	r4,r5,0
[	]*14: R_NIOS2_S16	undefined_symbol
0+0018 <[^>]*> 40402164 	muli	r4,r5,16448
0+001c <[^>]*> 7c043160 	mulxss	r4,r5,r6
0+0020 <[^>]*> 5c043160 	mulxsu	r4,r5,r6
0+0024 <[^>]*> 1c043160 	mulxuu	r4,r5,r6
