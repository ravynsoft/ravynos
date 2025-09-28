#name: Valid Armv8.1-M Mainline BF instruction
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f0c0 e803 	bf	2, 0000000a <.*>
0[0-9a-f]+ <[^>]+> 4609      	mov	r1, r1
0[0-9a-f]+ <[^>]+> f140 e801 	bf	4, 0000000c <.*>
0[0-9a-f]+ <[^>]+> 460a      	mov	r2, r1
0[0-9a-f]+ <[^>]+> 4613      	mov	r3, r2
0[0-9a-f]+ <[^>]+> 4614      	mov	r4, r2
