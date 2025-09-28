#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 cmp
#as: -march=r2
#source: cmp.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> a00b18a0 	cmpeq	r11,r2,r3
0+0004 <[^>]*> 400b18a0 	cmpge	r11,r2,r3
0+0008 <[^>]*> c00b18a0 	cmpgeu	r11,r2,r3
0+000c <[^>]*> 600b18a0 	cmplt	r11,r2,r3
0+0010 <[^>]*> e00b18a0 	cmpltu	r11,r2,r3
0+0014 <[^>]*> 800b18a0 	cmpne	r11,r2,r3
0+0018 <[^>]*> 00005896 	cmpgei	r11,r2,0
[	]*18: R_NIOS2_S16	value
0+001c <[^>]*> 000058b6 	cmpgeui	r11,r2,0
[	]*1c: R_NIOS2_U16	value\+0x200
0+0020 <[^>]*> 0000589e 	cmplti	r11,r2,0
[	]*20: R_NIOS2_S16	value
0+0024 <[^>]*> 000058be 	cmpltui	r11,r2,0
[	]*24: R_NIOS2_U16	value\+0x200
0+0028 <[^>]*> 7fff5896 	cmpgei	r11,r2,32767
0+002c <[^>]*> 800058b6 	cmpgeui	r11,r2,32768
0+0030 <[^>]*> 8000589e 	cmplti	r11,r2,-32768
0+0034 <[^>]*> ffff58be 	cmpltui	r11,r2,65535
