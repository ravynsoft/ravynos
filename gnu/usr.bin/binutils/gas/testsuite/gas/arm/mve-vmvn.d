# name: MVE vmvn instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ffb0 05c0 	vmvn	q0, q0
[^>]*> ffb0 05c2 	vmvn	q0, q1
[^>]*> ffb0 05c4 	vmvn	q0, q2
[^>]*> ffb0 05c8 	vmvn	q0, q4
[^>]*> ffb0 05ce 	vmvn	q0, q7
[^>]*> ef80 0070 	vmvn.i32	q0, #0	@ 0x00000000
[^>]*> ef80 0870 	vmvn.i16	q0, #0	@ 0x0000
[^>]*> ff87 007f 	vmvn.i32	q0, #255	@ 0x000000ff
[^>]*> ff87 027f 	vmvn.i32	q0, #65280	@ 0x0000ff00
[^>]*> ff87 047f 	vmvn.i32	q0, #16711680	@ 0x00ff0000
[^>]*> ff87 067f 	vmvn.i32	q0, #4278190080	@ 0xff000000
[^>]*> ff82 0c7b 	vmvn.i32	q0, #44031	@ 0x0000abff
[^>]*> ff87 087f 	vmvn.i16	q0, #255	@ 0x00ff
[^>]*> ff87 0a7f 	vmvn.i16	q0, #65280	@ 0xff00
[^>]*> ff87 0a5e 	vmov.i16	q0, #65024	@ 0xfe00
[^>]*> ef80 0e50 	vmov.i8	q0, #0	@ 0x00
[^>]*> ffb0 25c0 	vmvn	q1, q0
[^>]*> ffb0 25c2 	vmvn	q1, q1
[^>]*> ffb0 25c4 	vmvn	q1, q2
[^>]*> ffb0 25c8 	vmvn	q1, q4
[^>]*> ffb0 25ce 	vmvn	q1, q7
[^>]*> ef80 2070 	vmvn.i32	q1, #0	@ 0x00000000
[^>]*> ef80 2870 	vmvn.i16	q1, #0	@ 0x0000
[^>]*> ff87 207f 	vmvn.i32	q1, #255	@ 0x000000ff
[^>]*> ff87 227f 	vmvn.i32	q1, #65280	@ 0x0000ff00
[^>]*> ff87 247f 	vmvn.i32	q1, #16711680	@ 0x00ff0000
[^>]*> ff87 267f 	vmvn.i32	q1, #4278190080	@ 0xff000000
[^>]*> ff82 2c7b 	vmvn.i32	q1, #44031	@ 0x0000abff
[^>]*> ff87 287f 	vmvn.i16	q1, #255	@ 0x00ff
[^>]*> ff87 2a7f 	vmvn.i16	q1, #65280	@ 0xff00
[^>]*> ff87 2a5e 	vmov.i16	q1, #65024	@ 0xfe00
[^>]*> ef80 2e50 	vmov.i8	q1, #0	@ 0x00
[^>]*> ffb0 45c0 	vmvn	q2, q0
[^>]*> ffb0 45c2 	vmvn	q2, q1
[^>]*> ffb0 45c4 	vmvn	q2, q2
[^>]*> ffb0 45c8 	vmvn	q2, q4
[^>]*> ffb0 45ce 	vmvn	q2, q7
[^>]*> ef80 4070 	vmvn.i32	q2, #0	@ 0x00000000
[^>]*> ef80 4870 	vmvn.i16	q2, #0	@ 0x0000
[^>]*> ff87 407f 	vmvn.i32	q2, #255	@ 0x000000ff
[^>]*> ff87 427f 	vmvn.i32	q2, #65280	@ 0x0000ff00
[^>]*> ff87 447f 	vmvn.i32	q2, #16711680	@ 0x00ff0000
[^>]*> ff87 467f 	vmvn.i32	q2, #4278190080	@ 0xff000000
[^>]*> ff82 4c7b 	vmvn.i32	q2, #44031	@ 0x0000abff
[^>]*> ff87 487f 	vmvn.i16	q2, #255	@ 0x00ff
[^>]*> ff87 4a7f 	vmvn.i16	q2, #65280	@ 0xff00
[^>]*> ff87 4a5e 	vmov.i16	q2, #65024	@ 0xfe00
[^>]*> ef80 4e50 	vmov.i8	q2, #0	@ 0x00
[^>]*> ffb0 85c0 	vmvn	q4, q0
[^>]*> ffb0 85c2 	vmvn	q4, q1
[^>]*> ffb0 85c4 	vmvn	q4, q2
[^>]*> ffb0 85c8 	vmvn	q4, q4
[^>]*> ffb0 85ce 	vmvn	q4, q7
[^>]*> ef80 8070 	vmvn.i32	q4, #0	@ 0x00000000
[^>]*> ef80 8870 	vmvn.i16	q4, #0	@ 0x0000
[^>]*> ff87 807f 	vmvn.i32	q4, #255	@ 0x000000ff
[^>]*> ff87 827f 	vmvn.i32	q4, #65280	@ 0x0000ff00
[^>]*> ff87 847f 	vmvn.i32	q4, #16711680	@ 0x00ff0000
[^>]*> ff87 867f 	vmvn.i32	q4, #4278190080	@ 0xff000000
[^>]*> ff82 8c7b 	vmvn.i32	q4, #44031	@ 0x0000abff
[^>]*> ff87 887f 	vmvn.i16	q4, #255	@ 0x00ff
[^>]*> ff87 8a7f 	vmvn.i16	q4, #65280	@ 0xff00
[^>]*> ff87 8a5e 	vmov.i16	q4, #65024	@ 0xfe00
[^>]*> ef80 8e50 	vmov.i8	q4, #0	@ 0x00
[^>]*> ffb0 e5c0 	vmvn	q7, q0
[^>]*> ffb0 e5c2 	vmvn	q7, q1
[^>]*> ffb0 e5c4 	vmvn	q7, q2
[^>]*> ffb0 e5c8 	vmvn	q7, q4
[^>]*> ffb0 e5ce 	vmvn	q7, q7
[^>]*> ef80 e070 	vmvn.i32	q7, #0	@ 0x00000000
[^>]*> ef80 e870 	vmvn.i16	q7, #0	@ 0x0000
[^>]*> ff87 e07f 	vmvn.i32	q7, #255	@ 0x000000ff
[^>]*> ff87 e27f 	vmvn.i32	q7, #65280	@ 0x0000ff00
[^>]*> ff87 e47f 	vmvn.i32	q7, #16711680	@ 0x00ff0000
[^>]*> ff87 e67f 	vmvn.i32	q7, #4278190080	@ 0xff000000
[^>]*> ff82 ec7b 	vmvn.i32	q7, #44031	@ 0x0000abff
[^>]*> ff87 e87f 	vmvn.i16	q7, #255	@ 0x00ff
[^>]*> ff87 ea7f 	vmvn.i16	q7, #65280	@ 0xff00
[^>]*> ff87 ea5e 	vmov.i16	q7, #65024	@ 0xfe00
[^>]*> ef80 ee50 	vmov.i8	q7, #0	@ 0x00
[^>]*> fe71 ef4d 	vpstete
[^>]*> ff87 007f 	vmvnt.i32	q0, #255	@ 0x000000ff
[^>]*> ff87 ca7f 	vmvne.i16	q6, #65280	@ 0xff00
[^>]*> ffb0 05c2 	vmvnt	q0, q1
[^>]*> ffb0 e5c6 	vmvne	q7, q3
