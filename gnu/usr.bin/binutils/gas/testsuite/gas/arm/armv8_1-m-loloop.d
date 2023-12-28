#name: Valid Armv8.1-M Mainline Low Overhead loop instructions
#source: armv8_1-m-loloop.s
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f042 c00d 	wls	lr, r2, 0000001c <.*>
0[0-9a-f]+ <[^>]+> f042 e001 	dls	lr, r2
0[0-9a-f]+ <[^>]+> f04e e001 	dls	lr, lr
0[0-9a-f]+ <[^>]+> f00f c009 	le	lr, 00000000 <.*>
0[0-9a-f]+ <[^>]+> f02f c00b 	le	00000000 <.*>
0[0-9a-f]+ <[^>]+> f00f c24b 	le	lr, fffffb84 <.*>
0[0-9a-f]+ <[^>]+> f02f c007 	le	00000010 <.*>
0[0-9a-f]+ <[^>]+> 4613      	mov	r3, r2
#...
