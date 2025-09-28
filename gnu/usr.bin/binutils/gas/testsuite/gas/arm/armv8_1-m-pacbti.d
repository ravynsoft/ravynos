#name: Valid Armv8.1-M pointer authentication and branch target identification extention
#source: armv8_1-m-pacbti.s
#as: -march=armv8.1-m.main+pacbti
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f3af 800f 	bti
0[0-9a-f]+ <[^>]+> f3af 800d 	pacbti	r12, lr, sp
0[0-9a-f]+ <[^>]+> f3af 802d 	aut	r12, lr, sp
0[0-9a-f]+ <[^>]+> f3af 801d 	pac	r12, lr, sp
0[0-9a-f]+ <[^>]+> fb54 3f15 	bxaut	r3, r4, r5
0[0-9a-f]+ <[^>]+> fb54 3f05 	autg	r3, r4, r5
0[0-9a-f]+ <[^>]+> fb64 f305 	pacg	r3, r4, r5
#...
