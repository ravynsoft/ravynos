#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARMv8-R CRC32 instructions
#source: crc32-armv8-ar.s
#as: -march=armv8-r+crc
#notarget: *-*-pe *-*-wince

.*: *file format .*arm.*


Disassembly of section .text:
0+0 <[^>]*> e1010042 	crc32b	r0, r1, r2
0+4 <[^>]*> e1210042 	crc32h	r0, r1, r2
0+8 <[^>]*> e1410042 	crc32w	r0, r1, r2
0+c <[^>]*> e1010242 	crc32cb	r0, r1, r2
0+10 <[^>]*> e1210242 	crc32ch	r0, r1, r2
0+14 <[^>]*> e1410242 	crc32cw	r0, r1, r2
0+18 <[^>]*> fac1 f082 	crc32b	r0, r1, r2
0+1c <[^>]*> fac1 f092 	crc32h	r0, r1, r2
0+20 <[^>]*> fac1 f0a2 	crc32w	r0, r1, r2
0+24 <[^>]*> fad1 f082 	crc32cb	r0, r1, r2
0+28 <[^>]*> fad1 f092 	crc32ch	r0, r1, r2
0+2c <[^>]*> fad1 f0a2 	crc32cw	r0, r1, r2
0+30 <[^>]*> e101d042 	crc32b	sp, r1, r2
0+34 <[^>]*> e12db042 	crc32h	fp, sp, r2
0+38 <[^>]*> e141004d 	crc32w	r0, r1, sp
0+3c <[^>]*> e10d9242 	crc32cb	r9, sp, r2
0+40 <[^>]*> e121d248 	crc32ch	sp, r1, r8
0+44 <[^>]*> e141a24d 	crc32cw	sl, r1, sp
0+48 <[^>]*> fac1 fc8d 	crc32b	ip, r1, sp
0+4c <[^>]*> facd fa92 	crc32h	r5, sp, r2
0+50 <[^>]*> fac1 fda7 	crc32w	sp, r1, r7
0+54 <[^>]*> fadd f082 	crc32cb	r0, sp, r2
0+58 <[^>]*> fad5 f09d 	crc32ch	r0, r5, sp
0+5c <[^>]*> fad1 fda9 	crc32cw	sp, r1, r9
