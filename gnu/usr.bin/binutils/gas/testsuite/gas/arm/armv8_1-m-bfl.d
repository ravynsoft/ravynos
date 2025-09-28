#name: Valid Armv8.1-M Mainline BFL instruction
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f080 c803 	bfl	2, 0000000a <.*>
0[0-9a-f]+ <[^>]+> 4608      	mov	r0, r1
0[0-9a-f]+ <[^>]+> f100 c801 	bfl	4, 0000000c <.*>
0[0-9a-f]+ <[^>]+> 460a      	mov	r2, r1
0[0-9a-f]+ <[^>]+> 4613      	mov	r3, r2
0[0-9a-f]+ <[^>]+> 4614      	mov	r4, r2
