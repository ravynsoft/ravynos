# name: Armv8.1-M Mainline scalar vmul instructions in it blocks (with MVE)
# as: -march=armv8.1-m.main+mve.fp+fp.dp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> bfbc      	itt	lt
[^>]*> ee20 0a81 	vmullt.f32	s0, s1, s2
[^>]*> ee21 0b02 	vmullt.f64	d0, d1, d2
#...
