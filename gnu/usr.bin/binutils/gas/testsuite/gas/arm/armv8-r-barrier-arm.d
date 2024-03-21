#name: Valid v8-R barrier (ARM)
#as: -march=armv8-r
#source: armv8-ar-barrier.s
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f57ff04d 	dsb	ld
0[0-9a-f]+ <[^>]+> f57ff049 	dsb	ishld
0[0-9a-f]+ <[^>]+> f57ff045 	dsb	nshld
0[0-9a-f]+ <[^>]+> f57ff041 	dsb	oshld
0[0-9a-f]+ <[^>]+> f57ff05d 	dmb	ld
0[0-9a-f]+ <[^>]+> f57ff059 	dmb	ishld
0[0-9a-f]+ <[^>]+> f57ff055 	dmb	nshld
0[0-9a-f]+ <[^>]+> f57ff051 	dmb	oshld
0[0-9a-f]+ <[^>]+> f57ff04d 	dsb	ld
0[0-9a-f]+ <[^>]+> f57ff049 	dsb	ishld
0[0-9a-f]+ <[^>]+> f57ff045 	dsb	nshld
0[0-9a-f]+ <[^>]+> f57ff041 	dsb	oshld
0[0-9a-f]+ <[^>]+> f57ff05d 	dmb	ld
0[0-9a-f]+ <[^>]+> f57ff059 	dmb	ishld
0[0-9a-f]+ <[^>]+> f57ff055 	dmb	nshld
0[0-9a-f]+ <[^>]+> f57ff051 	dmb	oshld
