#name: Valid MVE shift instructions
#source: mve-shift.s
#as: -march=armv8.1-m.main+mve
#objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> ea52 136f 	asrl	r2, r3, #5
0[0-9a-f]+ <[^>]+> ea52 532d 	asrl	r2, r3, r5
0[0-9a-f]+ <[^>]+> ea52 134f 	lsll	r2, r3, #5
0[0-9a-f]+ <[^>]+> ea52 530d 	lsll	r2, r3, r5
0[0-9a-f]+ <[^>]+> ea52 135f 	lsrl	r2, r3, #5
0[0-9a-f]+ <[^>]+> ea53 53ad 	sqrshrl	r2, r3, #48, r5
0[0-9a-f]+ <[^>]+> ea53 532d 	sqrshrl	r2, r3, #64, r5
0[0-9a-f]+ <[^>]+> ea52 5f2d 	sqrshr	r2, r5
0[0-9a-f]+ <[^>]+> ea53 137f 	sqshll	r2, r3, #5
0[0-9a-f]+ <[^>]+> ea52 1f7f 	sqshl	r2, #5
0[0-9a-f]+ <[^>]+> ea53 73ef 	srshrl	r2, r3, #31
0[0-9a-f]+ <[^>]+> ea52 7fef 	srshr	r2, #31
0[0-9a-f]+ <[^>]+> ea53 538d 	uqrshll	r2, r3, #48, r5
0[0-9a-f]+ <[^>]+> ea53 530d 	uqrshll	r2, r3, #64, r5
0[0-9a-f]+ <[^>]+> ea52 5f0d 	uqrshl	r2, r5
0[0-9a-f]+ <[^>]+> ea53 73cf 	uqshll	r2, r3, #31
0[0-9a-f]+ <[^>]+> bfce      	itee	gt
0[0-9a-f]+ <[^>]+> ea52 0f0f 	uqshlgt	r2, #32
0[0-9a-f]+ <[^>]+> ea53 031f 	urshrlle	r2, r3, #32
0[0-9a-f]+ <[^>]+> ea52 0f1f 	urshrle	r2, #32
#...
