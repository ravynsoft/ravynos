# name: MINUS ZERO OFFSET
# as:
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+00 <[^>]+> e51f0000 ?	ldr	r0, \[pc, #-0\]	@ 0+8 <[^>]+>
0+04 <[^>]+> e59f0000 ?	ldr	r0, \[pc\]	@ 0+c <[^>]+>
0+08 <[^>]+> e5110000 ?	ldr	r0, \[r1, #-0\]
0+0c <[^>]+> e5910000 ?	ldr	r0, \[r1\]
0+10 <[^>]+> e4110000 ?	ldr	r0, \[r1\], #-0
0+14 <[^>]+> e4910000 ?	ldr	r0, \[r1\], #0
0+18 <[^>]+> e15f00b0 ?	ldrh	r0, \[pc, #-0\]	@ 0+20 <[^>]+>
0+1c <[^>]+> e1df00b0 ?	ldrh	r0, \[pc\]	@ 0+24 <[^>]+>
0+20 <[^>]+> e15100b0 ?	ldrh	r0, \[r1, #-0\]
0+24 <[^>]+> e1d100b0 ?	ldrh	r0, \[r1\]
0+28 <[^>]+> e05100b0 ?	ldrh	r0, \[r1\], #-0
0+2c <[^>]+> e0d100b0 ?	ldrh	r0, \[r1\], #0
0+30 <[^>]+> e5310000 ?	ldr	r0, \[r1, #-0\]!
0+34 <[^>]+> e5b10000 ?	ldr	r0, \[r1, #0\]!
0+38 <[^>]+> e17100b0 ?	ldrh	r0, \[r1, #-0\]!
0+3c <[^>]+> e1f100b0 ?	ldrh	r0, \[r1, #0\]!
