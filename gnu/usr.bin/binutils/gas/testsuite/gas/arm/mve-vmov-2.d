# name: MVE vmov instructions (Integer & FP)
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ee00 0a10 	vmov	s0, r0
[^>]*> ee10 0a10 	vmov	r0, s0
[^>]*> ee00 1a10 	vmov	s0, r1
[^>]*> ee10 1a10 	vmov	r1, s0
[^>]*> ee00 2a10 	vmov	s0, r2
[^>]*> ee10 2a10 	vmov	r2, s0
[^>]*> ee00 4a10 	vmov	s0, r4
[^>]*> ee10 4a10 	vmov	r4, s0
[^>]*> ee00 7a10 	vmov	s0, r7
[^>]*> ee10 7a10 	vmov	r7, s0
[^>]*> ee00 8a10 	vmov	s0, r8
[^>]*> ee10 8a10 	vmov	r8, s0
[^>]*> ee00 aa10 	vmov	s0, sl
[^>]*> ee10 aa10 	vmov	sl, s0
[^>]*> ee00 ca10 	vmov	s0, ip
[^>]*> ee10 ca10 	vmov	ip, s0
[^>]*> ee00 ea10 	vmov	s0, lr
[^>]*> ee10 ea10 	vmov	lr, s0
[^>]*> ee00 0a90 	vmov	s1, r0
[^>]*> ee10 0a90 	vmov	r0, s1
[^>]*> ee00 1a90 	vmov	s1, r1
[^>]*> ee10 1a90 	vmov	r1, s1
[^>]*> ee00 2a90 	vmov	s1, r2
[^>]*> ee10 2a90 	vmov	r2, s1
[^>]*> ee00 4a90 	vmov	s1, r4
[^>]*> ee10 4a90 	vmov	r4, s1
[^>]*> ee00 7a90 	vmov	s1, r7
[^>]*> ee10 7a90 	vmov	r7, s1
[^>]*> ee00 8a90 	vmov	s1, r8
[^>]*> ee10 8a90 	vmov	r8, s1
[^>]*> ee00 aa90 	vmov	s1, sl
[^>]*> ee10 aa90 	vmov	sl, s1
[^>]*> ee00 ca90 	vmov	s1, ip
[^>]*> ee10 ca90 	vmov	ip, s1
[^>]*> ee00 ea90 	vmov	s1, lr
[^>]*> ee10 ea90 	vmov	lr, s1
[^>]*> ee01 0a10 	vmov	s2, r0
[^>]*> ee11 0a10 	vmov	r0, s2
[^>]*> ee01 1a10 	vmov	s2, r1
[^>]*> ee11 1a10 	vmov	r1, s2
[^>]*> ee01 2a10 	vmov	s2, r2
[^>]*> ee11 2a10 	vmov	r2, s2
[^>]*> ee01 4a10 	vmov	s2, r4
[^>]*> ee11 4a10 	vmov	r4, s2
[^>]*> ee01 7a10 	vmov	s2, r7
[^>]*> ee11 7a10 	vmov	r7, s2
[^>]*> ee01 8a10 	vmov	s2, r8
[^>]*> ee11 8a10 	vmov	r8, s2
[^>]*> ee01 aa10 	vmov	s2, sl
[^>]*> ee11 aa10 	vmov	sl, s2
[^>]*> ee01 ca10 	vmov	s2, ip
[^>]*> ee11 ca10 	vmov	ip, s2
[^>]*> ee01 ea10 	vmov	s2, lr
[^>]*> ee11 ea10 	vmov	lr, s2
[^>]*> ee02 0a10 	vmov	s4, r0
[^>]*> ee12 0a10 	vmov	r0, s4
[^>]*> ee02 1a10 	vmov	s4, r1
[^>]*> ee12 1a10 	vmov	r1, s4
[^>]*> ee02 2a10 	vmov	s4, r2
[^>]*> ee12 2a10 	vmov	r2, s4
[^>]*> ee02 4a10 	vmov	s4, r4
[^>]*> ee12 4a10 	vmov	r4, s4
[^>]*> ee02 7a10 	vmov	s4, r7
[^>]*> ee12 7a10 	vmov	r7, s4
[^>]*> ee02 8a10 	vmov	s4, r8
[^>]*> ee12 8a10 	vmov	r8, s4
[^>]*> ee02 aa10 	vmov	s4, sl
[^>]*> ee12 aa10 	vmov	sl, s4
[^>]*> ee02 ca10 	vmov	s4, ip
[^>]*> ee12 ca10 	vmov	ip, s4
[^>]*> ee02 ea10 	vmov	s4, lr
[^>]*> ee12 ea10 	vmov	lr, s4
[^>]*> ee04 0a10 	vmov	s8, r0
[^>]*> ee14 0a10 	vmov	r0, s8
[^>]*> ee04 1a10 	vmov	s8, r1
[^>]*> ee14 1a10 	vmov	r1, s8
[^>]*> ee04 2a10 	vmov	s8, r2
[^>]*> ee14 2a10 	vmov	r2, s8
[^>]*> ee04 4a10 	vmov	s8, r4
[^>]*> ee14 4a10 	vmov	r4, s8
[^>]*> ee04 7a10 	vmov	s8, r7
[^>]*> ee14 7a10 	vmov	r7, s8
[^>]*> ee04 8a10 	vmov	s8, r8
[^>]*> ee14 8a10 	vmov	r8, s8
[^>]*> ee04 aa10 	vmov	s8, sl
[^>]*> ee14 aa10 	vmov	sl, s8
[^>]*> ee04 ca10 	vmov	s8, ip
[^>]*> ee14 ca10 	vmov	ip, s8
[^>]*> ee04 ea10 	vmov	s8, lr
[^>]*> ee14 ea10 	vmov	lr, s8
[^>]*> ee08 0a10 	vmov	s16, r0
[^>]*> ee18 0a10 	vmov	r0, s16
[^>]*> ee08 1a10 	vmov	s16, r1
[^>]*> ee18 1a10 	vmov	r1, s16
[^>]*> ee08 2a10 	vmov	s16, r2
[^>]*> ee18 2a10 	vmov	r2, s16
[^>]*> ee08 4a10 	vmov	s16, r4
[^>]*> ee18 4a10 	vmov	r4, s16
[^>]*> ee08 7a10 	vmov	s16, r7
[^>]*> ee18 7a10 	vmov	r7, s16
[^>]*> ee08 8a10 	vmov	s16, r8
[^>]*> ee18 8a10 	vmov	r8, s16
[^>]*> ee08 aa10 	vmov	s16, sl
[^>]*> ee18 aa10 	vmov	sl, s16
[^>]*> ee08 ca10 	vmov	s16, ip
[^>]*> ee18 ca10 	vmov	ip, s16
[^>]*> ee08 ea10 	vmov	s16, lr
[^>]*> ee18 ea10 	vmov	lr, s16
[^>]*> ee0f 0a10 	vmov	s30, r0
[^>]*> ee1f 0a10 	vmov	r0, s30
[^>]*> ee0f 1a10 	vmov	s30, r1
[^>]*> ee1f 1a10 	vmov	r1, s30
[^>]*> ee0f 2a10 	vmov	s30, r2
[^>]*> ee1f 2a10 	vmov	r2, s30
[^>]*> ee0f 4a10 	vmov	s30, r4
[^>]*> ee1f 4a10 	vmov	r4, s30
[^>]*> ee0f 7a10 	vmov	s30, r7
[^>]*> ee1f 7a10 	vmov	r7, s30
[^>]*> ee0f 8a10 	vmov	s30, r8
[^>]*> ee1f 8a10 	vmov	r8, s30
[^>]*> ee0f aa10 	vmov	s30, sl
[^>]*> ee1f aa10 	vmov	sl, s30
[^>]*> ee0f ca10 	vmov	s30, ip
[^>]*> ee1f ca10 	vmov	ip, s30
[^>]*> ee0f ea10 	vmov	s30, lr
[^>]*> ee1f ea10 	vmov	lr, s30
[^>]*> ee0f 0a90 	vmov	s31, r0
[^>]*> ee1f 0a90 	vmov	r0, s31
[^>]*> ee0f 1a90 	vmov	s31, r1
[^>]*> ee1f 1a90 	vmov	r1, s31
[^>]*> ee0f 2a90 	vmov	s31, r2
[^>]*> ee1f 2a90 	vmov	r2, s31
[^>]*> ee0f 4a90 	vmov	s31, r4
[^>]*> ee1f 4a90 	vmov	r4, s31
[^>]*> ee0f 7a90 	vmov	s31, r7
[^>]*> ee1f 7a90 	vmov	r7, s31
[^>]*> ee0f 8a90 	vmov	s31, r8
[^>]*> ee1f 8a90 	vmov	r8, s31
[^>]*> ee0f aa90 	vmov	s31, sl
[^>]*> ee1f aa90 	vmov	sl, s31
[^>]*> ee0f ca90 	vmov	s31, ip
[^>]*> ee1f ca90 	vmov	ip, s31
[^>]*> ee0f ea90 	vmov	s31, lr
[^>]*> ee1f ea90 	vmov	lr, s31
[^>]*> ec51 0b10 	vmov	r0, r1, d0
[^>]*> ec41 0b10 	vmov	d0, r0, r1
[^>]*> ec51 0b11 	vmov	r0, r1, d1
[^>]*> ec41 0b11 	vmov	d1, r0, r1
[^>]*> ec51 0b12 	vmov	r0, r1, d2
[^>]*> ec41 0b12 	vmov	d2, r0, r1
[^>]*> ec51 0b14 	vmov	r0, r1, d4
[^>]*> ec41 0b14 	vmov	d4, r0, r1
[^>]*> ec51 0b18 	vmov	r0, r1, d8
[^>]*> ec41 0b18 	vmov	d8, r0, r1
[^>]*> ec51 0b1f 	vmov	r0, r1, d15
[^>]*> ec41 0b1f 	vmov	d15, r0, r1
[^>]*> ec51 0a10 	vmov	r0, r1, s0, s1
[^>]*> ec51 0a30 	vmov	r0, r1, s1, s2
[^>]*> ec51 0a11 	vmov	r0, r1, s2, s3
[^>]*> ec51 0a12 	vmov	r0, r1, s4, s5
[^>]*> ec51 0a14 	vmov	r0, r1, s8, s9
[^>]*> ec51 0a18 	vmov	r0, r1, s16, s17
[^>]*> ec51 0a1f 	vmov	r0, r1, s30, s31
[^>]*> ec41 0a10 	vmov	s0, s1, r0, r1
[^>]*> ec41 0a30 	vmov	s1, s2, r0, r1
[^>]*> ec41 0a11 	vmov	s2, s3, r0, r1
[^>]*> ec41 0a12 	vmov	s4, s5, r0, r1
[^>]*> ec41 0a14 	vmov	s8, s9, r0, r1
[^>]*> ec41 0a18 	vmov	s16, s17, r0, r1
[^>]*> ec41 0a1f 	vmov	s30, s31, r0, r1
[^>]*> ec52 0b10 	vmov	r0, r2, d0
[^>]*> ec42 0b10 	vmov	d0, r0, r2
[^>]*> ec52 0b11 	vmov	r0, r2, d1
[^>]*> ec42 0b11 	vmov	d1, r0, r2
[^>]*> ec52 0b12 	vmov	r0, r2, d2
[^>]*> ec42 0b12 	vmov	d2, r0, r2
[^>]*> ec52 0b14 	vmov	r0, r2, d4
[^>]*> ec42 0b14 	vmov	d4, r0, r2
[^>]*> ec52 0b18 	vmov	r0, r2, d8
[^>]*> ec42 0b18 	vmov	d8, r0, r2
[^>]*> ec52 0b1f 	vmov	r0, r2, d15
[^>]*> ec42 0b1f 	vmov	d15, r0, r2
[^>]*> ec52 0a10 	vmov	r0, r2, s0, s1
[^>]*> ec52 0a30 	vmov	r0, r2, s1, s2
[^>]*> ec52 0a11 	vmov	r0, r2, s2, s3
[^>]*> ec52 0a12 	vmov	r0, r2, s4, s5
[^>]*> ec52 0a14 	vmov	r0, r2, s8, s9
[^>]*> ec52 0a18 	vmov	r0, r2, s16, s17
[^>]*> ec52 0a1f 	vmov	r0, r2, s30, s31
[^>]*> ec42 0a10 	vmov	s0, s1, r0, r2
[^>]*> ec42 0a30 	vmov	s1, s2, r0, r2
[^>]*> ec42 0a11 	vmov	s2, s3, r0, r2
[^>]*> ec42 0a12 	vmov	s4, s5, r0, r2
[^>]*> ec42 0a14 	vmov	s8, s9, r0, r2
[^>]*> ec42 0a18 	vmov	s16, s17, r0, r2
[^>]*> ec42 0a1f 	vmov	s30, s31, r0, r2
[^>]*> ec54 0b10 	vmov	r0, r4, d0
[^>]*> ec44 0b10 	vmov	d0, r0, r4
[^>]*> ec54 0b11 	vmov	r0, r4, d1
[^>]*> ec44 0b11 	vmov	d1, r0, r4
[^>]*> ec54 0b12 	vmov	r0, r4, d2
[^>]*> ec44 0b12 	vmov	d2, r0, r4
[^>]*> ec54 0b14 	vmov	r0, r4, d4
[^>]*> ec44 0b14 	vmov	d4, r0, r4
[^>]*> ec54 0b18 	vmov	r0, r4, d8
[^>]*> ec44 0b18 	vmov	d8, r0, r4
[^>]*> ec54 0b1f 	vmov	r0, r4, d15
[^>]*> ec44 0b1f 	vmov	d15, r0, r4
[^>]*> ec54 0a10 	vmov	r0, r4, s0, s1
[^>]*> ec54 0a30 	vmov	r0, r4, s1, s2
[^>]*> ec54 0a11 	vmov	r0, r4, s2, s3
[^>]*> ec54 0a12 	vmov	r0, r4, s4, s5
[^>]*> ec54 0a14 	vmov	r0, r4, s8, s9
[^>]*> ec54 0a18 	vmov	r0, r4, s16, s17
[^>]*> ec54 0a1f 	vmov	r0, r4, s30, s31
[^>]*> ec44 0a10 	vmov	s0, s1, r0, r4
[^>]*> ec44 0a30 	vmov	s1, s2, r0, r4
[^>]*> ec44 0a11 	vmov	s2, s3, r0, r4
[^>]*> ec44 0a12 	vmov	s4, s5, r0, r4
[^>]*> ec44 0a14 	vmov	s8, s9, r0, r4
[^>]*> ec44 0a18 	vmov	s16, s17, r0, r4
[^>]*> ec44 0a1f 	vmov	s30, s31, r0, r4
[^>]*> ec57 0b10 	vmov	r0, r7, d0
[^>]*> ec47 0b10 	vmov	d0, r0, r7
[^>]*> ec57 0b11 	vmov	r0, r7, d1
[^>]*> ec47 0b11 	vmov	d1, r0, r7
[^>]*> ec57 0b12 	vmov	r0, r7, d2
[^>]*> ec47 0b12 	vmov	d2, r0, r7
[^>]*> ec57 0b14 	vmov	r0, r7, d4
[^>]*> ec47 0b14 	vmov	d4, r0, r7
[^>]*> ec57 0b18 	vmov	r0, r7, d8
[^>]*> ec47 0b18 	vmov	d8, r0, r7
[^>]*> ec57 0b1f 	vmov	r0, r7, d15
[^>]*> ec47 0b1f 	vmov	d15, r0, r7
[^>]*> ec57 0a10 	vmov	r0, r7, s0, s1
[^>]*> ec57 0a30 	vmov	r0, r7, s1, s2
[^>]*> ec57 0a11 	vmov	r0, r7, s2, s3
[^>]*> ec57 0a12 	vmov	r0, r7, s4, s5
[^>]*> ec57 0a14 	vmov	r0, r7, s8, s9
[^>]*> ec57 0a18 	vmov	r0, r7, s16, s17
[^>]*> ec57 0a1f 	vmov	r0, r7, s30, s31
[^>]*> ec47 0a10 	vmov	s0, s1, r0, r7
[^>]*> ec47 0a30 	vmov	s1, s2, r0, r7
[^>]*> ec47 0a11 	vmov	s2, s3, r0, r7
[^>]*> ec47 0a12 	vmov	s4, s5, r0, r7
[^>]*> ec47 0a14 	vmov	s8, s9, r0, r7
[^>]*> ec47 0a18 	vmov	s16, s17, r0, r7
[^>]*> ec47 0a1f 	vmov	s30, s31, r0, r7
[^>]*> ec58 0b10 	vmov	r0, r8, d0
[^>]*> ec48 0b10 	vmov	d0, r0, r8
[^>]*> ec58 0b11 	vmov	r0, r8, d1
[^>]*> ec48 0b11 	vmov	d1, r0, r8
[^>]*> ec58 0b12 	vmov	r0, r8, d2
[^>]*> ec48 0b12 	vmov	d2, r0, r8
[^>]*> ec58 0b14 	vmov	r0, r8, d4
[^>]*> ec48 0b14 	vmov	d4, r0, r8
[^>]*> ec58 0b18 	vmov	r0, r8, d8
[^>]*> ec48 0b18 	vmov	d8, r0, r8
[^>]*> ec58 0b1f 	vmov	r0, r8, d15
[^>]*> ec48 0b1f 	vmov	d15, r0, r8
[^>]*> ec58 0a10 	vmov	r0, r8, s0, s1
[^>]*> ec58 0a30 	vmov	r0, r8, s1, s2
[^>]*> ec58 0a11 	vmov	r0, r8, s2, s3
[^>]*> ec58 0a12 	vmov	r0, r8, s4, s5
[^>]*> ec58 0a14 	vmov	r0, r8, s8, s9
[^>]*> ec58 0a18 	vmov	r0, r8, s16, s17
[^>]*> ec58 0a1f 	vmov	r0, r8, s30, s31
[^>]*> ec48 0a10 	vmov	s0, s1, r0, r8
[^>]*> ec48 0a30 	vmov	s1, s2, r0, r8
[^>]*> ec48 0a11 	vmov	s2, s3, r0, r8
[^>]*> ec48 0a12 	vmov	s4, s5, r0, r8
[^>]*> ec48 0a14 	vmov	s8, s9, r0, r8
[^>]*> ec48 0a18 	vmov	s16, s17, r0, r8
[^>]*> ec48 0a1f 	vmov	s30, s31, r0, r8
[^>]*> ec5a 0b10 	vmov	r0, sl, d0
[^>]*> ec4a 0b10 	vmov	d0, r0, sl
[^>]*> ec5a 0b11 	vmov	r0, sl, d1
[^>]*> ec4a 0b11 	vmov	d1, r0, sl
[^>]*> ec5a 0b12 	vmov	r0, sl, d2
[^>]*> ec4a 0b12 	vmov	d2, r0, sl
[^>]*> ec5a 0b14 	vmov	r0, sl, d4
[^>]*> ec4a 0b14 	vmov	d4, r0, sl
[^>]*> ec5a 0b18 	vmov	r0, sl, d8
[^>]*> ec4a 0b18 	vmov	d8, r0, sl
[^>]*> ec5a 0b1f 	vmov	r0, sl, d15
[^>]*> ec4a 0b1f 	vmov	d15, r0, sl
[^>]*> ec5a 0a10 	vmov	r0, sl, s0, s1
[^>]*> ec5a 0a30 	vmov	r0, sl, s1, s2
[^>]*> ec5a 0a11 	vmov	r0, sl, s2, s3
[^>]*> ec5a 0a12 	vmov	r0, sl, s4, s5
[^>]*> ec5a 0a14 	vmov	r0, sl, s8, s9
[^>]*> ec5a 0a18 	vmov	r0, sl, s16, s17
[^>]*> ec5a 0a1f 	vmov	r0, sl, s30, s31
[^>]*> ec4a 0a10 	vmov	s0, s1, r0, sl
[^>]*> ec4a 0a30 	vmov	s1, s2, r0, sl
[^>]*> ec4a 0a11 	vmov	s2, s3, r0, sl
[^>]*> ec4a 0a12 	vmov	s4, s5, r0, sl
[^>]*> ec4a 0a14 	vmov	s8, s9, r0, sl
[^>]*> ec4a 0a18 	vmov	s16, s17, r0, sl
[^>]*> ec4a 0a1f 	vmov	s30, s31, r0, sl
[^>]*> ec5c 0b10 	vmov	r0, ip, d0
[^>]*> ec4c 0b10 	vmov	d0, r0, ip
[^>]*> ec5c 0b11 	vmov	r0, ip, d1
[^>]*> ec4c 0b11 	vmov	d1, r0, ip
[^>]*> ec5c 0b12 	vmov	r0, ip, d2
[^>]*> ec4c 0b12 	vmov	d2, r0, ip
[^>]*> ec5c 0b14 	vmov	r0, ip, d4
[^>]*> ec4c 0b14 	vmov	d4, r0, ip
[^>]*> ec5c 0b18 	vmov	r0, ip, d8
[^>]*> ec4c 0b18 	vmov	d8, r0, ip
[^>]*> ec5c 0b1f 	vmov	r0, ip, d15
[^>]*> ec4c 0b1f 	vmov	d15, r0, ip
[^>]*> ec5c 0a10 	vmov	r0, ip, s0, s1
[^>]*> ec5c 0a30 	vmov	r0, ip, s1, s2
[^>]*> ec5c 0a11 	vmov	r0, ip, s2, s3
[^>]*> ec5c 0a12 	vmov	r0, ip, s4, s5
[^>]*> ec5c 0a14 	vmov	r0, ip, s8, s9
[^>]*> ec5c 0a18 	vmov	r0, ip, s16, s17
[^>]*> ec5c 0a1f 	vmov	r0, ip, s30, s31
[^>]*> ec4c 0a10 	vmov	s0, s1, r0, ip
[^>]*> ec4c 0a30 	vmov	s1, s2, r0, ip
[^>]*> ec4c 0a11 	vmov	s2, s3, r0, ip
[^>]*> ec4c 0a12 	vmov	s4, s5, r0, ip
[^>]*> ec4c 0a14 	vmov	s8, s9, r0, ip
[^>]*> ec4c 0a18 	vmov	s16, s17, r0, ip
[^>]*> ec4c 0a1f 	vmov	s30, s31, r0, ip
[^>]*> ec5e 0b10 	vmov	r0, lr, d0
[^>]*> ec4e 0b10 	vmov	d0, r0, lr
[^>]*> ec5e 0b11 	vmov	r0, lr, d1
[^>]*> ec4e 0b11 	vmov	d1, r0, lr
[^>]*> ec5e 0b12 	vmov	r0, lr, d2
[^>]*> ec4e 0b12 	vmov	d2, r0, lr
[^>]*> ec5e 0b14 	vmov	r0, lr, d4
[^>]*> ec4e 0b14 	vmov	d4, r0, lr
[^>]*> ec5e 0b18 	vmov	r0, lr, d8
[^>]*> ec4e 0b18 	vmov	d8, r0, lr
[^>]*> ec5e 0b1f 	vmov	r0, lr, d15
[^>]*> ec4e 0b1f 	vmov	d15, r0, lr
[^>]*> ec5e 0a10 	vmov	r0, lr, s0, s1
[^>]*> ec5e 0a30 	vmov	r0, lr, s1, s2
[^>]*> ec5e 0a11 	vmov	r0, lr, s2, s3
[^>]*> ec5e 0a12 	vmov	r0, lr, s4, s5
[^>]*> ec5e 0a14 	vmov	r0, lr, s8, s9
[^>]*> ec5e 0a18 	vmov	r0, lr, s16, s17
[^>]*> ec5e 0a1f 	vmov	r0, lr, s30, s31
[^>]*> ec4e 0a10 	vmov	s0, s1, r0, lr
[^>]*> ec4e 0a30 	vmov	s1, s2, r0, lr
[^>]*> ec4e 0a11 	vmov	s2, s3, r0, lr
[^>]*> ec4e 0a12 	vmov	s4, s5, r0, lr
[^>]*> ec4e 0a14 	vmov	s8, s9, r0, lr
[^>]*> ec4e 0a18 	vmov	s16, s17, r0, lr
[^>]*> ec4e 0a1f 	vmov	s30, s31, r0, lr
[^>]*> ec50 1b10 	vmov	r1, r0, d0
[^>]*> ec40 1b10 	vmov	d0, r1, r0
[^>]*> ec50 1b11 	vmov	r1, r0, d1
[^>]*> ec40 1b11 	vmov	d1, r1, r0
[^>]*> ec50 1b12 	vmov	r1, r0, d2
[^>]*> ec40 1b12 	vmov	d2, r1, r0
[^>]*> ec50 1b14 	vmov	r1, r0, d4
[^>]*> ec40 1b14 	vmov	d4, r1, r0
[^>]*> ec50 1b18 	vmov	r1, r0, d8
[^>]*> ec40 1b18 	vmov	d8, r1, r0
[^>]*> ec50 1b1f 	vmov	r1, r0, d15
[^>]*> ec40 1b1f 	vmov	d15, r1, r0
[^>]*> ec50 1a10 	vmov	r1, r0, s0, s1
[^>]*> ec50 1a30 	vmov	r1, r0, s1, s2
[^>]*> ec50 1a11 	vmov	r1, r0, s2, s3
[^>]*> ec50 1a12 	vmov	r1, r0, s4, s5
[^>]*> ec50 1a14 	vmov	r1, r0, s8, s9
[^>]*> ec50 1a18 	vmov	r1, r0, s16, s17
[^>]*> ec50 1a1f 	vmov	r1, r0, s30, s31
[^>]*> ec40 1a10 	vmov	s0, s1, r1, r0
[^>]*> ec40 1a30 	vmov	s1, s2, r1, r0
[^>]*> ec40 1a11 	vmov	s2, s3, r1, r0
[^>]*> ec40 1a12 	vmov	s4, s5, r1, r0
[^>]*> ec40 1a14 	vmov	s8, s9, r1, r0
[^>]*> ec40 1a18 	vmov	s16, s17, r1, r0
[^>]*> ec40 1a1f 	vmov	s30, s31, r1, r0
[^>]*> ec52 1b10 	vmov	r1, r2, d0
[^>]*> ec42 1b10 	vmov	d0, r1, r2
[^>]*> ec52 1b11 	vmov	r1, r2, d1
[^>]*> ec42 1b11 	vmov	d1, r1, r2
[^>]*> ec52 1b12 	vmov	r1, r2, d2
[^>]*> ec42 1b12 	vmov	d2, r1, r2
[^>]*> ec52 1b14 	vmov	r1, r2, d4
[^>]*> ec42 1b14 	vmov	d4, r1, r2
[^>]*> ec52 1b18 	vmov	r1, r2, d8
[^>]*> ec42 1b18 	vmov	d8, r1, r2
[^>]*> ec52 1b1f 	vmov	r1, r2, d15
[^>]*> ec42 1b1f 	vmov	d15, r1, r2
[^>]*> ec52 1a10 	vmov	r1, r2, s0, s1
[^>]*> ec52 1a30 	vmov	r1, r2, s1, s2
[^>]*> ec52 1a11 	vmov	r1, r2, s2, s3
[^>]*> ec52 1a12 	vmov	r1, r2, s4, s5
[^>]*> ec52 1a14 	vmov	r1, r2, s8, s9
[^>]*> ec52 1a18 	vmov	r1, r2, s16, s17
[^>]*> ec52 1a1f 	vmov	r1, r2, s30, s31
[^>]*> ec42 1a10 	vmov	s0, s1, r1, r2
[^>]*> ec42 1a30 	vmov	s1, s2, r1, r2
[^>]*> ec42 1a11 	vmov	s2, s3, r1, r2
[^>]*> ec42 1a12 	vmov	s4, s5, r1, r2
[^>]*> ec42 1a14 	vmov	s8, s9, r1, r2
[^>]*> ec42 1a18 	vmov	s16, s17, r1, r2
[^>]*> ec42 1a1f 	vmov	s30, s31, r1, r2
[^>]*> ec54 1b10 	vmov	r1, r4, d0
[^>]*> ec44 1b10 	vmov	d0, r1, r4
[^>]*> ec54 1b11 	vmov	r1, r4, d1
[^>]*> ec44 1b11 	vmov	d1, r1, r4
[^>]*> ec54 1b12 	vmov	r1, r4, d2
[^>]*> ec44 1b12 	vmov	d2, r1, r4
[^>]*> ec54 1b14 	vmov	r1, r4, d4
[^>]*> ec44 1b14 	vmov	d4, r1, r4
[^>]*> ec54 1b18 	vmov	r1, r4, d8
[^>]*> ec44 1b18 	vmov	d8, r1, r4
[^>]*> ec54 1b1f 	vmov	r1, r4, d15
[^>]*> ec44 1b1f 	vmov	d15, r1, r4
[^>]*> ec54 1a10 	vmov	r1, r4, s0, s1
[^>]*> ec54 1a30 	vmov	r1, r4, s1, s2
[^>]*> ec54 1a11 	vmov	r1, r4, s2, s3
[^>]*> ec54 1a12 	vmov	r1, r4, s4, s5
[^>]*> ec54 1a14 	vmov	r1, r4, s8, s9
[^>]*> ec54 1a18 	vmov	r1, r4, s16, s17
[^>]*> ec54 1a1f 	vmov	r1, r4, s30, s31
[^>]*> ec44 1a10 	vmov	s0, s1, r1, r4
[^>]*> ec44 1a30 	vmov	s1, s2, r1, r4
[^>]*> ec44 1a11 	vmov	s2, s3, r1, r4
[^>]*> ec44 1a12 	vmov	s4, s5, r1, r4
[^>]*> ec44 1a14 	vmov	s8, s9, r1, r4
[^>]*> ec44 1a18 	vmov	s16, s17, r1, r4
[^>]*> ec44 1a1f 	vmov	s30, s31, r1, r4
[^>]*> ec57 1b10 	vmov	r1, r7, d0
[^>]*> ec47 1b10 	vmov	d0, r1, r7
[^>]*> ec57 1b11 	vmov	r1, r7, d1
[^>]*> ec47 1b11 	vmov	d1, r1, r7
[^>]*> ec57 1b12 	vmov	r1, r7, d2
[^>]*> ec47 1b12 	vmov	d2, r1, r7
[^>]*> ec57 1b14 	vmov	r1, r7, d4
[^>]*> ec47 1b14 	vmov	d4, r1, r7
[^>]*> ec57 1b18 	vmov	r1, r7, d8
[^>]*> ec47 1b18 	vmov	d8, r1, r7
[^>]*> ec57 1b1f 	vmov	r1, r7, d15
[^>]*> ec47 1b1f 	vmov	d15, r1, r7
[^>]*> ec57 1a10 	vmov	r1, r7, s0, s1
[^>]*> ec57 1a30 	vmov	r1, r7, s1, s2
[^>]*> ec57 1a11 	vmov	r1, r7, s2, s3
[^>]*> ec57 1a12 	vmov	r1, r7, s4, s5
[^>]*> ec57 1a14 	vmov	r1, r7, s8, s9
[^>]*> ec57 1a18 	vmov	r1, r7, s16, s17
[^>]*> ec57 1a1f 	vmov	r1, r7, s30, s31
[^>]*> ec47 1a10 	vmov	s0, s1, r1, r7
[^>]*> ec47 1a30 	vmov	s1, s2, r1, r7
[^>]*> ec47 1a11 	vmov	s2, s3, r1, r7
[^>]*> ec47 1a12 	vmov	s4, s5, r1, r7
[^>]*> ec47 1a14 	vmov	s8, s9, r1, r7
[^>]*> ec47 1a18 	vmov	s16, s17, r1, r7
[^>]*> ec47 1a1f 	vmov	s30, s31, r1, r7
[^>]*> ec58 1b10 	vmov	r1, r8, d0
[^>]*> ec48 1b10 	vmov	d0, r1, r8
[^>]*> ec58 1b11 	vmov	r1, r8, d1
[^>]*> ec48 1b11 	vmov	d1, r1, r8
[^>]*> ec58 1b12 	vmov	r1, r8, d2
[^>]*> ec48 1b12 	vmov	d2, r1, r8
[^>]*> ec58 1b14 	vmov	r1, r8, d4
[^>]*> ec48 1b14 	vmov	d4, r1, r8
[^>]*> ec58 1b18 	vmov	r1, r8, d8
[^>]*> ec48 1b18 	vmov	d8, r1, r8
[^>]*> ec58 1b1f 	vmov	r1, r8, d15
[^>]*> ec48 1b1f 	vmov	d15, r1, r8
[^>]*> ec58 1a10 	vmov	r1, r8, s0, s1
[^>]*> ec58 1a30 	vmov	r1, r8, s1, s2
[^>]*> ec58 1a11 	vmov	r1, r8, s2, s3
[^>]*> ec58 1a12 	vmov	r1, r8, s4, s5
[^>]*> ec58 1a14 	vmov	r1, r8, s8, s9
[^>]*> ec58 1a18 	vmov	r1, r8, s16, s17
[^>]*> ec58 1a1f 	vmov	r1, r8, s30, s31
[^>]*> ec48 1a10 	vmov	s0, s1, r1, r8
[^>]*> ec48 1a30 	vmov	s1, s2, r1, r8
[^>]*> ec48 1a11 	vmov	s2, s3, r1, r8
[^>]*> ec48 1a12 	vmov	s4, s5, r1, r8
[^>]*> ec48 1a14 	vmov	s8, s9, r1, r8
[^>]*> ec48 1a18 	vmov	s16, s17, r1, r8
[^>]*> ec48 1a1f 	vmov	s30, s31, r1, r8
[^>]*> ec5a 1b10 	vmov	r1, sl, d0
[^>]*> ec4a 1b10 	vmov	d0, r1, sl
[^>]*> ec5a 1b11 	vmov	r1, sl, d1
[^>]*> ec4a 1b11 	vmov	d1, r1, sl
[^>]*> ec5a 1b12 	vmov	r1, sl, d2
[^>]*> ec4a 1b12 	vmov	d2, r1, sl
[^>]*> ec5a 1b14 	vmov	r1, sl, d4
[^>]*> ec4a 1b14 	vmov	d4, r1, sl
[^>]*> ec5a 1b18 	vmov	r1, sl, d8
[^>]*> ec4a 1b18 	vmov	d8, r1, sl
[^>]*> ec5a 1b1f 	vmov	r1, sl, d15
[^>]*> ec4a 1b1f 	vmov	d15, r1, sl
[^>]*> ec5a 1a10 	vmov	r1, sl, s0, s1
[^>]*> ec5a 1a30 	vmov	r1, sl, s1, s2
[^>]*> ec5a 1a11 	vmov	r1, sl, s2, s3
[^>]*> ec5a 1a12 	vmov	r1, sl, s4, s5
[^>]*> ec5a 1a14 	vmov	r1, sl, s8, s9
[^>]*> ec5a 1a18 	vmov	r1, sl, s16, s17
[^>]*> ec5a 1a1f 	vmov	r1, sl, s30, s31
[^>]*> ec4a 1a10 	vmov	s0, s1, r1, sl
[^>]*> ec4a 1a30 	vmov	s1, s2, r1, sl
[^>]*> ec4a 1a11 	vmov	s2, s3, r1, sl
[^>]*> ec4a 1a12 	vmov	s4, s5, r1, sl
[^>]*> ec4a 1a14 	vmov	s8, s9, r1, sl
[^>]*> ec4a 1a18 	vmov	s16, s17, r1, sl
[^>]*> ec4a 1a1f 	vmov	s30, s31, r1, sl
[^>]*> ec5c 1b10 	vmov	r1, ip, d0
[^>]*> ec4c 1b10 	vmov	d0, r1, ip
[^>]*> ec5c 1b11 	vmov	r1, ip, d1
[^>]*> ec4c 1b11 	vmov	d1, r1, ip
[^>]*> ec5c 1b12 	vmov	r1, ip, d2
[^>]*> ec4c 1b12 	vmov	d2, r1, ip
[^>]*> ec5c 1b14 	vmov	r1, ip, d4
[^>]*> ec4c 1b14 	vmov	d4, r1, ip
[^>]*> ec5c 1b18 	vmov	r1, ip, d8
[^>]*> ec4c 1b18 	vmov	d8, r1, ip
[^>]*> ec5c 1b1f 	vmov	r1, ip, d15
[^>]*> ec4c 1b1f 	vmov	d15, r1, ip
[^>]*> ec5c 1a10 	vmov	r1, ip, s0, s1
[^>]*> ec5c 1a30 	vmov	r1, ip, s1, s2
[^>]*> ec5c 1a11 	vmov	r1, ip, s2, s3
[^>]*> ec5c 1a12 	vmov	r1, ip, s4, s5
[^>]*> ec5c 1a14 	vmov	r1, ip, s8, s9
[^>]*> ec5c 1a18 	vmov	r1, ip, s16, s17
[^>]*> ec5c 1a1f 	vmov	r1, ip, s30, s31
[^>]*> ec4c 1a10 	vmov	s0, s1, r1, ip
[^>]*> ec4c 1a30 	vmov	s1, s2, r1, ip
[^>]*> ec4c 1a11 	vmov	s2, s3, r1, ip
[^>]*> ec4c 1a12 	vmov	s4, s5, r1, ip
[^>]*> ec4c 1a14 	vmov	s8, s9, r1, ip
[^>]*> ec4c 1a18 	vmov	s16, s17, r1, ip
[^>]*> ec4c 1a1f 	vmov	s30, s31, r1, ip
[^>]*> ec5e 1b10 	vmov	r1, lr, d0
[^>]*> ec4e 1b10 	vmov	d0, r1, lr
[^>]*> ec5e 1b11 	vmov	r1, lr, d1
[^>]*> ec4e 1b11 	vmov	d1, r1, lr
[^>]*> ec5e 1b12 	vmov	r1, lr, d2
[^>]*> ec4e 1b12 	vmov	d2, r1, lr
[^>]*> ec5e 1b14 	vmov	r1, lr, d4
[^>]*> ec4e 1b14 	vmov	d4, r1, lr
[^>]*> ec5e 1b18 	vmov	r1, lr, d8
[^>]*> ec4e 1b18 	vmov	d8, r1, lr
[^>]*> ec5e 1b1f 	vmov	r1, lr, d15
[^>]*> ec4e 1b1f 	vmov	d15, r1, lr
[^>]*> ec5e 1a10 	vmov	r1, lr, s0, s1
[^>]*> ec5e 1a30 	vmov	r1, lr, s1, s2
[^>]*> ec5e 1a11 	vmov	r1, lr, s2, s3
[^>]*> ec5e 1a12 	vmov	r1, lr, s4, s5
[^>]*> ec5e 1a14 	vmov	r1, lr, s8, s9
[^>]*> ec5e 1a18 	vmov	r1, lr, s16, s17
[^>]*> ec5e 1a1f 	vmov	r1, lr, s30, s31
[^>]*> ec4e 1a10 	vmov	s0, s1, r1, lr
[^>]*> ec4e 1a30 	vmov	s1, s2, r1, lr
[^>]*> ec4e 1a11 	vmov	s2, s3, r1, lr
[^>]*> ec4e 1a12 	vmov	s4, s5, r1, lr
[^>]*> ec4e 1a14 	vmov	s8, s9, r1, lr
[^>]*> ec4e 1a18 	vmov	s16, s17, r1, lr
[^>]*> ec4e 1a1f 	vmov	s30, s31, r1, lr
[^>]*> ec50 2b10 	vmov	r2, r0, d0
[^>]*> ec40 2b10 	vmov	d0, r2, r0
[^>]*> ec50 2b11 	vmov	r2, r0, d1
[^>]*> ec40 2b11 	vmov	d1, r2, r0
[^>]*> ec50 2b12 	vmov	r2, r0, d2
[^>]*> ec40 2b12 	vmov	d2, r2, r0
[^>]*> ec50 2b14 	vmov	r2, r0, d4
[^>]*> ec40 2b14 	vmov	d4, r2, r0
[^>]*> ec50 2b18 	vmov	r2, r0, d8
[^>]*> ec40 2b18 	vmov	d8, r2, r0
[^>]*> ec50 2b1f 	vmov	r2, r0, d15
[^>]*> ec40 2b1f 	vmov	d15, r2, r0
[^>]*> ec50 2a10 	vmov	r2, r0, s0, s1
[^>]*> ec50 2a30 	vmov	r2, r0, s1, s2
[^>]*> ec50 2a11 	vmov	r2, r0, s2, s3
[^>]*> ec50 2a12 	vmov	r2, r0, s4, s5
[^>]*> ec50 2a14 	vmov	r2, r0, s8, s9
[^>]*> ec50 2a18 	vmov	r2, r0, s16, s17
[^>]*> ec50 2a1f 	vmov	r2, r0, s30, s31
[^>]*> ec40 2a10 	vmov	s0, s1, r2, r0
[^>]*> ec40 2a30 	vmov	s1, s2, r2, r0
[^>]*> ec40 2a11 	vmov	s2, s3, r2, r0
[^>]*> ec40 2a12 	vmov	s4, s5, r2, r0
[^>]*> ec40 2a14 	vmov	s8, s9, r2, r0
[^>]*> ec40 2a18 	vmov	s16, s17, r2, r0
[^>]*> ec40 2a1f 	vmov	s30, s31, r2, r0
[^>]*> ec51 2b10 	vmov	r2, r1, d0
[^>]*> ec41 2b10 	vmov	d0, r2, r1
[^>]*> ec51 2b11 	vmov	r2, r1, d1
[^>]*> ec41 2b11 	vmov	d1, r2, r1
[^>]*> ec51 2b12 	vmov	r2, r1, d2
[^>]*> ec41 2b12 	vmov	d2, r2, r1
[^>]*> ec51 2b14 	vmov	r2, r1, d4
[^>]*> ec41 2b14 	vmov	d4, r2, r1
[^>]*> ec51 2b18 	vmov	r2, r1, d8
[^>]*> ec41 2b18 	vmov	d8, r2, r1
[^>]*> ec51 2b1f 	vmov	r2, r1, d15
[^>]*> ec41 2b1f 	vmov	d15, r2, r1
[^>]*> ec51 2a10 	vmov	r2, r1, s0, s1
[^>]*> ec51 2a30 	vmov	r2, r1, s1, s2
[^>]*> ec51 2a11 	vmov	r2, r1, s2, s3
[^>]*> ec51 2a12 	vmov	r2, r1, s4, s5
[^>]*> ec51 2a14 	vmov	r2, r1, s8, s9
[^>]*> ec51 2a18 	vmov	r2, r1, s16, s17
[^>]*> ec51 2a1f 	vmov	r2, r1, s30, s31
[^>]*> ec41 2a10 	vmov	s0, s1, r2, r1
[^>]*> ec41 2a30 	vmov	s1, s2, r2, r1
[^>]*> ec41 2a11 	vmov	s2, s3, r2, r1
[^>]*> ec41 2a12 	vmov	s4, s5, r2, r1
[^>]*> ec41 2a14 	vmov	s8, s9, r2, r1
[^>]*> ec41 2a18 	vmov	s16, s17, r2, r1
[^>]*> ec41 2a1f 	vmov	s30, s31, r2, r1
[^>]*> ec54 2b10 	vmov	r2, r4, d0
[^>]*> ec44 2b10 	vmov	d0, r2, r4
[^>]*> ec54 2b11 	vmov	r2, r4, d1
[^>]*> ec44 2b11 	vmov	d1, r2, r4
[^>]*> ec54 2b12 	vmov	r2, r4, d2
[^>]*> ec44 2b12 	vmov	d2, r2, r4
[^>]*> ec54 2b14 	vmov	r2, r4, d4
[^>]*> ec44 2b14 	vmov	d4, r2, r4
[^>]*> ec54 2b18 	vmov	r2, r4, d8
[^>]*> ec44 2b18 	vmov	d8, r2, r4
[^>]*> ec54 2b1f 	vmov	r2, r4, d15
[^>]*> ec44 2b1f 	vmov	d15, r2, r4
[^>]*> ec54 2a10 	vmov	r2, r4, s0, s1
[^>]*> ec54 2a30 	vmov	r2, r4, s1, s2
[^>]*> ec54 2a11 	vmov	r2, r4, s2, s3
[^>]*> ec54 2a12 	vmov	r2, r4, s4, s5
[^>]*> ec54 2a14 	vmov	r2, r4, s8, s9
[^>]*> ec54 2a18 	vmov	r2, r4, s16, s17
[^>]*> ec54 2a1f 	vmov	r2, r4, s30, s31
[^>]*> ec44 2a10 	vmov	s0, s1, r2, r4
[^>]*> ec44 2a30 	vmov	s1, s2, r2, r4
[^>]*> ec44 2a11 	vmov	s2, s3, r2, r4
[^>]*> ec44 2a12 	vmov	s4, s5, r2, r4
[^>]*> ec44 2a14 	vmov	s8, s9, r2, r4
[^>]*> ec44 2a18 	vmov	s16, s17, r2, r4
[^>]*> ec44 2a1f 	vmov	s30, s31, r2, r4
[^>]*> ec57 2b10 	vmov	r2, r7, d0
[^>]*> ec47 2b10 	vmov	d0, r2, r7
[^>]*> ec57 2b11 	vmov	r2, r7, d1
[^>]*> ec47 2b11 	vmov	d1, r2, r7
[^>]*> ec57 2b12 	vmov	r2, r7, d2
[^>]*> ec47 2b12 	vmov	d2, r2, r7
[^>]*> ec57 2b14 	vmov	r2, r7, d4
[^>]*> ec47 2b14 	vmov	d4, r2, r7
[^>]*> ec57 2b18 	vmov	r2, r7, d8
[^>]*> ec47 2b18 	vmov	d8, r2, r7
[^>]*> ec57 2b1f 	vmov	r2, r7, d15
[^>]*> ec47 2b1f 	vmov	d15, r2, r7
[^>]*> ec57 2a10 	vmov	r2, r7, s0, s1
[^>]*> ec57 2a30 	vmov	r2, r7, s1, s2
[^>]*> ec57 2a11 	vmov	r2, r7, s2, s3
[^>]*> ec57 2a12 	vmov	r2, r7, s4, s5
[^>]*> ec57 2a14 	vmov	r2, r7, s8, s9
[^>]*> ec57 2a18 	vmov	r2, r7, s16, s17
[^>]*> ec57 2a1f 	vmov	r2, r7, s30, s31
[^>]*> ec47 2a10 	vmov	s0, s1, r2, r7
[^>]*> ec47 2a30 	vmov	s1, s2, r2, r7
[^>]*> ec47 2a11 	vmov	s2, s3, r2, r7
[^>]*> ec47 2a12 	vmov	s4, s5, r2, r7
[^>]*> ec47 2a14 	vmov	s8, s9, r2, r7
[^>]*> ec47 2a18 	vmov	s16, s17, r2, r7
[^>]*> ec47 2a1f 	vmov	s30, s31, r2, r7
[^>]*> ec58 2b10 	vmov	r2, r8, d0
[^>]*> ec48 2b10 	vmov	d0, r2, r8
[^>]*> ec58 2b11 	vmov	r2, r8, d1
[^>]*> ec48 2b11 	vmov	d1, r2, r8
[^>]*> ec58 2b12 	vmov	r2, r8, d2
[^>]*> ec48 2b12 	vmov	d2, r2, r8
[^>]*> ec58 2b14 	vmov	r2, r8, d4
[^>]*> ec48 2b14 	vmov	d4, r2, r8
[^>]*> ec58 2b18 	vmov	r2, r8, d8
[^>]*> ec48 2b18 	vmov	d8, r2, r8
[^>]*> ec58 2b1f 	vmov	r2, r8, d15
[^>]*> ec48 2b1f 	vmov	d15, r2, r8
[^>]*> ec58 2a10 	vmov	r2, r8, s0, s1
[^>]*> ec58 2a30 	vmov	r2, r8, s1, s2
[^>]*> ec58 2a11 	vmov	r2, r8, s2, s3
[^>]*> ec58 2a12 	vmov	r2, r8, s4, s5
[^>]*> ec58 2a14 	vmov	r2, r8, s8, s9
[^>]*> ec58 2a18 	vmov	r2, r8, s16, s17
[^>]*> ec58 2a1f 	vmov	r2, r8, s30, s31
[^>]*> ec48 2a10 	vmov	s0, s1, r2, r8
[^>]*> ec48 2a30 	vmov	s1, s2, r2, r8
[^>]*> ec48 2a11 	vmov	s2, s3, r2, r8
[^>]*> ec48 2a12 	vmov	s4, s5, r2, r8
[^>]*> ec48 2a14 	vmov	s8, s9, r2, r8
[^>]*> ec48 2a18 	vmov	s16, s17, r2, r8
[^>]*> ec48 2a1f 	vmov	s30, s31, r2, r8
[^>]*> ec5a 2b10 	vmov	r2, sl, d0
[^>]*> ec4a 2b10 	vmov	d0, r2, sl
[^>]*> ec5a 2b11 	vmov	r2, sl, d1
[^>]*> ec4a 2b11 	vmov	d1, r2, sl
[^>]*> ec5a 2b12 	vmov	r2, sl, d2
[^>]*> ec4a 2b12 	vmov	d2, r2, sl
[^>]*> ec5a 2b14 	vmov	r2, sl, d4
[^>]*> ec4a 2b14 	vmov	d4, r2, sl
[^>]*> ec5a 2b18 	vmov	r2, sl, d8
[^>]*> ec4a 2b18 	vmov	d8, r2, sl
[^>]*> ec5a 2b1f 	vmov	r2, sl, d15
[^>]*> ec4a 2b1f 	vmov	d15, r2, sl
[^>]*> ec5a 2a10 	vmov	r2, sl, s0, s1
[^>]*> ec5a 2a30 	vmov	r2, sl, s1, s2
[^>]*> ec5a 2a11 	vmov	r2, sl, s2, s3
[^>]*> ec5a 2a12 	vmov	r2, sl, s4, s5
[^>]*> ec5a 2a14 	vmov	r2, sl, s8, s9
[^>]*> ec5a 2a18 	vmov	r2, sl, s16, s17
[^>]*> ec5a 2a1f 	vmov	r2, sl, s30, s31
[^>]*> ec4a 2a10 	vmov	s0, s1, r2, sl
[^>]*> ec4a 2a30 	vmov	s1, s2, r2, sl
[^>]*> ec4a 2a11 	vmov	s2, s3, r2, sl
[^>]*> ec4a 2a12 	vmov	s4, s5, r2, sl
[^>]*> ec4a 2a14 	vmov	s8, s9, r2, sl
[^>]*> ec4a 2a18 	vmov	s16, s17, r2, sl
[^>]*> ec4a 2a1f 	vmov	s30, s31, r2, sl
[^>]*> ec5c 2b10 	vmov	r2, ip, d0
[^>]*> ec4c 2b10 	vmov	d0, r2, ip
[^>]*> ec5c 2b11 	vmov	r2, ip, d1
[^>]*> ec4c 2b11 	vmov	d1, r2, ip
[^>]*> ec5c 2b12 	vmov	r2, ip, d2
[^>]*> ec4c 2b12 	vmov	d2, r2, ip
[^>]*> ec5c 2b14 	vmov	r2, ip, d4
[^>]*> ec4c 2b14 	vmov	d4, r2, ip
[^>]*> ec5c 2b18 	vmov	r2, ip, d8
[^>]*> ec4c 2b18 	vmov	d8, r2, ip
[^>]*> ec5c 2b1f 	vmov	r2, ip, d15
[^>]*> ec4c 2b1f 	vmov	d15, r2, ip
[^>]*> ec5c 2a10 	vmov	r2, ip, s0, s1
[^>]*> ec5c 2a30 	vmov	r2, ip, s1, s2
[^>]*> ec5c 2a11 	vmov	r2, ip, s2, s3
[^>]*> ec5c 2a12 	vmov	r2, ip, s4, s5
[^>]*> ec5c 2a14 	vmov	r2, ip, s8, s9
[^>]*> ec5c 2a18 	vmov	r2, ip, s16, s17
[^>]*> ec5c 2a1f 	vmov	r2, ip, s30, s31
[^>]*> ec4c 2a10 	vmov	s0, s1, r2, ip
[^>]*> ec4c 2a30 	vmov	s1, s2, r2, ip
[^>]*> ec4c 2a11 	vmov	s2, s3, r2, ip
[^>]*> ec4c 2a12 	vmov	s4, s5, r2, ip
[^>]*> ec4c 2a14 	vmov	s8, s9, r2, ip
[^>]*> ec4c 2a18 	vmov	s16, s17, r2, ip
[^>]*> ec4c 2a1f 	vmov	s30, s31, r2, ip
[^>]*> ec5e 2b10 	vmov	r2, lr, d0
[^>]*> ec4e 2b10 	vmov	d0, r2, lr
[^>]*> ec5e 2b11 	vmov	r2, lr, d1
[^>]*> ec4e 2b11 	vmov	d1, r2, lr
[^>]*> ec5e 2b12 	vmov	r2, lr, d2
[^>]*> ec4e 2b12 	vmov	d2, r2, lr
[^>]*> ec5e 2b14 	vmov	r2, lr, d4
[^>]*> ec4e 2b14 	vmov	d4, r2, lr
[^>]*> ec5e 2b18 	vmov	r2, lr, d8
[^>]*> ec4e 2b18 	vmov	d8, r2, lr
[^>]*> ec5e 2b1f 	vmov	r2, lr, d15
[^>]*> ec4e 2b1f 	vmov	d15, r2, lr
[^>]*> ec5e 2a10 	vmov	r2, lr, s0, s1
[^>]*> ec5e 2a30 	vmov	r2, lr, s1, s2
[^>]*> ec5e 2a11 	vmov	r2, lr, s2, s3
[^>]*> ec5e 2a12 	vmov	r2, lr, s4, s5
[^>]*> ec5e 2a14 	vmov	r2, lr, s8, s9
[^>]*> ec5e 2a18 	vmov	r2, lr, s16, s17
[^>]*> ec5e 2a1f 	vmov	r2, lr, s30, s31
[^>]*> ec4e 2a10 	vmov	s0, s1, r2, lr
[^>]*> ec4e 2a30 	vmov	s1, s2, r2, lr
[^>]*> ec4e 2a11 	vmov	s2, s3, r2, lr
[^>]*> ec4e 2a12 	vmov	s4, s5, r2, lr
[^>]*> ec4e 2a14 	vmov	s8, s9, r2, lr
[^>]*> ec4e 2a18 	vmov	s16, s17, r2, lr
[^>]*> ec4e 2a1f 	vmov	s30, s31, r2, lr
[^>]*> ec50 4b10 	vmov	r4, r0, d0
[^>]*> ec40 4b10 	vmov	d0, r4, r0
[^>]*> ec50 4b11 	vmov	r4, r0, d1
[^>]*> ec40 4b11 	vmov	d1, r4, r0
[^>]*> ec50 4b12 	vmov	r4, r0, d2
[^>]*> ec40 4b12 	vmov	d2, r4, r0
[^>]*> ec50 4b14 	vmov	r4, r0, d4
[^>]*> ec40 4b14 	vmov	d4, r4, r0
[^>]*> ec50 4b18 	vmov	r4, r0, d8
[^>]*> ec40 4b18 	vmov	d8, r4, r0
[^>]*> ec50 4b1f 	vmov	r4, r0, d15
[^>]*> ec40 4b1f 	vmov	d15, r4, r0
[^>]*> ec50 4a10 	vmov	r4, r0, s0, s1
[^>]*> ec50 4a30 	vmov	r4, r0, s1, s2
[^>]*> ec50 4a11 	vmov	r4, r0, s2, s3
[^>]*> ec50 4a12 	vmov	r4, r0, s4, s5
[^>]*> ec50 4a14 	vmov	r4, r0, s8, s9
[^>]*> ec50 4a18 	vmov	r4, r0, s16, s17
[^>]*> ec50 4a1f 	vmov	r4, r0, s30, s31
[^>]*> ec40 4a10 	vmov	s0, s1, r4, r0
[^>]*> ec40 4a30 	vmov	s1, s2, r4, r0
[^>]*> ec40 4a11 	vmov	s2, s3, r4, r0
[^>]*> ec40 4a12 	vmov	s4, s5, r4, r0
[^>]*> ec40 4a14 	vmov	s8, s9, r4, r0
[^>]*> ec40 4a18 	vmov	s16, s17, r4, r0
[^>]*> ec40 4a1f 	vmov	s30, s31, r4, r0
[^>]*> ec51 4b10 	vmov	r4, r1, d0
[^>]*> ec41 4b10 	vmov	d0, r4, r1
[^>]*> ec51 4b11 	vmov	r4, r1, d1
[^>]*> ec41 4b11 	vmov	d1, r4, r1
[^>]*> ec51 4b12 	vmov	r4, r1, d2
[^>]*> ec41 4b12 	vmov	d2, r4, r1
[^>]*> ec51 4b14 	vmov	r4, r1, d4
[^>]*> ec41 4b14 	vmov	d4, r4, r1
[^>]*> ec51 4b18 	vmov	r4, r1, d8
[^>]*> ec41 4b18 	vmov	d8, r4, r1
[^>]*> ec51 4b1f 	vmov	r4, r1, d15
[^>]*> ec41 4b1f 	vmov	d15, r4, r1
[^>]*> ec51 4a10 	vmov	r4, r1, s0, s1
[^>]*> ec51 4a30 	vmov	r4, r1, s1, s2
[^>]*> ec51 4a11 	vmov	r4, r1, s2, s3
[^>]*> ec51 4a12 	vmov	r4, r1, s4, s5
[^>]*> ec51 4a14 	vmov	r4, r1, s8, s9
[^>]*> ec51 4a18 	vmov	r4, r1, s16, s17
[^>]*> ec51 4a1f 	vmov	r4, r1, s30, s31
[^>]*> ec41 4a10 	vmov	s0, s1, r4, r1
[^>]*> ec41 4a30 	vmov	s1, s2, r4, r1
[^>]*> ec41 4a11 	vmov	s2, s3, r4, r1
[^>]*> ec41 4a12 	vmov	s4, s5, r4, r1
[^>]*> ec41 4a14 	vmov	s8, s9, r4, r1
[^>]*> ec41 4a18 	vmov	s16, s17, r4, r1
[^>]*> ec41 4a1f 	vmov	s30, s31, r4, r1
[^>]*> ec52 4b10 	vmov	r4, r2, d0
[^>]*> ec42 4b10 	vmov	d0, r4, r2
[^>]*> ec52 4b11 	vmov	r4, r2, d1
[^>]*> ec42 4b11 	vmov	d1, r4, r2
[^>]*> ec52 4b12 	vmov	r4, r2, d2
[^>]*> ec42 4b12 	vmov	d2, r4, r2
[^>]*> ec52 4b14 	vmov	r4, r2, d4
[^>]*> ec42 4b14 	vmov	d4, r4, r2
[^>]*> ec52 4b18 	vmov	r4, r2, d8
[^>]*> ec42 4b18 	vmov	d8, r4, r2
[^>]*> ec52 4b1f 	vmov	r4, r2, d15
[^>]*> ec42 4b1f 	vmov	d15, r4, r2
[^>]*> ec52 4a10 	vmov	r4, r2, s0, s1
[^>]*> ec52 4a30 	vmov	r4, r2, s1, s2
[^>]*> ec52 4a11 	vmov	r4, r2, s2, s3
[^>]*> ec52 4a12 	vmov	r4, r2, s4, s5
[^>]*> ec52 4a14 	vmov	r4, r2, s8, s9
[^>]*> ec52 4a18 	vmov	r4, r2, s16, s17
[^>]*> ec52 4a1f 	vmov	r4, r2, s30, s31
[^>]*> ec42 4a10 	vmov	s0, s1, r4, r2
[^>]*> ec42 4a30 	vmov	s1, s2, r4, r2
[^>]*> ec42 4a11 	vmov	s2, s3, r4, r2
[^>]*> ec42 4a12 	vmov	s4, s5, r4, r2
[^>]*> ec42 4a14 	vmov	s8, s9, r4, r2
[^>]*> ec42 4a18 	vmov	s16, s17, r4, r2
[^>]*> ec42 4a1f 	vmov	s30, s31, r4, r2
[^>]*> ec57 4b10 	vmov	r4, r7, d0
[^>]*> ec47 4b10 	vmov	d0, r4, r7
[^>]*> ec57 4b11 	vmov	r4, r7, d1
[^>]*> ec47 4b11 	vmov	d1, r4, r7
[^>]*> ec57 4b12 	vmov	r4, r7, d2
[^>]*> ec47 4b12 	vmov	d2, r4, r7
[^>]*> ec57 4b14 	vmov	r4, r7, d4
[^>]*> ec47 4b14 	vmov	d4, r4, r7
[^>]*> ec57 4b18 	vmov	r4, r7, d8
[^>]*> ec47 4b18 	vmov	d8, r4, r7
[^>]*> ec57 4b1f 	vmov	r4, r7, d15
[^>]*> ec47 4b1f 	vmov	d15, r4, r7
[^>]*> ec57 4a10 	vmov	r4, r7, s0, s1
[^>]*> ec57 4a30 	vmov	r4, r7, s1, s2
[^>]*> ec57 4a11 	vmov	r4, r7, s2, s3
[^>]*> ec57 4a12 	vmov	r4, r7, s4, s5
[^>]*> ec57 4a14 	vmov	r4, r7, s8, s9
[^>]*> ec57 4a18 	vmov	r4, r7, s16, s17
[^>]*> ec57 4a1f 	vmov	r4, r7, s30, s31
[^>]*> ec47 4a10 	vmov	s0, s1, r4, r7
[^>]*> ec47 4a30 	vmov	s1, s2, r4, r7
[^>]*> ec47 4a11 	vmov	s2, s3, r4, r7
[^>]*> ec47 4a12 	vmov	s4, s5, r4, r7
[^>]*> ec47 4a14 	vmov	s8, s9, r4, r7
[^>]*> ec47 4a18 	vmov	s16, s17, r4, r7
[^>]*> ec47 4a1f 	vmov	s30, s31, r4, r7
[^>]*> ec58 4b10 	vmov	r4, r8, d0
[^>]*> ec48 4b10 	vmov	d0, r4, r8
[^>]*> ec58 4b11 	vmov	r4, r8, d1
[^>]*> ec48 4b11 	vmov	d1, r4, r8
[^>]*> ec58 4b12 	vmov	r4, r8, d2
[^>]*> ec48 4b12 	vmov	d2, r4, r8
[^>]*> ec58 4b14 	vmov	r4, r8, d4
[^>]*> ec48 4b14 	vmov	d4, r4, r8
[^>]*> ec58 4b18 	vmov	r4, r8, d8
[^>]*> ec48 4b18 	vmov	d8, r4, r8
[^>]*> ec58 4b1f 	vmov	r4, r8, d15
[^>]*> ec48 4b1f 	vmov	d15, r4, r8
[^>]*> ec58 4a10 	vmov	r4, r8, s0, s1
[^>]*> ec58 4a30 	vmov	r4, r8, s1, s2
[^>]*> ec58 4a11 	vmov	r4, r8, s2, s3
[^>]*> ec58 4a12 	vmov	r4, r8, s4, s5
[^>]*> ec58 4a14 	vmov	r4, r8, s8, s9
[^>]*> ec58 4a18 	vmov	r4, r8, s16, s17
[^>]*> ec58 4a1f 	vmov	r4, r8, s30, s31
[^>]*> ec48 4a10 	vmov	s0, s1, r4, r8
[^>]*> ec48 4a30 	vmov	s1, s2, r4, r8
[^>]*> ec48 4a11 	vmov	s2, s3, r4, r8
[^>]*> ec48 4a12 	vmov	s4, s5, r4, r8
[^>]*> ec48 4a14 	vmov	s8, s9, r4, r8
[^>]*> ec48 4a18 	vmov	s16, s17, r4, r8
[^>]*> ec48 4a1f 	vmov	s30, s31, r4, r8
[^>]*> ec5a 4b10 	vmov	r4, sl, d0
[^>]*> ec4a 4b10 	vmov	d0, r4, sl
[^>]*> ec5a 4b11 	vmov	r4, sl, d1
[^>]*> ec4a 4b11 	vmov	d1, r4, sl
[^>]*> ec5a 4b12 	vmov	r4, sl, d2
[^>]*> ec4a 4b12 	vmov	d2, r4, sl
[^>]*> ec5a 4b14 	vmov	r4, sl, d4
[^>]*> ec4a 4b14 	vmov	d4, r4, sl
[^>]*> ec5a 4b18 	vmov	r4, sl, d8
[^>]*> ec4a 4b18 	vmov	d8, r4, sl
[^>]*> ec5a 4b1f 	vmov	r4, sl, d15
[^>]*> ec4a 4b1f 	vmov	d15, r4, sl
[^>]*> ec5a 4a10 	vmov	r4, sl, s0, s1
[^>]*> ec5a 4a30 	vmov	r4, sl, s1, s2
[^>]*> ec5a 4a11 	vmov	r4, sl, s2, s3
[^>]*> ec5a 4a12 	vmov	r4, sl, s4, s5
[^>]*> ec5a 4a14 	vmov	r4, sl, s8, s9
[^>]*> ec5a 4a18 	vmov	r4, sl, s16, s17
[^>]*> ec5a 4a1f 	vmov	r4, sl, s30, s31
[^>]*> ec4a 4a10 	vmov	s0, s1, r4, sl
[^>]*> ec4a 4a30 	vmov	s1, s2, r4, sl
[^>]*> ec4a 4a11 	vmov	s2, s3, r4, sl
[^>]*> ec4a 4a12 	vmov	s4, s5, r4, sl
[^>]*> ec4a 4a14 	vmov	s8, s9, r4, sl
[^>]*> ec4a 4a18 	vmov	s16, s17, r4, sl
[^>]*> ec4a 4a1f 	vmov	s30, s31, r4, sl
[^>]*> ec5c 4b10 	vmov	r4, ip, d0
[^>]*> ec4c 4b10 	vmov	d0, r4, ip
[^>]*> ec5c 4b11 	vmov	r4, ip, d1
[^>]*> ec4c 4b11 	vmov	d1, r4, ip
[^>]*> ec5c 4b12 	vmov	r4, ip, d2
[^>]*> ec4c 4b12 	vmov	d2, r4, ip
[^>]*> ec5c 4b14 	vmov	r4, ip, d4
[^>]*> ec4c 4b14 	vmov	d4, r4, ip
[^>]*> ec5c 4b18 	vmov	r4, ip, d8
[^>]*> ec4c 4b18 	vmov	d8, r4, ip
[^>]*> ec5c 4b1f 	vmov	r4, ip, d15
[^>]*> ec4c 4b1f 	vmov	d15, r4, ip
[^>]*> ec5c 4a10 	vmov	r4, ip, s0, s1
[^>]*> ec5c 4a30 	vmov	r4, ip, s1, s2
[^>]*> ec5c 4a11 	vmov	r4, ip, s2, s3
[^>]*> ec5c 4a12 	vmov	r4, ip, s4, s5
[^>]*> ec5c 4a14 	vmov	r4, ip, s8, s9
[^>]*> ec5c 4a18 	vmov	r4, ip, s16, s17
[^>]*> ec5c 4a1f 	vmov	r4, ip, s30, s31
[^>]*> ec4c 4a10 	vmov	s0, s1, r4, ip
[^>]*> ec4c 4a30 	vmov	s1, s2, r4, ip
[^>]*> ec4c 4a11 	vmov	s2, s3, r4, ip
[^>]*> ec4c 4a12 	vmov	s4, s5, r4, ip
[^>]*> ec4c 4a14 	vmov	s8, s9, r4, ip
[^>]*> ec4c 4a18 	vmov	s16, s17, r4, ip
[^>]*> ec4c 4a1f 	vmov	s30, s31, r4, ip
[^>]*> ec5e 4b10 	vmov	r4, lr, d0
[^>]*> ec4e 4b10 	vmov	d0, r4, lr
[^>]*> ec5e 4b11 	vmov	r4, lr, d1
[^>]*> ec4e 4b11 	vmov	d1, r4, lr
[^>]*> ec5e 4b12 	vmov	r4, lr, d2
[^>]*> ec4e 4b12 	vmov	d2, r4, lr
[^>]*> ec5e 4b14 	vmov	r4, lr, d4
[^>]*> ec4e 4b14 	vmov	d4, r4, lr
[^>]*> ec5e 4b18 	vmov	r4, lr, d8
[^>]*> ec4e 4b18 	vmov	d8, r4, lr
[^>]*> ec5e 4b1f 	vmov	r4, lr, d15
[^>]*> ec4e 4b1f 	vmov	d15, r4, lr
[^>]*> ec5e 4a10 	vmov	r4, lr, s0, s1
[^>]*> ec5e 4a30 	vmov	r4, lr, s1, s2
[^>]*> ec5e 4a11 	vmov	r4, lr, s2, s3
[^>]*> ec5e 4a12 	vmov	r4, lr, s4, s5
[^>]*> ec5e 4a14 	vmov	r4, lr, s8, s9
[^>]*> ec5e 4a18 	vmov	r4, lr, s16, s17
[^>]*> ec5e 4a1f 	vmov	r4, lr, s30, s31
[^>]*> ec4e 4a10 	vmov	s0, s1, r4, lr
[^>]*> ec4e 4a30 	vmov	s1, s2, r4, lr
[^>]*> ec4e 4a11 	vmov	s2, s3, r4, lr
[^>]*> ec4e 4a12 	vmov	s4, s5, r4, lr
[^>]*> ec4e 4a14 	vmov	s8, s9, r4, lr
[^>]*> ec4e 4a18 	vmov	s16, s17, r4, lr
[^>]*> ec4e 4a1f 	vmov	s30, s31, r4, lr
[^>]*> ec50 7b10 	vmov	r7, r0, d0
[^>]*> ec40 7b10 	vmov	d0, r7, r0
[^>]*> ec50 7b11 	vmov	r7, r0, d1
[^>]*> ec40 7b11 	vmov	d1, r7, r0
[^>]*> ec50 7b12 	vmov	r7, r0, d2
[^>]*> ec40 7b12 	vmov	d2, r7, r0
[^>]*> ec50 7b14 	vmov	r7, r0, d4
[^>]*> ec40 7b14 	vmov	d4, r7, r0
[^>]*> ec50 7b18 	vmov	r7, r0, d8
[^>]*> ec40 7b18 	vmov	d8, r7, r0
[^>]*> ec50 7b1f 	vmov	r7, r0, d15
[^>]*> ec40 7b1f 	vmov	d15, r7, r0
[^>]*> ec50 7a10 	vmov	r7, r0, s0, s1
[^>]*> ec50 7a30 	vmov	r7, r0, s1, s2
[^>]*> ec50 7a11 	vmov	r7, r0, s2, s3
[^>]*> ec50 7a12 	vmov	r7, r0, s4, s5
[^>]*> ec50 7a14 	vmov	r7, r0, s8, s9
[^>]*> ec50 7a18 	vmov	r7, r0, s16, s17
[^>]*> ec50 7a1f 	vmov	r7, r0, s30, s31
[^>]*> ec40 7a10 	vmov	s0, s1, r7, r0
[^>]*> ec40 7a30 	vmov	s1, s2, r7, r0
[^>]*> ec40 7a11 	vmov	s2, s3, r7, r0
[^>]*> ec40 7a12 	vmov	s4, s5, r7, r0
[^>]*> ec40 7a14 	vmov	s8, s9, r7, r0
[^>]*> ec40 7a18 	vmov	s16, s17, r7, r0
[^>]*> ec40 7a1f 	vmov	s30, s31, r7, r0
[^>]*> ec51 7b10 	vmov	r7, r1, d0
[^>]*> ec41 7b10 	vmov	d0, r7, r1
[^>]*> ec51 7b11 	vmov	r7, r1, d1
[^>]*> ec41 7b11 	vmov	d1, r7, r1
[^>]*> ec51 7b12 	vmov	r7, r1, d2
[^>]*> ec41 7b12 	vmov	d2, r7, r1
[^>]*> ec51 7b14 	vmov	r7, r1, d4
[^>]*> ec41 7b14 	vmov	d4, r7, r1
[^>]*> ec51 7b18 	vmov	r7, r1, d8
[^>]*> ec41 7b18 	vmov	d8, r7, r1
[^>]*> ec51 7b1f 	vmov	r7, r1, d15
[^>]*> ec41 7b1f 	vmov	d15, r7, r1
[^>]*> ec51 7a10 	vmov	r7, r1, s0, s1
[^>]*> ec51 7a30 	vmov	r7, r1, s1, s2
[^>]*> ec51 7a11 	vmov	r7, r1, s2, s3
[^>]*> ec51 7a12 	vmov	r7, r1, s4, s5
[^>]*> ec51 7a14 	vmov	r7, r1, s8, s9
[^>]*> ec51 7a18 	vmov	r7, r1, s16, s17
[^>]*> ec51 7a1f 	vmov	r7, r1, s30, s31
[^>]*> ec41 7a10 	vmov	s0, s1, r7, r1
[^>]*> ec41 7a30 	vmov	s1, s2, r7, r1
[^>]*> ec41 7a11 	vmov	s2, s3, r7, r1
[^>]*> ec41 7a12 	vmov	s4, s5, r7, r1
[^>]*> ec41 7a14 	vmov	s8, s9, r7, r1
[^>]*> ec41 7a18 	vmov	s16, s17, r7, r1
[^>]*> ec41 7a1f 	vmov	s30, s31, r7, r1
[^>]*> ec52 7b10 	vmov	r7, r2, d0
[^>]*> ec42 7b10 	vmov	d0, r7, r2
[^>]*> ec52 7b11 	vmov	r7, r2, d1
[^>]*> ec42 7b11 	vmov	d1, r7, r2
[^>]*> ec52 7b12 	vmov	r7, r2, d2
[^>]*> ec42 7b12 	vmov	d2, r7, r2
[^>]*> ec52 7b14 	vmov	r7, r2, d4
[^>]*> ec42 7b14 	vmov	d4, r7, r2
[^>]*> ec52 7b18 	vmov	r7, r2, d8
[^>]*> ec42 7b18 	vmov	d8, r7, r2
[^>]*> ec52 7b1f 	vmov	r7, r2, d15
[^>]*> ec42 7b1f 	vmov	d15, r7, r2
[^>]*> ec52 7a10 	vmov	r7, r2, s0, s1
[^>]*> ec52 7a30 	vmov	r7, r2, s1, s2
[^>]*> ec52 7a11 	vmov	r7, r2, s2, s3
[^>]*> ec52 7a12 	vmov	r7, r2, s4, s5
[^>]*> ec52 7a14 	vmov	r7, r2, s8, s9
[^>]*> ec52 7a18 	vmov	r7, r2, s16, s17
[^>]*> ec52 7a1f 	vmov	r7, r2, s30, s31
[^>]*> ec42 7a10 	vmov	s0, s1, r7, r2
[^>]*> ec42 7a30 	vmov	s1, s2, r7, r2
[^>]*> ec42 7a11 	vmov	s2, s3, r7, r2
[^>]*> ec42 7a12 	vmov	s4, s5, r7, r2
[^>]*> ec42 7a14 	vmov	s8, s9, r7, r2
[^>]*> ec42 7a18 	vmov	s16, s17, r7, r2
[^>]*> ec42 7a1f 	vmov	s30, s31, r7, r2
[^>]*> ec54 7b10 	vmov	r7, r4, d0
[^>]*> ec44 7b10 	vmov	d0, r7, r4
[^>]*> ec54 7b11 	vmov	r7, r4, d1
[^>]*> ec44 7b11 	vmov	d1, r7, r4
[^>]*> ec54 7b12 	vmov	r7, r4, d2
[^>]*> ec44 7b12 	vmov	d2, r7, r4
[^>]*> ec54 7b14 	vmov	r7, r4, d4
[^>]*> ec44 7b14 	vmov	d4, r7, r4
[^>]*> ec54 7b18 	vmov	r7, r4, d8
[^>]*> ec44 7b18 	vmov	d8, r7, r4
[^>]*> ec54 7b1f 	vmov	r7, r4, d15
[^>]*> ec44 7b1f 	vmov	d15, r7, r4
[^>]*> ec54 7a10 	vmov	r7, r4, s0, s1
[^>]*> ec54 7a30 	vmov	r7, r4, s1, s2
[^>]*> ec54 7a11 	vmov	r7, r4, s2, s3
[^>]*> ec54 7a12 	vmov	r7, r4, s4, s5
[^>]*> ec54 7a14 	vmov	r7, r4, s8, s9
[^>]*> ec54 7a18 	vmov	r7, r4, s16, s17
[^>]*> ec54 7a1f 	vmov	r7, r4, s30, s31
[^>]*> ec44 7a10 	vmov	s0, s1, r7, r4
[^>]*> ec44 7a30 	vmov	s1, s2, r7, r4
[^>]*> ec44 7a11 	vmov	s2, s3, r7, r4
[^>]*> ec44 7a12 	vmov	s4, s5, r7, r4
[^>]*> ec44 7a14 	vmov	s8, s9, r7, r4
[^>]*> ec44 7a18 	vmov	s16, s17, r7, r4
[^>]*> ec44 7a1f 	vmov	s30, s31, r7, r4
[^>]*> ec58 7b10 	vmov	r7, r8, d0
[^>]*> ec48 7b10 	vmov	d0, r7, r8
[^>]*> ec58 7b11 	vmov	r7, r8, d1
[^>]*> ec48 7b11 	vmov	d1, r7, r8
[^>]*> ec58 7b12 	vmov	r7, r8, d2
[^>]*> ec48 7b12 	vmov	d2, r7, r8
[^>]*> ec58 7b14 	vmov	r7, r8, d4
[^>]*> ec48 7b14 	vmov	d4, r7, r8
[^>]*> ec58 7b18 	vmov	r7, r8, d8
[^>]*> ec48 7b18 	vmov	d8, r7, r8
[^>]*> ec58 7b1f 	vmov	r7, r8, d15
[^>]*> ec48 7b1f 	vmov	d15, r7, r8
[^>]*> ec58 7a10 	vmov	r7, r8, s0, s1
[^>]*> ec58 7a30 	vmov	r7, r8, s1, s2
[^>]*> ec58 7a11 	vmov	r7, r8, s2, s3
[^>]*> ec58 7a12 	vmov	r7, r8, s4, s5
[^>]*> ec58 7a14 	vmov	r7, r8, s8, s9
[^>]*> ec58 7a18 	vmov	r7, r8, s16, s17
[^>]*> ec58 7a1f 	vmov	r7, r8, s30, s31
[^>]*> ec48 7a10 	vmov	s0, s1, r7, r8
[^>]*> ec48 7a30 	vmov	s1, s2, r7, r8
[^>]*> ec48 7a11 	vmov	s2, s3, r7, r8
[^>]*> ec48 7a12 	vmov	s4, s5, r7, r8
[^>]*> ec48 7a14 	vmov	s8, s9, r7, r8
[^>]*> ec48 7a18 	vmov	s16, s17, r7, r8
[^>]*> ec48 7a1f 	vmov	s30, s31, r7, r8
[^>]*> ec5a 7b10 	vmov	r7, sl, d0
[^>]*> ec4a 7b10 	vmov	d0, r7, sl
[^>]*> ec5a 7b11 	vmov	r7, sl, d1
[^>]*> ec4a 7b11 	vmov	d1, r7, sl
[^>]*> ec5a 7b12 	vmov	r7, sl, d2
[^>]*> ec4a 7b12 	vmov	d2, r7, sl
[^>]*> ec5a 7b14 	vmov	r7, sl, d4
[^>]*> ec4a 7b14 	vmov	d4, r7, sl
[^>]*> ec5a 7b18 	vmov	r7, sl, d8
[^>]*> ec4a 7b18 	vmov	d8, r7, sl
[^>]*> ec5a 7b1f 	vmov	r7, sl, d15
[^>]*> ec4a 7b1f 	vmov	d15, r7, sl
[^>]*> ec5a 7a10 	vmov	r7, sl, s0, s1
[^>]*> ec5a 7a30 	vmov	r7, sl, s1, s2
[^>]*> ec5a 7a11 	vmov	r7, sl, s2, s3
[^>]*> ec5a 7a12 	vmov	r7, sl, s4, s5
[^>]*> ec5a 7a14 	vmov	r7, sl, s8, s9
[^>]*> ec5a 7a18 	vmov	r7, sl, s16, s17
[^>]*> ec5a 7a1f 	vmov	r7, sl, s30, s31
[^>]*> ec4a 7a10 	vmov	s0, s1, r7, sl
[^>]*> ec4a 7a30 	vmov	s1, s2, r7, sl
[^>]*> ec4a 7a11 	vmov	s2, s3, r7, sl
[^>]*> ec4a 7a12 	vmov	s4, s5, r7, sl
[^>]*> ec4a 7a14 	vmov	s8, s9, r7, sl
[^>]*> ec4a 7a18 	vmov	s16, s17, r7, sl
[^>]*> ec4a 7a1f 	vmov	s30, s31, r7, sl
[^>]*> ec5c 7b10 	vmov	r7, ip, d0
[^>]*> ec4c 7b10 	vmov	d0, r7, ip
[^>]*> ec5c 7b11 	vmov	r7, ip, d1
[^>]*> ec4c 7b11 	vmov	d1, r7, ip
[^>]*> ec5c 7b12 	vmov	r7, ip, d2
[^>]*> ec4c 7b12 	vmov	d2, r7, ip
[^>]*> ec5c 7b14 	vmov	r7, ip, d4
[^>]*> ec4c 7b14 	vmov	d4, r7, ip
[^>]*> ec5c 7b18 	vmov	r7, ip, d8
[^>]*> ec4c 7b18 	vmov	d8, r7, ip
[^>]*> ec5c 7b1f 	vmov	r7, ip, d15
[^>]*> ec4c 7b1f 	vmov	d15, r7, ip
[^>]*> ec5c 7a10 	vmov	r7, ip, s0, s1
[^>]*> ec5c 7a30 	vmov	r7, ip, s1, s2
[^>]*> ec5c 7a11 	vmov	r7, ip, s2, s3
[^>]*> ec5c 7a12 	vmov	r7, ip, s4, s5
[^>]*> ec5c 7a14 	vmov	r7, ip, s8, s9
[^>]*> ec5c 7a18 	vmov	r7, ip, s16, s17
[^>]*> ec5c 7a1f 	vmov	r7, ip, s30, s31
[^>]*> ec4c 7a10 	vmov	s0, s1, r7, ip
[^>]*> ec4c 7a30 	vmov	s1, s2, r7, ip
[^>]*> ec4c 7a11 	vmov	s2, s3, r7, ip
[^>]*> ec4c 7a12 	vmov	s4, s5, r7, ip
[^>]*> ec4c 7a14 	vmov	s8, s9, r7, ip
[^>]*> ec4c 7a18 	vmov	s16, s17, r7, ip
[^>]*> ec4c 7a1f 	vmov	s30, s31, r7, ip
[^>]*> ec5e 7b10 	vmov	r7, lr, d0
[^>]*> ec4e 7b10 	vmov	d0, r7, lr
[^>]*> ec5e 7b11 	vmov	r7, lr, d1
[^>]*> ec4e 7b11 	vmov	d1, r7, lr
[^>]*> ec5e 7b12 	vmov	r7, lr, d2
[^>]*> ec4e 7b12 	vmov	d2, r7, lr
[^>]*> ec5e 7b14 	vmov	r7, lr, d4
[^>]*> ec4e 7b14 	vmov	d4, r7, lr
[^>]*> ec5e 7b18 	vmov	r7, lr, d8
[^>]*> ec4e 7b18 	vmov	d8, r7, lr
[^>]*> ec5e 7b1f 	vmov	r7, lr, d15
[^>]*> ec4e 7b1f 	vmov	d15, r7, lr
[^>]*> ec5e 7a10 	vmov	r7, lr, s0, s1
[^>]*> ec5e 7a30 	vmov	r7, lr, s1, s2
[^>]*> ec5e 7a11 	vmov	r7, lr, s2, s3
[^>]*> ec5e 7a12 	vmov	r7, lr, s4, s5
[^>]*> ec5e 7a14 	vmov	r7, lr, s8, s9
[^>]*> ec5e 7a18 	vmov	r7, lr, s16, s17
[^>]*> ec5e 7a1f 	vmov	r7, lr, s30, s31
[^>]*> ec4e 7a10 	vmov	s0, s1, r7, lr
[^>]*> ec4e 7a30 	vmov	s1, s2, r7, lr
[^>]*> ec4e 7a11 	vmov	s2, s3, r7, lr
[^>]*> ec4e 7a12 	vmov	s4, s5, r7, lr
[^>]*> ec4e 7a14 	vmov	s8, s9, r7, lr
[^>]*> ec4e 7a18 	vmov	s16, s17, r7, lr
[^>]*> ec4e 7a1f 	vmov	s30, s31, r7, lr
[^>]*> ec50 8b10 	vmov	r8, r0, d0
[^>]*> ec40 8b10 	vmov	d0, r8, r0
[^>]*> ec50 8b11 	vmov	r8, r0, d1
[^>]*> ec40 8b11 	vmov	d1, r8, r0
[^>]*> ec50 8b12 	vmov	r8, r0, d2
[^>]*> ec40 8b12 	vmov	d2, r8, r0
[^>]*> ec50 8b14 	vmov	r8, r0, d4
[^>]*> ec40 8b14 	vmov	d4, r8, r0
[^>]*> ec50 8b18 	vmov	r8, r0, d8
[^>]*> ec40 8b18 	vmov	d8, r8, r0
[^>]*> ec50 8b1f 	vmov	r8, r0, d15
[^>]*> ec40 8b1f 	vmov	d15, r8, r0
[^>]*> ec50 8a10 	vmov	r8, r0, s0, s1
[^>]*> ec50 8a30 	vmov	r8, r0, s1, s2
[^>]*> ec50 8a11 	vmov	r8, r0, s2, s3
[^>]*> ec50 8a12 	vmov	r8, r0, s4, s5
[^>]*> ec50 8a14 	vmov	r8, r0, s8, s9
[^>]*> ec50 8a18 	vmov	r8, r0, s16, s17
[^>]*> ec50 8a1f 	vmov	r8, r0, s30, s31
[^>]*> ec40 8a10 	vmov	s0, s1, r8, r0
[^>]*> ec40 8a30 	vmov	s1, s2, r8, r0
[^>]*> ec40 8a11 	vmov	s2, s3, r8, r0
[^>]*> ec40 8a12 	vmov	s4, s5, r8, r0
[^>]*> ec40 8a14 	vmov	s8, s9, r8, r0
[^>]*> ec40 8a18 	vmov	s16, s17, r8, r0
[^>]*> ec40 8a1f 	vmov	s30, s31, r8, r0
[^>]*> ec51 8b10 	vmov	r8, r1, d0
[^>]*> ec41 8b10 	vmov	d0, r8, r1
[^>]*> ec51 8b11 	vmov	r8, r1, d1
[^>]*> ec41 8b11 	vmov	d1, r8, r1
[^>]*> ec51 8b12 	vmov	r8, r1, d2
[^>]*> ec41 8b12 	vmov	d2, r8, r1
[^>]*> ec51 8b14 	vmov	r8, r1, d4
[^>]*> ec41 8b14 	vmov	d4, r8, r1
[^>]*> ec51 8b18 	vmov	r8, r1, d8
[^>]*> ec41 8b18 	vmov	d8, r8, r1
[^>]*> ec51 8b1f 	vmov	r8, r1, d15
[^>]*> ec41 8b1f 	vmov	d15, r8, r1
[^>]*> ec51 8a10 	vmov	r8, r1, s0, s1
[^>]*> ec51 8a30 	vmov	r8, r1, s1, s2
[^>]*> ec51 8a11 	vmov	r8, r1, s2, s3
[^>]*> ec51 8a12 	vmov	r8, r1, s4, s5
[^>]*> ec51 8a14 	vmov	r8, r1, s8, s9
[^>]*> ec51 8a18 	vmov	r8, r1, s16, s17
[^>]*> ec51 8a1f 	vmov	r8, r1, s30, s31
[^>]*> ec41 8a10 	vmov	s0, s1, r8, r1
[^>]*> ec41 8a30 	vmov	s1, s2, r8, r1
[^>]*> ec41 8a11 	vmov	s2, s3, r8, r1
[^>]*> ec41 8a12 	vmov	s4, s5, r8, r1
[^>]*> ec41 8a14 	vmov	s8, s9, r8, r1
[^>]*> ec41 8a18 	vmov	s16, s17, r8, r1
[^>]*> ec41 8a1f 	vmov	s30, s31, r8, r1
[^>]*> ec52 8b10 	vmov	r8, r2, d0
[^>]*> ec42 8b10 	vmov	d0, r8, r2
[^>]*> ec52 8b11 	vmov	r8, r2, d1
[^>]*> ec42 8b11 	vmov	d1, r8, r2
[^>]*> ec52 8b12 	vmov	r8, r2, d2
[^>]*> ec42 8b12 	vmov	d2, r8, r2
[^>]*> ec52 8b14 	vmov	r8, r2, d4
[^>]*> ec42 8b14 	vmov	d4, r8, r2
[^>]*> ec52 8b18 	vmov	r8, r2, d8
[^>]*> ec42 8b18 	vmov	d8, r8, r2
[^>]*> ec52 8b1f 	vmov	r8, r2, d15
[^>]*> ec42 8b1f 	vmov	d15, r8, r2
[^>]*> ec52 8a10 	vmov	r8, r2, s0, s1
[^>]*> ec52 8a30 	vmov	r8, r2, s1, s2
[^>]*> ec52 8a11 	vmov	r8, r2, s2, s3
[^>]*> ec52 8a12 	vmov	r8, r2, s4, s5
[^>]*> ec52 8a14 	vmov	r8, r2, s8, s9
[^>]*> ec52 8a18 	vmov	r8, r2, s16, s17
[^>]*> ec52 8a1f 	vmov	r8, r2, s30, s31
[^>]*> ec42 8a10 	vmov	s0, s1, r8, r2
[^>]*> ec42 8a30 	vmov	s1, s2, r8, r2
[^>]*> ec42 8a11 	vmov	s2, s3, r8, r2
[^>]*> ec42 8a12 	vmov	s4, s5, r8, r2
[^>]*> ec42 8a14 	vmov	s8, s9, r8, r2
[^>]*> ec42 8a18 	vmov	s16, s17, r8, r2
[^>]*> ec42 8a1f 	vmov	s30, s31, r8, r2
[^>]*> ec54 8b10 	vmov	r8, r4, d0
[^>]*> ec44 8b10 	vmov	d0, r8, r4
[^>]*> ec54 8b11 	vmov	r8, r4, d1
[^>]*> ec44 8b11 	vmov	d1, r8, r4
[^>]*> ec54 8b12 	vmov	r8, r4, d2
[^>]*> ec44 8b12 	vmov	d2, r8, r4
[^>]*> ec54 8b14 	vmov	r8, r4, d4
[^>]*> ec44 8b14 	vmov	d4, r8, r4
[^>]*> ec54 8b18 	vmov	r8, r4, d8
[^>]*> ec44 8b18 	vmov	d8, r8, r4
[^>]*> ec54 8b1f 	vmov	r8, r4, d15
[^>]*> ec44 8b1f 	vmov	d15, r8, r4
[^>]*> ec54 8a10 	vmov	r8, r4, s0, s1
[^>]*> ec54 8a30 	vmov	r8, r4, s1, s2
[^>]*> ec54 8a11 	vmov	r8, r4, s2, s3
[^>]*> ec54 8a12 	vmov	r8, r4, s4, s5
[^>]*> ec54 8a14 	vmov	r8, r4, s8, s9
[^>]*> ec54 8a18 	vmov	r8, r4, s16, s17
[^>]*> ec54 8a1f 	vmov	r8, r4, s30, s31
[^>]*> ec44 8a10 	vmov	s0, s1, r8, r4
[^>]*> ec44 8a30 	vmov	s1, s2, r8, r4
[^>]*> ec44 8a11 	vmov	s2, s3, r8, r4
[^>]*> ec44 8a12 	vmov	s4, s5, r8, r4
[^>]*> ec44 8a14 	vmov	s8, s9, r8, r4
[^>]*> ec44 8a18 	vmov	s16, s17, r8, r4
[^>]*> ec44 8a1f 	vmov	s30, s31, r8, r4
[^>]*> ec57 8b10 	vmov	r8, r7, d0
[^>]*> ec47 8b10 	vmov	d0, r8, r7
[^>]*> ec57 8b11 	vmov	r8, r7, d1
[^>]*> ec47 8b11 	vmov	d1, r8, r7
[^>]*> ec57 8b12 	vmov	r8, r7, d2
[^>]*> ec47 8b12 	vmov	d2, r8, r7
[^>]*> ec57 8b14 	vmov	r8, r7, d4
[^>]*> ec47 8b14 	vmov	d4, r8, r7
[^>]*> ec57 8b18 	vmov	r8, r7, d8
[^>]*> ec47 8b18 	vmov	d8, r8, r7
[^>]*> ec57 8b1f 	vmov	r8, r7, d15
[^>]*> ec47 8b1f 	vmov	d15, r8, r7
[^>]*> ec57 8a10 	vmov	r8, r7, s0, s1
[^>]*> ec57 8a30 	vmov	r8, r7, s1, s2
[^>]*> ec57 8a11 	vmov	r8, r7, s2, s3
[^>]*> ec57 8a12 	vmov	r8, r7, s4, s5
[^>]*> ec57 8a14 	vmov	r8, r7, s8, s9
[^>]*> ec57 8a18 	vmov	r8, r7, s16, s17
[^>]*> ec57 8a1f 	vmov	r8, r7, s30, s31
[^>]*> ec47 8a10 	vmov	s0, s1, r8, r7
[^>]*> ec47 8a30 	vmov	s1, s2, r8, r7
[^>]*> ec47 8a11 	vmov	s2, s3, r8, r7
[^>]*> ec47 8a12 	vmov	s4, s5, r8, r7
[^>]*> ec47 8a14 	vmov	s8, s9, r8, r7
[^>]*> ec47 8a18 	vmov	s16, s17, r8, r7
[^>]*> ec47 8a1f 	vmov	s30, s31, r8, r7
[^>]*> ec5a 8b10 	vmov	r8, sl, d0
[^>]*> ec4a 8b10 	vmov	d0, r8, sl
[^>]*> ec5a 8b11 	vmov	r8, sl, d1
[^>]*> ec4a 8b11 	vmov	d1, r8, sl
[^>]*> ec5a 8b12 	vmov	r8, sl, d2
[^>]*> ec4a 8b12 	vmov	d2, r8, sl
[^>]*> ec5a 8b14 	vmov	r8, sl, d4
[^>]*> ec4a 8b14 	vmov	d4, r8, sl
[^>]*> ec5a 8b18 	vmov	r8, sl, d8
[^>]*> ec4a 8b18 	vmov	d8, r8, sl
[^>]*> ec5a 8b1f 	vmov	r8, sl, d15
[^>]*> ec4a 8b1f 	vmov	d15, r8, sl
[^>]*> ec5a 8a10 	vmov	r8, sl, s0, s1
[^>]*> ec5a 8a30 	vmov	r8, sl, s1, s2
[^>]*> ec5a 8a11 	vmov	r8, sl, s2, s3
[^>]*> ec5a 8a12 	vmov	r8, sl, s4, s5
[^>]*> ec5a 8a14 	vmov	r8, sl, s8, s9
[^>]*> ec5a 8a18 	vmov	r8, sl, s16, s17
[^>]*> ec5a 8a1f 	vmov	r8, sl, s30, s31
[^>]*> ec4a 8a10 	vmov	s0, s1, r8, sl
[^>]*> ec4a 8a30 	vmov	s1, s2, r8, sl
[^>]*> ec4a 8a11 	vmov	s2, s3, r8, sl
[^>]*> ec4a 8a12 	vmov	s4, s5, r8, sl
[^>]*> ec4a 8a14 	vmov	s8, s9, r8, sl
[^>]*> ec4a 8a18 	vmov	s16, s17, r8, sl
[^>]*> ec4a 8a1f 	vmov	s30, s31, r8, sl
[^>]*> ec5c 8b10 	vmov	r8, ip, d0
[^>]*> ec4c 8b10 	vmov	d0, r8, ip
[^>]*> ec5c 8b11 	vmov	r8, ip, d1
[^>]*> ec4c 8b11 	vmov	d1, r8, ip
[^>]*> ec5c 8b12 	vmov	r8, ip, d2
[^>]*> ec4c 8b12 	vmov	d2, r8, ip
[^>]*> ec5c 8b14 	vmov	r8, ip, d4
[^>]*> ec4c 8b14 	vmov	d4, r8, ip
[^>]*> ec5c 8b18 	vmov	r8, ip, d8
[^>]*> ec4c 8b18 	vmov	d8, r8, ip
[^>]*> ec5c 8b1f 	vmov	r8, ip, d15
[^>]*> ec4c 8b1f 	vmov	d15, r8, ip
[^>]*> ec5c 8a10 	vmov	r8, ip, s0, s1
[^>]*> ec5c 8a30 	vmov	r8, ip, s1, s2
[^>]*> ec5c 8a11 	vmov	r8, ip, s2, s3
[^>]*> ec5c 8a12 	vmov	r8, ip, s4, s5
[^>]*> ec5c 8a14 	vmov	r8, ip, s8, s9
[^>]*> ec5c 8a18 	vmov	r8, ip, s16, s17
[^>]*> ec5c 8a1f 	vmov	r8, ip, s30, s31
[^>]*> ec4c 8a10 	vmov	s0, s1, r8, ip
[^>]*> ec4c 8a30 	vmov	s1, s2, r8, ip
[^>]*> ec4c 8a11 	vmov	s2, s3, r8, ip
[^>]*> ec4c 8a12 	vmov	s4, s5, r8, ip
[^>]*> ec4c 8a14 	vmov	s8, s9, r8, ip
[^>]*> ec4c 8a18 	vmov	s16, s17, r8, ip
[^>]*> ec4c 8a1f 	vmov	s30, s31, r8, ip
[^>]*> ec5e 8b10 	vmov	r8, lr, d0
[^>]*> ec4e 8b10 	vmov	d0, r8, lr
[^>]*> ec5e 8b11 	vmov	r8, lr, d1
[^>]*> ec4e 8b11 	vmov	d1, r8, lr
[^>]*> ec5e 8b12 	vmov	r8, lr, d2
[^>]*> ec4e 8b12 	vmov	d2, r8, lr
[^>]*> ec5e 8b14 	vmov	r8, lr, d4
[^>]*> ec4e 8b14 	vmov	d4, r8, lr
[^>]*> ec5e 8b18 	vmov	r8, lr, d8
[^>]*> ec4e 8b18 	vmov	d8, r8, lr
[^>]*> ec5e 8b1f 	vmov	r8, lr, d15
[^>]*> ec4e 8b1f 	vmov	d15, r8, lr
[^>]*> ec5e 8a10 	vmov	r8, lr, s0, s1
[^>]*> ec5e 8a30 	vmov	r8, lr, s1, s2
[^>]*> ec5e 8a11 	vmov	r8, lr, s2, s3
[^>]*> ec5e 8a12 	vmov	r8, lr, s4, s5
[^>]*> ec5e 8a14 	vmov	r8, lr, s8, s9
[^>]*> ec5e 8a18 	vmov	r8, lr, s16, s17
[^>]*> ec5e 8a1f 	vmov	r8, lr, s30, s31
[^>]*> ec4e 8a10 	vmov	s0, s1, r8, lr
[^>]*> ec4e 8a30 	vmov	s1, s2, r8, lr
[^>]*> ec4e 8a11 	vmov	s2, s3, r8, lr
[^>]*> ec4e 8a12 	vmov	s4, s5, r8, lr
[^>]*> ec4e 8a14 	vmov	s8, s9, r8, lr
[^>]*> ec4e 8a18 	vmov	s16, s17, r8, lr
[^>]*> ec4e 8a1f 	vmov	s30, s31, r8, lr
[^>]*> ec50 ab10 	vmov	sl, r0, d0
[^>]*> ec40 ab10 	vmov	d0, sl, r0
[^>]*> ec50 ab11 	vmov	sl, r0, d1
[^>]*> ec40 ab11 	vmov	d1, sl, r0
[^>]*> ec50 ab12 	vmov	sl, r0, d2
[^>]*> ec40 ab12 	vmov	d2, sl, r0
[^>]*> ec50 ab14 	vmov	sl, r0, d4
[^>]*> ec40 ab14 	vmov	d4, sl, r0
[^>]*> ec50 ab18 	vmov	sl, r0, d8
[^>]*> ec40 ab18 	vmov	d8, sl, r0
[^>]*> ec50 ab1f 	vmov	sl, r0, d15
[^>]*> ec40 ab1f 	vmov	d15, sl, r0
[^>]*> ec50 aa10 	vmov	sl, r0, s0, s1
[^>]*> ec50 aa30 	vmov	sl, r0, s1, s2
[^>]*> ec50 aa11 	vmov	sl, r0, s2, s3
[^>]*> ec50 aa12 	vmov	sl, r0, s4, s5
[^>]*> ec50 aa14 	vmov	sl, r0, s8, s9
[^>]*> ec50 aa18 	vmov	sl, r0, s16, s17
[^>]*> ec50 aa1f 	vmov	sl, r0, s30, s31
[^>]*> ec40 aa10 	vmov	s0, s1, sl, r0
[^>]*> ec40 aa30 	vmov	s1, s2, sl, r0
[^>]*> ec40 aa11 	vmov	s2, s3, sl, r0
[^>]*> ec40 aa12 	vmov	s4, s5, sl, r0
[^>]*> ec40 aa14 	vmov	s8, s9, sl, r0
[^>]*> ec40 aa18 	vmov	s16, s17, sl, r0
[^>]*> ec40 aa1f 	vmov	s30, s31, sl, r0
[^>]*> ec51 ab10 	vmov	sl, r1, d0
[^>]*> ec41 ab10 	vmov	d0, sl, r1
[^>]*> ec51 ab11 	vmov	sl, r1, d1
[^>]*> ec41 ab11 	vmov	d1, sl, r1
[^>]*> ec51 ab12 	vmov	sl, r1, d2
[^>]*> ec41 ab12 	vmov	d2, sl, r1
[^>]*> ec51 ab14 	vmov	sl, r1, d4
[^>]*> ec41 ab14 	vmov	d4, sl, r1
[^>]*> ec51 ab18 	vmov	sl, r1, d8
[^>]*> ec41 ab18 	vmov	d8, sl, r1
[^>]*> ec51 ab1f 	vmov	sl, r1, d15
[^>]*> ec41 ab1f 	vmov	d15, sl, r1
[^>]*> ec51 aa10 	vmov	sl, r1, s0, s1
[^>]*> ec51 aa30 	vmov	sl, r1, s1, s2
[^>]*> ec51 aa11 	vmov	sl, r1, s2, s3
[^>]*> ec51 aa12 	vmov	sl, r1, s4, s5
[^>]*> ec51 aa14 	vmov	sl, r1, s8, s9
[^>]*> ec51 aa18 	vmov	sl, r1, s16, s17
[^>]*> ec51 aa1f 	vmov	sl, r1, s30, s31
[^>]*> ec41 aa10 	vmov	s0, s1, sl, r1
[^>]*> ec41 aa30 	vmov	s1, s2, sl, r1
[^>]*> ec41 aa11 	vmov	s2, s3, sl, r1
[^>]*> ec41 aa12 	vmov	s4, s5, sl, r1
[^>]*> ec41 aa14 	vmov	s8, s9, sl, r1
[^>]*> ec41 aa18 	vmov	s16, s17, sl, r1
[^>]*> ec41 aa1f 	vmov	s30, s31, sl, r1
[^>]*> ec52 ab10 	vmov	sl, r2, d0
[^>]*> ec42 ab10 	vmov	d0, sl, r2
[^>]*> ec52 ab11 	vmov	sl, r2, d1
[^>]*> ec42 ab11 	vmov	d1, sl, r2
[^>]*> ec52 ab12 	vmov	sl, r2, d2
[^>]*> ec42 ab12 	vmov	d2, sl, r2
[^>]*> ec52 ab14 	vmov	sl, r2, d4
[^>]*> ec42 ab14 	vmov	d4, sl, r2
[^>]*> ec52 ab18 	vmov	sl, r2, d8
[^>]*> ec42 ab18 	vmov	d8, sl, r2
[^>]*> ec52 ab1f 	vmov	sl, r2, d15
[^>]*> ec42 ab1f 	vmov	d15, sl, r2
[^>]*> ec52 aa10 	vmov	sl, r2, s0, s1
[^>]*> ec52 aa30 	vmov	sl, r2, s1, s2
[^>]*> ec52 aa11 	vmov	sl, r2, s2, s3
[^>]*> ec52 aa12 	vmov	sl, r2, s4, s5
[^>]*> ec52 aa14 	vmov	sl, r2, s8, s9
[^>]*> ec52 aa18 	vmov	sl, r2, s16, s17
[^>]*> ec52 aa1f 	vmov	sl, r2, s30, s31
[^>]*> ec42 aa10 	vmov	s0, s1, sl, r2
[^>]*> ec42 aa30 	vmov	s1, s2, sl, r2
[^>]*> ec42 aa11 	vmov	s2, s3, sl, r2
[^>]*> ec42 aa12 	vmov	s4, s5, sl, r2
[^>]*> ec42 aa14 	vmov	s8, s9, sl, r2
[^>]*> ec42 aa18 	vmov	s16, s17, sl, r2
[^>]*> ec42 aa1f 	vmov	s30, s31, sl, r2
[^>]*> ec54 ab10 	vmov	sl, r4, d0
[^>]*> ec44 ab10 	vmov	d0, sl, r4
[^>]*> ec54 ab11 	vmov	sl, r4, d1
[^>]*> ec44 ab11 	vmov	d1, sl, r4
[^>]*> ec54 ab12 	vmov	sl, r4, d2
[^>]*> ec44 ab12 	vmov	d2, sl, r4
[^>]*> ec54 ab14 	vmov	sl, r4, d4
[^>]*> ec44 ab14 	vmov	d4, sl, r4
[^>]*> ec54 ab18 	vmov	sl, r4, d8
[^>]*> ec44 ab18 	vmov	d8, sl, r4
[^>]*> ec54 ab1f 	vmov	sl, r4, d15
[^>]*> ec44 ab1f 	vmov	d15, sl, r4
[^>]*> ec54 aa10 	vmov	sl, r4, s0, s1
[^>]*> ec54 aa30 	vmov	sl, r4, s1, s2
[^>]*> ec54 aa11 	vmov	sl, r4, s2, s3
[^>]*> ec54 aa12 	vmov	sl, r4, s4, s5
[^>]*> ec54 aa14 	vmov	sl, r4, s8, s9
[^>]*> ec54 aa18 	vmov	sl, r4, s16, s17
[^>]*> ec54 aa1f 	vmov	sl, r4, s30, s31
[^>]*> ec44 aa10 	vmov	s0, s1, sl, r4
[^>]*> ec44 aa30 	vmov	s1, s2, sl, r4
[^>]*> ec44 aa11 	vmov	s2, s3, sl, r4
[^>]*> ec44 aa12 	vmov	s4, s5, sl, r4
[^>]*> ec44 aa14 	vmov	s8, s9, sl, r4
[^>]*> ec44 aa18 	vmov	s16, s17, sl, r4
[^>]*> ec44 aa1f 	vmov	s30, s31, sl, r4
[^>]*> ec57 ab10 	vmov	sl, r7, d0
[^>]*> ec47 ab10 	vmov	d0, sl, r7
[^>]*> ec57 ab11 	vmov	sl, r7, d1
[^>]*> ec47 ab11 	vmov	d1, sl, r7
[^>]*> ec57 ab12 	vmov	sl, r7, d2
[^>]*> ec47 ab12 	vmov	d2, sl, r7
[^>]*> ec57 ab14 	vmov	sl, r7, d4
[^>]*> ec47 ab14 	vmov	d4, sl, r7
[^>]*> ec57 ab18 	vmov	sl, r7, d8
[^>]*> ec47 ab18 	vmov	d8, sl, r7
[^>]*> ec57 ab1f 	vmov	sl, r7, d15
[^>]*> ec47 ab1f 	vmov	d15, sl, r7
[^>]*> ec57 aa10 	vmov	sl, r7, s0, s1
[^>]*> ec57 aa30 	vmov	sl, r7, s1, s2
[^>]*> ec57 aa11 	vmov	sl, r7, s2, s3
[^>]*> ec57 aa12 	vmov	sl, r7, s4, s5
[^>]*> ec57 aa14 	vmov	sl, r7, s8, s9
[^>]*> ec57 aa18 	vmov	sl, r7, s16, s17
[^>]*> ec57 aa1f 	vmov	sl, r7, s30, s31
[^>]*> ec47 aa10 	vmov	s0, s1, sl, r7
[^>]*> ec47 aa30 	vmov	s1, s2, sl, r7
[^>]*> ec47 aa11 	vmov	s2, s3, sl, r7
[^>]*> ec47 aa12 	vmov	s4, s5, sl, r7
[^>]*> ec47 aa14 	vmov	s8, s9, sl, r7
[^>]*> ec47 aa18 	vmov	s16, s17, sl, r7
[^>]*> ec47 aa1f 	vmov	s30, s31, sl, r7
[^>]*> ec58 ab10 	vmov	sl, r8, d0
[^>]*> ec48 ab10 	vmov	d0, sl, r8
[^>]*> ec58 ab11 	vmov	sl, r8, d1
[^>]*> ec48 ab11 	vmov	d1, sl, r8
[^>]*> ec58 ab12 	vmov	sl, r8, d2
[^>]*> ec48 ab12 	vmov	d2, sl, r8
[^>]*> ec58 ab14 	vmov	sl, r8, d4
[^>]*> ec48 ab14 	vmov	d4, sl, r8
[^>]*> ec58 ab18 	vmov	sl, r8, d8
[^>]*> ec48 ab18 	vmov	d8, sl, r8
[^>]*> ec58 ab1f 	vmov	sl, r8, d15
[^>]*> ec48 ab1f 	vmov	d15, sl, r8
[^>]*> ec58 aa10 	vmov	sl, r8, s0, s1
[^>]*> ec58 aa30 	vmov	sl, r8, s1, s2
[^>]*> ec58 aa11 	vmov	sl, r8, s2, s3
[^>]*> ec58 aa12 	vmov	sl, r8, s4, s5
[^>]*> ec58 aa14 	vmov	sl, r8, s8, s9
[^>]*> ec58 aa18 	vmov	sl, r8, s16, s17
[^>]*> ec58 aa1f 	vmov	sl, r8, s30, s31
[^>]*> ec48 aa10 	vmov	s0, s1, sl, r8
[^>]*> ec48 aa30 	vmov	s1, s2, sl, r8
[^>]*> ec48 aa11 	vmov	s2, s3, sl, r8
[^>]*> ec48 aa12 	vmov	s4, s5, sl, r8
[^>]*> ec48 aa14 	vmov	s8, s9, sl, r8
[^>]*> ec48 aa18 	vmov	s16, s17, sl, r8
[^>]*> ec48 aa1f 	vmov	s30, s31, sl, r8
[^>]*> ec5c ab10 	vmov	sl, ip, d0
[^>]*> ec4c ab10 	vmov	d0, sl, ip
[^>]*> ec5c ab11 	vmov	sl, ip, d1
[^>]*> ec4c ab11 	vmov	d1, sl, ip
[^>]*> ec5c ab12 	vmov	sl, ip, d2
[^>]*> ec4c ab12 	vmov	d2, sl, ip
[^>]*> ec5c ab14 	vmov	sl, ip, d4
[^>]*> ec4c ab14 	vmov	d4, sl, ip
[^>]*> ec5c ab18 	vmov	sl, ip, d8
[^>]*> ec4c ab18 	vmov	d8, sl, ip
[^>]*> ec5c ab1f 	vmov	sl, ip, d15
[^>]*> ec4c ab1f 	vmov	d15, sl, ip
[^>]*> ec5c aa10 	vmov	sl, ip, s0, s1
[^>]*> ec5c aa30 	vmov	sl, ip, s1, s2
[^>]*> ec5c aa11 	vmov	sl, ip, s2, s3
[^>]*> ec5c aa12 	vmov	sl, ip, s4, s5
[^>]*> ec5c aa14 	vmov	sl, ip, s8, s9
[^>]*> ec5c aa18 	vmov	sl, ip, s16, s17
[^>]*> ec5c aa1f 	vmov	sl, ip, s30, s31
[^>]*> ec4c aa10 	vmov	s0, s1, sl, ip
[^>]*> ec4c aa30 	vmov	s1, s2, sl, ip
[^>]*> ec4c aa11 	vmov	s2, s3, sl, ip
[^>]*> ec4c aa12 	vmov	s4, s5, sl, ip
[^>]*> ec4c aa14 	vmov	s8, s9, sl, ip
[^>]*> ec4c aa18 	vmov	s16, s17, sl, ip
[^>]*> ec4c aa1f 	vmov	s30, s31, sl, ip
[^>]*> ec5e ab10 	vmov	sl, lr, d0
[^>]*> ec4e ab10 	vmov	d0, sl, lr
[^>]*> ec5e ab11 	vmov	sl, lr, d1
[^>]*> ec4e ab11 	vmov	d1, sl, lr
[^>]*> ec5e ab12 	vmov	sl, lr, d2
[^>]*> ec4e ab12 	vmov	d2, sl, lr
[^>]*> ec5e ab14 	vmov	sl, lr, d4
[^>]*> ec4e ab14 	vmov	d4, sl, lr
[^>]*> ec5e ab18 	vmov	sl, lr, d8
[^>]*> ec4e ab18 	vmov	d8, sl, lr
[^>]*> ec5e ab1f 	vmov	sl, lr, d15
[^>]*> ec4e ab1f 	vmov	d15, sl, lr
[^>]*> ec5e aa10 	vmov	sl, lr, s0, s1
[^>]*> ec5e aa30 	vmov	sl, lr, s1, s2
[^>]*> ec5e aa11 	vmov	sl, lr, s2, s3
[^>]*> ec5e aa12 	vmov	sl, lr, s4, s5
[^>]*> ec5e aa14 	vmov	sl, lr, s8, s9
[^>]*> ec5e aa18 	vmov	sl, lr, s16, s17
[^>]*> ec5e aa1f 	vmov	sl, lr, s30, s31
[^>]*> ec4e aa10 	vmov	s0, s1, sl, lr
[^>]*> ec4e aa30 	vmov	s1, s2, sl, lr
[^>]*> ec4e aa11 	vmov	s2, s3, sl, lr
[^>]*> ec4e aa12 	vmov	s4, s5, sl, lr
[^>]*> ec4e aa14 	vmov	s8, s9, sl, lr
[^>]*> ec4e aa18 	vmov	s16, s17, sl, lr
[^>]*> ec4e aa1f 	vmov	s30, s31, sl, lr
[^>]*> ec50 cb10 	vmov	ip, r0, d0
[^>]*> ec40 cb10 	vmov	d0, ip, r0
[^>]*> ec50 cb11 	vmov	ip, r0, d1
[^>]*> ec40 cb11 	vmov	d1, ip, r0
[^>]*> ec50 cb12 	vmov	ip, r0, d2
[^>]*> ec40 cb12 	vmov	d2, ip, r0
[^>]*> ec50 cb14 	vmov	ip, r0, d4
[^>]*> ec40 cb14 	vmov	d4, ip, r0
[^>]*> ec50 cb18 	vmov	ip, r0, d8
[^>]*> ec40 cb18 	vmov	d8, ip, r0
[^>]*> ec50 cb1f 	vmov	ip, r0, d15
[^>]*> ec40 cb1f 	vmov	d15, ip, r0
[^>]*> ec50 ca10 	vmov	ip, r0, s0, s1
[^>]*> ec50 ca30 	vmov	ip, r0, s1, s2
[^>]*> ec50 ca11 	vmov	ip, r0, s2, s3
[^>]*> ec50 ca12 	vmov	ip, r0, s4, s5
[^>]*> ec50 ca14 	vmov	ip, r0, s8, s9
[^>]*> ec50 ca18 	vmov	ip, r0, s16, s17
[^>]*> ec50 ca1f 	vmov	ip, r0, s30, s31
[^>]*> ec40 ca10 	vmov	s0, s1, ip, r0
[^>]*> ec40 ca30 	vmov	s1, s2, ip, r0
[^>]*> ec40 ca11 	vmov	s2, s3, ip, r0
[^>]*> ec40 ca12 	vmov	s4, s5, ip, r0
[^>]*> ec40 ca14 	vmov	s8, s9, ip, r0
[^>]*> ec40 ca18 	vmov	s16, s17, ip, r0
[^>]*> ec40 ca1f 	vmov	s30, s31, ip, r0
[^>]*> ec51 cb10 	vmov	ip, r1, d0
[^>]*> ec41 cb10 	vmov	d0, ip, r1
[^>]*> ec51 cb11 	vmov	ip, r1, d1
[^>]*> ec41 cb11 	vmov	d1, ip, r1
[^>]*> ec51 cb12 	vmov	ip, r1, d2
[^>]*> ec41 cb12 	vmov	d2, ip, r1
[^>]*> ec51 cb14 	vmov	ip, r1, d4
[^>]*> ec41 cb14 	vmov	d4, ip, r1
[^>]*> ec51 cb18 	vmov	ip, r1, d8
[^>]*> ec41 cb18 	vmov	d8, ip, r1
[^>]*> ec51 cb1f 	vmov	ip, r1, d15
[^>]*> ec41 cb1f 	vmov	d15, ip, r1
[^>]*> ec51 ca10 	vmov	ip, r1, s0, s1
[^>]*> ec51 ca30 	vmov	ip, r1, s1, s2
[^>]*> ec51 ca11 	vmov	ip, r1, s2, s3
[^>]*> ec51 ca12 	vmov	ip, r1, s4, s5
[^>]*> ec51 ca14 	vmov	ip, r1, s8, s9
[^>]*> ec51 ca18 	vmov	ip, r1, s16, s17
[^>]*> ec51 ca1f 	vmov	ip, r1, s30, s31
[^>]*> ec41 ca10 	vmov	s0, s1, ip, r1
[^>]*> ec41 ca30 	vmov	s1, s2, ip, r1
[^>]*> ec41 ca11 	vmov	s2, s3, ip, r1
[^>]*> ec41 ca12 	vmov	s4, s5, ip, r1
[^>]*> ec41 ca14 	vmov	s8, s9, ip, r1
[^>]*> ec41 ca18 	vmov	s16, s17, ip, r1
[^>]*> ec41 ca1f 	vmov	s30, s31, ip, r1
[^>]*> ec52 cb10 	vmov	ip, r2, d0
[^>]*> ec42 cb10 	vmov	d0, ip, r2
[^>]*> ec52 cb11 	vmov	ip, r2, d1
[^>]*> ec42 cb11 	vmov	d1, ip, r2
[^>]*> ec52 cb12 	vmov	ip, r2, d2
[^>]*> ec42 cb12 	vmov	d2, ip, r2
[^>]*> ec52 cb14 	vmov	ip, r2, d4
[^>]*> ec42 cb14 	vmov	d4, ip, r2
[^>]*> ec52 cb18 	vmov	ip, r2, d8
[^>]*> ec42 cb18 	vmov	d8, ip, r2
[^>]*> ec52 cb1f 	vmov	ip, r2, d15
[^>]*> ec42 cb1f 	vmov	d15, ip, r2
[^>]*> ec52 ca10 	vmov	ip, r2, s0, s1
[^>]*> ec52 ca30 	vmov	ip, r2, s1, s2
[^>]*> ec52 ca11 	vmov	ip, r2, s2, s3
[^>]*> ec52 ca12 	vmov	ip, r2, s4, s5
[^>]*> ec52 ca14 	vmov	ip, r2, s8, s9
[^>]*> ec52 ca18 	vmov	ip, r2, s16, s17
[^>]*> ec52 ca1f 	vmov	ip, r2, s30, s31
[^>]*> ec42 ca10 	vmov	s0, s1, ip, r2
[^>]*> ec42 ca30 	vmov	s1, s2, ip, r2
[^>]*> ec42 ca11 	vmov	s2, s3, ip, r2
[^>]*> ec42 ca12 	vmov	s4, s5, ip, r2
[^>]*> ec42 ca14 	vmov	s8, s9, ip, r2
[^>]*> ec42 ca18 	vmov	s16, s17, ip, r2
[^>]*> ec42 ca1f 	vmov	s30, s31, ip, r2
[^>]*> ec54 cb10 	vmov	ip, r4, d0
[^>]*> ec44 cb10 	vmov	d0, ip, r4
[^>]*> ec54 cb11 	vmov	ip, r4, d1
[^>]*> ec44 cb11 	vmov	d1, ip, r4
[^>]*> ec54 cb12 	vmov	ip, r4, d2
[^>]*> ec44 cb12 	vmov	d2, ip, r4
[^>]*> ec54 cb14 	vmov	ip, r4, d4
[^>]*> ec44 cb14 	vmov	d4, ip, r4
[^>]*> ec54 cb18 	vmov	ip, r4, d8
[^>]*> ec44 cb18 	vmov	d8, ip, r4
[^>]*> ec54 cb1f 	vmov	ip, r4, d15
[^>]*> ec44 cb1f 	vmov	d15, ip, r4
[^>]*> ec54 ca10 	vmov	ip, r4, s0, s1
[^>]*> ec54 ca30 	vmov	ip, r4, s1, s2
[^>]*> ec54 ca11 	vmov	ip, r4, s2, s3
[^>]*> ec54 ca12 	vmov	ip, r4, s4, s5
[^>]*> ec54 ca14 	vmov	ip, r4, s8, s9
[^>]*> ec54 ca18 	vmov	ip, r4, s16, s17
[^>]*> ec54 ca1f 	vmov	ip, r4, s30, s31
[^>]*> ec44 ca10 	vmov	s0, s1, ip, r4
[^>]*> ec44 ca30 	vmov	s1, s2, ip, r4
[^>]*> ec44 ca11 	vmov	s2, s3, ip, r4
[^>]*> ec44 ca12 	vmov	s4, s5, ip, r4
[^>]*> ec44 ca14 	vmov	s8, s9, ip, r4
[^>]*> ec44 ca18 	vmov	s16, s17, ip, r4
[^>]*> ec44 ca1f 	vmov	s30, s31, ip, r4
[^>]*> ec57 cb10 	vmov	ip, r7, d0
[^>]*> ec47 cb10 	vmov	d0, ip, r7
[^>]*> ec57 cb11 	vmov	ip, r7, d1
[^>]*> ec47 cb11 	vmov	d1, ip, r7
[^>]*> ec57 cb12 	vmov	ip, r7, d2
[^>]*> ec47 cb12 	vmov	d2, ip, r7
[^>]*> ec57 cb14 	vmov	ip, r7, d4
[^>]*> ec47 cb14 	vmov	d4, ip, r7
[^>]*> ec57 cb18 	vmov	ip, r7, d8
[^>]*> ec47 cb18 	vmov	d8, ip, r7
[^>]*> ec57 cb1f 	vmov	ip, r7, d15
[^>]*> ec47 cb1f 	vmov	d15, ip, r7
[^>]*> ec57 ca10 	vmov	ip, r7, s0, s1
[^>]*> ec57 ca30 	vmov	ip, r7, s1, s2
[^>]*> ec57 ca11 	vmov	ip, r7, s2, s3
[^>]*> ec57 ca12 	vmov	ip, r7, s4, s5
[^>]*> ec57 ca14 	vmov	ip, r7, s8, s9
[^>]*> ec57 ca18 	vmov	ip, r7, s16, s17
[^>]*> ec57 ca1f 	vmov	ip, r7, s30, s31
[^>]*> ec47 ca10 	vmov	s0, s1, ip, r7
[^>]*> ec47 ca30 	vmov	s1, s2, ip, r7
[^>]*> ec47 ca11 	vmov	s2, s3, ip, r7
[^>]*> ec47 ca12 	vmov	s4, s5, ip, r7
[^>]*> ec47 ca14 	vmov	s8, s9, ip, r7
[^>]*> ec47 ca18 	vmov	s16, s17, ip, r7
[^>]*> ec47 ca1f 	vmov	s30, s31, ip, r7
[^>]*> ec58 cb10 	vmov	ip, r8, d0
[^>]*> ec48 cb10 	vmov	d0, ip, r8
[^>]*> ec58 cb11 	vmov	ip, r8, d1
[^>]*> ec48 cb11 	vmov	d1, ip, r8
[^>]*> ec58 cb12 	vmov	ip, r8, d2
[^>]*> ec48 cb12 	vmov	d2, ip, r8
[^>]*> ec58 cb14 	vmov	ip, r8, d4
[^>]*> ec48 cb14 	vmov	d4, ip, r8
[^>]*> ec58 cb18 	vmov	ip, r8, d8
[^>]*> ec48 cb18 	vmov	d8, ip, r8
[^>]*> ec58 cb1f 	vmov	ip, r8, d15
[^>]*> ec48 cb1f 	vmov	d15, ip, r8
[^>]*> ec58 ca10 	vmov	ip, r8, s0, s1
[^>]*> ec58 ca30 	vmov	ip, r8, s1, s2
[^>]*> ec58 ca11 	vmov	ip, r8, s2, s3
[^>]*> ec58 ca12 	vmov	ip, r8, s4, s5
[^>]*> ec58 ca14 	vmov	ip, r8, s8, s9
[^>]*> ec58 ca18 	vmov	ip, r8, s16, s17
[^>]*> ec58 ca1f 	vmov	ip, r8, s30, s31
[^>]*> ec48 ca10 	vmov	s0, s1, ip, r8
[^>]*> ec48 ca30 	vmov	s1, s2, ip, r8
[^>]*> ec48 ca11 	vmov	s2, s3, ip, r8
[^>]*> ec48 ca12 	vmov	s4, s5, ip, r8
[^>]*> ec48 ca14 	vmov	s8, s9, ip, r8
[^>]*> ec48 ca18 	vmov	s16, s17, ip, r8
[^>]*> ec48 ca1f 	vmov	s30, s31, ip, r8
[^>]*> ec5a cb10 	vmov	ip, sl, d0
[^>]*> ec4a cb10 	vmov	d0, ip, sl
[^>]*> ec5a cb11 	vmov	ip, sl, d1
[^>]*> ec4a cb11 	vmov	d1, ip, sl
[^>]*> ec5a cb12 	vmov	ip, sl, d2
[^>]*> ec4a cb12 	vmov	d2, ip, sl
[^>]*> ec5a cb14 	vmov	ip, sl, d4
[^>]*> ec4a cb14 	vmov	d4, ip, sl
[^>]*> ec5a cb18 	vmov	ip, sl, d8
[^>]*> ec4a cb18 	vmov	d8, ip, sl
[^>]*> ec5a cb1f 	vmov	ip, sl, d15
[^>]*> ec4a cb1f 	vmov	d15, ip, sl
[^>]*> ec5a ca10 	vmov	ip, sl, s0, s1
[^>]*> ec5a ca30 	vmov	ip, sl, s1, s2
[^>]*> ec5a ca11 	vmov	ip, sl, s2, s3
[^>]*> ec5a ca12 	vmov	ip, sl, s4, s5
[^>]*> ec5a ca14 	vmov	ip, sl, s8, s9
[^>]*> ec5a ca18 	vmov	ip, sl, s16, s17
[^>]*> ec5a ca1f 	vmov	ip, sl, s30, s31
[^>]*> ec4a ca10 	vmov	s0, s1, ip, sl
[^>]*> ec4a ca30 	vmov	s1, s2, ip, sl
[^>]*> ec4a ca11 	vmov	s2, s3, ip, sl
[^>]*> ec4a ca12 	vmov	s4, s5, ip, sl
[^>]*> ec4a ca14 	vmov	s8, s9, ip, sl
[^>]*> ec4a ca18 	vmov	s16, s17, ip, sl
[^>]*> ec4a ca1f 	vmov	s30, s31, ip, sl
[^>]*> ec5e cb10 	vmov	ip, lr, d0
[^>]*> ec4e cb10 	vmov	d0, ip, lr
[^>]*> ec5e cb11 	vmov	ip, lr, d1
[^>]*> ec4e cb11 	vmov	d1, ip, lr
[^>]*> ec5e cb12 	vmov	ip, lr, d2
[^>]*> ec4e cb12 	vmov	d2, ip, lr
[^>]*> ec5e cb14 	vmov	ip, lr, d4
[^>]*> ec4e cb14 	vmov	d4, ip, lr
[^>]*> ec5e cb18 	vmov	ip, lr, d8
[^>]*> ec4e cb18 	vmov	d8, ip, lr
[^>]*> ec5e cb1f 	vmov	ip, lr, d15
[^>]*> ec4e cb1f 	vmov	d15, ip, lr
[^>]*> ec5e ca10 	vmov	ip, lr, s0, s1
[^>]*> ec5e ca30 	vmov	ip, lr, s1, s2
[^>]*> ec5e ca11 	vmov	ip, lr, s2, s3
[^>]*> ec5e ca12 	vmov	ip, lr, s4, s5
[^>]*> ec5e ca14 	vmov	ip, lr, s8, s9
[^>]*> ec5e ca18 	vmov	ip, lr, s16, s17
[^>]*> ec5e ca1f 	vmov	ip, lr, s30, s31
[^>]*> ec4e ca10 	vmov	s0, s1, ip, lr
[^>]*> ec4e ca30 	vmov	s1, s2, ip, lr
[^>]*> ec4e ca11 	vmov	s2, s3, ip, lr
[^>]*> ec4e ca12 	vmov	s4, s5, ip, lr
[^>]*> ec4e ca14 	vmov	s8, s9, ip, lr
[^>]*> ec4e ca18 	vmov	s16, s17, ip, lr
[^>]*> ec4e ca1f 	vmov	s30, s31, ip, lr
[^>]*> ec50 eb10 	vmov	lr, r0, d0
[^>]*> ec40 eb10 	vmov	d0, lr, r0
[^>]*> ec50 eb11 	vmov	lr, r0, d1
[^>]*> ec40 eb11 	vmov	d1, lr, r0
[^>]*> ec50 eb12 	vmov	lr, r0, d2
[^>]*> ec40 eb12 	vmov	d2, lr, r0
[^>]*> ec50 eb14 	vmov	lr, r0, d4
[^>]*> ec40 eb14 	vmov	d4, lr, r0
[^>]*> ec50 eb18 	vmov	lr, r0, d8
[^>]*> ec40 eb18 	vmov	d8, lr, r0
[^>]*> ec50 eb1f 	vmov	lr, r0, d15
[^>]*> ec40 eb1f 	vmov	d15, lr, r0
[^>]*> ec50 ea10 	vmov	lr, r0, s0, s1
[^>]*> ec50 ea30 	vmov	lr, r0, s1, s2
[^>]*> ec50 ea11 	vmov	lr, r0, s2, s3
[^>]*> ec50 ea12 	vmov	lr, r0, s4, s5
[^>]*> ec50 ea14 	vmov	lr, r0, s8, s9
[^>]*> ec50 ea18 	vmov	lr, r0, s16, s17
[^>]*> ec50 ea1f 	vmov	lr, r0, s30, s31
[^>]*> ec40 ea10 	vmov	s0, s1, lr, r0
[^>]*> ec40 ea30 	vmov	s1, s2, lr, r0
[^>]*> ec40 ea11 	vmov	s2, s3, lr, r0
[^>]*> ec40 ea12 	vmov	s4, s5, lr, r0
[^>]*> ec40 ea14 	vmov	s8, s9, lr, r0
[^>]*> ec40 ea18 	vmov	s16, s17, lr, r0
[^>]*> ec40 ea1f 	vmov	s30, s31, lr, r0
[^>]*> ec51 eb10 	vmov	lr, r1, d0
[^>]*> ec41 eb10 	vmov	d0, lr, r1
[^>]*> ec51 eb11 	vmov	lr, r1, d1
[^>]*> ec41 eb11 	vmov	d1, lr, r1
[^>]*> ec51 eb12 	vmov	lr, r1, d2
[^>]*> ec41 eb12 	vmov	d2, lr, r1
[^>]*> ec51 eb14 	vmov	lr, r1, d4
[^>]*> ec41 eb14 	vmov	d4, lr, r1
[^>]*> ec51 eb18 	vmov	lr, r1, d8
[^>]*> ec41 eb18 	vmov	d8, lr, r1
[^>]*> ec51 eb1f 	vmov	lr, r1, d15
[^>]*> ec41 eb1f 	vmov	d15, lr, r1
[^>]*> ec51 ea10 	vmov	lr, r1, s0, s1
[^>]*> ec51 ea30 	vmov	lr, r1, s1, s2
[^>]*> ec51 ea11 	vmov	lr, r1, s2, s3
[^>]*> ec51 ea12 	vmov	lr, r1, s4, s5
[^>]*> ec51 ea14 	vmov	lr, r1, s8, s9
[^>]*> ec51 ea18 	vmov	lr, r1, s16, s17
[^>]*> ec51 ea1f 	vmov	lr, r1, s30, s31
[^>]*> ec41 ea10 	vmov	s0, s1, lr, r1
[^>]*> ec41 ea30 	vmov	s1, s2, lr, r1
[^>]*> ec41 ea11 	vmov	s2, s3, lr, r1
[^>]*> ec41 ea12 	vmov	s4, s5, lr, r1
[^>]*> ec41 ea14 	vmov	s8, s9, lr, r1
[^>]*> ec41 ea18 	vmov	s16, s17, lr, r1
[^>]*> ec41 ea1f 	vmov	s30, s31, lr, r1
[^>]*> ec52 eb10 	vmov	lr, r2, d0
[^>]*> ec42 eb10 	vmov	d0, lr, r2
[^>]*> ec52 eb11 	vmov	lr, r2, d1
[^>]*> ec42 eb11 	vmov	d1, lr, r2
[^>]*> ec52 eb12 	vmov	lr, r2, d2
[^>]*> ec42 eb12 	vmov	d2, lr, r2
[^>]*> ec52 eb14 	vmov	lr, r2, d4
[^>]*> ec42 eb14 	vmov	d4, lr, r2
[^>]*> ec52 eb18 	vmov	lr, r2, d8
[^>]*> ec42 eb18 	vmov	d8, lr, r2
[^>]*> ec52 eb1f 	vmov	lr, r2, d15
[^>]*> ec42 eb1f 	vmov	d15, lr, r2
[^>]*> ec52 ea10 	vmov	lr, r2, s0, s1
[^>]*> ec52 ea30 	vmov	lr, r2, s1, s2
[^>]*> ec52 ea11 	vmov	lr, r2, s2, s3
[^>]*> ec52 ea12 	vmov	lr, r2, s4, s5
[^>]*> ec52 ea14 	vmov	lr, r2, s8, s9
[^>]*> ec52 ea18 	vmov	lr, r2, s16, s17
[^>]*> ec52 ea1f 	vmov	lr, r2, s30, s31
[^>]*> ec42 ea10 	vmov	s0, s1, lr, r2
[^>]*> ec42 ea30 	vmov	s1, s2, lr, r2
[^>]*> ec42 ea11 	vmov	s2, s3, lr, r2
[^>]*> ec42 ea12 	vmov	s4, s5, lr, r2
[^>]*> ec42 ea14 	vmov	s8, s9, lr, r2
[^>]*> ec42 ea18 	vmov	s16, s17, lr, r2
[^>]*> ec42 ea1f 	vmov	s30, s31, lr, r2
[^>]*> ec54 eb10 	vmov	lr, r4, d0
[^>]*> ec44 eb10 	vmov	d0, lr, r4
[^>]*> ec54 eb11 	vmov	lr, r4, d1
[^>]*> ec44 eb11 	vmov	d1, lr, r4
[^>]*> ec54 eb12 	vmov	lr, r4, d2
[^>]*> ec44 eb12 	vmov	d2, lr, r4
[^>]*> ec54 eb14 	vmov	lr, r4, d4
[^>]*> ec44 eb14 	vmov	d4, lr, r4
[^>]*> ec54 eb18 	vmov	lr, r4, d8
[^>]*> ec44 eb18 	vmov	d8, lr, r4
[^>]*> ec54 eb1f 	vmov	lr, r4, d15
[^>]*> ec44 eb1f 	vmov	d15, lr, r4
[^>]*> ec54 ea10 	vmov	lr, r4, s0, s1
[^>]*> ec54 ea30 	vmov	lr, r4, s1, s2
[^>]*> ec54 ea11 	vmov	lr, r4, s2, s3
[^>]*> ec54 ea12 	vmov	lr, r4, s4, s5
[^>]*> ec54 ea14 	vmov	lr, r4, s8, s9
[^>]*> ec54 ea18 	vmov	lr, r4, s16, s17
[^>]*> ec54 ea1f 	vmov	lr, r4, s30, s31
[^>]*> ec44 ea10 	vmov	s0, s1, lr, r4
[^>]*> ec44 ea30 	vmov	s1, s2, lr, r4
[^>]*> ec44 ea11 	vmov	s2, s3, lr, r4
[^>]*> ec44 ea12 	vmov	s4, s5, lr, r4
[^>]*> ec44 ea14 	vmov	s8, s9, lr, r4
[^>]*> ec44 ea18 	vmov	s16, s17, lr, r4
[^>]*> ec44 ea1f 	vmov	s30, s31, lr, r4
[^>]*> ec57 eb10 	vmov	lr, r7, d0
[^>]*> ec47 eb10 	vmov	d0, lr, r7
[^>]*> ec57 eb11 	vmov	lr, r7, d1
[^>]*> ec47 eb11 	vmov	d1, lr, r7
[^>]*> ec57 eb12 	vmov	lr, r7, d2
[^>]*> ec47 eb12 	vmov	d2, lr, r7
[^>]*> ec57 eb14 	vmov	lr, r7, d4
[^>]*> ec47 eb14 	vmov	d4, lr, r7
[^>]*> ec57 eb18 	vmov	lr, r7, d8
[^>]*> ec47 eb18 	vmov	d8, lr, r7
[^>]*> ec57 eb1f 	vmov	lr, r7, d15
[^>]*> ec47 eb1f 	vmov	d15, lr, r7
[^>]*> ec57 ea10 	vmov	lr, r7, s0, s1
[^>]*> ec57 ea30 	vmov	lr, r7, s1, s2
[^>]*> ec57 ea11 	vmov	lr, r7, s2, s3
[^>]*> ec57 ea12 	vmov	lr, r7, s4, s5
[^>]*> ec57 ea14 	vmov	lr, r7, s8, s9
[^>]*> ec57 ea18 	vmov	lr, r7, s16, s17
[^>]*> ec57 ea1f 	vmov	lr, r7, s30, s31
[^>]*> ec47 ea10 	vmov	s0, s1, lr, r7
[^>]*> ec47 ea30 	vmov	s1, s2, lr, r7
[^>]*> ec47 ea11 	vmov	s2, s3, lr, r7
[^>]*> ec47 ea12 	vmov	s4, s5, lr, r7
[^>]*> ec47 ea14 	vmov	s8, s9, lr, r7
[^>]*> ec47 ea18 	vmov	s16, s17, lr, r7
[^>]*> ec47 ea1f 	vmov	s30, s31, lr, r7
[^>]*> ec58 eb10 	vmov	lr, r8, d0
[^>]*> ec48 eb10 	vmov	d0, lr, r8
[^>]*> ec58 eb11 	vmov	lr, r8, d1
[^>]*> ec48 eb11 	vmov	d1, lr, r8
[^>]*> ec58 eb12 	vmov	lr, r8, d2
[^>]*> ec48 eb12 	vmov	d2, lr, r8
[^>]*> ec58 eb14 	vmov	lr, r8, d4
[^>]*> ec48 eb14 	vmov	d4, lr, r8
[^>]*> ec58 eb18 	vmov	lr, r8, d8
[^>]*> ec48 eb18 	vmov	d8, lr, r8
[^>]*> ec58 eb1f 	vmov	lr, r8, d15
[^>]*> ec48 eb1f 	vmov	d15, lr, r8
[^>]*> ec58 ea10 	vmov	lr, r8, s0, s1
[^>]*> ec58 ea30 	vmov	lr, r8, s1, s2
[^>]*> ec58 ea11 	vmov	lr, r8, s2, s3
[^>]*> ec58 ea12 	vmov	lr, r8, s4, s5
[^>]*> ec58 ea14 	vmov	lr, r8, s8, s9
[^>]*> ec58 ea18 	vmov	lr, r8, s16, s17
[^>]*> ec58 ea1f 	vmov	lr, r8, s30, s31
[^>]*> ec48 ea10 	vmov	s0, s1, lr, r8
[^>]*> ec48 ea30 	vmov	s1, s2, lr, r8
[^>]*> ec48 ea11 	vmov	s2, s3, lr, r8
[^>]*> ec48 ea12 	vmov	s4, s5, lr, r8
[^>]*> ec48 ea14 	vmov	s8, s9, lr, r8
[^>]*> ec48 ea18 	vmov	s16, s17, lr, r8
[^>]*> ec48 ea1f 	vmov	s30, s31, lr, r8
[^>]*> ec5a eb10 	vmov	lr, sl, d0
[^>]*> ec4a eb10 	vmov	d0, lr, sl
[^>]*> ec5a eb11 	vmov	lr, sl, d1
[^>]*> ec4a eb11 	vmov	d1, lr, sl
[^>]*> ec5a eb12 	vmov	lr, sl, d2
[^>]*> ec4a eb12 	vmov	d2, lr, sl
[^>]*> ec5a eb14 	vmov	lr, sl, d4
[^>]*> ec4a eb14 	vmov	d4, lr, sl
[^>]*> ec5a eb18 	vmov	lr, sl, d8
[^>]*> ec4a eb18 	vmov	d8, lr, sl
[^>]*> ec5a eb1f 	vmov	lr, sl, d15
[^>]*> ec4a eb1f 	vmov	d15, lr, sl
[^>]*> ec5a ea10 	vmov	lr, sl, s0, s1
[^>]*> ec5a ea30 	vmov	lr, sl, s1, s2
[^>]*> ec5a ea11 	vmov	lr, sl, s2, s3
[^>]*> ec5a ea12 	vmov	lr, sl, s4, s5
[^>]*> ec5a ea14 	vmov	lr, sl, s8, s9
[^>]*> ec5a ea18 	vmov	lr, sl, s16, s17
[^>]*> ec5a ea1f 	vmov	lr, sl, s30, s31
[^>]*> ec4a ea10 	vmov	s0, s1, lr, sl
[^>]*> ec4a ea30 	vmov	s1, s2, lr, sl
[^>]*> ec4a ea11 	vmov	s2, s3, lr, sl
[^>]*> ec4a ea12 	vmov	s4, s5, lr, sl
[^>]*> ec4a ea14 	vmov	s8, s9, lr, sl
[^>]*> ec4a ea18 	vmov	s16, s17, lr, sl
[^>]*> ec4a ea1f 	vmov	s30, s31, lr, sl
[^>]*> ec5c eb10 	vmov	lr, ip, d0
[^>]*> ec4c eb10 	vmov	d0, lr, ip
[^>]*> ec5c eb11 	vmov	lr, ip, d1
[^>]*> ec4c eb11 	vmov	d1, lr, ip
[^>]*> ec5c eb12 	vmov	lr, ip, d2
[^>]*> ec4c eb12 	vmov	d2, lr, ip
[^>]*> ec5c eb14 	vmov	lr, ip, d4
[^>]*> ec4c eb14 	vmov	d4, lr, ip
[^>]*> ec5c eb18 	vmov	lr, ip, d8
[^>]*> ec4c eb18 	vmov	d8, lr, ip
[^>]*> ec5c eb1f 	vmov	lr, ip, d15
[^>]*> ec4c eb1f 	vmov	d15, lr, ip
[^>]*> ec5c ea10 	vmov	lr, ip, s0, s1
[^>]*> ec5c ea30 	vmov	lr, ip, s1, s2
[^>]*> ec5c ea11 	vmov	lr, ip, s2, s3
[^>]*> ec5c ea12 	vmov	lr, ip, s4, s5
[^>]*> ec5c ea14 	vmov	lr, ip, s8, s9
[^>]*> ec5c ea18 	vmov	lr, ip, s16, s17
[^>]*> ec5c ea1f 	vmov	lr, ip, s30, s31
[^>]*> ec4c ea10 	vmov	s0, s1, lr, ip
[^>]*> ec4c ea30 	vmov	s1, s2, lr, ip
[^>]*> ec4c ea11 	vmov	s2, s3, lr, ip
[^>]*> ec4c ea12 	vmov	s4, s5, lr, ip
[^>]*> ec4c ea14 	vmov	s8, s9, lr, ip
[^>]*> ec4c ea18 	vmov	s16, s17, lr, ip
[^>]*> ec4c ea1f 	vmov	s30, s31, lr, ip
[^>]*> ee40 0b10 	vmov.8	d0\[0\], r0
[^>]*> ee40 1b10 	vmov.8	d0\[0\], r1
[^>]*> ee40 2b10 	vmov.8	d0\[0\], r2
[^>]*> ee40 4b10 	vmov.8	d0\[0\], r4
[^>]*> ee40 7b10 	vmov.8	d0\[0\], r7
[^>]*> ee40 8b10 	vmov.8	d0\[0\], r8
[^>]*> ee40 ab10 	vmov.8	d0\[0\], sl
[^>]*> ee40 cb10 	vmov.8	d0\[0\], ip
[^>]*> ee40 eb10 	vmov.8	d0\[0\], lr
[^>]*> ee42 0b10 	vmov.8	d2\[0\], r0
[^>]*> ee42 1b10 	vmov.8	d2\[0\], r1
[^>]*> ee42 2b10 	vmov.8	d2\[0\], r2
[^>]*> ee42 4b10 	vmov.8	d2\[0\], r4
[^>]*> ee42 7b10 	vmov.8	d2\[0\], r7
[^>]*> ee42 8b10 	vmov.8	d2\[0\], r8
[^>]*> ee42 ab10 	vmov.8	d2\[0\], sl
[^>]*> ee42 cb10 	vmov.8	d2\[0\], ip
[^>]*> ee42 eb10 	vmov.8	d2\[0\], lr
[^>]*> ee44 0b10 	vmov.8	d4\[0\], r0
[^>]*> ee44 1b10 	vmov.8	d4\[0\], r1
[^>]*> ee44 2b10 	vmov.8	d4\[0\], r2
[^>]*> ee44 4b10 	vmov.8	d4\[0\], r4
[^>]*> ee44 7b10 	vmov.8	d4\[0\], r7
[^>]*> ee44 8b10 	vmov.8	d4\[0\], r8
[^>]*> ee44 ab10 	vmov.8	d4\[0\], sl
[^>]*> ee44 cb10 	vmov.8	d4\[0\], ip
[^>]*> ee44 eb10 	vmov.8	d4\[0\], lr
[^>]*> ee48 0b10 	vmov.8	d8\[0\], r0
[^>]*> ee48 1b10 	vmov.8	d8\[0\], r1
[^>]*> ee48 2b10 	vmov.8	d8\[0\], r2
[^>]*> ee48 4b10 	vmov.8	d8\[0\], r4
[^>]*> ee48 7b10 	vmov.8	d8\[0\], r7
[^>]*> ee48 8b10 	vmov.8	d8\[0\], r8
[^>]*> ee48 ab10 	vmov.8	d8\[0\], sl
[^>]*> ee48 cb10 	vmov.8	d8\[0\], ip
[^>]*> ee48 eb10 	vmov.8	d8\[0\], lr
[^>]*> ee4e 0b10 	vmov.8	d14\[0\], r0
[^>]*> ee4e 1b10 	vmov.8	d14\[0\], r1
[^>]*> ee4e 2b10 	vmov.8	d14\[0\], r2
[^>]*> ee4e 4b10 	vmov.8	d14\[0\], r4
[^>]*> ee4e 7b10 	vmov.8	d14\[0\], r7
[^>]*> ee4e 8b10 	vmov.8	d14\[0\], r8
[^>]*> ee4e ab10 	vmov.8	d14\[0\], sl
[^>]*> ee4e cb10 	vmov.8	d14\[0\], ip
[^>]*> ee4e eb10 	vmov.8	d14\[0\], lr
[^>]*> ee40 0b30 	vmov.8	d0\[1\], r0
[^>]*> ee40 1b30 	vmov.8	d0\[1\], r1
[^>]*> ee40 2b30 	vmov.8	d0\[1\], r2
[^>]*> ee40 4b30 	vmov.8	d0\[1\], r4
[^>]*> ee40 7b30 	vmov.8	d0\[1\], r7
[^>]*> ee40 8b30 	vmov.8	d0\[1\], r8
[^>]*> ee40 ab30 	vmov.8	d0\[1\], sl
[^>]*> ee40 cb30 	vmov.8	d0\[1\], ip
[^>]*> ee40 eb30 	vmov.8	d0\[1\], lr
[^>]*> ee42 0b30 	vmov.8	d2\[1\], r0
[^>]*> ee42 1b30 	vmov.8	d2\[1\], r1
[^>]*> ee42 2b30 	vmov.8	d2\[1\], r2
[^>]*> ee42 4b30 	vmov.8	d2\[1\], r4
[^>]*> ee42 7b30 	vmov.8	d2\[1\], r7
[^>]*> ee42 8b30 	vmov.8	d2\[1\], r8
[^>]*> ee42 ab30 	vmov.8	d2\[1\], sl
[^>]*> ee42 cb30 	vmov.8	d2\[1\], ip
[^>]*> ee42 eb30 	vmov.8	d2\[1\], lr
[^>]*> ee44 0b30 	vmov.8	d4\[1\], r0
[^>]*> ee44 1b30 	vmov.8	d4\[1\], r1
[^>]*> ee44 2b30 	vmov.8	d4\[1\], r2
[^>]*> ee44 4b30 	vmov.8	d4\[1\], r4
[^>]*> ee44 7b30 	vmov.8	d4\[1\], r7
[^>]*> ee44 8b30 	vmov.8	d4\[1\], r8
[^>]*> ee44 ab30 	vmov.8	d4\[1\], sl
[^>]*> ee44 cb30 	vmov.8	d4\[1\], ip
[^>]*> ee44 eb30 	vmov.8	d4\[1\], lr
[^>]*> ee48 0b30 	vmov.8	d8\[1\], r0
[^>]*> ee48 1b30 	vmov.8	d8\[1\], r1
[^>]*> ee48 2b30 	vmov.8	d8\[1\], r2
[^>]*> ee48 4b30 	vmov.8	d8\[1\], r4
[^>]*> ee48 7b30 	vmov.8	d8\[1\], r7
[^>]*> ee48 8b30 	vmov.8	d8\[1\], r8
[^>]*> ee48 ab30 	vmov.8	d8\[1\], sl
[^>]*> ee48 cb30 	vmov.8	d8\[1\], ip
[^>]*> ee48 eb30 	vmov.8	d8\[1\], lr
[^>]*> ee4e 0b30 	vmov.8	d14\[1\], r0
[^>]*> ee4e 1b30 	vmov.8	d14\[1\], r1
[^>]*> ee4e 2b30 	vmov.8	d14\[1\], r2
[^>]*> ee4e 4b30 	vmov.8	d14\[1\], r4
[^>]*> ee4e 7b30 	vmov.8	d14\[1\], r7
[^>]*> ee4e 8b30 	vmov.8	d14\[1\], r8
[^>]*> ee4e ab30 	vmov.8	d14\[1\], sl
[^>]*> ee4e cb30 	vmov.8	d14\[1\], ip
[^>]*> ee4e eb30 	vmov.8	d14\[1\], lr
[^>]*> ee40 0b50 	vmov.8	d0\[2\], r0
[^>]*> ee40 1b50 	vmov.8	d0\[2\], r1
[^>]*> ee40 2b50 	vmov.8	d0\[2\], r2
[^>]*> ee40 4b50 	vmov.8	d0\[2\], r4
[^>]*> ee40 7b50 	vmov.8	d0\[2\], r7
[^>]*> ee40 8b50 	vmov.8	d0\[2\], r8
[^>]*> ee40 ab50 	vmov.8	d0\[2\], sl
[^>]*> ee40 cb50 	vmov.8	d0\[2\], ip
[^>]*> ee40 eb50 	vmov.8	d0\[2\], lr
[^>]*> ee42 0b50 	vmov.8	d2\[2\], r0
[^>]*> ee42 1b50 	vmov.8	d2\[2\], r1
[^>]*> ee42 2b50 	vmov.8	d2\[2\], r2
[^>]*> ee42 4b50 	vmov.8	d2\[2\], r4
[^>]*> ee42 7b50 	vmov.8	d2\[2\], r7
[^>]*> ee42 8b50 	vmov.8	d2\[2\], r8
[^>]*> ee42 ab50 	vmov.8	d2\[2\], sl
[^>]*> ee42 cb50 	vmov.8	d2\[2\], ip
[^>]*> ee42 eb50 	vmov.8	d2\[2\], lr
[^>]*> ee44 0b50 	vmov.8	d4\[2\], r0
[^>]*> ee44 1b50 	vmov.8	d4\[2\], r1
[^>]*> ee44 2b50 	vmov.8	d4\[2\], r2
[^>]*> ee44 4b50 	vmov.8	d4\[2\], r4
[^>]*> ee44 7b50 	vmov.8	d4\[2\], r7
[^>]*> ee44 8b50 	vmov.8	d4\[2\], r8
[^>]*> ee44 ab50 	vmov.8	d4\[2\], sl
[^>]*> ee44 cb50 	vmov.8	d4\[2\], ip
[^>]*> ee44 eb50 	vmov.8	d4\[2\], lr
[^>]*> ee48 0b50 	vmov.8	d8\[2\], r0
[^>]*> ee48 1b50 	vmov.8	d8\[2\], r1
[^>]*> ee48 2b50 	vmov.8	d8\[2\], r2
[^>]*> ee48 4b50 	vmov.8	d8\[2\], r4
[^>]*> ee48 7b50 	vmov.8	d8\[2\], r7
[^>]*> ee48 8b50 	vmov.8	d8\[2\], r8
[^>]*> ee48 ab50 	vmov.8	d8\[2\], sl
[^>]*> ee48 cb50 	vmov.8	d8\[2\], ip
[^>]*> ee48 eb50 	vmov.8	d8\[2\], lr
[^>]*> ee4e 0b50 	vmov.8	d14\[2\], r0
[^>]*> ee4e 1b50 	vmov.8	d14\[2\], r1
[^>]*> ee4e 2b50 	vmov.8	d14\[2\], r2
[^>]*> ee4e 4b50 	vmov.8	d14\[2\], r4
[^>]*> ee4e 7b50 	vmov.8	d14\[2\], r7
[^>]*> ee4e 8b50 	vmov.8	d14\[2\], r8
[^>]*> ee4e ab50 	vmov.8	d14\[2\], sl
[^>]*> ee4e cb50 	vmov.8	d14\[2\], ip
[^>]*> ee4e eb50 	vmov.8	d14\[2\], lr
[^>]*> ee60 0b10 	vmov.8	d0\[4\], r0
[^>]*> ee60 1b10 	vmov.8	d0\[4\], r1
[^>]*> ee60 2b10 	vmov.8	d0\[4\], r2
[^>]*> ee60 4b10 	vmov.8	d0\[4\], r4
[^>]*> ee60 7b10 	vmov.8	d0\[4\], r7
[^>]*> ee60 8b10 	vmov.8	d0\[4\], r8
[^>]*> ee60 ab10 	vmov.8	d0\[4\], sl
[^>]*> ee60 cb10 	vmov.8	d0\[4\], ip
[^>]*> ee60 eb10 	vmov.8	d0\[4\], lr
[^>]*> ee62 0b10 	vmov.8	d2\[4\], r0
[^>]*> ee62 1b10 	vmov.8	d2\[4\], r1
[^>]*> ee62 2b10 	vmov.8	d2\[4\], r2
[^>]*> ee62 4b10 	vmov.8	d2\[4\], r4
[^>]*> ee62 7b10 	vmov.8	d2\[4\], r7
[^>]*> ee62 8b10 	vmov.8	d2\[4\], r8
[^>]*> ee62 ab10 	vmov.8	d2\[4\], sl
[^>]*> ee62 cb10 	vmov.8	d2\[4\], ip
[^>]*> ee62 eb10 	vmov.8	d2\[4\], lr
[^>]*> ee64 0b10 	vmov.8	d4\[4\], r0
[^>]*> ee64 1b10 	vmov.8	d4\[4\], r1
[^>]*> ee64 2b10 	vmov.8	d4\[4\], r2
[^>]*> ee64 4b10 	vmov.8	d4\[4\], r4
[^>]*> ee64 7b10 	vmov.8	d4\[4\], r7
[^>]*> ee64 8b10 	vmov.8	d4\[4\], r8
[^>]*> ee64 ab10 	vmov.8	d4\[4\], sl
[^>]*> ee64 cb10 	vmov.8	d4\[4\], ip
[^>]*> ee64 eb10 	vmov.8	d4\[4\], lr
[^>]*> ee68 0b10 	vmov.8	d8\[4\], r0
[^>]*> ee68 1b10 	vmov.8	d8\[4\], r1
[^>]*> ee68 2b10 	vmov.8	d8\[4\], r2
[^>]*> ee68 4b10 	vmov.8	d8\[4\], r4
[^>]*> ee68 7b10 	vmov.8	d8\[4\], r7
[^>]*> ee68 8b10 	vmov.8	d8\[4\], r8
[^>]*> ee68 ab10 	vmov.8	d8\[4\], sl
[^>]*> ee68 cb10 	vmov.8	d8\[4\], ip
[^>]*> ee68 eb10 	vmov.8	d8\[4\], lr
[^>]*> ee6e 0b10 	vmov.8	d14\[4\], r0
[^>]*> ee6e 1b10 	vmov.8	d14\[4\], r1
[^>]*> ee6e 2b10 	vmov.8	d14\[4\], r2
[^>]*> ee6e 4b10 	vmov.8	d14\[4\], r4
[^>]*> ee6e 7b10 	vmov.8	d14\[4\], r7
[^>]*> ee6e 8b10 	vmov.8	d14\[4\], r8
[^>]*> ee6e ab10 	vmov.8	d14\[4\], sl
[^>]*> ee6e cb10 	vmov.8	d14\[4\], ip
[^>]*> ee6e eb10 	vmov.8	d14\[4\], lr
[^>]*> ee41 0b10 	vmov.8	d1\[0\], r0
[^>]*> ee41 1b10 	vmov.8	d1\[0\], r1
[^>]*> ee41 2b10 	vmov.8	d1\[0\], r2
[^>]*> ee41 4b10 	vmov.8	d1\[0\], r4
[^>]*> ee41 7b10 	vmov.8	d1\[0\], r7
[^>]*> ee41 8b10 	vmov.8	d1\[0\], r8
[^>]*> ee41 ab10 	vmov.8	d1\[0\], sl
[^>]*> ee41 cb10 	vmov.8	d1\[0\], ip
[^>]*> ee41 eb10 	vmov.8	d1\[0\], lr
[^>]*> ee43 0b10 	vmov.8	d3\[0\], r0
[^>]*> ee43 1b10 	vmov.8	d3\[0\], r1
[^>]*> ee43 2b10 	vmov.8	d3\[0\], r2
[^>]*> ee43 4b10 	vmov.8	d3\[0\], r4
[^>]*> ee43 7b10 	vmov.8	d3\[0\], r7
[^>]*> ee43 8b10 	vmov.8	d3\[0\], r8
[^>]*> ee43 ab10 	vmov.8	d3\[0\], sl
[^>]*> ee43 cb10 	vmov.8	d3\[0\], ip
[^>]*> ee43 eb10 	vmov.8	d3\[0\], lr
[^>]*> ee45 0b10 	vmov.8	d5\[0\], r0
[^>]*> ee45 1b10 	vmov.8	d5\[0\], r1
[^>]*> ee45 2b10 	vmov.8	d5\[0\], r2
[^>]*> ee45 4b10 	vmov.8	d5\[0\], r4
[^>]*> ee45 7b10 	vmov.8	d5\[0\], r7
[^>]*> ee45 8b10 	vmov.8	d5\[0\], r8
[^>]*> ee45 ab10 	vmov.8	d5\[0\], sl
[^>]*> ee45 cb10 	vmov.8	d5\[0\], ip
[^>]*> ee45 eb10 	vmov.8	d5\[0\], lr
[^>]*> ee49 0b10 	vmov.8	d9\[0\], r0
[^>]*> ee49 1b10 	vmov.8	d9\[0\], r1
[^>]*> ee49 2b10 	vmov.8	d9\[0\], r2
[^>]*> ee49 4b10 	vmov.8	d9\[0\], r4
[^>]*> ee49 7b10 	vmov.8	d9\[0\], r7
[^>]*> ee49 8b10 	vmov.8	d9\[0\], r8
[^>]*> ee49 ab10 	vmov.8	d9\[0\], sl
[^>]*> ee49 cb10 	vmov.8	d9\[0\], ip
[^>]*> ee49 eb10 	vmov.8	d9\[0\], lr
[^>]*> ee4f 0b10 	vmov.8	d15\[0\], r0
[^>]*> ee4f 1b10 	vmov.8	d15\[0\], r1
[^>]*> ee4f 2b10 	vmov.8	d15\[0\], r2
[^>]*> ee4f 4b10 	vmov.8	d15\[0\], r4
[^>]*> ee4f 7b10 	vmov.8	d15\[0\], r7
[^>]*> ee4f 8b10 	vmov.8	d15\[0\], r8
[^>]*> ee4f ab10 	vmov.8	d15\[0\], sl
[^>]*> ee4f cb10 	vmov.8	d15\[0\], ip
[^>]*> ee4f eb10 	vmov.8	d15\[0\], lr
[^>]*> ee61 0b70 	vmov.8	d1\[7\], r0
[^>]*> ee61 1b70 	vmov.8	d1\[7\], r1
[^>]*> ee61 2b70 	vmov.8	d1\[7\], r2
[^>]*> ee61 4b70 	vmov.8	d1\[7\], r4
[^>]*> ee61 7b70 	vmov.8	d1\[7\], r7
[^>]*> ee61 8b70 	vmov.8	d1\[7\], r8
[^>]*> ee61 ab70 	vmov.8	d1\[7\], sl
[^>]*> ee61 cb70 	vmov.8	d1\[7\], ip
[^>]*> ee61 eb70 	vmov.8	d1\[7\], lr
[^>]*> ee63 0b70 	vmov.8	d3\[7\], r0
[^>]*> ee63 1b70 	vmov.8	d3\[7\], r1
[^>]*> ee63 2b70 	vmov.8	d3\[7\], r2
[^>]*> ee63 4b70 	vmov.8	d3\[7\], r4
[^>]*> ee63 7b70 	vmov.8	d3\[7\], r7
[^>]*> ee63 8b70 	vmov.8	d3\[7\], r8
[^>]*> ee63 ab70 	vmov.8	d3\[7\], sl
[^>]*> ee63 cb70 	vmov.8	d3\[7\], ip
[^>]*> ee63 eb70 	vmov.8	d3\[7\], lr
[^>]*> ee65 0b70 	vmov.8	d5\[7\], r0
[^>]*> ee65 1b70 	vmov.8	d5\[7\], r1
[^>]*> ee65 2b70 	vmov.8	d5\[7\], r2
[^>]*> ee65 4b70 	vmov.8	d5\[7\], r4
[^>]*> ee65 7b70 	vmov.8	d5\[7\], r7
[^>]*> ee65 8b70 	vmov.8	d5\[7\], r8
[^>]*> ee65 ab70 	vmov.8	d5\[7\], sl
[^>]*> ee65 cb70 	vmov.8	d5\[7\], ip
[^>]*> ee65 eb70 	vmov.8	d5\[7\], lr
[^>]*> ee69 0b70 	vmov.8	d9\[7\], r0
[^>]*> ee69 1b70 	vmov.8	d9\[7\], r1
[^>]*> ee69 2b70 	vmov.8	d9\[7\], r2
[^>]*> ee69 4b70 	vmov.8	d9\[7\], r4
[^>]*> ee69 7b70 	vmov.8	d9\[7\], r7
[^>]*> ee69 8b70 	vmov.8	d9\[7\], r8
[^>]*> ee69 ab70 	vmov.8	d9\[7\], sl
[^>]*> ee69 cb70 	vmov.8	d9\[7\], ip
[^>]*> ee69 eb70 	vmov.8	d9\[7\], lr
[^>]*> ee6f 0b70 	vmov.8	d15\[7\], r0
[^>]*> ee6f 1b70 	vmov.8	d15\[7\], r1
[^>]*> ee6f 2b70 	vmov.8	d15\[7\], r2
[^>]*> ee6f 4b70 	vmov.8	d15\[7\], r4
[^>]*> ee6f 7b70 	vmov.8	d15\[7\], r7
[^>]*> ee6f 8b70 	vmov.8	d15\[7\], r8
[^>]*> ee6f ab70 	vmov.8	d15\[7\], sl
[^>]*> ee6f cb70 	vmov.8	d15\[7\], ip
[^>]*> ee6f eb70 	vmov.8	d15\[7\], lr
[^>]*> ee61 0b30 	vmov.8	d1\[5\], r0
[^>]*> ee61 1b30 	vmov.8	d1\[5\], r1
[^>]*> ee61 2b30 	vmov.8	d1\[5\], r2
[^>]*> ee61 4b30 	vmov.8	d1\[5\], r4
[^>]*> ee61 7b30 	vmov.8	d1\[5\], r7
[^>]*> ee61 8b30 	vmov.8	d1\[5\], r8
[^>]*> ee61 ab30 	vmov.8	d1\[5\], sl
[^>]*> ee61 cb30 	vmov.8	d1\[5\], ip
[^>]*> ee61 eb30 	vmov.8	d1\[5\], lr
[^>]*> ee63 0b30 	vmov.8	d3\[5\], r0
[^>]*> ee63 1b30 	vmov.8	d3\[5\], r1
[^>]*> ee63 2b30 	vmov.8	d3\[5\], r2
[^>]*> ee63 4b30 	vmov.8	d3\[5\], r4
[^>]*> ee63 7b30 	vmov.8	d3\[5\], r7
[^>]*> ee63 8b30 	vmov.8	d3\[5\], r8
[^>]*> ee63 ab30 	vmov.8	d3\[5\], sl
[^>]*> ee63 cb30 	vmov.8	d3\[5\], ip
[^>]*> ee63 eb30 	vmov.8	d3\[5\], lr
[^>]*> ee65 0b30 	vmov.8	d5\[5\], r0
[^>]*> ee65 1b30 	vmov.8	d5\[5\], r1
[^>]*> ee65 2b30 	vmov.8	d5\[5\], r2
[^>]*> ee65 4b30 	vmov.8	d5\[5\], r4
[^>]*> ee65 7b30 	vmov.8	d5\[5\], r7
[^>]*> ee65 8b30 	vmov.8	d5\[5\], r8
[^>]*> ee65 ab30 	vmov.8	d5\[5\], sl
[^>]*> ee65 cb30 	vmov.8	d5\[5\], ip
[^>]*> ee65 eb30 	vmov.8	d5\[5\], lr
[^>]*> ee69 0b30 	vmov.8	d9\[5\], r0
[^>]*> ee69 1b30 	vmov.8	d9\[5\], r1
[^>]*> ee69 2b30 	vmov.8	d9\[5\], r2
[^>]*> ee69 4b30 	vmov.8	d9\[5\], r4
[^>]*> ee69 7b30 	vmov.8	d9\[5\], r7
[^>]*> ee69 8b30 	vmov.8	d9\[5\], r8
[^>]*> ee69 ab30 	vmov.8	d9\[5\], sl
[^>]*> ee69 cb30 	vmov.8	d9\[5\], ip
[^>]*> ee69 eb30 	vmov.8	d9\[5\], lr
[^>]*> ee6f 0b30 	vmov.8	d15\[5\], r0
[^>]*> ee6f 1b30 	vmov.8	d15\[5\], r1
[^>]*> ee6f 2b30 	vmov.8	d15\[5\], r2
[^>]*> ee6f 4b30 	vmov.8	d15\[5\], r4
[^>]*> ee6f 7b30 	vmov.8	d15\[5\], r7
[^>]*> ee6f 8b30 	vmov.8	d15\[5\], r8
[^>]*> ee6f ab30 	vmov.8	d15\[5\], sl
[^>]*> ee6f cb30 	vmov.8	d15\[5\], ip
[^>]*> ee6f eb30 	vmov.8	d15\[5\], lr
[^>]*> ee60 0b50 	vmov.8	d0\[6\], r0
[^>]*> ee60 1b50 	vmov.8	d0\[6\], r1
[^>]*> ee60 2b50 	vmov.8	d0\[6\], r2
[^>]*> ee60 4b50 	vmov.8	d0\[6\], r4
[^>]*> ee60 7b50 	vmov.8	d0\[6\], r7
[^>]*> ee60 8b50 	vmov.8	d0\[6\], r8
[^>]*> ee60 ab50 	vmov.8	d0\[6\], sl
[^>]*> ee60 cb50 	vmov.8	d0\[6\], ip
[^>]*> ee60 eb50 	vmov.8	d0\[6\], lr
[^>]*> ee62 0b50 	vmov.8	d2\[6\], r0
[^>]*> ee62 1b50 	vmov.8	d2\[6\], r1
[^>]*> ee62 2b50 	vmov.8	d2\[6\], r2
[^>]*> ee62 4b50 	vmov.8	d2\[6\], r4
[^>]*> ee62 7b50 	vmov.8	d2\[6\], r7
[^>]*> ee62 8b50 	vmov.8	d2\[6\], r8
[^>]*> ee62 ab50 	vmov.8	d2\[6\], sl
[^>]*> ee62 cb50 	vmov.8	d2\[6\], ip
[^>]*> ee62 eb50 	vmov.8	d2\[6\], lr
[^>]*> ee64 0b50 	vmov.8	d4\[6\], r0
[^>]*> ee64 1b50 	vmov.8	d4\[6\], r1
[^>]*> ee64 2b50 	vmov.8	d4\[6\], r2
[^>]*> ee64 4b50 	vmov.8	d4\[6\], r4
[^>]*> ee64 7b50 	vmov.8	d4\[6\], r7
[^>]*> ee64 8b50 	vmov.8	d4\[6\], r8
[^>]*> ee64 ab50 	vmov.8	d4\[6\], sl
[^>]*> ee64 cb50 	vmov.8	d4\[6\], ip
[^>]*> ee64 eb50 	vmov.8	d4\[6\], lr
[^>]*> ee68 0b50 	vmov.8	d8\[6\], r0
[^>]*> ee68 1b50 	vmov.8	d8\[6\], r1
[^>]*> ee68 2b50 	vmov.8	d8\[6\], r2
[^>]*> ee68 4b50 	vmov.8	d8\[6\], r4
[^>]*> ee68 7b50 	vmov.8	d8\[6\], r7
[^>]*> ee68 8b50 	vmov.8	d8\[6\], r8
[^>]*> ee68 ab50 	vmov.8	d8\[6\], sl
[^>]*> ee68 cb50 	vmov.8	d8\[6\], ip
[^>]*> ee68 eb50 	vmov.8	d8\[6\], lr
[^>]*> ee6e 0b50 	vmov.8	d14\[6\], r0
[^>]*> ee6e 1b50 	vmov.8	d14\[6\], r1
[^>]*> ee6e 2b50 	vmov.8	d14\[6\], r2
[^>]*> ee6e 4b50 	vmov.8	d14\[6\], r4
[^>]*> ee6e 7b50 	vmov.8	d14\[6\], r7
[^>]*> ee6e 8b50 	vmov.8	d14\[6\], r8
[^>]*> ee6e ab50 	vmov.8	d14\[6\], sl
[^>]*> ee6e cb50 	vmov.8	d14\[6\], ip
[^>]*> ee6e eb50 	vmov.8	d14\[6\], lr
[^>]*> ee00 0b30 	vmov.16	d0\[0\], r0
[^>]*> ee00 1b30 	vmov.16	d0\[0\], r1
[^>]*> ee00 2b30 	vmov.16	d0\[0\], r2
[^>]*> ee00 4b30 	vmov.16	d0\[0\], r4
[^>]*> ee00 7b30 	vmov.16	d0\[0\], r7
[^>]*> ee00 8b30 	vmov.16	d0\[0\], r8
[^>]*> ee00 ab30 	vmov.16	d0\[0\], sl
[^>]*> ee00 cb30 	vmov.16	d0\[0\], ip
[^>]*> ee00 eb30 	vmov.16	d0\[0\], lr
[^>]*> ee02 0b30 	vmov.16	d2\[0\], r0
[^>]*> ee02 1b30 	vmov.16	d2\[0\], r1
[^>]*> ee02 2b30 	vmov.16	d2\[0\], r2
[^>]*> ee02 4b30 	vmov.16	d2\[0\], r4
[^>]*> ee02 7b30 	vmov.16	d2\[0\], r7
[^>]*> ee02 8b30 	vmov.16	d2\[0\], r8
[^>]*> ee02 ab30 	vmov.16	d2\[0\], sl
[^>]*> ee02 cb30 	vmov.16	d2\[0\], ip
[^>]*> ee02 eb30 	vmov.16	d2\[0\], lr
[^>]*> ee04 0b30 	vmov.16	d4\[0\], r0
[^>]*> ee04 1b30 	vmov.16	d4\[0\], r1
[^>]*> ee04 2b30 	vmov.16	d4\[0\], r2
[^>]*> ee04 4b30 	vmov.16	d4\[0\], r4
[^>]*> ee04 7b30 	vmov.16	d4\[0\], r7
[^>]*> ee04 8b30 	vmov.16	d4\[0\], r8
[^>]*> ee04 ab30 	vmov.16	d4\[0\], sl
[^>]*> ee04 cb30 	vmov.16	d4\[0\], ip
[^>]*> ee04 eb30 	vmov.16	d4\[0\], lr
[^>]*> ee08 0b30 	vmov.16	d8\[0\], r0
[^>]*> ee08 1b30 	vmov.16	d8\[0\], r1
[^>]*> ee08 2b30 	vmov.16	d8\[0\], r2
[^>]*> ee08 4b30 	vmov.16	d8\[0\], r4
[^>]*> ee08 7b30 	vmov.16	d8\[0\], r7
[^>]*> ee08 8b30 	vmov.16	d8\[0\], r8
[^>]*> ee08 ab30 	vmov.16	d8\[0\], sl
[^>]*> ee08 cb30 	vmov.16	d8\[0\], ip
[^>]*> ee08 eb30 	vmov.16	d8\[0\], lr
[^>]*> ee0e 0b30 	vmov.16	d14\[0\], r0
[^>]*> ee0e 1b30 	vmov.16	d14\[0\], r1
[^>]*> ee0e 2b30 	vmov.16	d14\[0\], r2
[^>]*> ee0e 4b30 	vmov.16	d14\[0\], r4
[^>]*> ee0e 7b30 	vmov.16	d14\[0\], r7
[^>]*> ee0e 8b30 	vmov.16	d14\[0\], r8
[^>]*> ee0e ab30 	vmov.16	d14\[0\], sl
[^>]*> ee0e cb30 	vmov.16	d14\[0\], ip
[^>]*> ee0e eb30 	vmov.16	d14\[0\], lr
[^>]*> ee00 0b70 	vmov.16	d0\[1\], r0
[^>]*> ee00 1b70 	vmov.16	d0\[1\], r1
[^>]*> ee00 2b70 	vmov.16	d0\[1\], r2
[^>]*> ee00 4b70 	vmov.16	d0\[1\], r4
[^>]*> ee00 7b70 	vmov.16	d0\[1\], r7
[^>]*> ee00 8b70 	vmov.16	d0\[1\], r8
[^>]*> ee00 ab70 	vmov.16	d0\[1\], sl
[^>]*> ee00 cb70 	vmov.16	d0\[1\], ip
[^>]*> ee00 eb70 	vmov.16	d0\[1\], lr
[^>]*> ee02 0b70 	vmov.16	d2\[1\], r0
[^>]*> ee02 1b70 	vmov.16	d2\[1\], r1
[^>]*> ee02 2b70 	vmov.16	d2\[1\], r2
[^>]*> ee02 4b70 	vmov.16	d2\[1\], r4
[^>]*> ee02 7b70 	vmov.16	d2\[1\], r7
[^>]*> ee02 8b70 	vmov.16	d2\[1\], r8
[^>]*> ee02 ab70 	vmov.16	d2\[1\], sl
[^>]*> ee02 cb70 	vmov.16	d2\[1\], ip
[^>]*> ee02 eb70 	vmov.16	d2\[1\], lr
[^>]*> ee04 0b70 	vmov.16	d4\[1\], r0
[^>]*> ee04 1b70 	vmov.16	d4\[1\], r1
[^>]*> ee04 2b70 	vmov.16	d4\[1\], r2
[^>]*> ee04 4b70 	vmov.16	d4\[1\], r4
[^>]*> ee04 7b70 	vmov.16	d4\[1\], r7
[^>]*> ee04 8b70 	vmov.16	d4\[1\], r8
[^>]*> ee04 ab70 	vmov.16	d4\[1\], sl
[^>]*> ee04 cb70 	vmov.16	d4\[1\], ip
[^>]*> ee04 eb70 	vmov.16	d4\[1\], lr
[^>]*> ee08 0b70 	vmov.16	d8\[1\], r0
[^>]*> ee08 1b70 	vmov.16	d8\[1\], r1
[^>]*> ee08 2b70 	vmov.16	d8\[1\], r2
[^>]*> ee08 4b70 	vmov.16	d8\[1\], r4
[^>]*> ee08 7b70 	vmov.16	d8\[1\], r7
[^>]*> ee08 8b70 	vmov.16	d8\[1\], r8
[^>]*> ee08 ab70 	vmov.16	d8\[1\], sl
[^>]*> ee08 cb70 	vmov.16	d8\[1\], ip
[^>]*> ee08 eb70 	vmov.16	d8\[1\], lr
[^>]*> ee0e 0b70 	vmov.16	d14\[1\], r0
[^>]*> ee0e 1b70 	vmov.16	d14\[1\], r1
[^>]*> ee0e 2b70 	vmov.16	d14\[1\], r2
[^>]*> ee0e 4b70 	vmov.16	d14\[1\], r4
[^>]*> ee0e 7b70 	vmov.16	d14\[1\], r7
[^>]*> ee0e 8b70 	vmov.16	d14\[1\], r8
[^>]*> ee0e ab70 	vmov.16	d14\[1\], sl
[^>]*> ee0e cb70 	vmov.16	d14\[1\], ip
[^>]*> ee0e eb70 	vmov.16	d14\[1\], lr
[^>]*> ee20 0b30 	vmov.16	d0\[2\], r0
[^>]*> ee20 1b30 	vmov.16	d0\[2\], r1
[^>]*> ee20 2b30 	vmov.16	d0\[2\], r2
[^>]*> ee20 4b30 	vmov.16	d0\[2\], r4
[^>]*> ee20 7b30 	vmov.16	d0\[2\], r7
[^>]*> ee20 8b30 	vmov.16	d0\[2\], r8
[^>]*> ee20 ab30 	vmov.16	d0\[2\], sl
[^>]*> ee20 cb30 	vmov.16	d0\[2\], ip
[^>]*> ee20 eb30 	vmov.16	d0\[2\], lr
[^>]*> ee22 0b30 	vmov.16	d2\[2\], r0
[^>]*> ee22 1b30 	vmov.16	d2\[2\], r1
[^>]*> ee22 2b30 	vmov.16	d2\[2\], r2
[^>]*> ee22 4b30 	vmov.16	d2\[2\], r4
[^>]*> ee22 7b30 	vmov.16	d2\[2\], r7
[^>]*> ee22 8b30 	vmov.16	d2\[2\], r8
[^>]*> ee22 ab30 	vmov.16	d2\[2\], sl
[^>]*> ee22 cb30 	vmov.16	d2\[2\], ip
[^>]*> ee22 eb30 	vmov.16	d2\[2\], lr
[^>]*> ee24 0b30 	vmov.16	d4\[2\], r0
[^>]*> ee24 1b30 	vmov.16	d4\[2\], r1
[^>]*> ee24 2b30 	vmov.16	d4\[2\], r2
[^>]*> ee24 4b30 	vmov.16	d4\[2\], r4
[^>]*> ee24 7b30 	vmov.16	d4\[2\], r7
[^>]*> ee24 8b30 	vmov.16	d4\[2\], r8
[^>]*> ee24 ab30 	vmov.16	d4\[2\], sl
[^>]*> ee24 cb30 	vmov.16	d4\[2\], ip
[^>]*> ee24 eb30 	vmov.16	d4\[2\], lr
[^>]*> ee28 0b30 	vmov.16	d8\[2\], r0
[^>]*> ee28 1b30 	vmov.16	d8\[2\], r1
[^>]*> ee28 2b30 	vmov.16	d8\[2\], r2
[^>]*> ee28 4b30 	vmov.16	d8\[2\], r4
[^>]*> ee28 7b30 	vmov.16	d8\[2\], r7
[^>]*> ee28 8b30 	vmov.16	d8\[2\], r8
[^>]*> ee28 ab30 	vmov.16	d8\[2\], sl
[^>]*> ee28 cb30 	vmov.16	d8\[2\], ip
[^>]*> ee28 eb30 	vmov.16	d8\[2\], lr
[^>]*> ee2e 0b30 	vmov.16	d14\[2\], r0
[^>]*> ee2e 1b30 	vmov.16	d14\[2\], r1
[^>]*> ee2e 2b30 	vmov.16	d14\[2\], r2
[^>]*> ee2e 4b30 	vmov.16	d14\[2\], r4
[^>]*> ee2e 7b30 	vmov.16	d14\[2\], r7
[^>]*> ee2e 8b30 	vmov.16	d14\[2\], r8
[^>]*> ee2e ab30 	vmov.16	d14\[2\], sl
[^>]*> ee2e cb30 	vmov.16	d14\[2\], ip
[^>]*> ee2e eb30 	vmov.16	d14\[2\], lr
[^>]*> ee01 0b30 	vmov.16	d1\[0\], r0
[^>]*> ee01 1b30 	vmov.16	d1\[0\], r1
[^>]*> ee01 2b30 	vmov.16	d1\[0\], r2
[^>]*> ee01 4b30 	vmov.16	d1\[0\], r4
[^>]*> ee01 7b30 	vmov.16	d1\[0\], r7
[^>]*> ee01 8b30 	vmov.16	d1\[0\], r8
[^>]*> ee01 ab30 	vmov.16	d1\[0\], sl
[^>]*> ee01 cb30 	vmov.16	d1\[0\], ip
[^>]*> ee01 eb30 	vmov.16	d1\[0\], lr
[^>]*> ee03 0b30 	vmov.16	d3\[0\], r0
[^>]*> ee03 1b30 	vmov.16	d3\[0\], r1
[^>]*> ee03 2b30 	vmov.16	d3\[0\], r2
[^>]*> ee03 4b30 	vmov.16	d3\[0\], r4
[^>]*> ee03 7b30 	vmov.16	d3\[0\], r7
[^>]*> ee03 8b30 	vmov.16	d3\[0\], r8
[^>]*> ee03 ab30 	vmov.16	d3\[0\], sl
[^>]*> ee03 cb30 	vmov.16	d3\[0\], ip
[^>]*> ee03 eb30 	vmov.16	d3\[0\], lr
[^>]*> ee05 0b30 	vmov.16	d5\[0\], r0
[^>]*> ee05 1b30 	vmov.16	d5\[0\], r1
[^>]*> ee05 2b30 	vmov.16	d5\[0\], r2
[^>]*> ee05 4b30 	vmov.16	d5\[0\], r4
[^>]*> ee05 7b30 	vmov.16	d5\[0\], r7
[^>]*> ee05 8b30 	vmov.16	d5\[0\], r8
[^>]*> ee05 ab30 	vmov.16	d5\[0\], sl
[^>]*> ee05 cb30 	vmov.16	d5\[0\], ip
[^>]*> ee05 eb30 	vmov.16	d5\[0\], lr
[^>]*> ee09 0b30 	vmov.16	d9\[0\], r0
[^>]*> ee09 1b30 	vmov.16	d9\[0\], r1
[^>]*> ee09 2b30 	vmov.16	d9\[0\], r2
[^>]*> ee09 4b30 	vmov.16	d9\[0\], r4
[^>]*> ee09 7b30 	vmov.16	d9\[0\], r7
[^>]*> ee09 8b30 	vmov.16	d9\[0\], r8
[^>]*> ee09 ab30 	vmov.16	d9\[0\], sl
[^>]*> ee09 cb30 	vmov.16	d9\[0\], ip
[^>]*> ee09 eb30 	vmov.16	d9\[0\], lr
[^>]*> ee0f 0b30 	vmov.16	d15\[0\], r0
[^>]*> ee0f 1b30 	vmov.16	d15\[0\], r1
[^>]*> ee0f 2b30 	vmov.16	d15\[0\], r2
[^>]*> ee0f 4b30 	vmov.16	d15\[0\], r4
[^>]*> ee0f 7b30 	vmov.16	d15\[0\], r7
[^>]*> ee0f 8b30 	vmov.16	d15\[0\], r8
[^>]*> ee0f ab30 	vmov.16	d15\[0\], sl
[^>]*> ee0f cb30 	vmov.16	d15\[0\], ip
[^>]*> ee0f eb30 	vmov.16	d15\[0\], lr
[^>]*> ee21 0b70 	vmov.16	d1\[3\], r0
[^>]*> ee21 1b70 	vmov.16	d1\[3\], r1
[^>]*> ee21 2b70 	vmov.16	d1\[3\], r2
[^>]*> ee21 4b70 	vmov.16	d1\[3\], r4
[^>]*> ee21 7b70 	vmov.16	d1\[3\], r7
[^>]*> ee21 8b70 	vmov.16	d1\[3\], r8
[^>]*> ee21 ab70 	vmov.16	d1\[3\], sl
[^>]*> ee21 cb70 	vmov.16	d1\[3\], ip
[^>]*> ee21 eb70 	vmov.16	d1\[3\], lr
[^>]*> ee23 0b70 	vmov.16	d3\[3\], r0
[^>]*> ee23 1b70 	vmov.16	d3\[3\], r1
[^>]*> ee23 2b70 	vmov.16	d3\[3\], r2
[^>]*> ee23 4b70 	vmov.16	d3\[3\], r4
[^>]*> ee23 7b70 	vmov.16	d3\[3\], r7
[^>]*> ee23 8b70 	vmov.16	d3\[3\], r8
[^>]*> ee23 ab70 	vmov.16	d3\[3\], sl
[^>]*> ee23 cb70 	vmov.16	d3\[3\], ip
[^>]*> ee23 eb70 	vmov.16	d3\[3\], lr
[^>]*> ee25 0b70 	vmov.16	d5\[3\], r0
[^>]*> ee25 1b70 	vmov.16	d5\[3\], r1
[^>]*> ee25 2b70 	vmov.16	d5\[3\], r2
[^>]*> ee25 4b70 	vmov.16	d5\[3\], r4
[^>]*> ee25 7b70 	vmov.16	d5\[3\], r7
[^>]*> ee25 8b70 	vmov.16	d5\[3\], r8
[^>]*> ee25 ab70 	vmov.16	d5\[3\], sl
[^>]*> ee25 cb70 	vmov.16	d5\[3\], ip
[^>]*> ee25 eb70 	vmov.16	d5\[3\], lr
[^>]*> ee29 0b70 	vmov.16	d9\[3\], r0
[^>]*> ee29 1b70 	vmov.16	d9\[3\], r1
[^>]*> ee29 2b70 	vmov.16	d9\[3\], r2
[^>]*> ee29 4b70 	vmov.16	d9\[3\], r4
[^>]*> ee29 7b70 	vmov.16	d9\[3\], r7
[^>]*> ee29 8b70 	vmov.16	d9\[3\], r8
[^>]*> ee29 ab70 	vmov.16	d9\[3\], sl
[^>]*> ee29 cb70 	vmov.16	d9\[3\], ip
[^>]*> ee29 eb70 	vmov.16	d9\[3\], lr
[^>]*> ee2f 0b70 	vmov.16	d15\[3\], r0
[^>]*> ee2f 1b70 	vmov.16	d15\[3\], r1
[^>]*> ee2f 2b70 	vmov.16	d15\[3\], r2
[^>]*> ee2f 4b70 	vmov.16	d15\[3\], r4
[^>]*> ee2f 7b70 	vmov.16	d15\[3\], r7
[^>]*> ee2f 8b70 	vmov.16	d15\[3\], r8
[^>]*> ee2f ab70 	vmov.16	d15\[3\], sl
[^>]*> ee2f cb70 	vmov.16	d15\[3\], ip
[^>]*> ee2f eb70 	vmov.16	d15\[3\], lr
[^>]*> ee00 0b10 	vmov.32	d0\[0\], r0
[^>]*> ee00 1b10 	vmov.32	d0\[0\], r1
[^>]*> ee00 2b10 	vmov.32	d0\[0\], r2
[^>]*> ee00 4b10 	vmov.32	d0\[0\], r4
[^>]*> ee00 7b10 	vmov.32	d0\[0\], r7
[^>]*> ee00 8b10 	vmov.32	d0\[0\], r8
[^>]*> ee00 ab10 	vmov.32	d0\[0\], sl
[^>]*> ee00 cb10 	vmov.32	d0\[0\], ip
[^>]*> ee00 eb10 	vmov.32	d0\[0\], lr
[^>]*> ee02 0b10 	vmov.32	d2\[0\], r0
[^>]*> ee02 1b10 	vmov.32	d2\[0\], r1
[^>]*> ee02 2b10 	vmov.32	d2\[0\], r2
[^>]*> ee02 4b10 	vmov.32	d2\[0\], r4
[^>]*> ee02 7b10 	vmov.32	d2\[0\], r7
[^>]*> ee02 8b10 	vmov.32	d2\[0\], r8
[^>]*> ee02 ab10 	vmov.32	d2\[0\], sl
[^>]*> ee02 cb10 	vmov.32	d2\[0\], ip
[^>]*> ee02 eb10 	vmov.32	d2\[0\], lr
[^>]*> ee04 0b10 	vmov.32	d4\[0\], r0
[^>]*> ee04 1b10 	vmov.32	d4\[0\], r1
[^>]*> ee04 2b10 	vmov.32	d4\[0\], r2
[^>]*> ee04 4b10 	vmov.32	d4\[0\], r4
[^>]*> ee04 7b10 	vmov.32	d4\[0\], r7
[^>]*> ee04 8b10 	vmov.32	d4\[0\], r8
[^>]*> ee04 ab10 	vmov.32	d4\[0\], sl
[^>]*> ee04 cb10 	vmov.32	d4\[0\], ip
[^>]*> ee04 eb10 	vmov.32	d4\[0\], lr
[^>]*> ee08 0b10 	vmov.32	d8\[0\], r0
[^>]*> ee08 1b10 	vmov.32	d8\[0\], r1
[^>]*> ee08 2b10 	vmov.32	d8\[0\], r2
[^>]*> ee08 4b10 	vmov.32	d8\[0\], r4
[^>]*> ee08 7b10 	vmov.32	d8\[0\], r7
[^>]*> ee08 8b10 	vmov.32	d8\[0\], r8
[^>]*> ee08 ab10 	vmov.32	d8\[0\], sl
[^>]*> ee08 cb10 	vmov.32	d8\[0\], ip
[^>]*> ee08 eb10 	vmov.32	d8\[0\], lr
[^>]*> ee0e 0b10 	vmov.32	d14\[0\], r0
[^>]*> ee0e 1b10 	vmov.32	d14\[0\], r1
[^>]*> ee0e 2b10 	vmov.32	d14\[0\], r2
[^>]*> ee0e 4b10 	vmov.32	d14\[0\], r4
[^>]*> ee0e 7b10 	vmov.32	d14\[0\], r7
[^>]*> ee0e 8b10 	vmov.32	d14\[0\], r8
[^>]*> ee0e ab10 	vmov.32	d14\[0\], sl
[^>]*> ee0e cb10 	vmov.32	d14\[0\], ip
[^>]*> ee0e eb10 	vmov.32	d14\[0\], lr
[^>]*> ee20 0b10 	vmov.32	d0\[1\], r0
[^>]*> ee20 1b10 	vmov.32	d0\[1\], r1
[^>]*> ee20 2b10 	vmov.32	d0\[1\], r2
[^>]*> ee20 4b10 	vmov.32	d0\[1\], r4
[^>]*> ee20 7b10 	vmov.32	d0\[1\], r7
[^>]*> ee20 8b10 	vmov.32	d0\[1\], r8
[^>]*> ee20 ab10 	vmov.32	d0\[1\], sl
[^>]*> ee20 cb10 	vmov.32	d0\[1\], ip
[^>]*> ee20 eb10 	vmov.32	d0\[1\], lr
[^>]*> ee22 0b10 	vmov.32	d2\[1\], r0
[^>]*> ee22 1b10 	vmov.32	d2\[1\], r1
[^>]*> ee22 2b10 	vmov.32	d2\[1\], r2
[^>]*> ee22 4b10 	vmov.32	d2\[1\], r4
[^>]*> ee22 7b10 	vmov.32	d2\[1\], r7
[^>]*> ee22 8b10 	vmov.32	d2\[1\], r8
[^>]*> ee22 ab10 	vmov.32	d2\[1\], sl
[^>]*> ee22 cb10 	vmov.32	d2\[1\], ip
[^>]*> ee22 eb10 	vmov.32	d2\[1\], lr
[^>]*> ee24 0b10 	vmov.32	d4\[1\], r0
[^>]*> ee24 1b10 	vmov.32	d4\[1\], r1
[^>]*> ee24 2b10 	vmov.32	d4\[1\], r2
[^>]*> ee24 4b10 	vmov.32	d4\[1\], r4
[^>]*> ee24 7b10 	vmov.32	d4\[1\], r7
[^>]*> ee24 8b10 	vmov.32	d4\[1\], r8
[^>]*> ee24 ab10 	vmov.32	d4\[1\], sl
[^>]*> ee24 cb10 	vmov.32	d4\[1\], ip
[^>]*> ee24 eb10 	vmov.32	d4\[1\], lr
[^>]*> ee28 0b10 	vmov.32	d8\[1\], r0
[^>]*> ee28 1b10 	vmov.32	d8\[1\], r1
[^>]*> ee28 2b10 	vmov.32	d8\[1\], r2
[^>]*> ee28 4b10 	vmov.32	d8\[1\], r4
[^>]*> ee28 7b10 	vmov.32	d8\[1\], r7
[^>]*> ee28 8b10 	vmov.32	d8\[1\], r8
[^>]*> ee28 ab10 	vmov.32	d8\[1\], sl
[^>]*> ee28 cb10 	vmov.32	d8\[1\], ip
[^>]*> ee28 eb10 	vmov.32	d8\[1\], lr
[^>]*> ee2e 0b10 	vmov.32	d14\[1\], r0
[^>]*> ee2e 1b10 	vmov.32	d14\[1\], r1
[^>]*> ee2e 2b10 	vmov.32	d14\[1\], r2
[^>]*> ee2e 4b10 	vmov.32	d14\[1\], r4
[^>]*> ee2e 7b10 	vmov.32	d14\[1\], r7
[^>]*> ee2e 8b10 	vmov.32	d14\[1\], r8
[^>]*> ee2e ab10 	vmov.32	d14\[1\], sl
[^>]*> ee2e cb10 	vmov.32	d14\[1\], ip
[^>]*> ee2e eb10 	vmov.32	d14\[1\], lr
[^>]*> ee01 0b10 	vmov.32	d1\[0\], r0
[^>]*> ee01 1b10 	vmov.32	d1\[0\], r1
[^>]*> ee01 2b10 	vmov.32	d1\[0\], r2
[^>]*> ee01 4b10 	vmov.32	d1\[0\], r4
[^>]*> ee01 7b10 	vmov.32	d1\[0\], r7
[^>]*> ee01 8b10 	vmov.32	d1\[0\], r8
[^>]*> ee01 ab10 	vmov.32	d1\[0\], sl
[^>]*> ee01 cb10 	vmov.32	d1\[0\], ip
[^>]*> ee01 eb10 	vmov.32	d1\[0\], lr
[^>]*> ee03 0b10 	vmov.32	d3\[0\], r0
[^>]*> ee03 1b10 	vmov.32	d3\[0\], r1
[^>]*> ee03 2b10 	vmov.32	d3\[0\], r2
[^>]*> ee03 4b10 	vmov.32	d3\[0\], r4
[^>]*> ee03 7b10 	vmov.32	d3\[0\], r7
[^>]*> ee03 8b10 	vmov.32	d3\[0\], r8
[^>]*> ee03 ab10 	vmov.32	d3\[0\], sl
[^>]*> ee03 cb10 	vmov.32	d3\[0\], ip
[^>]*> ee03 eb10 	vmov.32	d3\[0\], lr
[^>]*> ee05 0b10 	vmov.32	d5\[0\], r0
[^>]*> ee05 1b10 	vmov.32	d5\[0\], r1
[^>]*> ee05 2b10 	vmov.32	d5\[0\], r2
[^>]*> ee05 4b10 	vmov.32	d5\[0\], r4
[^>]*> ee05 7b10 	vmov.32	d5\[0\], r7
[^>]*> ee05 8b10 	vmov.32	d5\[0\], r8
[^>]*> ee05 ab10 	vmov.32	d5\[0\], sl
[^>]*> ee05 cb10 	vmov.32	d5\[0\], ip
[^>]*> ee05 eb10 	vmov.32	d5\[0\], lr
[^>]*> ee09 0b10 	vmov.32	d9\[0\], r0
[^>]*> ee09 1b10 	vmov.32	d9\[0\], r1
[^>]*> ee09 2b10 	vmov.32	d9\[0\], r2
[^>]*> ee09 4b10 	vmov.32	d9\[0\], r4
[^>]*> ee09 7b10 	vmov.32	d9\[0\], r7
[^>]*> ee09 8b10 	vmov.32	d9\[0\], r8
[^>]*> ee09 ab10 	vmov.32	d9\[0\], sl
[^>]*> ee09 cb10 	vmov.32	d9\[0\], ip
[^>]*> ee09 eb10 	vmov.32	d9\[0\], lr
[^>]*> ee0f 0b10 	vmov.32	d15\[0\], r0
[^>]*> ee0f 1b10 	vmov.32	d15\[0\], r1
[^>]*> ee0f 2b10 	vmov.32	d15\[0\], r2
[^>]*> ee0f 4b10 	vmov.32	d15\[0\], r4
[^>]*> ee0f 7b10 	vmov.32	d15\[0\], r7
[^>]*> ee0f 8b10 	vmov.32	d15\[0\], r8
[^>]*> ee0f ab10 	vmov.32	d15\[0\], sl
[^>]*> ee0f cb10 	vmov.32	d15\[0\], ip
[^>]*> ee0f eb10 	vmov.32	d15\[0\], lr
[^>]*> ee21 0b10 	vmov.32	d1\[1\], r0
[^>]*> ee21 1b10 	vmov.32	d1\[1\], r1
[^>]*> ee21 2b10 	vmov.32	d1\[1\], r2
[^>]*> ee21 4b10 	vmov.32	d1\[1\], r4
[^>]*> ee21 7b10 	vmov.32	d1\[1\], r7
[^>]*> ee21 8b10 	vmov.32	d1\[1\], r8
[^>]*> ee21 ab10 	vmov.32	d1\[1\], sl
[^>]*> ee21 cb10 	vmov.32	d1\[1\], ip
[^>]*> ee21 eb10 	vmov.32	d1\[1\], lr
[^>]*> ee23 0b10 	vmov.32	d3\[1\], r0
[^>]*> ee23 1b10 	vmov.32	d3\[1\], r1
[^>]*> ee23 2b10 	vmov.32	d3\[1\], r2
[^>]*> ee23 4b10 	vmov.32	d3\[1\], r4
[^>]*> ee23 7b10 	vmov.32	d3\[1\], r7
[^>]*> ee23 8b10 	vmov.32	d3\[1\], r8
[^>]*> ee23 ab10 	vmov.32	d3\[1\], sl
[^>]*> ee23 cb10 	vmov.32	d3\[1\], ip
[^>]*> ee23 eb10 	vmov.32	d3\[1\], lr
[^>]*> ee25 0b10 	vmov.32	d5\[1\], r0
[^>]*> ee25 1b10 	vmov.32	d5\[1\], r1
[^>]*> ee25 2b10 	vmov.32	d5\[1\], r2
[^>]*> ee25 4b10 	vmov.32	d5\[1\], r4
[^>]*> ee25 7b10 	vmov.32	d5\[1\], r7
[^>]*> ee25 8b10 	vmov.32	d5\[1\], r8
[^>]*> ee25 ab10 	vmov.32	d5\[1\], sl
[^>]*> ee25 cb10 	vmov.32	d5\[1\], ip
[^>]*> ee25 eb10 	vmov.32	d5\[1\], lr
[^>]*> ee29 0b10 	vmov.32	d9\[1\], r0
[^>]*> ee29 1b10 	vmov.32	d9\[1\], r1
[^>]*> ee29 2b10 	vmov.32	d9\[1\], r2
[^>]*> ee29 4b10 	vmov.32	d9\[1\], r4
[^>]*> ee29 7b10 	vmov.32	d9\[1\], r7
[^>]*> ee29 8b10 	vmov.32	d9\[1\], r8
[^>]*> ee29 ab10 	vmov.32	d9\[1\], sl
[^>]*> ee29 cb10 	vmov.32	d9\[1\], ip
[^>]*> ee29 eb10 	vmov.32	d9\[1\], lr
[^>]*> ee2f 0b10 	vmov.32	d15\[1\], r0
[^>]*> ee2f 1b10 	vmov.32	d15\[1\], r1
[^>]*> ee2f 2b10 	vmov.32	d15\[1\], r2
[^>]*> ee2f 4b10 	vmov.32	d15\[1\], r4
[^>]*> ee2f 7b10 	vmov.32	d15\[1\], r7
[^>]*> ee2f 8b10 	vmov.32	d15\[1\], r8
[^>]*> ee2f ab10 	vmov.32	d15\[1\], sl
[^>]*> ee2f cb10 	vmov.32	d15\[1\], ip
[^>]*> ee2f eb10 	vmov.32	d15\[1\], lr
[^>]*> eed0 0b10 	vmov.u8	r0, d0\[0\]
[^>]*> eed2 0b10 	vmov.u8	r0, d2\[0\]
[^>]*> eed4 0b10 	vmov.u8	r0, d4\[0\]
[^>]*> eed8 0b10 	vmov.u8	r0, d8\[0\]
[^>]*> eede 0b10 	vmov.u8	r0, d14\[0\]
[^>]*> eed0 1b10 	vmov.u8	r1, d0\[0\]
[^>]*> eed2 1b10 	vmov.u8	r1, d2\[0\]
[^>]*> eed4 1b10 	vmov.u8	r1, d4\[0\]
[^>]*> eed8 1b10 	vmov.u8	r1, d8\[0\]
[^>]*> eede 1b10 	vmov.u8	r1, d14\[0\]
[^>]*> eed0 2b10 	vmov.u8	r2, d0\[0\]
[^>]*> eed2 2b10 	vmov.u8	r2, d2\[0\]
[^>]*> eed4 2b10 	vmov.u8	r2, d4\[0\]
[^>]*> eed8 2b10 	vmov.u8	r2, d8\[0\]
[^>]*> eede 2b10 	vmov.u8	r2, d14\[0\]
[^>]*> eed0 4b10 	vmov.u8	r4, d0\[0\]
[^>]*> eed2 4b10 	vmov.u8	r4, d2\[0\]
[^>]*> eed4 4b10 	vmov.u8	r4, d4\[0\]
[^>]*> eed8 4b10 	vmov.u8	r4, d8\[0\]
[^>]*> eede 4b10 	vmov.u8	r4, d14\[0\]
[^>]*> eed0 7b10 	vmov.u8	r7, d0\[0\]
[^>]*> eed2 7b10 	vmov.u8	r7, d2\[0\]
[^>]*> eed4 7b10 	vmov.u8	r7, d4\[0\]
[^>]*> eed8 7b10 	vmov.u8	r7, d8\[0\]
[^>]*> eede 7b10 	vmov.u8	r7, d14\[0\]
[^>]*> eed0 8b10 	vmov.u8	r8, d0\[0\]
[^>]*> eed2 8b10 	vmov.u8	r8, d2\[0\]
[^>]*> eed4 8b10 	vmov.u8	r8, d4\[0\]
[^>]*> eed8 8b10 	vmov.u8	r8, d8\[0\]
[^>]*> eede 8b10 	vmov.u8	r8, d14\[0\]
[^>]*> eed0 ab10 	vmov.u8	sl, d0\[0\]
[^>]*> eed2 ab10 	vmov.u8	sl, d2\[0\]
[^>]*> eed4 ab10 	vmov.u8	sl, d4\[0\]
[^>]*> eed8 ab10 	vmov.u8	sl, d8\[0\]
[^>]*> eede ab10 	vmov.u8	sl, d14\[0\]
[^>]*> eed0 cb10 	vmov.u8	ip, d0\[0\]
[^>]*> eed2 cb10 	vmov.u8	ip, d2\[0\]
[^>]*> eed4 cb10 	vmov.u8	ip, d4\[0\]
[^>]*> eed8 cb10 	vmov.u8	ip, d8\[0\]
[^>]*> eede cb10 	vmov.u8	ip, d14\[0\]
[^>]*> eed0 eb10 	vmov.u8	lr, d0\[0\]
[^>]*> eed2 eb10 	vmov.u8	lr, d2\[0\]
[^>]*> eed4 eb10 	vmov.u8	lr, d4\[0\]
[^>]*> eed8 eb10 	vmov.u8	lr, d8\[0\]
[^>]*> eede eb10 	vmov.u8	lr, d14\[0\]
[^>]*> eed0 0b30 	vmov.u8	r0, d0\[1\]
[^>]*> eed2 0b30 	vmov.u8	r0, d2\[1\]
[^>]*> eed4 0b30 	vmov.u8	r0, d4\[1\]
[^>]*> eed8 0b30 	vmov.u8	r0, d8\[1\]
[^>]*> eede 0b30 	vmov.u8	r0, d14\[1\]
[^>]*> eed0 1b30 	vmov.u8	r1, d0\[1\]
[^>]*> eed2 1b30 	vmov.u8	r1, d2\[1\]
[^>]*> eed4 1b30 	vmov.u8	r1, d4\[1\]
[^>]*> eed8 1b30 	vmov.u8	r1, d8\[1\]
[^>]*> eede 1b30 	vmov.u8	r1, d14\[1\]
[^>]*> eed0 2b30 	vmov.u8	r2, d0\[1\]
[^>]*> eed2 2b30 	vmov.u8	r2, d2\[1\]
[^>]*> eed4 2b30 	vmov.u8	r2, d4\[1\]
[^>]*> eed8 2b30 	vmov.u8	r2, d8\[1\]
[^>]*> eede 2b30 	vmov.u8	r2, d14\[1\]
[^>]*> eed0 4b30 	vmov.u8	r4, d0\[1\]
[^>]*> eed2 4b30 	vmov.u8	r4, d2\[1\]
[^>]*> eed4 4b30 	vmov.u8	r4, d4\[1\]
[^>]*> eed8 4b30 	vmov.u8	r4, d8\[1\]
[^>]*> eede 4b30 	vmov.u8	r4, d14\[1\]
[^>]*> eed0 7b30 	vmov.u8	r7, d0\[1\]
[^>]*> eed2 7b30 	vmov.u8	r7, d2\[1\]
[^>]*> eed4 7b30 	vmov.u8	r7, d4\[1\]
[^>]*> eed8 7b30 	vmov.u8	r7, d8\[1\]
[^>]*> eede 7b30 	vmov.u8	r7, d14\[1\]
[^>]*> eed0 8b30 	vmov.u8	r8, d0\[1\]
[^>]*> eed2 8b30 	vmov.u8	r8, d2\[1\]
[^>]*> eed4 8b30 	vmov.u8	r8, d4\[1\]
[^>]*> eed8 8b30 	vmov.u8	r8, d8\[1\]
[^>]*> eede 8b30 	vmov.u8	r8, d14\[1\]
[^>]*> eed0 ab30 	vmov.u8	sl, d0\[1\]
[^>]*> eed2 ab30 	vmov.u8	sl, d2\[1\]
[^>]*> eed4 ab30 	vmov.u8	sl, d4\[1\]
[^>]*> eed8 ab30 	vmov.u8	sl, d8\[1\]
[^>]*> eede ab30 	vmov.u8	sl, d14\[1\]
[^>]*> eed0 cb30 	vmov.u8	ip, d0\[1\]
[^>]*> eed2 cb30 	vmov.u8	ip, d2\[1\]
[^>]*> eed4 cb30 	vmov.u8	ip, d4\[1\]
[^>]*> eed8 cb30 	vmov.u8	ip, d8\[1\]
[^>]*> eede cb30 	vmov.u8	ip, d14\[1\]
[^>]*> eed0 eb30 	vmov.u8	lr, d0\[1\]
[^>]*> eed2 eb30 	vmov.u8	lr, d2\[1\]
[^>]*> eed4 eb30 	vmov.u8	lr, d4\[1\]
[^>]*> eed8 eb30 	vmov.u8	lr, d8\[1\]
[^>]*> eede eb30 	vmov.u8	lr, d14\[1\]
[^>]*> eed0 0b50 	vmov.u8	r0, d0\[2\]
[^>]*> eed2 0b50 	vmov.u8	r0, d2\[2\]
[^>]*> eed4 0b50 	vmov.u8	r0, d4\[2\]
[^>]*> eed8 0b50 	vmov.u8	r0, d8\[2\]
[^>]*> eede 0b50 	vmov.u8	r0, d14\[2\]
[^>]*> eed0 1b50 	vmov.u8	r1, d0\[2\]
[^>]*> eed2 1b50 	vmov.u8	r1, d2\[2\]
[^>]*> eed4 1b50 	vmov.u8	r1, d4\[2\]
[^>]*> eed8 1b50 	vmov.u8	r1, d8\[2\]
[^>]*> eede 1b50 	vmov.u8	r1, d14\[2\]
[^>]*> eed0 2b50 	vmov.u8	r2, d0\[2\]
[^>]*> eed2 2b50 	vmov.u8	r2, d2\[2\]
[^>]*> eed4 2b50 	vmov.u8	r2, d4\[2\]
[^>]*> eed8 2b50 	vmov.u8	r2, d8\[2\]
[^>]*> eede 2b50 	vmov.u8	r2, d14\[2\]
[^>]*> eed0 4b50 	vmov.u8	r4, d0\[2\]
[^>]*> eed2 4b50 	vmov.u8	r4, d2\[2\]
[^>]*> eed4 4b50 	vmov.u8	r4, d4\[2\]
[^>]*> eed8 4b50 	vmov.u8	r4, d8\[2\]
[^>]*> eede 4b50 	vmov.u8	r4, d14\[2\]
[^>]*> eed0 7b50 	vmov.u8	r7, d0\[2\]
[^>]*> eed2 7b50 	vmov.u8	r7, d2\[2\]
[^>]*> eed4 7b50 	vmov.u8	r7, d4\[2\]
[^>]*> eed8 7b50 	vmov.u8	r7, d8\[2\]
[^>]*> eede 7b50 	vmov.u8	r7, d14\[2\]
[^>]*> eed0 8b50 	vmov.u8	r8, d0\[2\]
[^>]*> eed2 8b50 	vmov.u8	r8, d2\[2\]
[^>]*> eed4 8b50 	vmov.u8	r8, d4\[2\]
[^>]*> eed8 8b50 	vmov.u8	r8, d8\[2\]
[^>]*> eede 8b50 	vmov.u8	r8, d14\[2\]
[^>]*> eed0 ab50 	vmov.u8	sl, d0\[2\]
[^>]*> eed2 ab50 	vmov.u8	sl, d2\[2\]
[^>]*> eed4 ab50 	vmov.u8	sl, d4\[2\]
[^>]*> eed8 ab50 	vmov.u8	sl, d8\[2\]
[^>]*> eede ab50 	vmov.u8	sl, d14\[2\]
[^>]*> eed0 cb50 	vmov.u8	ip, d0\[2\]
[^>]*> eed2 cb50 	vmov.u8	ip, d2\[2\]
[^>]*> eed4 cb50 	vmov.u8	ip, d4\[2\]
[^>]*> eed8 cb50 	vmov.u8	ip, d8\[2\]
[^>]*> eede cb50 	vmov.u8	ip, d14\[2\]
[^>]*> eed0 eb50 	vmov.u8	lr, d0\[2\]
[^>]*> eed2 eb50 	vmov.u8	lr, d2\[2\]
[^>]*> eed4 eb50 	vmov.u8	lr, d4\[2\]
[^>]*> eed8 eb50 	vmov.u8	lr, d8\[2\]
[^>]*> eede eb50 	vmov.u8	lr, d14\[2\]
[^>]*> eef0 0b10 	vmov.u8	r0, d0\[4\]
[^>]*> eef2 0b10 	vmov.u8	r0, d2\[4\]
[^>]*> eef4 0b10 	vmov.u8	r0, d4\[4\]
[^>]*> eef8 0b10 	vmov.u8	r0, d8\[4\]
[^>]*> eefe 0b10 	vmov.u8	r0, d14\[4\]
[^>]*> eef0 1b10 	vmov.u8	r1, d0\[4\]
[^>]*> eef2 1b10 	vmov.u8	r1, d2\[4\]
[^>]*> eef4 1b10 	vmov.u8	r1, d4\[4\]
[^>]*> eef8 1b10 	vmov.u8	r1, d8\[4\]
[^>]*> eefe 1b10 	vmov.u8	r1, d14\[4\]
[^>]*> eef0 2b10 	vmov.u8	r2, d0\[4\]
[^>]*> eef2 2b10 	vmov.u8	r2, d2\[4\]
[^>]*> eef4 2b10 	vmov.u8	r2, d4\[4\]
[^>]*> eef8 2b10 	vmov.u8	r2, d8\[4\]
[^>]*> eefe 2b10 	vmov.u8	r2, d14\[4\]
[^>]*> eef0 4b10 	vmov.u8	r4, d0\[4\]
[^>]*> eef2 4b10 	vmov.u8	r4, d2\[4\]
[^>]*> eef4 4b10 	vmov.u8	r4, d4\[4\]
[^>]*> eef8 4b10 	vmov.u8	r4, d8\[4\]
[^>]*> eefe 4b10 	vmov.u8	r4, d14\[4\]
[^>]*> eef0 7b10 	vmov.u8	r7, d0\[4\]
[^>]*> eef2 7b10 	vmov.u8	r7, d2\[4\]
[^>]*> eef4 7b10 	vmov.u8	r7, d4\[4\]
[^>]*> eef8 7b10 	vmov.u8	r7, d8\[4\]
[^>]*> eefe 7b10 	vmov.u8	r7, d14\[4\]
[^>]*> eef0 8b10 	vmov.u8	r8, d0\[4\]
[^>]*> eef2 8b10 	vmov.u8	r8, d2\[4\]
[^>]*> eef4 8b10 	vmov.u8	r8, d4\[4\]
[^>]*> eef8 8b10 	vmov.u8	r8, d8\[4\]
[^>]*> eefe 8b10 	vmov.u8	r8, d14\[4\]
[^>]*> eef0 ab10 	vmov.u8	sl, d0\[4\]
[^>]*> eef2 ab10 	vmov.u8	sl, d2\[4\]
[^>]*> eef4 ab10 	vmov.u8	sl, d4\[4\]
[^>]*> eef8 ab10 	vmov.u8	sl, d8\[4\]
[^>]*> eefe ab10 	vmov.u8	sl, d14\[4\]
[^>]*> eef0 cb10 	vmov.u8	ip, d0\[4\]
[^>]*> eef2 cb10 	vmov.u8	ip, d2\[4\]
[^>]*> eef4 cb10 	vmov.u8	ip, d4\[4\]
[^>]*> eef8 cb10 	vmov.u8	ip, d8\[4\]
[^>]*> eefe cb10 	vmov.u8	ip, d14\[4\]
[^>]*> eef0 eb10 	vmov.u8	lr, d0\[4\]
[^>]*> eef2 eb10 	vmov.u8	lr, d2\[4\]
[^>]*> eef4 eb10 	vmov.u8	lr, d4\[4\]
[^>]*> eef8 eb10 	vmov.u8	lr, d8\[4\]
[^>]*> eefe eb10 	vmov.u8	lr, d14\[4\]
[^>]*> eed1 0b10 	vmov.u8	r0, d1\[0\]
[^>]*> eed3 0b10 	vmov.u8	r0, d3\[0\]
[^>]*> eed5 0b10 	vmov.u8	r0, d5\[0\]
[^>]*> eed9 0b10 	vmov.u8	r0, d9\[0\]
[^>]*> eedf 0b10 	vmov.u8	r0, d15\[0\]
[^>]*> eed1 1b10 	vmov.u8	r1, d1\[0\]
[^>]*> eed3 1b10 	vmov.u8	r1, d3\[0\]
[^>]*> eed5 1b10 	vmov.u8	r1, d5\[0\]
[^>]*> eed9 1b10 	vmov.u8	r1, d9\[0\]
[^>]*> eedf 1b10 	vmov.u8	r1, d15\[0\]
[^>]*> eed1 2b10 	vmov.u8	r2, d1\[0\]
[^>]*> eed3 2b10 	vmov.u8	r2, d3\[0\]
[^>]*> eed5 2b10 	vmov.u8	r2, d5\[0\]
[^>]*> eed9 2b10 	vmov.u8	r2, d9\[0\]
[^>]*> eedf 2b10 	vmov.u8	r2, d15\[0\]
[^>]*> eed1 4b10 	vmov.u8	r4, d1\[0\]
[^>]*> eed3 4b10 	vmov.u8	r4, d3\[0\]
[^>]*> eed5 4b10 	vmov.u8	r4, d5\[0\]
[^>]*> eed9 4b10 	vmov.u8	r4, d9\[0\]
[^>]*> eedf 4b10 	vmov.u8	r4, d15\[0\]
[^>]*> eed1 7b10 	vmov.u8	r7, d1\[0\]
[^>]*> eed3 7b10 	vmov.u8	r7, d3\[0\]
[^>]*> eed5 7b10 	vmov.u8	r7, d5\[0\]
[^>]*> eed9 7b10 	vmov.u8	r7, d9\[0\]
[^>]*> eedf 7b10 	vmov.u8	r7, d15\[0\]
[^>]*> eed1 8b10 	vmov.u8	r8, d1\[0\]
[^>]*> eed3 8b10 	vmov.u8	r8, d3\[0\]
[^>]*> eed5 8b10 	vmov.u8	r8, d5\[0\]
[^>]*> eed9 8b10 	vmov.u8	r8, d9\[0\]
[^>]*> eedf 8b10 	vmov.u8	r8, d15\[0\]
[^>]*> eed1 ab10 	vmov.u8	sl, d1\[0\]
[^>]*> eed3 ab10 	vmov.u8	sl, d3\[0\]
[^>]*> eed5 ab10 	vmov.u8	sl, d5\[0\]
[^>]*> eed9 ab10 	vmov.u8	sl, d9\[0\]
[^>]*> eedf ab10 	vmov.u8	sl, d15\[0\]
[^>]*> eed1 cb10 	vmov.u8	ip, d1\[0\]
[^>]*> eed3 cb10 	vmov.u8	ip, d3\[0\]
[^>]*> eed5 cb10 	vmov.u8	ip, d5\[0\]
[^>]*> eed9 cb10 	vmov.u8	ip, d9\[0\]
[^>]*> eedf cb10 	vmov.u8	ip, d15\[0\]
[^>]*> eed1 eb10 	vmov.u8	lr, d1\[0\]
[^>]*> eed3 eb10 	vmov.u8	lr, d3\[0\]
[^>]*> eed5 eb10 	vmov.u8	lr, d5\[0\]
[^>]*> eed9 eb10 	vmov.u8	lr, d9\[0\]
[^>]*> eedf eb10 	vmov.u8	lr, d15\[0\]
[^>]*> eef1 0b70 	vmov.u8	r0, d1\[7\]
[^>]*> eef3 0b70 	vmov.u8	r0, d3\[7\]
[^>]*> eef5 0b70 	vmov.u8	r0, d5\[7\]
[^>]*> eef9 0b70 	vmov.u8	r0, d9\[7\]
[^>]*> eeff 0b70 	vmov.u8	r0, d15\[7\]
[^>]*> eef1 1b70 	vmov.u8	r1, d1\[7\]
[^>]*> eef3 1b70 	vmov.u8	r1, d3\[7\]
[^>]*> eef5 1b70 	vmov.u8	r1, d5\[7\]
[^>]*> eef9 1b70 	vmov.u8	r1, d9\[7\]
[^>]*> eeff 1b70 	vmov.u8	r1, d15\[7\]
[^>]*> eef1 2b70 	vmov.u8	r2, d1\[7\]
[^>]*> eef3 2b70 	vmov.u8	r2, d3\[7\]
[^>]*> eef5 2b70 	vmov.u8	r2, d5\[7\]
[^>]*> eef9 2b70 	vmov.u8	r2, d9\[7\]
[^>]*> eeff 2b70 	vmov.u8	r2, d15\[7\]
[^>]*> eef1 4b70 	vmov.u8	r4, d1\[7\]
[^>]*> eef3 4b70 	vmov.u8	r4, d3\[7\]
[^>]*> eef5 4b70 	vmov.u8	r4, d5\[7\]
[^>]*> eef9 4b70 	vmov.u8	r4, d9\[7\]
[^>]*> eeff 4b70 	vmov.u8	r4, d15\[7\]
[^>]*> eef1 7b70 	vmov.u8	r7, d1\[7\]
[^>]*> eef3 7b70 	vmov.u8	r7, d3\[7\]
[^>]*> eef5 7b70 	vmov.u8	r7, d5\[7\]
[^>]*> eef9 7b70 	vmov.u8	r7, d9\[7\]
[^>]*> eeff 7b70 	vmov.u8	r7, d15\[7\]
[^>]*> eef1 8b70 	vmov.u8	r8, d1\[7\]
[^>]*> eef3 8b70 	vmov.u8	r8, d3\[7\]
[^>]*> eef5 8b70 	vmov.u8	r8, d5\[7\]
[^>]*> eef9 8b70 	vmov.u8	r8, d9\[7\]
[^>]*> eeff 8b70 	vmov.u8	r8, d15\[7\]
[^>]*> eef1 ab70 	vmov.u8	sl, d1\[7\]
[^>]*> eef3 ab70 	vmov.u8	sl, d3\[7\]
[^>]*> eef5 ab70 	vmov.u8	sl, d5\[7\]
[^>]*> eef9 ab70 	vmov.u8	sl, d9\[7\]
[^>]*> eeff ab70 	vmov.u8	sl, d15\[7\]
[^>]*> eef1 cb70 	vmov.u8	ip, d1\[7\]
[^>]*> eef3 cb70 	vmov.u8	ip, d3\[7\]
[^>]*> eef5 cb70 	vmov.u8	ip, d5\[7\]
[^>]*> eef9 cb70 	vmov.u8	ip, d9\[7\]
[^>]*> eeff cb70 	vmov.u8	ip, d15\[7\]
[^>]*> eef1 eb70 	vmov.u8	lr, d1\[7\]
[^>]*> eef3 eb70 	vmov.u8	lr, d3\[7\]
[^>]*> eef5 eb70 	vmov.u8	lr, d5\[7\]
[^>]*> eef9 eb70 	vmov.u8	lr, d9\[7\]
[^>]*> eeff eb70 	vmov.u8	lr, d15\[7\]
[^>]*> eef1 0b30 	vmov.u8	r0, d1\[5\]
[^>]*> eef3 0b30 	vmov.u8	r0, d3\[5\]
[^>]*> eef5 0b30 	vmov.u8	r0, d5\[5\]
[^>]*> eef9 0b30 	vmov.u8	r0, d9\[5\]
[^>]*> eeff 0b30 	vmov.u8	r0, d15\[5\]
[^>]*> eef1 1b30 	vmov.u8	r1, d1\[5\]
[^>]*> eef3 1b30 	vmov.u8	r1, d3\[5\]
[^>]*> eef5 1b30 	vmov.u8	r1, d5\[5\]
[^>]*> eef9 1b30 	vmov.u8	r1, d9\[5\]
[^>]*> eeff 1b30 	vmov.u8	r1, d15\[5\]
[^>]*> eef1 2b30 	vmov.u8	r2, d1\[5\]
[^>]*> eef3 2b30 	vmov.u8	r2, d3\[5\]
[^>]*> eef5 2b30 	vmov.u8	r2, d5\[5\]
[^>]*> eef9 2b30 	vmov.u8	r2, d9\[5\]
[^>]*> eeff 2b30 	vmov.u8	r2, d15\[5\]
[^>]*> eef1 4b30 	vmov.u8	r4, d1\[5\]
[^>]*> eef3 4b30 	vmov.u8	r4, d3\[5\]
[^>]*> eef5 4b30 	vmov.u8	r4, d5\[5\]
[^>]*> eef9 4b30 	vmov.u8	r4, d9\[5\]
[^>]*> eeff 4b30 	vmov.u8	r4, d15\[5\]
[^>]*> eef1 7b30 	vmov.u8	r7, d1\[5\]
[^>]*> eef3 7b30 	vmov.u8	r7, d3\[5\]
[^>]*> eef5 7b30 	vmov.u8	r7, d5\[5\]
[^>]*> eef9 7b30 	vmov.u8	r7, d9\[5\]
[^>]*> eeff 7b30 	vmov.u8	r7, d15\[5\]
[^>]*> eef1 8b30 	vmov.u8	r8, d1\[5\]
[^>]*> eef3 8b30 	vmov.u8	r8, d3\[5\]
[^>]*> eef5 8b30 	vmov.u8	r8, d5\[5\]
[^>]*> eef9 8b30 	vmov.u8	r8, d9\[5\]
[^>]*> eeff 8b30 	vmov.u8	r8, d15\[5\]
[^>]*> eef1 ab30 	vmov.u8	sl, d1\[5\]
[^>]*> eef3 ab30 	vmov.u8	sl, d3\[5\]
[^>]*> eef5 ab30 	vmov.u8	sl, d5\[5\]
[^>]*> eef9 ab30 	vmov.u8	sl, d9\[5\]
[^>]*> eeff ab30 	vmov.u8	sl, d15\[5\]
[^>]*> eef1 cb30 	vmov.u8	ip, d1\[5\]
[^>]*> eef3 cb30 	vmov.u8	ip, d3\[5\]
[^>]*> eef5 cb30 	vmov.u8	ip, d5\[5\]
[^>]*> eef9 cb30 	vmov.u8	ip, d9\[5\]
[^>]*> eeff cb30 	vmov.u8	ip, d15\[5\]
[^>]*> eef1 eb30 	vmov.u8	lr, d1\[5\]
[^>]*> eef3 eb30 	vmov.u8	lr, d3\[5\]
[^>]*> eef5 eb30 	vmov.u8	lr, d5\[5\]
[^>]*> eef9 eb30 	vmov.u8	lr, d9\[5\]
[^>]*> eeff eb30 	vmov.u8	lr, d15\[5\]
[^>]*> eef0 0b50 	vmov.u8	r0, d0\[6\]
[^>]*> eef2 0b50 	vmov.u8	r0, d2\[6\]
[^>]*> eef4 0b50 	vmov.u8	r0, d4\[6\]
[^>]*> eef8 0b50 	vmov.u8	r0, d8\[6\]
[^>]*> eefe 0b50 	vmov.u8	r0, d14\[6\]
[^>]*> eef0 1b50 	vmov.u8	r1, d0\[6\]
[^>]*> eef2 1b50 	vmov.u8	r1, d2\[6\]
[^>]*> eef4 1b50 	vmov.u8	r1, d4\[6\]
[^>]*> eef8 1b50 	vmov.u8	r1, d8\[6\]
[^>]*> eefe 1b50 	vmov.u8	r1, d14\[6\]
[^>]*> eef0 2b50 	vmov.u8	r2, d0\[6\]
[^>]*> eef2 2b50 	vmov.u8	r2, d2\[6\]
[^>]*> eef4 2b50 	vmov.u8	r2, d4\[6\]
[^>]*> eef8 2b50 	vmov.u8	r2, d8\[6\]
[^>]*> eefe 2b50 	vmov.u8	r2, d14\[6\]
[^>]*> eef0 4b50 	vmov.u8	r4, d0\[6\]
[^>]*> eef2 4b50 	vmov.u8	r4, d2\[6\]
[^>]*> eef4 4b50 	vmov.u8	r4, d4\[6\]
[^>]*> eef8 4b50 	vmov.u8	r4, d8\[6\]
[^>]*> eefe 4b50 	vmov.u8	r4, d14\[6\]
[^>]*> eef0 7b50 	vmov.u8	r7, d0\[6\]
[^>]*> eef2 7b50 	vmov.u8	r7, d2\[6\]
[^>]*> eef4 7b50 	vmov.u8	r7, d4\[6\]
[^>]*> eef8 7b50 	vmov.u8	r7, d8\[6\]
[^>]*> eefe 7b50 	vmov.u8	r7, d14\[6\]
[^>]*> eef0 8b50 	vmov.u8	r8, d0\[6\]
[^>]*> eef2 8b50 	vmov.u8	r8, d2\[6\]
[^>]*> eef4 8b50 	vmov.u8	r8, d4\[6\]
[^>]*> eef8 8b50 	vmov.u8	r8, d8\[6\]
[^>]*> eefe 8b50 	vmov.u8	r8, d14\[6\]
[^>]*> eef0 ab50 	vmov.u8	sl, d0\[6\]
[^>]*> eef2 ab50 	vmov.u8	sl, d2\[6\]
[^>]*> eef4 ab50 	vmov.u8	sl, d4\[6\]
[^>]*> eef8 ab50 	vmov.u8	sl, d8\[6\]
[^>]*> eefe ab50 	vmov.u8	sl, d14\[6\]
[^>]*> eef0 cb50 	vmov.u8	ip, d0\[6\]
[^>]*> eef2 cb50 	vmov.u8	ip, d2\[6\]
[^>]*> eef4 cb50 	vmov.u8	ip, d4\[6\]
[^>]*> eef8 cb50 	vmov.u8	ip, d8\[6\]
[^>]*> eefe cb50 	vmov.u8	ip, d14\[6\]
[^>]*> eef0 eb50 	vmov.u8	lr, d0\[6\]
[^>]*> eef2 eb50 	vmov.u8	lr, d2\[6\]
[^>]*> eef4 eb50 	vmov.u8	lr, d4\[6\]
[^>]*> eef8 eb50 	vmov.u8	lr, d8\[6\]
[^>]*> eefe eb50 	vmov.u8	lr, d14\[6\]
[^>]*> ee50 0b10 	vmov.s8	r0, d0\[0\]
[^>]*> ee52 0b10 	vmov.s8	r0, d2\[0\]
[^>]*> ee54 0b10 	vmov.s8	r0, d4\[0\]
[^>]*> ee58 0b10 	vmov.s8	r0, d8\[0\]
[^>]*> ee5e 0b10 	vmov.s8	r0, d14\[0\]
[^>]*> ee50 1b10 	vmov.s8	r1, d0\[0\]
[^>]*> ee52 1b10 	vmov.s8	r1, d2\[0\]
[^>]*> ee54 1b10 	vmov.s8	r1, d4\[0\]
[^>]*> ee58 1b10 	vmov.s8	r1, d8\[0\]
[^>]*> ee5e 1b10 	vmov.s8	r1, d14\[0\]
[^>]*> ee50 2b10 	vmov.s8	r2, d0\[0\]
[^>]*> ee52 2b10 	vmov.s8	r2, d2\[0\]
[^>]*> ee54 2b10 	vmov.s8	r2, d4\[0\]
[^>]*> ee58 2b10 	vmov.s8	r2, d8\[0\]
[^>]*> ee5e 2b10 	vmov.s8	r2, d14\[0\]
[^>]*> ee50 4b10 	vmov.s8	r4, d0\[0\]
[^>]*> ee52 4b10 	vmov.s8	r4, d2\[0\]
[^>]*> ee54 4b10 	vmov.s8	r4, d4\[0\]
[^>]*> ee58 4b10 	vmov.s8	r4, d8\[0\]
[^>]*> ee5e 4b10 	vmov.s8	r4, d14\[0\]
[^>]*> ee50 7b10 	vmov.s8	r7, d0\[0\]
[^>]*> ee52 7b10 	vmov.s8	r7, d2\[0\]
[^>]*> ee54 7b10 	vmov.s8	r7, d4\[0\]
[^>]*> ee58 7b10 	vmov.s8	r7, d8\[0\]
[^>]*> ee5e 7b10 	vmov.s8	r7, d14\[0\]
[^>]*> ee50 8b10 	vmov.s8	r8, d0\[0\]
[^>]*> ee52 8b10 	vmov.s8	r8, d2\[0\]
[^>]*> ee54 8b10 	vmov.s8	r8, d4\[0\]
[^>]*> ee58 8b10 	vmov.s8	r8, d8\[0\]
[^>]*> ee5e 8b10 	vmov.s8	r8, d14\[0\]
[^>]*> ee50 ab10 	vmov.s8	sl, d0\[0\]
[^>]*> ee52 ab10 	vmov.s8	sl, d2\[0\]
[^>]*> ee54 ab10 	vmov.s8	sl, d4\[0\]
[^>]*> ee58 ab10 	vmov.s8	sl, d8\[0\]
[^>]*> ee5e ab10 	vmov.s8	sl, d14\[0\]
[^>]*> ee50 cb10 	vmov.s8	ip, d0\[0\]
[^>]*> ee52 cb10 	vmov.s8	ip, d2\[0\]
[^>]*> ee54 cb10 	vmov.s8	ip, d4\[0\]
[^>]*> ee58 cb10 	vmov.s8	ip, d8\[0\]
[^>]*> ee5e cb10 	vmov.s8	ip, d14\[0\]
[^>]*> ee50 eb10 	vmov.s8	lr, d0\[0\]
[^>]*> ee52 eb10 	vmov.s8	lr, d2\[0\]
[^>]*> ee54 eb10 	vmov.s8	lr, d4\[0\]
[^>]*> ee58 eb10 	vmov.s8	lr, d8\[0\]
[^>]*> ee5e eb10 	vmov.s8	lr, d14\[0\]
[^>]*> ee50 0b30 	vmov.s8	r0, d0\[1\]
[^>]*> ee52 0b30 	vmov.s8	r0, d2\[1\]
[^>]*> ee54 0b30 	vmov.s8	r0, d4\[1\]
[^>]*> ee58 0b30 	vmov.s8	r0, d8\[1\]
[^>]*> ee5e 0b30 	vmov.s8	r0, d14\[1\]
[^>]*> ee50 1b30 	vmov.s8	r1, d0\[1\]
[^>]*> ee52 1b30 	vmov.s8	r1, d2\[1\]
[^>]*> ee54 1b30 	vmov.s8	r1, d4\[1\]
[^>]*> ee58 1b30 	vmov.s8	r1, d8\[1\]
[^>]*> ee5e 1b30 	vmov.s8	r1, d14\[1\]
[^>]*> ee50 2b30 	vmov.s8	r2, d0\[1\]
[^>]*> ee52 2b30 	vmov.s8	r2, d2\[1\]
[^>]*> ee54 2b30 	vmov.s8	r2, d4\[1\]
[^>]*> ee58 2b30 	vmov.s8	r2, d8\[1\]
[^>]*> ee5e 2b30 	vmov.s8	r2, d14\[1\]
[^>]*> ee50 4b30 	vmov.s8	r4, d0\[1\]
[^>]*> ee52 4b30 	vmov.s8	r4, d2\[1\]
[^>]*> ee54 4b30 	vmov.s8	r4, d4\[1\]
[^>]*> ee58 4b30 	vmov.s8	r4, d8\[1\]
[^>]*> ee5e 4b30 	vmov.s8	r4, d14\[1\]
[^>]*> ee50 7b30 	vmov.s8	r7, d0\[1\]
[^>]*> ee52 7b30 	vmov.s8	r7, d2\[1\]
[^>]*> ee54 7b30 	vmov.s8	r7, d4\[1\]
[^>]*> ee58 7b30 	vmov.s8	r7, d8\[1\]
[^>]*> ee5e 7b30 	vmov.s8	r7, d14\[1\]
[^>]*> ee50 8b30 	vmov.s8	r8, d0\[1\]
[^>]*> ee52 8b30 	vmov.s8	r8, d2\[1\]
[^>]*> ee54 8b30 	vmov.s8	r8, d4\[1\]
[^>]*> ee58 8b30 	vmov.s8	r8, d8\[1\]
[^>]*> ee5e 8b30 	vmov.s8	r8, d14\[1\]
[^>]*> ee50 ab30 	vmov.s8	sl, d0\[1\]
[^>]*> ee52 ab30 	vmov.s8	sl, d2\[1\]
[^>]*> ee54 ab30 	vmov.s8	sl, d4\[1\]
[^>]*> ee58 ab30 	vmov.s8	sl, d8\[1\]
[^>]*> ee5e ab30 	vmov.s8	sl, d14\[1\]
[^>]*> ee50 cb30 	vmov.s8	ip, d0\[1\]
[^>]*> ee52 cb30 	vmov.s8	ip, d2\[1\]
[^>]*> ee54 cb30 	vmov.s8	ip, d4\[1\]
[^>]*> ee58 cb30 	vmov.s8	ip, d8\[1\]
[^>]*> ee5e cb30 	vmov.s8	ip, d14\[1\]
[^>]*> ee50 eb30 	vmov.s8	lr, d0\[1\]
[^>]*> ee52 eb30 	vmov.s8	lr, d2\[1\]
[^>]*> ee54 eb30 	vmov.s8	lr, d4\[1\]
[^>]*> ee58 eb30 	vmov.s8	lr, d8\[1\]
[^>]*> ee5e eb30 	vmov.s8	lr, d14\[1\]
[^>]*> ee50 0b50 	vmov.s8	r0, d0\[2\]
[^>]*> ee52 0b50 	vmov.s8	r0, d2\[2\]
[^>]*> ee54 0b50 	vmov.s8	r0, d4\[2\]
[^>]*> ee58 0b50 	vmov.s8	r0, d8\[2\]
[^>]*> ee5e 0b50 	vmov.s8	r0, d14\[2\]
[^>]*> ee50 1b50 	vmov.s8	r1, d0\[2\]
[^>]*> ee52 1b50 	vmov.s8	r1, d2\[2\]
[^>]*> ee54 1b50 	vmov.s8	r1, d4\[2\]
[^>]*> ee58 1b50 	vmov.s8	r1, d8\[2\]
[^>]*> ee5e 1b50 	vmov.s8	r1, d14\[2\]
[^>]*> ee50 2b50 	vmov.s8	r2, d0\[2\]
[^>]*> ee52 2b50 	vmov.s8	r2, d2\[2\]
[^>]*> ee54 2b50 	vmov.s8	r2, d4\[2\]
[^>]*> ee58 2b50 	vmov.s8	r2, d8\[2\]
[^>]*> ee5e 2b50 	vmov.s8	r2, d14\[2\]
[^>]*> ee50 4b50 	vmov.s8	r4, d0\[2\]
[^>]*> ee52 4b50 	vmov.s8	r4, d2\[2\]
[^>]*> ee54 4b50 	vmov.s8	r4, d4\[2\]
[^>]*> ee58 4b50 	vmov.s8	r4, d8\[2\]
[^>]*> ee5e 4b50 	vmov.s8	r4, d14\[2\]
[^>]*> ee50 7b50 	vmov.s8	r7, d0\[2\]
[^>]*> ee52 7b50 	vmov.s8	r7, d2\[2\]
[^>]*> ee54 7b50 	vmov.s8	r7, d4\[2\]
[^>]*> ee58 7b50 	vmov.s8	r7, d8\[2\]
[^>]*> ee5e 7b50 	vmov.s8	r7, d14\[2\]
[^>]*> ee50 8b50 	vmov.s8	r8, d0\[2\]
[^>]*> ee52 8b50 	vmov.s8	r8, d2\[2\]
[^>]*> ee54 8b50 	vmov.s8	r8, d4\[2\]
[^>]*> ee58 8b50 	vmov.s8	r8, d8\[2\]
[^>]*> ee5e 8b50 	vmov.s8	r8, d14\[2\]
[^>]*> ee50 ab50 	vmov.s8	sl, d0\[2\]
[^>]*> ee52 ab50 	vmov.s8	sl, d2\[2\]
[^>]*> ee54 ab50 	vmov.s8	sl, d4\[2\]
[^>]*> ee58 ab50 	vmov.s8	sl, d8\[2\]
[^>]*> ee5e ab50 	vmov.s8	sl, d14\[2\]
[^>]*> ee50 cb50 	vmov.s8	ip, d0\[2\]
[^>]*> ee52 cb50 	vmov.s8	ip, d2\[2\]
[^>]*> ee54 cb50 	vmov.s8	ip, d4\[2\]
[^>]*> ee58 cb50 	vmov.s8	ip, d8\[2\]
[^>]*> ee5e cb50 	vmov.s8	ip, d14\[2\]
[^>]*> ee50 eb50 	vmov.s8	lr, d0\[2\]
[^>]*> ee52 eb50 	vmov.s8	lr, d2\[2\]
[^>]*> ee54 eb50 	vmov.s8	lr, d4\[2\]
[^>]*> ee58 eb50 	vmov.s8	lr, d8\[2\]
[^>]*> ee5e eb50 	vmov.s8	lr, d14\[2\]
[^>]*> ee70 0b10 	vmov.s8	r0, d0\[4\]
[^>]*> ee72 0b10 	vmov.s8	r0, d2\[4\]
[^>]*> ee74 0b10 	vmov.s8	r0, d4\[4\]
[^>]*> ee78 0b10 	vmov.s8	r0, d8\[4\]
[^>]*> ee7e 0b10 	vmov.s8	r0, d14\[4\]
[^>]*> ee70 1b10 	vmov.s8	r1, d0\[4\]
[^>]*> ee72 1b10 	vmov.s8	r1, d2\[4\]
[^>]*> ee74 1b10 	vmov.s8	r1, d4\[4\]
[^>]*> ee78 1b10 	vmov.s8	r1, d8\[4\]
[^>]*> ee7e 1b10 	vmov.s8	r1, d14\[4\]
[^>]*> ee70 2b10 	vmov.s8	r2, d0\[4\]
[^>]*> ee72 2b10 	vmov.s8	r2, d2\[4\]
[^>]*> ee74 2b10 	vmov.s8	r2, d4\[4\]
[^>]*> ee78 2b10 	vmov.s8	r2, d8\[4\]
[^>]*> ee7e 2b10 	vmov.s8	r2, d14\[4\]
[^>]*> ee70 4b10 	vmov.s8	r4, d0\[4\]
[^>]*> ee72 4b10 	vmov.s8	r4, d2\[4\]
[^>]*> ee74 4b10 	vmov.s8	r4, d4\[4\]
[^>]*> ee78 4b10 	vmov.s8	r4, d8\[4\]
[^>]*> ee7e 4b10 	vmov.s8	r4, d14\[4\]
[^>]*> ee70 7b10 	vmov.s8	r7, d0\[4\]
[^>]*> ee72 7b10 	vmov.s8	r7, d2\[4\]
[^>]*> ee74 7b10 	vmov.s8	r7, d4\[4\]
[^>]*> ee78 7b10 	vmov.s8	r7, d8\[4\]
[^>]*> ee7e 7b10 	vmov.s8	r7, d14\[4\]
[^>]*> ee70 8b10 	vmov.s8	r8, d0\[4\]
[^>]*> ee72 8b10 	vmov.s8	r8, d2\[4\]
[^>]*> ee74 8b10 	vmov.s8	r8, d4\[4\]
[^>]*> ee78 8b10 	vmov.s8	r8, d8\[4\]
[^>]*> ee7e 8b10 	vmov.s8	r8, d14\[4\]
[^>]*> ee70 ab10 	vmov.s8	sl, d0\[4\]
[^>]*> ee72 ab10 	vmov.s8	sl, d2\[4\]
[^>]*> ee74 ab10 	vmov.s8	sl, d4\[4\]
[^>]*> ee78 ab10 	vmov.s8	sl, d8\[4\]
[^>]*> ee7e ab10 	vmov.s8	sl, d14\[4\]
[^>]*> ee70 cb10 	vmov.s8	ip, d0\[4\]
[^>]*> ee72 cb10 	vmov.s8	ip, d2\[4\]
[^>]*> ee74 cb10 	vmov.s8	ip, d4\[4\]
[^>]*> ee78 cb10 	vmov.s8	ip, d8\[4\]
[^>]*> ee7e cb10 	vmov.s8	ip, d14\[4\]
[^>]*> ee70 eb10 	vmov.s8	lr, d0\[4\]
[^>]*> ee72 eb10 	vmov.s8	lr, d2\[4\]
[^>]*> ee74 eb10 	vmov.s8	lr, d4\[4\]
[^>]*> ee78 eb10 	vmov.s8	lr, d8\[4\]
[^>]*> ee7e eb10 	vmov.s8	lr, d14\[4\]
[^>]*> ee51 0b10 	vmov.s8	r0, d1\[0\]
[^>]*> ee53 0b10 	vmov.s8	r0, d3\[0\]
[^>]*> ee55 0b10 	vmov.s8	r0, d5\[0\]
[^>]*> ee59 0b10 	vmov.s8	r0, d9\[0\]
[^>]*> ee5f 0b10 	vmov.s8	r0, d15\[0\]
[^>]*> ee51 1b10 	vmov.s8	r1, d1\[0\]
[^>]*> ee53 1b10 	vmov.s8	r1, d3\[0\]
[^>]*> ee55 1b10 	vmov.s8	r1, d5\[0\]
[^>]*> ee59 1b10 	vmov.s8	r1, d9\[0\]
[^>]*> ee5f 1b10 	vmov.s8	r1, d15\[0\]
[^>]*> ee51 2b10 	vmov.s8	r2, d1\[0\]
[^>]*> ee53 2b10 	vmov.s8	r2, d3\[0\]
[^>]*> ee55 2b10 	vmov.s8	r2, d5\[0\]
[^>]*> ee59 2b10 	vmov.s8	r2, d9\[0\]
[^>]*> ee5f 2b10 	vmov.s8	r2, d15\[0\]
[^>]*> ee51 4b10 	vmov.s8	r4, d1\[0\]
[^>]*> ee53 4b10 	vmov.s8	r4, d3\[0\]
[^>]*> ee55 4b10 	vmov.s8	r4, d5\[0\]
[^>]*> ee59 4b10 	vmov.s8	r4, d9\[0\]
[^>]*> ee5f 4b10 	vmov.s8	r4, d15\[0\]
[^>]*> ee51 7b10 	vmov.s8	r7, d1\[0\]
[^>]*> ee53 7b10 	vmov.s8	r7, d3\[0\]
[^>]*> ee55 7b10 	vmov.s8	r7, d5\[0\]
[^>]*> ee59 7b10 	vmov.s8	r7, d9\[0\]
[^>]*> ee5f 7b10 	vmov.s8	r7, d15\[0\]
[^>]*> ee51 8b10 	vmov.s8	r8, d1\[0\]
[^>]*> ee53 8b10 	vmov.s8	r8, d3\[0\]
[^>]*> ee55 8b10 	vmov.s8	r8, d5\[0\]
[^>]*> ee59 8b10 	vmov.s8	r8, d9\[0\]
[^>]*> ee5f 8b10 	vmov.s8	r8, d15\[0\]
[^>]*> ee51 ab10 	vmov.s8	sl, d1\[0\]
[^>]*> ee53 ab10 	vmov.s8	sl, d3\[0\]
[^>]*> ee55 ab10 	vmov.s8	sl, d5\[0\]
[^>]*> ee59 ab10 	vmov.s8	sl, d9\[0\]
[^>]*> ee5f ab10 	vmov.s8	sl, d15\[0\]
[^>]*> ee51 cb10 	vmov.s8	ip, d1\[0\]
[^>]*> ee53 cb10 	vmov.s8	ip, d3\[0\]
[^>]*> ee55 cb10 	vmov.s8	ip, d5\[0\]
[^>]*> ee59 cb10 	vmov.s8	ip, d9\[0\]
[^>]*> ee5f cb10 	vmov.s8	ip, d15\[0\]
[^>]*> ee51 eb10 	vmov.s8	lr, d1\[0\]
[^>]*> ee53 eb10 	vmov.s8	lr, d3\[0\]
[^>]*> ee55 eb10 	vmov.s8	lr, d5\[0\]
[^>]*> ee59 eb10 	vmov.s8	lr, d9\[0\]
[^>]*> ee5f eb10 	vmov.s8	lr, d15\[0\]
[^>]*> ee71 0b70 	vmov.s8	r0, d1\[7\]
[^>]*> ee73 0b70 	vmov.s8	r0, d3\[7\]
[^>]*> ee75 0b70 	vmov.s8	r0, d5\[7\]
[^>]*> ee79 0b70 	vmov.s8	r0, d9\[7\]
[^>]*> ee7f 0b70 	vmov.s8	r0, d15\[7\]
[^>]*> ee71 1b70 	vmov.s8	r1, d1\[7\]
[^>]*> ee73 1b70 	vmov.s8	r1, d3\[7\]
[^>]*> ee75 1b70 	vmov.s8	r1, d5\[7\]
[^>]*> ee79 1b70 	vmov.s8	r1, d9\[7\]
[^>]*> ee7f 1b70 	vmov.s8	r1, d15\[7\]
[^>]*> ee71 2b70 	vmov.s8	r2, d1\[7\]
[^>]*> ee73 2b70 	vmov.s8	r2, d3\[7\]
[^>]*> ee75 2b70 	vmov.s8	r2, d5\[7\]
[^>]*> ee79 2b70 	vmov.s8	r2, d9\[7\]
[^>]*> ee7f 2b70 	vmov.s8	r2, d15\[7\]
[^>]*> ee71 4b70 	vmov.s8	r4, d1\[7\]
[^>]*> ee73 4b70 	vmov.s8	r4, d3\[7\]
[^>]*> ee75 4b70 	vmov.s8	r4, d5\[7\]
[^>]*> ee79 4b70 	vmov.s8	r4, d9\[7\]
[^>]*> ee7f 4b70 	vmov.s8	r4, d15\[7\]
[^>]*> ee71 7b70 	vmov.s8	r7, d1\[7\]
[^>]*> ee73 7b70 	vmov.s8	r7, d3\[7\]
[^>]*> ee75 7b70 	vmov.s8	r7, d5\[7\]
[^>]*> ee79 7b70 	vmov.s8	r7, d9\[7\]
[^>]*> ee7f 7b70 	vmov.s8	r7, d15\[7\]
[^>]*> ee71 8b70 	vmov.s8	r8, d1\[7\]
[^>]*> ee73 8b70 	vmov.s8	r8, d3\[7\]
[^>]*> ee75 8b70 	vmov.s8	r8, d5\[7\]
[^>]*> ee79 8b70 	vmov.s8	r8, d9\[7\]
[^>]*> ee7f 8b70 	vmov.s8	r8, d15\[7\]
[^>]*> ee71 ab70 	vmov.s8	sl, d1\[7\]
[^>]*> ee73 ab70 	vmov.s8	sl, d3\[7\]
[^>]*> ee75 ab70 	vmov.s8	sl, d5\[7\]
[^>]*> ee79 ab70 	vmov.s8	sl, d9\[7\]
[^>]*> ee7f ab70 	vmov.s8	sl, d15\[7\]
[^>]*> ee71 cb70 	vmov.s8	ip, d1\[7\]
[^>]*> ee73 cb70 	vmov.s8	ip, d3\[7\]
[^>]*> ee75 cb70 	vmov.s8	ip, d5\[7\]
[^>]*> ee79 cb70 	vmov.s8	ip, d9\[7\]
[^>]*> ee7f cb70 	vmov.s8	ip, d15\[7\]
[^>]*> ee71 eb70 	vmov.s8	lr, d1\[7\]
[^>]*> ee73 eb70 	vmov.s8	lr, d3\[7\]
[^>]*> ee75 eb70 	vmov.s8	lr, d5\[7\]
[^>]*> ee79 eb70 	vmov.s8	lr, d9\[7\]
[^>]*> ee7f eb70 	vmov.s8	lr, d15\[7\]
[^>]*> ee71 0b30 	vmov.s8	r0, d1\[5\]
[^>]*> ee73 0b30 	vmov.s8	r0, d3\[5\]
[^>]*> ee75 0b30 	vmov.s8	r0, d5\[5\]
[^>]*> ee79 0b30 	vmov.s8	r0, d9\[5\]
[^>]*> ee7f 0b30 	vmov.s8	r0, d15\[5\]
[^>]*> ee71 1b30 	vmov.s8	r1, d1\[5\]
[^>]*> ee73 1b30 	vmov.s8	r1, d3\[5\]
[^>]*> ee75 1b30 	vmov.s8	r1, d5\[5\]
[^>]*> ee79 1b30 	vmov.s8	r1, d9\[5\]
[^>]*> ee7f 1b30 	vmov.s8	r1, d15\[5\]
[^>]*> ee71 2b30 	vmov.s8	r2, d1\[5\]
[^>]*> ee73 2b30 	vmov.s8	r2, d3\[5\]
[^>]*> ee75 2b30 	vmov.s8	r2, d5\[5\]
[^>]*> ee79 2b30 	vmov.s8	r2, d9\[5\]
[^>]*> ee7f 2b30 	vmov.s8	r2, d15\[5\]
[^>]*> ee71 4b30 	vmov.s8	r4, d1\[5\]
[^>]*> ee73 4b30 	vmov.s8	r4, d3\[5\]
[^>]*> ee75 4b30 	vmov.s8	r4, d5\[5\]
[^>]*> ee79 4b30 	vmov.s8	r4, d9\[5\]
[^>]*> ee7f 4b30 	vmov.s8	r4, d15\[5\]
[^>]*> ee71 7b30 	vmov.s8	r7, d1\[5\]
[^>]*> ee73 7b30 	vmov.s8	r7, d3\[5\]
[^>]*> ee75 7b30 	vmov.s8	r7, d5\[5\]
[^>]*> ee79 7b30 	vmov.s8	r7, d9\[5\]
[^>]*> ee7f 7b30 	vmov.s8	r7, d15\[5\]
[^>]*> ee71 8b30 	vmov.s8	r8, d1\[5\]
[^>]*> ee73 8b30 	vmov.s8	r8, d3\[5\]
[^>]*> ee75 8b30 	vmov.s8	r8, d5\[5\]
[^>]*> ee79 8b30 	vmov.s8	r8, d9\[5\]
[^>]*> ee7f 8b30 	vmov.s8	r8, d15\[5\]
[^>]*> ee71 ab30 	vmov.s8	sl, d1\[5\]
[^>]*> ee73 ab30 	vmov.s8	sl, d3\[5\]
[^>]*> ee75 ab30 	vmov.s8	sl, d5\[5\]
[^>]*> ee79 ab30 	vmov.s8	sl, d9\[5\]
[^>]*> ee7f ab30 	vmov.s8	sl, d15\[5\]
[^>]*> ee71 cb30 	vmov.s8	ip, d1\[5\]
[^>]*> ee73 cb30 	vmov.s8	ip, d3\[5\]
[^>]*> ee75 cb30 	vmov.s8	ip, d5\[5\]
[^>]*> ee79 cb30 	vmov.s8	ip, d9\[5\]
[^>]*> ee7f cb30 	vmov.s8	ip, d15\[5\]
[^>]*> ee71 eb30 	vmov.s8	lr, d1\[5\]
[^>]*> ee73 eb30 	vmov.s8	lr, d3\[5\]
[^>]*> ee75 eb30 	vmov.s8	lr, d5\[5\]
[^>]*> ee79 eb30 	vmov.s8	lr, d9\[5\]
[^>]*> ee7f eb30 	vmov.s8	lr, d15\[5\]
[^>]*> ee70 0b50 	vmov.s8	r0, d0\[6\]
[^>]*> ee72 0b50 	vmov.s8	r0, d2\[6\]
[^>]*> ee74 0b50 	vmov.s8	r0, d4\[6\]
[^>]*> ee78 0b50 	vmov.s8	r0, d8\[6\]
[^>]*> ee7e 0b50 	vmov.s8	r0, d14\[6\]
[^>]*> ee70 1b50 	vmov.s8	r1, d0\[6\]
[^>]*> ee72 1b50 	vmov.s8	r1, d2\[6\]
[^>]*> ee74 1b50 	vmov.s8	r1, d4\[6\]
[^>]*> ee78 1b50 	vmov.s8	r1, d8\[6\]
[^>]*> ee7e 1b50 	vmov.s8	r1, d14\[6\]
[^>]*> ee70 2b50 	vmov.s8	r2, d0\[6\]
[^>]*> ee72 2b50 	vmov.s8	r2, d2\[6\]
[^>]*> ee74 2b50 	vmov.s8	r2, d4\[6\]
[^>]*> ee78 2b50 	vmov.s8	r2, d8\[6\]
[^>]*> ee7e 2b50 	vmov.s8	r2, d14\[6\]
[^>]*> ee70 4b50 	vmov.s8	r4, d0\[6\]
[^>]*> ee72 4b50 	vmov.s8	r4, d2\[6\]
[^>]*> ee74 4b50 	vmov.s8	r4, d4\[6\]
[^>]*> ee78 4b50 	vmov.s8	r4, d8\[6\]
[^>]*> ee7e 4b50 	vmov.s8	r4, d14\[6\]
[^>]*> ee70 7b50 	vmov.s8	r7, d0\[6\]
[^>]*> ee72 7b50 	vmov.s8	r7, d2\[6\]
[^>]*> ee74 7b50 	vmov.s8	r7, d4\[6\]
[^>]*> ee78 7b50 	vmov.s8	r7, d8\[6\]
[^>]*> ee7e 7b50 	vmov.s8	r7, d14\[6\]
[^>]*> ee70 8b50 	vmov.s8	r8, d0\[6\]
[^>]*> ee72 8b50 	vmov.s8	r8, d2\[6\]
[^>]*> ee74 8b50 	vmov.s8	r8, d4\[6\]
[^>]*> ee78 8b50 	vmov.s8	r8, d8\[6\]
[^>]*> ee7e 8b50 	vmov.s8	r8, d14\[6\]
[^>]*> ee70 ab50 	vmov.s8	sl, d0\[6\]
[^>]*> ee72 ab50 	vmov.s8	sl, d2\[6\]
[^>]*> ee74 ab50 	vmov.s8	sl, d4\[6\]
[^>]*> ee78 ab50 	vmov.s8	sl, d8\[6\]
[^>]*> ee7e ab50 	vmov.s8	sl, d14\[6\]
[^>]*> ee70 cb50 	vmov.s8	ip, d0\[6\]
[^>]*> ee72 cb50 	vmov.s8	ip, d2\[6\]
[^>]*> ee74 cb50 	vmov.s8	ip, d4\[6\]
[^>]*> ee78 cb50 	vmov.s8	ip, d8\[6\]
[^>]*> ee7e cb50 	vmov.s8	ip, d14\[6\]
[^>]*> ee70 eb50 	vmov.s8	lr, d0\[6\]
[^>]*> ee72 eb50 	vmov.s8	lr, d2\[6\]
[^>]*> ee74 eb50 	vmov.s8	lr, d4\[6\]
[^>]*> ee78 eb50 	vmov.s8	lr, d8\[6\]
[^>]*> ee7e eb50 	vmov.s8	lr, d14\[6\]
[^>]*> ee90 0b30 	vmov.u16	r0, d0\[0\]
[^>]*> ee92 0b30 	vmov.u16	r0, d2\[0\]
[^>]*> ee94 0b30 	vmov.u16	r0, d4\[0\]
[^>]*> ee98 0b30 	vmov.u16	r0, d8\[0\]
[^>]*> ee9e 0b30 	vmov.u16	r0, d14\[0\]
[^>]*> ee90 1b30 	vmov.u16	r1, d0\[0\]
[^>]*> ee92 1b30 	vmov.u16	r1, d2\[0\]
[^>]*> ee94 1b30 	vmov.u16	r1, d4\[0\]
[^>]*> ee98 1b30 	vmov.u16	r1, d8\[0\]
[^>]*> ee9e 1b30 	vmov.u16	r1, d14\[0\]
[^>]*> ee90 2b30 	vmov.u16	r2, d0\[0\]
[^>]*> ee92 2b30 	vmov.u16	r2, d2\[0\]
[^>]*> ee94 2b30 	vmov.u16	r2, d4\[0\]
[^>]*> ee98 2b30 	vmov.u16	r2, d8\[0\]
[^>]*> ee9e 2b30 	vmov.u16	r2, d14\[0\]
[^>]*> ee90 4b30 	vmov.u16	r4, d0\[0\]
[^>]*> ee92 4b30 	vmov.u16	r4, d2\[0\]
[^>]*> ee94 4b30 	vmov.u16	r4, d4\[0\]
[^>]*> ee98 4b30 	vmov.u16	r4, d8\[0\]
[^>]*> ee9e 4b30 	vmov.u16	r4, d14\[0\]
[^>]*> ee90 7b30 	vmov.u16	r7, d0\[0\]
[^>]*> ee92 7b30 	vmov.u16	r7, d2\[0\]
[^>]*> ee94 7b30 	vmov.u16	r7, d4\[0\]
[^>]*> ee98 7b30 	vmov.u16	r7, d8\[0\]
[^>]*> ee9e 7b30 	vmov.u16	r7, d14\[0\]
[^>]*> ee90 8b30 	vmov.u16	r8, d0\[0\]
[^>]*> ee92 8b30 	vmov.u16	r8, d2\[0\]
[^>]*> ee94 8b30 	vmov.u16	r8, d4\[0\]
[^>]*> ee98 8b30 	vmov.u16	r8, d8\[0\]
[^>]*> ee9e 8b30 	vmov.u16	r8, d14\[0\]
[^>]*> ee90 ab30 	vmov.u16	sl, d0\[0\]
[^>]*> ee92 ab30 	vmov.u16	sl, d2\[0\]
[^>]*> ee94 ab30 	vmov.u16	sl, d4\[0\]
[^>]*> ee98 ab30 	vmov.u16	sl, d8\[0\]
[^>]*> ee9e ab30 	vmov.u16	sl, d14\[0\]
[^>]*> ee90 cb30 	vmov.u16	ip, d0\[0\]
[^>]*> ee92 cb30 	vmov.u16	ip, d2\[0\]
[^>]*> ee94 cb30 	vmov.u16	ip, d4\[0\]
[^>]*> ee98 cb30 	vmov.u16	ip, d8\[0\]
[^>]*> ee9e cb30 	vmov.u16	ip, d14\[0\]
[^>]*> ee90 eb30 	vmov.u16	lr, d0\[0\]
[^>]*> ee92 eb30 	vmov.u16	lr, d2\[0\]
[^>]*> ee94 eb30 	vmov.u16	lr, d4\[0\]
[^>]*> ee98 eb30 	vmov.u16	lr, d8\[0\]
[^>]*> ee9e eb30 	vmov.u16	lr, d14\[0\]
[^>]*> ee90 0b70 	vmov.u16	r0, d0\[1\]
[^>]*> ee92 0b70 	vmov.u16	r0, d2\[1\]
[^>]*> ee94 0b70 	vmov.u16	r0, d4\[1\]
[^>]*> ee98 0b70 	vmov.u16	r0, d8\[1\]
[^>]*> ee9e 0b70 	vmov.u16	r0, d14\[1\]
[^>]*> ee90 1b70 	vmov.u16	r1, d0\[1\]
[^>]*> ee92 1b70 	vmov.u16	r1, d2\[1\]
[^>]*> ee94 1b70 	vmov.u16	r1, d4\[1\]
[^>]*> ee98 1b70 	vmov.u16	r1, d8\[1\]
[^>]*> ee9e 1b70 	vmov.u16	r1, d14\[1\]
[^>]*> ee90 2b70 	vmov.u16	r2, d0\[1\]
[^>]*> ee92 2b70 	vmov.u16	r2, d2\[1\]
[^>]*> ee94 2b70 	vmov.u16	r2, d4\[1\]
[^>]*> ee98 2b70 	vmov.u16	r2, d8\[1\]
[^>]*> ee9e 2b70 	vmov.u16	r2, d14\[1\]
[^>]*> ee90 4b70 	vmov.u16	r4, d0\[1\]
[^>]*> ee92 4b70 	vmov.u16	r4, d2\[1\]
[^>]*> ee94 4b70 	vmov.u16	r4, d4\[1\]
[^>]*> ee98 4b70 	vmov.u16	r4, d8\[1\]
[^>]*> ee9e 4b70 	vmov.u16	r4, d14\[1\]
[^>]*> ee90 7b70 	vmov.u16	r7, d0\[1\]
[^>]*> ee92 7b70 	vmov.u16	r7, d2\[1\]
[^>]*> ee94 7b70 	vmov.u16	r7, d4\[1\]
[^>]*> ee98 7b70 	vmov.u16	r7, d8\[1\]
[^>]*> ee9e 7b70 	vmov.u16	r7, d14\[1\]
[^>]*> ee90 8b70 	vmov.u16	r8, d0\[1\]
[^>]*> ee92 8b70 	vmov.u16	r8, d2\[1\]
[^>]*> ee94 8b70 	vmov.u16	r8, d4\[1\]
[^>]*> ee98 8b70 	vmov.u16	r8, d8\[1\]
[^>]*> ee9e 8b70 	vmov.u16	r8, d14\[1\]
[^>]*> ee90 ab70 	vmov.u16	sl, d0\[1\]
[^>]*> ee92 ab70 	vmov.u16	sl, d2\[1\]
[^>]*> ee94 ab70 	vmov.u16	sl, d4\[1\]
[^>]*> ee98 ab70 	vmov.u16	sl, d8\[1\]
[^>]*> ee9e ab70 	vmov.u16	sl, d14\[1\]
[^>]*> ee90 cb70 	vmov.u16	ip, d0\[1\]
[^>]*> ee92 cb70 	vmov.u16	ip, d2\[1\]
[^>]*> ee94 cb70 	vmov.u16	ip, d4\[1\]
[^>]*> ee98 cb70 	vmov.u16	ip, d8\[1\]
[^>]*> ee9e cb70 	vmov.u16	ip, d14\[1\]
[^>]*> ee90 eb70 	vmov.u16	lr, d0\[1\]
[^>]*> ee92 eb70 	vmov.u16	lr, d2\[1\]
[^>]*> ee94 eb70 	vmov.u16	lr, d4\[1\]
[^>]*> ee98 eb70 	vmov.u16	lr, d8\[1\]
[^>]*> ee9e eb70 	vmov.u16	lr, d14\[1\]
[^>]*> eeb0 0b30 	vmov.u16	r0, d0\[2\]
[^>]*> eeb2 0b30 	vmov.u16	r0, d2\[2\]
[^>]*> eeb4 0b30 	vmov.u16	r0, d4\[2\]
[^>]*> eeb8 0b30 	vmov.u16	r0, d8\[2\]
[^>]*> eebe 0b30 	vmov.u16	r0, d14\[2\]
[^>]*> eeb0 1b30 	vmov.u16	r1, d0\[2\]
[^>]*> eeb2 1b30 	vmov.u16	r1, d2\[2\]
[^>]*> eeb4 1b30 	vmov.u16	r1, d4\[2\]
[^>]*> eeb8 1b30 	vmov.u16	r1, d8\[2\]
[^>]*> eebe 1b30 	vmov.u16	r1, d14\[2\]
[^>]*> eeb0 2b30 	vmov.u16	r2, d0\[2\]
[^>]*> eeb2 2b30 	vmov.u16	r2, d2\[2\]
[^>]*> eeb4 2b30 	vmov.u16	r2, d4\[2\]
[^>]*> eeb8 2b30 	vmov.u16	r2, d8\[2\]
[^>]*> eebe 2b30 	vmov.u16	r2, d14\[2\]
[^>]*> eeb0 4b30 	vmov.u16	r4, d0\[2\]
[^>]*> eeb2 4b30 	vmov.u16	r4, d2\[2\]
[^>]*> eeb4 4b30 	vmov.u16	r4, d4\[2\]
[^>]*> eeb8 4b30 	vmov.u16	r4, d8\[2\]
[^>]*> eebe 4b30 	vmov.u16	r4, d14\[2\]
[^>]*> eeb0 7b30 	vmov.u16	r7, d0\[2\]
[^>]*> eeb2 7b30 	vmov.u16	r7, d2\[2\]
[^>]*> eeb4 7b30 	vmov.u16	r7, d4\[2\]
[^>]*> eeb8 7b30 	vmov.u16	r7, d8\[2\]
[^>]*> eebe 7b30 	vmov.u16	r7, d14\[2\]
[^>]*> eeb0 8b30 	vmov.u16	r8, d0\[2\]
[^>]*> eeb2 8b30 	vmov.u16	r8, d2\[2\]
[^>]*> eeb4 8b30 	vmov.u16	r8, d4\[2\]
[^>]*> eeb8 8b30 	vmov.u16	r8, d8\[2\]
[^>]*> eebe 8b30 	vmov.u16	r8, d14\[2\]
[^>]*> eeb0 ab30 	vmov.u16	sl, d0\[2\]
[^>]*> eeb2 ab30 	vmov.u16	sl, d2\[2\]
[^>]*> eeb4 ab30 	vmov.u16	sl, d4\[2\]
[^>]*> eeb8 ab30 	vmov.u16	sl, d8\[2\]
[^>]*> eebe ab30 	vmov.u16	sl, d14\[2\]
[^>]*> eeb0 cb30 	vmov.u16	ip, d0\[2\]
[^>]*> eeb2 cb30 	vmov.u16	ip, d2\[2\]
[^>]*> eeb4 cb30 	vmov.u16	ip, d4\[2\]
[^>]*> eeb8 cb30 	vmov.u16	ip, d8\[2\]
[^>]*> eebe cb30 	vmov.u16	ip, d14\[2\]
[^>]*> eeb0 eb30 	vmov.u16	lr, d0\[2\]
[^>]*> eeb2 eb30 	vmov.u16	lr, d2\[2\]
[^>]*> eeb4 eb30 	vmov.u16	lr, d4\[2\]
[^>]*> eeb8 eb30 	vmov.u16	lr, d8\[2\]
[^>]*> eebe eb30 	vmov.u16	lr, d14\[2\]
[^>]*> ee91 0b30 	vmov.u16	r0, d1\[0\]
[^>]*> ee93 0b30 	vmov.u16	r0, d3\[0\]
[^>]*> ee95 0b30 	vmov.u16	r0, d5\[0\]
[^>]*> ee99 0b30 	vmov.u16	r0, d9\[0\]
[^>]*> ee9f 0b30 	vmov.u16	r0, d15\[0\]
[^>]*> ee91 1b30 	vmov.u16	r1, d1\[0\]
[^>]*> ee93 1b30 	vmov.u16	r1, d3\[0\]
[^>]*> ee95 1b30 	vmov.u16	r1, d5\[0\]
[^>]*> ee99 1b30 	vmov.u16	r1, d9\[0\]
[^>]*> ee9f 1b30 	vmov.u16	r1, d15\[0\]
[^>]*> ee91 2b30 	vmov.u16	r2, d1\[0\]
[^>]*> ee93 2b30 	vmov.u16	r2, d3\[0\]
[^>]*> ee95 2b30 	vmov.u16	r2, d5\[0\]
[^>]*> ee99 2b30 	vmov.u16	r2, d9\[0\]
[^>]*> ee9f 2b30 	vmov.u16	r2, d15\[0\]
[^>]*> ee91 4b30 	vmov.u16	r4, d1\[0\]
[^>]*> ee93 4b30 	vmov.u16	r4, d3\[0\]
[^>]*> ee95 4b30 	vmov.u16	r4, d5\[0\]
[^>]*> ee99 4b30 	vmov.u16	r4, d9\[0\]
[^>]*> ee9f 4b30 	vmov.u16	r4, d15\[0\]
[^>]*> ee91 7b30 	vmov.u16	r7, d1\[0\]
[^>]*> ee93 7b30 	vmov.u16	r7, d3\[0\]
[^>]*> ee95 7b30 	vmov.u16	r7, d5\[0\]
[^>]*> ee99 7b30 	vmov.u16	r7, d9\[0\]
[^>]*> ee9f 7b30 	vmov.u16	r7, d15\[0\]
[^>]*> ee91 8b30 	vmov.u16	r8, d1\[0\]
[^>]*> ee93 8b30 	vmov.u16	r8, d3\[0\]
[^>]*> ee95 8b30 	vmov.u16	r8, d5\[0\]
[^>]*> ee99 8b30 	vmov.u16	r8, d9\[0\]
[^>]*> ee9f 8b30 	vmov.u16	r8, d15\[0\]
[^>]*> ee91 ab30 	vmov.u16	sl, d1\[0\]
[^>]*> ee93 ab30 	vmov.u16	sl, d3\[0\]
[^>]*> ee95 ab30 	vmov.u16	sl, d5\[0\]
[^>]*> ee99 ab30 	vmov.u16	sl, d9\[0\]
[^>]*> ee9f ab30 	vmov.u16	sl, d15\[0\]
[^>]*> ee91 cb30 	vmov.u16	ip, d1\[0\]
[^>]*> ee93 cb30 	vmov.u16	ip, d3\[0\]
[^>]*> ee95 cb30 	vmov.u16	ip, d5\[0\]
[^>]*> ee99 cb30 	vmov.u16	ip, d9\[0\]
[^>]*> ee9f cb30 	vmov.u16	ip, d15\[0\]
[^>]*> ee91 eb30 	vmov.u16	lr, d1\[0\]
[^>]*> ee93 eb30 	vmov.u16	lr, d3\[0\]
[^>]*> ee95 eb30 	vmov.u16	lr, d5\[0\]
[^>]*> ee99 eb30 	vmov.u16	lr, d9\[0\]
[^>]*> ee9f eb30 	vmov.u16	lr, d15\[0\]
[^>]*> eeb1 0b70 	vmov.u16	r0, d1\[3\]
[^>]*> eeb3 0b70 	vmov.u16	r0, d3\[3\]
[^>]*> eeb5 0b70 	vmov.u16	r0, d5\[3\]
[^>]*> eeb9 0b70 	vmov.u16	r0, d9\[3\]
[^>]*> eebf 0b70 	vmov.u16	r0, d15\[3\]
[^>]*> eeb1 1b70 	vmov.u16	r1, d1\[3\]
[^>]*> eeb3 1b70 	vmov.u16	r1, d3\[3\]
[^>]*> eeb5 1b70 	vmov.u16	r1, d5\[3\]
[^>]*> eeb9 1b70 	vmov.u16	r1, d9\[3\]
[^>]*> eebf 1b70 	vmov.u16	r1, d15\[3\]
[^>]*> eeb1 2b70 	vmov.u16	r2, d1\[3\]
[^>]*> eeb3 2b70 	vmov.u16	r2, d3\[3\]
[^>]*> eeb5 2b70 	vmov.u16	r2, d5\[3\]
[^>]*> eeb9 2b70 	vmov.u16	r2, d9\[3\]
[^>]*> eebf 2b70 	vmov.u16	r2, d15\[3\]
[^>]*> eeb1 4b70 	vmov.u16	r4, d1\[3\]
[^>]*> eeb3 4b70 	vmov.u16	r4, d3\[3\]
[^>]*> eeb5 4b70 	vmov.u16	r4, d5\[3\]
[^>]*> eeb9 4b70 	vmov.u16	r4, d9\[3\]
[^>]*> eebf 4b70 	vmov.u16	r4, d15\[3\]
[^>]*> eeb1 7b70 	vmov.u16	r7, d1\[3\]
[^>]*> eeb3 7b70 	vmov.u16	r7, d3\[3\]
[^>]*> eeb5 7b70 	vmov.u16	r7, d5\[3\]
[^>]*> eeb9 7b70 	vmov.u16	r7, d9\[3\]
[^>]*> eebf 7b70 	vmov.u16	r7, d15\[3\]
[^>]*> eeb1 8b70 	vmov.u16	r8, d1\[3\]
[^>]*> eeb3 8b70 	vmov.u16	r8, d3\[3\]
[^>]*> eeb5 8b70 	vmov.u16	r8, d5\[3\]
[^>]*> eeb9 8b70 	vmov.u16	r8, d9\[3\]
[^>]*> eebf 8b70 	vmov.u16	r8, d15\[3\]
[^>]*> eeb1 ab70 	vmov.u16	sl, d1\[3\]
[^>]*> eeb3 ab70 	vmov.u16	sl, d3\[3\]
[^>]*> eeb5 ab70 	vmov.u16	sl, d5\[3\]
[^>]*> eeb9 ab70 	vmov.u16	sl, d9\[3\]
[^>]*> eebf ab70 	vmov.u16	sl, d15\[3\]
[^>]*> eeb1 cb70 	vmov.u16	ip, d1\[3\]
[^>]*> eeb3 cb70 	vmov.u16	ip, d3\[3\]
[^>]*> eeb5 cb70 	vmov.u16	ip, d5\[3\]
[^>]*> eeb9 cb70 	vmov.u16	ip, d9\[3\]
[^>]*> eebf cb70 	vmov.u16	ip, d15\[3\]
[^>]*> eeb1 eb70 	vmov.u16	lr, d1\[3\]
[^>]*> eeb3 eb70 	vmov.u16	lr, d3\[3\]
[^>]*> eeb5 eb70 	vmov.u16	lr, d5\[3\]
[^>]*> eeb9 eb70 	vmov.u16	lr, d9\[3\]
[^>]*> eebf eb70 	vmov.u16	lr, d15\[3\]
[^>]*> ee10 0b30 	vmov.s16	r0, d0\[0\]
[^>]*> ee12 0b30 	vmov.s16	r0, d2\[0\]
[^>]*> ee14 0b30 	vmov.s16	r0, d4\[0\]
[^>]*> ee18 0b30 	vmov.s16	r0, d8\[0\]
[^>]*> ee1e 0b30 	vmov.s16	r0, d14\[0\]
[^>]*> ee10 1b30 	vmov.s16	r1, d0\[0\]
[^>]*> ee12 1b30 	vmov.s16	r1, d2\[0\]
[^>]*> ee14 1b30 	vmov.s16	r1, d4\[0\]
[^>]*> ee18 1b30 	vmov.s16	r1, d8\[0\]
[^>]*> ee1e 1b30 	vmov.s16	r1, d14\[0\]
[^>]*> ee10 2b30 	vmov.s16	r2, d0\[0\]
[^>]*> ee12 2b30 	vmov.s16	r2, d2\[0\]
[^>]*> ee14 2b30 	vmov.s16	r2, d4\[0\]
[^>]*> ee18 2b30 	vmov.s16	r2, d8\[0\]
[^>]*> ee1e 2b30 	vmov.s16	r2, d14\[0\]
[^>]*> ee10 4b30 	vmov.s16	r4, d0\[0\]
[^>]*> ee12 4b30 	vmov.s16	r4, d2\[0\]
[^>]*> ee14 4b30 	vmov.s16	r4, d4\[0\]
[^>]*> ee18 4b30 	vmov.s16	r4, d8\[0\]
[^>]*> ee1e 4b30 	vmov.s16	r4, d14\[0\]
[^>]*> ee10 7b30 	vmov.s16	r7, d0\[0\]
[^>]*> ee12 7b30 	vmov.s16	r7, d2\[0\]
[^>]*> ee14 7b30 	vmov.s16	r7, d4\[0\]
[^>]*> ee18 7b30 	vmov.s16	r7, d8\[0\]
[^>]*> ee1e 7b30 	vmov.s16	r7, d14\[0\]
[^>]*> ee10 8b30 	vmov.s16	r8, d0\[0\]
[^>]*> ee12 8b30 	vmov.s16	r8, d2\[0\]
[^>]*> ee14 8b30 	vmov.s16	r8, d4\[0\]
[^>]*> ee18 8b30 	vmov.s16	r8, d8\[0\]
[^>]*> ee1e 8b30 	vmov.s16	r8, d14\[0\]
[^>]*> ee10 ab30 	vmov.s16	sl, d0\[0\]
[^>]*> ee12 ab30 	vmov.s16	sl, d2\[0\]
[^>]*> ee14 ab30 	vmov.s16	sl, d4\[0\]
[^>]*> ee18 ab30 	vmov.s16	sl, d8\[0\]
[^>]*> ee1e ab30 	vmov.s16	sl, d14\[0\]
[^>]*> ee10 cb30 	vmov.s16	ip, d0\[0\]
[^>]*> ee12 cb30 	vmov.s16	ip, d2\[0\]
[^>]*> ee14 cb30 	vmov.s16	ip, d4\[0\]
[^>]*> ee18 cb30 	vmov.s16	ip, d8\[0\]
[^>]*> ee1e cb30 	vmov.s16	ip, d14\[0\]
[^>]*> ee10 eb30 	vmov.s16	lr, d0\[0\]
[^>]*> ee12 eb30 	vmov.s16	lr, d2\[0\]
[^>]*> ee14 eb30 	vmov.s16	lr, d4\[0\]
[^>]*> ee18 eb30 	vmov.s16	lr, d8\[0\]
[^>]*> ee1e eb30 	vmov.s16	lr, d14\[0\]
[^>]*> ee10 0b70 	vmov.s16	r0, d0\[1\]
[^>]*> ee12 0b70 	vmov.s16	r0, d2\[1\]
[^>]*> ee14 0b70 	vmov.s16	r0, d4\[1\]
[^>]*> ee18 0b70 	vmov.s16	r0, d8\[1\]
[^>]*> ee1e 0b70 	vmov.s16	r0, d14\[1\]
[^>]*> ee10 1b70 	vmov.s16	r1, d0\[1\]
[^>]*> ee12 1b70 	vmov.s16	r1, d2\[1\]
[^>]*> ee14 1b70 	vmov.s16	r1, d4\[1\]
[^>]*> ee18 1b70 	vmov.s16	r1, d8\[1\]
[^>]*> ee1e 1b70 	vmov.s16	r1, d14\[1\]
[^>]*> ee10 2b70 	vmov.s16	r2, d0\[1\]
[^>]*> ee12 2b70 	vmov.s16	r2, d2\[1\]
[^>]*> ee14 2b70 	vmov.s16	r2, d4\[1\]
[^>]*> ee18 2b70 	vmov.s16	r2, d8\[1\]
[^>]*> ee1e 2b70 	vmov.s16	r2, d14\[1\]
[^>]*> ee10 4b70 	vmov.s16	r4, d0\[1\]
[^>]*> ee12 4b70 	vmov.s16	r4, d2\[1\]
[^>]*> ee14 4b70 	vmov.s16	r4, d4\[1\]
[^>]*> ee18 4b70 	vmov.s16	r4, d8\[1\]
[^>]*> ee1e 4b70 	vmov.s16	r4, d14\[1\]
[^>]*> ee10 7b70 	vmov.s16	r7, d0\[1\]
[^>]*> ee12 7b70 	vmov.s16	r7, d2\[1\]
[^>]*> ee14 7b70 	vmov.s16	r7, d4\[1\]
[^>]*> ee18 7b70 	vmov.s16	r7, d8\[1\]
[^>]*> ee1e 7b70 	vmov.s16	r7, d14\[1\]
[^>]*> ee10 8b70 	vmov.s16	r8, d0\[1\]
[^>]*> ee12 8b70 	vmov.s16	r8, d2\[1\]
[^>]*> ee14 8b70 	vmov.s16	r8, d4\[1\]
[^>]*> ee18 8b70 	vmov.s16	r8, d8\[1\]
[^>]*> ee1e 8b70 	vmov.s16	r8, d14\[1\]
[^>]*> ee10 ab70 	vmov.s16	sl, d0\[1\]
[^>]*> ee12 ab70 	vmov.s16	sl, d2\[1\]
[^>]*> ee14 ab70 	vmov.s16	sl, d4\[1\]
[^>]*> ee18 ab70 	vmov.s16	sl, d8\[1\]
[^>]*> ee1e ab70 	vmov.s16	sl, d14\[1\]
[^>]*> ee10 cb70 	vmov.s16	ip, d0\[1\]
[^>]*> ee12 cb70 	vmov.s16	ip, d2\[1\]
[^>]*> ee14 cb70 	vmov.s16	ip, d4\[1\]
[^>]*> ee18 cb70 	vmov.s16	ip, d8\[1\]
[^>]*> ee1e cb70 	vmov.s16	ip, d14\[1\]
[^>]*> ee10 eb70 	vmov.s16	lr, d0\[1\]
[^>]*> ee12 eb70 	vmov.s16	lr, d2\[1\]
[^>]*> ee14 eb70 	vmov.s16	lr, d4\[1\]
[^>]*> ee18 eb70 	vmov.s16	lr, d8\[1\]
[^>]*> ee1e eb70 	vmov.s16	lr, d14\[1\]
[^>]*> ee30 0b30 	vmov.s16	r0, d0\[2\]
[^>]*> ee32 0b30 	vmov.s16	r0, d2\[2\]
[^>]*> ee34 0b30 	vmov.s16	r0, d4\[2\]
[^>]*> ee38 0b30 	vmov.s16	r0, d8\[2\]
[^>]*> ee3e 0b30 	vmov.s16	r0, d14\[2\]
[^>]*> ee30 1b30 	vmov.s16	r1, d0\[2\]
[^>]*> ee32 1b30 	vmov.s16	r1, d2\[2\]
[^>]*> ee34 1b30 	vmov.s16	r1, d4\[2\]
[^>]*> ee38 1b30 	vmov.s16	r1, d8\[2\]
[^>]*> ee3e 1b30 	vmov.s16	r1, d14\[2\]
[^>]*> ee30 2b30 	vmov.s16	r2, d0\[2\]
[^>]*> ee32 2b30 	vmov.s16	r2, d2\[2\]
[^>]*> ee34 2b30 	vmov.s16	r2, d4\[2\]
[^>]*> ee38 2b30 	vmov.s16	r2, d8\[2\]
[^>]*> ee3e 2b30 	vmov.s16	r2, d14\[2\]
[^>]*> ee30 4b30 	vmov.s16	r4, d0\[2\]
[^>]*> ee32 4b30 	vmov.s16	r4, d2\[2\]
[^>]*> ee34 4b30 	vmov.s16	r4, d4\[2\]
[^>]*> ee38 4b30 	vmov.s16	r4, d8\[2\]
[^>]*> ee3e 4b30 	vmov.s16	r4, d14\[2\]
[^>]*> ee30 7b30 	vmov.s16	r7, d0\[2\]
[^>]*> ee32 7b30 	vmov.s16	r7, d2\[2\]
[^>]*> ee34 7b30 	vmov.s16	r7, d4\[2\]
[^>]*> ee38 7b30 	vmov.s16	r7, d8\[2\]
[^>]*> ee3e 7b30 	vmov.s16	r7, d14\[2\]
[^>]*> ee30 8b30 	vmov.s16	r8, d0\[2\]
[^>]*> ee32 8b30 	vmov.s16	r8, d2\[2\]
[^>]*> ee34 8b30 	vmov.s16	r8, d4\[2\]
[^>]*> ee38 8b30 	vmov.s16	r8, d8\[2\]
[^>]*> ee3e 8b30 	vmov.s16	r8, d14\[2\]
[^>]*> ee30 ab30 	vmov.s16	sl, d0\[2\]
[^>]*> ee32 ab30 	vmov.s16	sl, d2\[2\]
[^>]*> ee34 ab30 	vmov.s16	sl, d4\[2\]
[^>]*> ee38 ab30 	vmov.s16	sl, d8\[2\]
[^>]*> ee3e ab30 	vmov.s16	sl, d14\[2\]
[^>]*> ee30 cb30 	vmov.s16	ip, d0\[2\]
[^>]*> ee32 cb30 	vmov.s16	ip, d2\[2\]
[^>]*> ee34 cb30 	vmov.s16	ip, d4\[2\]
[^>]*> ee38 cb30 	vmov.s16	ip, d8\[2\]
[^>]*> ee3e cb30 	vmov.s16	ip, d14\[2\]
[^>]*> ee30 eb30 	vmov.s16	lr, d0\[2\]
[^>]*> ee32 eb30 	vmov.s16	lr, d2\[2\]
[^>]*> ee34 eb30 	vmov.s16	lr, d4\[2\]
[^>]*> ee38 eb30 	vmov.s16	lr, d8\[2\]
[^>]*> ee3e eb30 	vmov.s16	lr, d14\[2\]
[^>]*> ee11 0b30 	vmov.s16	r0, d1\[0\]
[^>]*> ee13 0b30 	vmov.s16	r0, d3\[0\]
[^>]*> ee15 0b30 	vmov.s16	r0, d5\[0\]
[^>]*> ee19 0b30 	vmov.s16	r0, d9\[0\]
[^>]*> ee1f 0b30 	vmov.s16	r0, d15\[0\]
[^>]*> ee11 1b30 	vmov.s16	r1, d1\[0\]
[^>]*> ee13 1b30 	vmov.s16	r1, d3\[0\]
[^>]*> ee15 1b30 	vmov.s16	r1, d5\[0\]
[^>]*> ee19 1b30 	vmov.s16	r1, d9\[0\]
[^>]*> ee1f 1b30 	vmov.s16	r1, d15\[0\]
[^>]*> ee11 2b30 	vmov.s16	r2, d1\[0\]
[^>]*> ee13 2b30 	vmov.s16	r2, d3\[0\]
[^>]*> ee15 2b30 	vmov.s16	r2, d5\[0\]
[^>]*> ee19 2b30 	vmov.s16	r2, d9\[0\]
[^>]*> ee1f 2b30 	vmov.s16	r2, d15\[0\]
[^>]*> ee11 4b30 	vmov.s16	r4, d1\[0\]
[^>]*> ee13 4b30 	vmov.s16	r4, d3\[0\]
[^>]*> ee15 4b30 	vmov.s16	r4, d5\[0\]
[^>]*> ee19 4b30 	vmov.s16	r4, d9\[0\]
[^>]*> ee1f 4b30 	vmov.s16	r4, d15\[0\]
[^>]*> ee11 7b30 	vmov.s16	r7, d1\[0\]
[^>]*> ee13 7b30 	vmov.s16	r7, d3\[0\]
[^>]*> ee15 7b30 	vmov.s16	r7, d5\[0\]
[^>]*> ee19 7b30 	vmov.s16	r7, d9\[0\]
[^>]*> ee1f 7b30 	vmov.s16	r7, d15\[0\]
[^>]*> ee11 8b30 	vmov.s16	r8, d1\[0\]
[^>]*> ee13 8b30 	vmov.s16	r8, d3\[0\]
[^>]*> ee15 8b30 	vmov.s16	r8, d5\[0\]
[^>]*> ee19 8b30 	vmov.s16	r8, d9\[0\]
[^>]*> ee1f 8b30 	vmov.s16	r8, d15\[0\]
[^>]*> ee11 ab30 	vmov.s16	sl, d1\[0\]
[^>]*> ee13 ab30 	vmov.s16	sl, d3\[0\]
[^>]*> ee15 ab30 	vmov.s16	sl, d5\[0\]
[^>]*> ee19 ab30 	vmov.s16	sl, d9\[0\]
[^>]*> ee1f ab30 	vmov.s16	sl, d15\[0\]
[^>]*> ee11 cb30 	vmov.s16	ip, d1\[0\]
[^>]*> ee13 cb30 	vmov.s16	ip, d3\[0\]
[^>]*> ee15 cb30 	vmov.s16	ip, d5\[0\]
[^>]*> ee19 cb30 	vmov.s16	ip, d9\[0\]
[^>]*> ee1f cb30 	vmov.s16	ip, d15\[0\]
[^>]*> ee11 eb30 	vmov.s16	lr, d1\[0\]
[^>]*> ee13 eb30 	vmov.s16	lr, d3\[0\]
[^>]*> ee15 eb30 	vmov.s16	lr, d5\[0\]
[^>]*> ee19 eb30 	vmov.s16	lr, d9\[0\]
[^>]*> ee1f eb30 	vmov.s16	lr, d15\[0\]
[^>]*> ee31 0b70 	vmov.s16	r0, d1\[3\]
[^>]*> ee33 0b70 	vmov.s16	r0, d3\[3\]
[^>]*> ee35 0b70 	vmov.s16	r0, d5\[3\]
[^>]*> ee39 0b70 	vmov.s16	r0, d9\[3\]
[^>]*> ee3f 0b70 	vmov.s16	r0, d15\[3\]
[^>]*> ee31 1b70 	vmov.s16	r1, d1\[3\]
[^>]*> ee33 1b70 	vmov.s16	r1, d3\[3\]
[^>]*> ee35 1b70 	vmov.s16	r1, d5\[3\]
[^>]*> ee39 1b70 	vmov.s16	r1, d9\[3\]
[^>]*> ee3f 1b70 	vmov.s16	r1, d15\[3\]
[^>]*> ee31 2b70 	vmov.s16	r2, d1\[3\]
[^>]*> ee33 2b70 	vmov.s16	r2, d3\[3\]
[^>]*> ee35 2b70 	vmov.s16	r2, d5\[3\]
[^>]*> ee39 2b70 	vmov.s16	r2, d9\[3\]
[^>]*> ee3f 2b70 	vmov.s16	r2, d15\[3\]
[^>]*> ee31 4b70 	vmov.s16	r4, d1\[3\]
[^>]*> ee33 4b70 	vmov.s16	r4, d3\[3\]
[^>]*> ee35 4b70 	vmov.s16	r4, d5\[3\]
[^>]*> ee39 4b70 	vmov.s16	r4, d9\[3\]
[^>]*> ee3f 4b70 	vmov.s16	r4, d15\[3\]
[^>]*> ee31 7b70 	vmov.s16	r7, d1\[3\]
[^>]*> ee33 7b70 	vmov.s16	r7, d3\[3\]
[^>]*> ee35 7b70 	vmov.s16	r7, d5\[3\]
[^>]*> ee39 7b70 	vmov.s16	r7, d9\[3\]
[^>]*> ee3f 7b70 	vmov.s16	r7, d15\[3\]
[^>]*> ee31 8b70 	vmov.s16	r8, d1\[3\]
[^>]*> ee33 8b70 	vmov.s16	r8, d3\[3\]
[^>]*> ee35 8b70 	vmov.s16	r8, d5\[3\]
[^>]*> ee39 8b70 	vmov.s16	r8, d9\[3\]
[^>]*> ee3f 8b70 	vmov.s16	r8, d15\[3\]
[^>]*> ee31 ab70 	vmov.s16	sl, d1\[3\]
[^>]*> ee33 ab70 	vmov.s16	sl, d3\[3\]
[^>]*> ee35 ab70 	vmov.s16	sl, d5\[3\]
[^>]*> ee39 ab70 	vmov.s16	sl, d9\[3\]
[^>]*> ee3f ab70 	vmov.s16	sl, d15\[3\]
[^>]*> ee31 cb70 	vmov.s16	ip, d1\[3\]
[^>]*> ee33 cb70 	vmov.s16	ip, d3\[3\]
[^>]*> ee35 cb70 	vmov.s16	ip, d5\[3\]
[^>]*> ee39 cb70 	vmov.s16	ip, d9\[3\]
[^>]*> ee3f cb70 	vmov.s16	ip, d15\[3\]
[^>]*> ee31 eb70 	vmov.s16	lr, d1\[3\]
[^>]*> ee33 eb70 	vmov.s16	lr, d3\[3\]
[^>]*> ee35 eb70 	vmov.s16	lr, d5\[3\]
[^>]*> ee39 eb70 	vmov.s16	lr, d9\[3\]
[^>]*> ee3f eb70 	vmov.s16	lr, d15\[3\]
[^>]*> ee10 0b10 	vmov.32	r0, d0\[0\]
[^>]*> ee12 0b10 	vmov.32	r0, d2\[0\]
[^>]*> ee14 0b10 	vmov.32	r0, d4\[0\]
[^>]*> ee18 0b10 	vmov.32	r0, d8\[0\]
[^>]*> ee1e 0b10 	vmov.32	r0, d14\[0\]
[^>]*> ee10 1b10 	vmov.32	r1, d0\[0\]
[^>]*> ee12 1b10 	vmov.32	r1, d2\[0\]
[^>]*> ee14 1b10 	vmov.32	r1, d4\[0\]
[^>]*> ee18 1b10 	vmov.32	r1, d8\[0\]
[^>]*> ee1e 1b10 	vmov.32	r1, d14\[0\]
[^>]*> ee10 2b10 	vmov.32	r2, d0\[0\]
[^>]*> ee12 2b10 	vmov.32	r2, d2\[0\]
[^>]*> ee14 2b10 	vmov.32	r2, d4\[0\]
[^>]*> ee18 2b10 	vmov.32	r2, d8\[0\]
[^>]*> ee1e 2b10 	vmov.32	r2, d14\[0\]
[^>]*> ee10 4b10 	vmov.32	r4, d0\[0\]
[^>]*> ee12 4b10 	vmov.32	r4, d2\[0\]
[^>]*> ee14 4b10 	vmov.32	r4, d4\[0\]
[^>]*> ee18 4b10 	vmov.32	r4, d8\[0\]
[^>]*> ee1e 4b10 	vmov.32	r4, d14\[0\]
[^>]*> ee10 7b10 	vmov.32	r7, d0\[0\]
[^>]*> ee12 7b10 	vmov.32	r7, d2\[0\]
[^>]*> ee14 7b10 	vmov.32	r7, d4\[0\]
[^>]*> ee18 7b10 	vmov.32	r7, d8\[0\]
[^>]*> ee1e 7b10 	vmov.32	r7, d14\[0\]
[^>]*> ee10 8b10 	vmov.32	r8, d0\[0\]
[^>]*> ee12 8b10 	vmov.32	r8, d2\[0\]
[^>]*> ee14 8b10 	vmov.32	r8, d4\[0\]
[^>]*> ee18 8b10 	vmov.32	r8, d8\[0\]
[^>]*> ee1e 8b10 	vmov.32	r8, d14\[0\]
[^>]*> ee10 ab10 	vmov.32	sl, d0\[0\]
[^>]*> ee12 ab10 	vmov.32	sl, d2\[0\]
[^>]*> ee14 ab10 	vmov.32	sl, d4\[0\]
[^>]*> ee18 ab10 	vmov.32	sl, d8\[0\]
[^>]*> ee1e ab10 	vmov.32	sl, d14\[0\]
[^>]*> ee10 cb10 	vmov.32	ip, d0\[0\]
[^>]*> ee12 cb10 	vmov.32	ip, d2\[0\]
[^>]*> ee14 cb10 	vmov.32	ip, d4\[0\]
[^>]*> ee18 cb10 	vmov.32	ip, d8\[0\]
[^>]*> ee1e cb10 	vmov.32	ip, d14\[0\]
[^>]*> ee10 eb10 	vmov.32	lr, d0\[0\]
[^>]*> ee12 eb10 	vmov.32	lr, d2\[0\]
[^>]*> ee14 eb10 	vmov.32	lr, d4\[0\]
[^>]*> ee18 eb10 	vmov.32	lr, d8\[0\]
[^>]*> ee1e eb10 	vmov.32	lr, d14\[0\]
[^>]*> ee30 0b10 	vmov.32	r0, d0\[1\]
[^>]*> ee32 0b10 	vmov.32	r0, d2\[1\]
[^>]*> ee34 0b10 	vmov.32	r0, d4\[1\]
[^>]*> ee38 0b10 	vmov.32	r0, d8\[1\]
[^>]*> ee3e 0b10 	vmov.32	r0, d14\[1\]
[^>]*> ee30 1b10 	vmov.32	r1, d0\[1\]
[^>]*> ee32 1b10 	vmov.32	r1, d2\[1\]
[^>]*> ee34 1b10 	vmov.32	r1, d4\[1\]
[^>]*> ee38 1b10 	vmov.32	r1, d8\[1\]
[^>]*> ee3e 1b10 	vmov.32	r1, d14\[1\]
[^>]*> ee30 2b10 	vmov.32	r2, d0\[1\]
[^>]*> ee32 2b10 	vmov.32	r2, d2\[1\]
[^>]*> ee34 2b10 	vmov.32	r2, d4\[1\]
[^>]*> ee38 2b10 	vmov.32	r2, d8\[1\]
[^>]*> ee3e 2b10 	vmov.32	r2, d14\[1\]
[^>]*> ee30 4b10 	vmov.32	r4, d0\[1\]
[^>]*> ee32 4b10 	vmov.32	r4, d2\[1\]
[^>]*> ee34 4b10 	vmov.32	r4, d4\[1\]
[^>]*> ee38 4b10 	vmov.32	r4, d8\[1\]
[^>]*> ee3e 4b10 	vmov.32	r4, d14\[1\]
[^>]*> ee30 7b10 	vmov.32	r7, d0\[1\]
[^>]*> ee32 7b10 	vmov.32	r7, d2\[1\]
[^>]*> ee34 7b10 	vmov.32	r7, d4\[1\]
[^>]*> ee38 7b10 	vmov.32	r7, d8\[1\]
[^>]*> ee3e 7b10 	vmov.32	r7, d14\[1\]
[^>]*> ee30 8b10 	vmov.32	r8, d0\[1\]
[^>]*> ee32 8b10 	vmov.32	r8, d2\[1\]
[^>]*> ee34 8b10 	vmov.32	r8, d4\[1\]
[^>]*> ee38 8b10 	vmov.32	r8, d8\[1\]
[^>]*> ee3e 8b10 	vmov.32	r8, d14\[1\]
[^>]*> ee30 ab10 	vmov.32	sl, d0\[1\]
[^>]*> ee32 ab10 	vmov.32	sl, d2\[1\]
[^>]*> ee34 ab10 	vmov.32	sl, d4\[1\]
[^>]*> ee38 ab10 	vmov.32	sl, d8\[1\]
[^>]*> ee3e ab10 	vmov.32	sl, d14\[1\]
[^>]*> ee30 cb10 	vmov.32	ip, d0\[1\]
[^>]*> ee32 cb10 	vmov.32	ip, d2\[1\]
[^>]*> ee34 cb10 	vmov.32	ip, d4\[1\]
[^>]*> ee38 cb10 	vmov.32	ip, d8\[1\]
[^>]*> ee3e cb10 	vmov.32	ip, d14\[1\]
[^>]*> ee30 eb10 	vmov.32	lr, d0\[1\]
[^>]*> ee32 eb10 	vmov.32	lr, d2\[1\]
[^>]*> ee34 eb10 	vmov.32	lr, d4\[1\]
[^>]*> ee38 eb10 	vmov.32	lr, d8\[1\]
[^>]*> ee3e eb10 	vmov.32	lr, d14\[1\]
[^>]*> ee11 0b10 	vmov.32	r0, d1\[0\]
[^>]*> ee13 0b10 	vmov.32	r0, d3\[0\]
[^>]*> ee15 0b10 	vmov.32	r0, d5\[0\]
[^>]*> ee19 0b10 	vmov.32	r0, d9\[0\]
[^>]*> ee1f 0b10 	vmov.32	r0, d15\[0\]
[^>]*> ee11 1b10 	vmov.32	r1, d1\[0\]
[^>]*> ee13 1b10 	vmov.32	r1, d3\[0\]
[^>]*> ee15 1b10 	vmov.32	r1, d5\[0\]
[^>]*> ee19 1b10 	vmov.32	r1, d9\[0\]
[^>]*> ee1f 1b10 	vmov.32	r1, d15\[0\]
[^>]*> ee11 2b10 	vmov.32	r2, d1\[0\]
[^>]*> ee13 2b10 	vmov.32	r2, d3\[0\]
[^>]*> ee15 2b10 	vmov.32	r2, d5\[0\]
[^>]*> ee19 2b10 	vmov.32	r2, d9\[0\]
[^>]*> ee1f 2b10 	vmov.32	r2, d15\[0\]
[^>]*> ee11 4b10 	vmov.32	r4, d1\[0\]
[^>]*> ee13 4b10 	vmov.32	r4, d3\[0\]
[^>]*> ee15 4b10 	vmov.32	r4, d5\[0\]
[^>]*> ee19 4b10 	vmov.32	r4, d9\[0\]
[^>]*> ee1f 4b10 	vmov.32	r4, d15\[0\]
[^>]*> ee11 7b10 	vmov.32	r7, d1\[0\]
[^>]*> ee13 7b10 	vmov.32	r7, d3\[0\]
[^>]*> ee15 7b10 	vmov.32	r7, d5\[0\]
[^>]*> ee19 7b10 	vmov.32	r7, d9\[0\]
[^>]*> ee1f 7b10 	vmov.32	r7, d15\[0\]
[^>]*> ee11 8b10 	vmov.32	r8, d1\[0\]
[^>]*> ee13 8b10 	vmov.32	r8, d3\[0\]
[^>]*> ee15 8b10 	vmov.32	r8, d5\[0\]
[^>]*> ee19 8b10 	vmov.32	r8, d9\[0\]
[^>]*> ee1f 8b10 	vmov.32	r8, d15\[0\]
[^>]*> ee11 ab10 	vmov.32	sl, d1\[0\]
[^>]*> ee13 ab10 	vmov.32	sl, d3\[0\]
[^>]*> ee15 ab10 	vmov.32	sl, d5\[0\]
[^>]*> ee19 ab10 	vmov.32	sl, d9\[0\]
[^>]*> ee1f ab10 	vmov.32	sl, d15\[0\]
[^>]*> ee11 cb10 	vmov.32	ip, d1\[0\]
[^>]*> ee13 cb10 	vmov.32	ip, d3\[0\]
[^>]*> ee15 cb10 	vmov.32	ip, d5\[0\]
[^>]*> ee19 cb10 	vmov.32	ip, d9\[0\]
[^>]*> ee1f cb10 	vmov.32	ip, d15\[0\]
[^>]*> ee11 eb10 	vmov.32	lr, d1\[0\]
[^>]*> ee13 eb10 	vmov.32	lr, d3\[0\]
[^>]*> ee15 eb10 	vmov.32	lr, d5\[0\]
[^>]*> ee19 eb10 	vmov.32	lr, d9\[0\]
[^>]*> ee1f eb10 	vmov.32	lr, d15\[0\]
[^>]*> ee31 0b10 	vmov.32	r0, d1\[1\]
[^>]*> ee33 0b10 	vmov.32	r0, d3\[1\]
[^>]*> ee35 0b10 	vmov.32	r0, d5\[1\]
[^>]*> ee39 0b10 	vmov.32	r0, d9\[1\]
[^>]*> ee3f 0b10 	vmov.32	r0, d15\[1\]
[^>]*> ee31 1b10 	vmov.32	r1, d1\[1\]
[^>]*> ee33 1b10 	vmov.32	r1, d3\[1\]
[^>]*> ee35 1b10 	vmov.32	r1, d5\[1\]
[^>]*> ee39 1b10 	vmov.32	r1, d9\[1\]
[^>]*> ee3f 1b10 	vmov.32	r1, d15\[1\]
[^>]*> ee31 2b10 	vmov.32	r2, d1\[1\]
[^>]*> ee33 2b10 	vmov.32	r2, d3\[1\]
[^>]*> ee35 2b10 	vmov.32	r2, d5\[1\]
[^>]*> ee39 2b10 	vmov.32	r2, d9\[1\]
[^>]*> ee3f 2b10 	vmov.32	r2, d15\[1\]
[^>]*> ee31 4b10 	vmov.32	r4, d1\[1\]
[^>]*> ee33 4b10 	vmov.32	r4, d3\[1\]
[^>]*> ee35 4b10 	vmov.32	r4, d5\[1\]
[^>]*> ee39 4b10 	vmov.32	r4, d9\[1\]
[^>]*> ee3f 4b10 	vmov.32	r4, d15\[1\]
[^>]*> ee31 7b10 	vmov.32	r7, d1\[1\]
[^>]*> ee33 7b10 	vmov.32	r7, d3\[1\]
[^>]*> ee35 7b10 	vmov.32	r7, d5\[1\]
[^>]*> ee39 7b10 	vmov.32	r7, d9\[1\]
[^>]*> ee3f 7b10 	vmov.32	r7, d15\[1\]
[^>]*> ee31 8b10 	vmov.32	r8, d1\[1\]
[^>]*> ee33 8b10 	vmov.32	r8, d3\[1\]
[^>]*> ee35 8b10 	vmov.32	r8, d5\[1\]
[^>]*> ee39 8b10 	vmov.32	r8, d9\[1\]
[^>]*> ee3f 8b10 	vmov.32	r8, d15\[1\]
[^>]*> ee31 ab10 	vmov.32	sl, d1\[1\]
[^>]*> ee33 ab10 	vmov.32	sl, d3\[1\]
[^>]*> ee35 ab10 	vmov.32	sl, d5\[1\]
[^>]*> ee39 ab10 	vmov.32	sl, d9\[1\]
[^>]*> ee3f ab10 	vmov.32	sl, d15\[1\]
[^>]*> ee31 cb10 	vmov.32	ip, d1\[1\]
[^>]*> ee33 cb10 	vmov.32	ip, d3\[1\]
[^>]*> ee35 cb10 	vmov.32	ip, d5\[1\]
[^>]*> ee39 cb10 	vmov.32	ip, d9\[1\]
[^>]*> ee3f cb10 	vmov.32	ip, d15\[1\]
[^>]*> ee31 eb10 	vmov.32	lr, d1\[1\]
[^>]*> ee33 eb10 	vmov.32	lr, d3\[1\]
[^>]*> ee35 eb10 	vmov.32	lr, d5\[1\]
[^>]*> ee39 eb10 	vmov.32	lr, d9\[1\]
[^>]*> ee3f eb10 	vmov.32	lr, d15\[1\]
[^>]*> ef80 0050 	vmov.i32	q0, #0	@ 0x00000000
[^>]*> ff87 005f 	vmov.i32	q0, #255	@ 0x000000ff
[^>]*> ff87 025f 	vmov.i32	q0, #65280	@ 0x0000ff00
[^>]*> ff87 065f 	vmov.i32	q0, #4278190080	@ 0xff000000
[^>]*> ff87 045f 	vmov.i32	q0, #16711680	@ 0x00ff0000
[^>]*> ef80 0850 	vmov.i16	q0, #0	@ 0x0000
[^>]*> ff87 085f 	vmov.i16	q0, #255	@ 0x00ff
[^>]*> ff87 0a5f 	vmov.i16	q0, #65280	@ 0xff00
[^>]*> ef80 0e50 	vmov.i8	q0, #0	@ 0x00
[^>]*> ff87 0e5f 	vmov.i8	q0, #255	@ 0xff
[^>]*> ff80 0e70 	vmov.i64	q0, #0xff00000000000000
[^>]*> ef84 0e70 	vmov.i64	q0, #0x00ff000000000000
[^>]*> ef82 0e70 	vmov.i64	q0, #0x0000ff0000000000
[^>]*> ef81 0e70 	vmov.i64	q0, #0x000000ff00000000
[^>]*> ef80 0e78 	vmov.i64	q0, #0x00000000ff000000
[^>]*> ef80 0e74 	vmov.i64	q0, #0x0000000000ff0000
[^>]*> ef80 0e72 	vmov.i64	q0, #0x000000000000ff00
[^>]*> ef80 0e71 	vmov.i64	q0, #0x00000000000000ff
[^>]*> ee00 0910 	vmov.f16	s0, r0
[^>]*> ee10 0910 	vmov.f16	r0, s0
[^>]*> ee00 1910 	vmov.f16	s0, r1
[^>]*> ee10 1910 	vmov.f16	r1, s0
[^>]*> ee00 2910 	vmov.f16	s0, r2
[^>]*> ee10 2910 	vmov.f16	r2, s0
[^>]*> ee00 4910 	vmov.f16	s0, r4
[^>]*> ee10 4910 	vmov.f16	r4, s0
[^>]*> ee00 7910 	vmov.f16	s0, r7
[^>]*> ee10 7910 	vmov.f16	r7, s0
[^>]*> ee00 8910 	vmov.f16	s0, r8
[^>]*> ee10 8910 	vmov.f16	r8, s0
[^>]*> ee00 a910 	vmov.f16	s0, sl
[^>]*> ee10 a910 	vmov.f16	sl, s0
[^>]*> ee00 c910 	vmov.f16	s0, ip
[^>]*> ee10 c910 	vmov.f16	ip, s0
[^>]*> ee00 e910 	vmov.f16	s0, lr
[^>]*> ee10 e910 	vmov.f16	lr, s0
[^>]*> ee00 0990 	vmov.f16	s1, r0
[^>]*> ee10 0990 	vmov.f16	r0, s1
[^>]*> ee00 1990 	vmov.f16	s1, r1
[^>]*> ee10 1990 	vmov.f16	r1, s1
[^>]*> ee00 2990 	vmov.f16	s1, r2
[^>]*> ee10 2990 	vmov.f16	r2, s1
[^>]*> ee00 4990 	vmov.f16	s1, r4
[^>]*> ee10 4990 	vmov.f16	r4, s1
[^>]*> ee00 7990 	vmov.f16	s1, r7
[^>]*> ee10 7990 	vmov.f16	r7, s1
[^>]*> ee00 8990 	vmov.f16	s1, r8
[^>]*> ee10 8990 	vmov.f16	r8, s1
[^>]*> ee00 a990 	vmov.f16	s1, sl
[^>]*> ee10 a990 	vmov.f16	sl, s1
[^>]*> ee00 c990 	vmov.f16	s1, ip
[^>]*> ee10 c990 	vmov.f16	ip, s1
[^>]*> ee00 e990 	vmov.f16	s1, lr
[^>]*> ee10 e990 	vmov.f16	lr, s1
[^>]*> ee01 0910 	vmov.f16	s2, r0
[^>]*> ee11 0910 	vmov.f16	r0, s2
[^>]*> ee01 1910 	vmov.f16	s2, r1
[^>]*> ee11 1910 	vmov.f16	r1, s2
[^>]*> ee01 2910 	vmov.f16	s2, r2
[^>]*> ee11 2910 	vmov.f16	r2, s2
[^>]*> ee01 4910 	vmov.f16	s2, r4
[^>]*> ee11 4910 	vmov.f16	r4, s2
[^>]*> ee01 7910 	vmov.f16	s2, r7
[^>]*> ee11 7910 	vmov.f16	r7, s2
[^>]*> ee01 8910 	vmov.f16	s2, r8
[^>]*> ee11 8910 	vmov.f16	r8, s2
[^>]*> ee01 a910 	vmov.f16	s2, sl
[^>]*> ee11 a910 	vmov.f16	sl, s2
[^>]*> ee01 c910 	vmov.f16	s2, ip
[^>]*> ee11 c910 	vmov.f16	ip, s2
[^>]*> ee01 e910 	vmov.f16	s2, lr
[^>]*> ee11 e910 	vmov.f16	lr, s2
[^>]*> ee02 0910 	vmov.f16	s4, r0
[^>]*> ee12 0910 	vmov.f16	r0, s4
[^>]*> ee02 1910 	vmov.f16	s4, r1
[^>]*> ee12 1910 	vmov.f16	r1, s4
[^>]*> ee02 2910 	vmov.f16	s4, r2
[^>]*> ee12 2910 	vmov.f16	r2, s4
[^>]*> ee02 4910 	vmov.f16	s4, r4
[^>]*> ee12 4910 	vmov.f16	r4, s4
[^>]*> ee02 7910 	vmov.f16	s4, r7
[^>]*> ee12 7910 	vmov.f16	r7, s4
[^>]*> ee02 8910 	vmov.f16	s4, r8
[^>]*> ee12 8910 	vmov.f16	r8, s4
[^>]*> ee02 a910 	vmov.f16	s4, sl
[^>]*> ee12 a910 	vmov.f16	sl, s4
[^>]*> ee02 c910 	vmov.f16	s4, ip
[^>]*> ee12 c910 	vmov.f16	ip, s4
[^>]*> ee02 e910 	vmov.f16	s4, lr
[^>]*> ee12 e910 	vmov.f16	lr, s4
[^>]*> ee04 0910 	vmov.f16	s8, r0
[^>]*> ee14 0910 	vmov.f16	r0, s8
[^>]*> ee04 1910 	vmov.f16	s8, r1
[^>]*> ee14 1910 	vmov.f16	r1, s8
[^>]*> ee04 2910 	vmov.f16	s8, r2
[^>]*> ee14 2910 	vmov.f16	r2, s8
[^>]*> ee04 4910 	vmov.f16	s8, r4
[^>]*> ee14 4910 	vmov.f16	r4, s8
[^>]*> ee04 7910 	vmov.f16	s8, r7
[^>]*> ee14 7910 	vmov.f16	r7, s8
[^>]*> ee04 8910 	vmov.f16	s8, r8
[^>]*> ee14 8910 	vmov.f16	r8, s8
[^>]*> ee04 a910 	vmov.f16	s8, sl
[^>]*> ee14 a910 	vmov.f16	sl, s8
[^>]*> ee04 c910 	vmov.f16	s8, ip
[^>]*> ee14 c910 	vmov.f16	ip, s8
[^>]*> ee04 e910 	vmov.f16	s8, lr
[^>]*> ee14 e910 	vmov.f16	lr, s8
[^>]*> ee08 0910 	vmov.f16	s16, r0
[^>]*> ee18 0910 	vmov.f16	r0, s16
[^>]*> ee08 1910 	vmov.f16	s16, r1
[^>]*> ee18 1910 	vmov.f16	r1, s16
[^>]*> ee08 2910 	vmov.f16	s16, r2
[^>]*> ee18 2910 	vmov.f16	r2, s16
[^>]*> ee08 4910 	vmov.f16	s16, r4
[^>]*> ee18 4910 	vmov.f16	r4, s16
[^>]*> ee08 7910 	vmov.f16	s16, r7
[^>]*> ee18 7910 	vmov.f16	r7, s16
[^>]*> ee08 8910 	vmov.f16	s16, r8
[^>]*> ee18 8910 	vmov.f16	r8, s16
[^>]*> ee08 a910 	vmov.f16	s16, sl
[^>]*> ee18 a910 	vmov.f16	sl, s16
[^>]*> ee08 c910 	vmov.f16	s16, ip
[^>]*> ee18 c910 	vmov.f16	ip, s16
[^>]*> ee08 e910 	vmov.f16	s16, lr
[^>]*> ee18 e910 	vmov.f16	lr, s16
[^>]*> ee0f 0910 	vmov.f16	s30, r0
[^>]*> ee1f 0910 	vmov.f16	r0, s30
[^>]*> ee0f 1910 	vmov.f16	s30, r1
[^>]*> ee1f 1910 	vmov.f16	r1, s30
[^>]*> ee0f 2910 	vmov.f16	s30, r2
[^>]*> ee1f 2910 	vmov.f16	r2, s30
[^>]*> ee0f 4910 	vmov.f16	s30, r4
[^>]*> ee1f 4910 	vmov.f16	r4, s30
[^>]*> ee0f 7910 	vmov.f16	s30, r7
[^>]*> ee1f 7910 	vmov.f16	r7, s30
[^>]*> ee0f 8910 	vmov.f16	s30, r8
[^>]*> ee1f 8910 	vmov.f16	r8, s30
[^>]*> ee0f a910 	vmov.f16	s30, sl
[^>]*> ee1f a910 	vmov.f16	sl, s30
[^>]*> ee0f c910 	vmov.f16	s30, ip
[^>]*> ee1f c910 	vmov.f16	ip, s30
[^>]*> ee0f e910 	vmov.f16	s30, lr
[^>]*> ee1f e910 	vmov.f16	lr, s30
[^>]*> ee0f 0990 	vmov.f16	s31, r0
[^>]*> ee1f 0990 	vmov.f16	r0, s31
[^>]*> ee0f 1990 	vmov.f16	s31, r1
[^>]*> ee1f 1990 	vmov.f16	r1, s31
[^>]*> ee0f 2990 	vmov.f16	s31, r2
[^>]*> ee1f 2990 	vmov.f16	r2, s31
[^>]*> ee0f 4990 	vmov.f16	s31, r4
[^>]*> ee1f 4990 	vmov.f16	r4, s31
[^>]*> ee0f 7990 	vmov.f16	s31, r7
[^>]*> ee1f 7990 	vmov.f16	r7, s31
[^>]*> ee0f 8990 	vmov.f16	s31, r8
[^>]*> ee1f 8990 	vmov.f16	r8, s31
[^>]*> ee0f a990 	vmov.f16	s31, sl
[^>]*> ee1f a990 	vmov.f16	sl, s31
[^>]*> ee0f c990 	vmov.f16	s31, ip
[^>]*> ee1f c990 	vmov.f16	ip, s31
[^>]*> ee0f e990 	vmov.f16	s31, lr
[^>]*> ee1f e990 	vmov.f16	lr, s31
[^>]*> ef80 0050 	vmov.i32	q0, #0	@ 0x00000000
[^>]*> ff83 0f5f 	vmov.f32	q0, #-31	@ 0xc1f80000
[^>]*> ff83 0f5f 	vmov.f32	q0, #-31	@ 0xc1f80000
[^>]*> ff87 0f5f 	vmov.f32	q0, #-1.9375	@ 0xbff80000
[^>]*> ef87 0f50 	vmov.f32	q0, #1	@ 0x3f800000
[^>]*> eeb0 0900 	vmov.f16	s0, #0	@ 0x40000000  2.0
[^>]*> eeb0 0a04 	vmov.f32	s0, #4	@ 0x40200000  2.5
