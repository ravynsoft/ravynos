#name: Valid Armv8.1-M Mainline BF-exchange instructions
#source: armv8_1-m-bf-exchange.s
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f265 e001 	bfx	8, r5
0[0-9a-f]+ <[^>]+> 4609      	mov	r1, r1
0[0-9a-f]+ <[^>]+> f173 e001 	bflx	4, r3
0[0-9a-f]+ <[^>]+> 460a      	mov	r2, r1
0[0-9a-f]+ <[^>]+> 4613      	mov	r3, r2
0[0-9a-f]+ <[^>]+> 4614      	mov	r4, r2
