# name: MVE vmov, vmvn, vbic, vorr aliases
# as: -march=armv8.1-m.main+mve
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main
# skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ff80 0d70 	vmvn.i32	q0, #8454143	@ 0x0080ffff
[^>]*> ff80 0f70 			@ <UNDEFINED> instruction: 0xff800f70
[^>]*> ef80 0e70 	vmov.i64	q0, #0x0000000000000000
[^>]*> ef80 0070 	vmvn.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0270 	vmvn.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0470 	vmvn.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0670 	vmvn.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0870 	vmvn.i16	q0, #0	@ 0x0000
[^>]*> ef80 0a70 	vmvn.i16	q0, #0	@ 0x0000
[^>]*> ef80 0c70 	vmvn.i32	q0, #255	@ 0x000000ff
[^>]*> ef80 0150 	vorr.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0350 	vorr.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0550 	vorr.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0950 	vorr.i16	q0, #0	@ 0x0000
[^>]*> ef80 0b50 	vorr.i16	q0, #0	@ 0x0000
[^>]*> ef80 0170 	vbic.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0370 	vbic.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0570 	vbic.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0770 	vbic.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0970 	vbic.i16	q0, #0	@ 0x0000
[^>]*> ef80 0b70 	vbic.i16	q0, #0	@ 0x0000
[^>]*> ef80 0050 	vmov.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0250 	vmov.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0450 	vmov.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0650 	vmov.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0850 	vmov.i16	q0, #0	@ 0x0000
[^>]*> ef80 0a50 	vmov.i16	q0, #0	@ 0x0000
[^>]*> ef80 0c50 	vmov.i32	q0, #255	@ 0x000000ff
[^>]*> ef80 0e50 	vmov.i8	q0, #0	@ 0x00
[^>]*> ef80 0d50 	vmov.i32	q0, #65535	@ 0x0000ffff
[^>]*> ef80 0f50 	vmov.f32	q0, #2	@ 0x40000000
[^>]*> ff80 0d70 	vmvn.i32	q0, #8454143	@ 0x0080ffff
[^>]*> ff80 0d50 	vmov.i32	q0, #8454143	@ 0x0080ffff
