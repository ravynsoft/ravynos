#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 mul

# Test the mul macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2989383a 	mul	r4,r5,r6
0+0004 <[^>]*> 29000024 	muli	r4,r5,0
0+0008 <[^>]*> 29000064 	muli	r4,r5,1
0+000c <[^>]*> 29200024 	muli	r4,r5,-32768
0+0010 <[^>]*> 291fffe4 	muli	r4,r5,32767
0+0014 <[^>]*> 29000024 	muli	r4,r5,0
[	]*14: R_NIOS2_S16	undefined_symbol
0+0018 <[^>]*> 29101024 	muli	r4,r5,16448
0+001c <[^>]*> 2988f83a 	mulxss	r4,r5,r6
0+0020 <[^>]*> 2988b83a 	mulxsu	r4,r5,r6
0+0024 <[^>]*> 2988383a 	mulxuu	r4,r5,r6
