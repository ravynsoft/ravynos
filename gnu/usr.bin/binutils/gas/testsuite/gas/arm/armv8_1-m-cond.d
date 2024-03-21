#name: Valid Armv8.1-M Mainline conditional instructions
#source: armv8_1-m-cond.s
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> ea5f 940f 	cset	r4, ne
0[0-9a-f]+ <[^>]+> ea5f a40f 	csetm	r4, ne
0[0-9a-f]+ <[^>]+> ea52 93a2 	cinc	r3, r2, lt
0[0-9a-f]+ <[^>]+> ea52 a3a2 	cinv	r3, r2, lt
0[0-9a-f]+ <[^>]+> ea52 b3a2 	cneg	r3, r2, lt
0[0-9a-f]+ <[^>]+> ea52 93b4 	csinc	r3, r2, r4, lt
0[0-9a-f]+ <[^>]+> ea54 93b4 	cinc	r3, r4, ge
0[0-9a-f]+ <[^>]+> ea5f 93bf 	cset	r3, ge
0[0-9a-f]+ <[^>]+> ea52 a3b4 	csinv	r3, r2, r4, lt
0[0-9a-f]+ <[^>]+> ea54 a3b4 	cinv	r3, r4, ge
0[0-9a-f]+ <[^>]+> ea5f a3bf 	csetm	r3, ge
0[0-9a-f]+ <[^>]+> ea52 b3b4 	csneg	r3, r2, r4, lt
0[0-9a-f]+ <[^>]+> ea54 b3b4 	cneg	r3, r4, ge
