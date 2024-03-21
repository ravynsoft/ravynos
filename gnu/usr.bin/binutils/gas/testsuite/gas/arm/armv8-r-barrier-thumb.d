#name: Valid v8-R barrier (Thumb)
#as: -march=armv8-r -mthumb
#source: armv8-ar-barrier.s
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f3bf 8f4d 	dsb	ld
0[0-9a-f]+ <[^>]+> f3bf 8f49 	dsb	ishld
0[0-9a-f]+ <[^>]+> f3bf 8f45 	dsb	nshld
0[0-9a-f]+ <[^>]+> f3bf 8f41 	dsb	oshld
0[0-9a-f]+ <[^>]+> f3bf 8f5d 	dmb	ld
0[0-9a-f]+ <[^>]+> f3bf 8f59 	dmb	ishld
0[0-9a-f]+ <[^>]+> f3bf 8f55 	dmb	nshld
0[0-9a-f]+ <[^>]+> f3bf 8f51 	dmb	oshld
0[0-9a-f]+ <[^>]+> f3bf 8f4d 	dsb	ld
0[0-9a-f]+ <[^>]+> f3bf 8f49 	dsb	ishld
0[0-9a-f]+ <[^>]+> f3bf 8f45 	dsb	nshld
0[0-9a-f]+ <[^>]+> f3bf 8f41 	dsb	oshld
0[0-9a-f]+ <[^>]+> f3bf 8f5d 	dmb	ld
0[0-9a-f]+ <[^>]+> f3bf 8f59 	dmb	ishld
0[0-9a-f]+ <[^>]+> f3bf 8f55 	dmb	nshld
0[0-9a-f]+ <[^>]+> f3bf 8f51 	dmb	oshld
