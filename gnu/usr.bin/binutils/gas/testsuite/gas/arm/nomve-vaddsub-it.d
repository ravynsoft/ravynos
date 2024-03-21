# name: Armv8.1-M Mainline vadd/vsub instructions in it blocks (without MVE)
# as: -march=armv8.1-m.main+fp.dp
# source: mve-vaddsub-it.s
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> bfdc      	itt	le
[^>]*> ee72 1aa3 	vaddle.f32	s3, s5, s7
[^>]*> ee34 7b06 	vaddle.f64	d7, d4, d6
[^>]*> bfbc      	itt	lt
[^>]*> ee72 1aa3 	vaddlt.f32	s3, s5, s7
[^>]*> ee34 7b06 	vaddlt.f64	d7, d4, d6
[^>]*> bfdc      	itt	le
[^>]*> ee72 1ae3 	vsuble.f32	s3, s5, s7
[^>]*> ee34 7b46 	vsuble.f64	d7, d4, d6
[^>]*> bfbc      	itt	lt
[^>]*> ee72 1ae3 	vsublt.f32	s3, s5, s7
[^>]*> ee34 7b46 	vsublt.f64	d7, d4, d6
[^>]*> bfdc      	itt	le
[^>]*> ee30 0a06 	vaddle.f32	s0, s0, s12
[^>]*> ee30 0b41 	vsuble.f64	d0, d0, d1
#...
