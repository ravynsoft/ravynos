# name: ldr - arm
#objdump: -dr --prefix-address --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:
0+00 <[^>]+> e5911005 	ldr	r1, \[r1, #5\]
0+04 <[^>]+> e5b21005 	ldr	r1, \[r2, #5\]!
0+08 <[^>]+> e59f1005 	ldr	r1, \[pc, #5\]	@ 0+15 <[^>]+0x15>
0+0c <[^>]+> e591f005 	ldr	pc, \[r1, #5\]
0+10 <[^>]+> e59ff004 	ldr	pc, \[pc, #4\]	@ 0+1c <[^>]+0x1c>
0+14 <[^>]+> e51ffabc 	ldr	pc, \[pc, #-2748\]	@ fffff560 <[^>]+>
0+18 <[^>]+> e51f1abf 	ldr	r1, \[pc, #-2751\]	@ fffff561 <[^>]+>
0+1c <[^>]+> e7911002 	ldr	r1, \[r1, r2\]
0+20 <[^>]+> e79f2002 	ldr	r2, \[pc, r2\]
0+24 <[^>]+> e7b21003 	ldr	r1, \[r2, r3\]!
0+28 <[^>]+> e791100c 	ldr	r1, \[r1, ip\]
0+2c <[^>]+> e581100a 	str	r1, \[r1, #10\]
0+30 <[^>]+> e58f100a 	str	r1, \[pc, #10\]	@ 0+42 <[^>]+0x42>
0+34 <[^>]+> e5a2100a 	str	r1, \[r2, #10\]!
0+38 <[^>]+> e7811002 	str	r1, \[r1, r2\]
0+3c <[^>]+> e78f1002 	str	r1, \[pc, r2\]
0+40 <[^>]+> e7a21003 	str	r1, \[r2, r3\]!

