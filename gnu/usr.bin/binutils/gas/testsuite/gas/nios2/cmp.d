#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 cmp

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 10d7003a 	cmpeq	r11,r2,r3
0+0004 <[^>]*> 10d6403a 	cmpge	r11,r2,r3
0+0008 <[^>]*> 10d7403a 	cmpgeu	r11,r2,r3
0+000c <[^>]*> 10d6803a 	cmplt	r11,r2,r3
0+0010 <[^>]*> 10d7803a 	cmpltu	r11,r2,r3
0+0014 <[^>]*> 10d6c03a 	cmpne	r11,r2,r3
0+0018 <[^>]*> 12c00008 	cmpgei	r11,r2,0
[	]*18: R_NIOS2_S16	value
0+001c <[^>]*> 12c00028 	cmpgeui	r11,r2,0
[	]*1c: R_NIOS2_U16	value\+0x200
0+0020 <[^>]*> 12c00010 	cmplti	r11,r2,0
[	]*20: R_NIOS2_S16	value
0+0024 <[^>]*> 12c00030 	cmpltui	r11,r2,0
[	]*24: R_NIOS2_U16	value\+0x200
0+0028 <[^>]*> 12dfffc8 	cmpgei	r11,r2,32767
0+002c <[^>]*> 12e00028 	cmpgeui	r11,r2,32768
0+0030 <[^>]*> 12e00010 	cmplti	r11,r2,-32768
0+0034 <[^>]*> 12fffff0 	cmpltui	r11,r2,65535
