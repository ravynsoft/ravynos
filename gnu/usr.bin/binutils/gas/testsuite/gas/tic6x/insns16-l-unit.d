#objdump: -dr --prefix-addresses --show-raw-insn
#name:
#as: -march=c64x+ -mlittle-endian

.*: *file format elf32-tic6x-le


Disassembly of section .text:
[ \t]*\.\.\.
[0-9a-f]+[02468ace] <[^>]*> 0010[ \t]+add \.L1 a0,a0,a1
[0-9a-f]+[02468ace] <[^>]*> 2120[ \t]+add \.L1 a1,a2,a2
[0-9a-f]+[02468ace] <[^>]*> 4230[ \t]+add \.L1 a2,a4,a3
[0-9a-f]+[02468ace] <[^>]*> 6340[ \t]+add \.L1 a3,a6,a4
[0-9a-f]+[02468ace] <[^>]*> 8050[ \t]+add \.L1 a4,a0,a5
[0-9a-f]+[02468ace] <[^>]*> a160[ \t]+add \.L1 a5,a2,a6
[0-9a-f]+[02468ace] <[^>]*> c270[ \t]+add \.L1 a6,a4,a7
[0-9a-f]+[02468ace] <[^>]*> eb80[ \t]+sub \.L1 a7,a7,a0
[0-9a-f]+[02468ace] <[^>]*> 0890[ \t]+sub \.L1 a0,a1,a1
[0-9a-f]+[02468ace] <[^>]*> 29a0[ \t]+sub \.L1 a1,a3,a2
[0-9a-f]+[02468ace] <[^>]*> 4ab0[ \t]+sub \.L1 a2,a5,a3
[0-9a-f]+[02468ace] <[^>]*> 6bc0[ \t]+sub \.L1 a3,a7,a4
[0-9a-f]+[02468ace] <[^>]*> 88d0[ \t]+sub \.L1 a4,a1,a5
[0-9a-f]+[02468ace] <[^>]*> a9e0[ \t]+sub \.L1 a5,a3,a6
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> c2f0[ \t]+sadd \.L1 a22,a21,a23
[0-9a-f]+[02468ace] <[^>]*> e300[ \t]+sadd \.L1 a23,a22,a16
[0-9a-f]+[02468ace] <[^>]*> 0010[ \t]+sadd \.L1 a16,a16,a17
[0-9a-f]+[02468ace] <[^>]*> 2120[ \t]+sadd \.L1 a17,a18,a18
[0-9a-f]+[02468ace] <[^>]*> 4230[ \t]+sadd \.L1 a18,a20,a19
[0-9a-f]+[02468ace] <[^>]*> 6340[ \t]+sadd \.L1 a19,a22,a20
[0-9a-f]+[02468ace] <[^>]*> 8050[ \t]+sadd \.L1 a20,a16,a21
[0-9a-f]+[02468ace] <[^>]*> a960[ \t]+ssub \.L1 a21,a18,a22
[0-9a-f]+[02468ace] <[^>]*> ca70[ \t]+ssub \.L1 a22,a20,a23
[0-9a-f]+[02468ace] <[^>]*> eb80[ \t]+ssub \.L1 a23,a23,a16
[0-9a-f]+[02468ace] <[^>]*> 0890[ \t]+ssub \.L1 a16,a17,a17
[0-9a-f]+[02468ace] <[^>]*> 29a0[ \t]+ssub \.L1 a17,a19,a18
[0-9a-f]+[02468ace] <[^>]*> 4ab0[ \t]+ssub \.L1 a18,a21,a19
[0-9a-f]+[02468ace] <[^>]*> 6bc0[ \t]+ssub \.L1 a19,a23,a20
[0-9a-f]+[02468ace] <[^>]*> efe84000[ \t]+<fetch packet header 0xefe84000>
[0-9a-f]+[02468ace] <[^>]*> 0410[ \t]+add \.L1 8,a0,a1
[0-9a-f]+[02468ace] <[^>]*> 3520[ \t]+add \.L1X 1,b2,a2
[0-9a-f]+[02468ace] <[^>]*> 4630[ \t]+add \.L1 2,a4,a3
[0-9a-f]+[02468ace] <[^>]*> 7740[ \t]+add \.L1X 3,b6,a4
[0-9a-f]+[02468ace] <[^>]*> 8550[ \t]+add \.L1 4,a2,a5
[0-9a-f]+[02468ace] <[^>]*> b660[ \t]+add \.L1X 5,b4,a6
[0-9a-f]+[02468ace] <[^>]*> c770[ \t]+add \.L1 6,a6,a7
[0-9a-f]+[02468ace] <[^>]*> 1c80[ \t]+add \.L1X -8,b1,a0
[0-9a-f]+[02468ace] <[^>]*> 2d90[ \t]+add \.L1 -7,a3,a1
[0-9a-f]+[02468ace] <[^>]*> 5ea0[ \t]+add \.L1X -6,b5,a2
[0-9a-f]+[02468ace] <[^>]*> 6fb0[ \t]+add \.L1 -5,a7,a3
[0-9a-f]+[02468ace] <[^>]*> 9cc0[ \t]+add \.L1X -4,b1,a4
[0-9a-f]+[02468ace] <[^>]*> add0[ \t]+add \.L1 -3,a3,a5
[0-9a-f]+[02468ace] <[^>]*> dee0[ \t]+add \.L1X -2,b5,a6
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0408[ \t]+and \.L1 a0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 2409[ \t]+and \.L2 b1,b0,b0
[0-9a-f]+[02468ace] <[^>]*> 4418[ \t]+and \.L1 a2,a0,a1
[0-9a-f]+[02468ace] <[^>]*> 6419[ \t]+and \.L2 b3,b0,b1
[0-9a-f]+[02468ace] <[^>]*> 9408[ \t]+and \.L1X a4,b0,a0
[0-9a-f]+[02468ace] <[^>]*> b409[ \t]+and \.L2X b5,a0,b0
[0-9a-f]+[02468ace] <[^>]*> d418[ \t]+and \.L1X a6,b0,a1
[0-9a-f]+[02468ace] <[^>]*> f439[ \t]+or \.L2X b7,a0,b1
[0-9a-f]+[02468ace] <[^>]*> 2428[ \t]+or \.L1 a1,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 4429[ \t]+or \.L2 b2,b0,b0
[0-9a-f]+[02468ace] <[^>]*> 6438[ \t]+or \.L1 a3,a0,a1
[0-9a-f]+[02468ace] <[^>]*> 8439[ \t]+or \.L2 b4,b0,b1
[0-9a-f]+[02468ace] <[^>]*> b428[ \t]+or \.L1X a5,b0,a0
[0-9a-f]+[02468ace] <[^>]*> d429[ \t]+or \.L2X b6,a0,b0
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0458[ \t]+xor \.L1 a16,a16,a1
[0-9a-f]+[02468ace] <[^>]*> 2449[ \t]+xor \.L2 b17,b16,b0
[0-9a-f]+[02468ace] <[^>]*> 4448[ \t]+xor \.L1 a18,a16,a0
[0-9a-f]+[02468ace] <[^>]*> 6459[ \t]+xor \.L2 b19,b16,b1
[0-9a-f]+[02468ace] <[^>]*> 9458[ \t]+xor \.L1X a20,b16,a1
[0-9a-f]+[02468ace] <[^>]*> b449[ \t]+xor \.L2X b21,a16,b0
[0-9a-f]+[02468ace] <[^>]*> d448[ \t]+xor \.L1X a22,b16,a0
[0-9a-f]+[02468ace] <[^>]*> f479[ \t]+cmpeq \.L2X b23,a16,b1
[0-9a-f]+[02468ace] <[^>]*> 2468[ \t]+cmpeq \.L1 a17,a16,a0
[0-9a-f]+[02468ace] <[^>]*> 4469[ \t]+cmpeq \.L2 b18,b16,b0
[0-9a-f]+[02468ace] <[^>]*> 6478[ \t]+cmpeq \.L1 a19,a16,a1
[0-9a-f]+[02468ace] <[^>]*> 8479[ \t]+cmpeq \.L2 b20,b16,b1
[0-9a-f]+[02468ace] <[^>]*> b468[ \t]+cmpeq \.L1X a21,b16,a0
[0-9a-f]+[02468ace] <[^>]*> d469[ \t]+cmpeq \.L2X b22,a16,b0
[0-9a-f]+[02468ace] <[^>]*> efe80000[ \t]+<fetch packet header 0xefe80000>
[0-9a-f]+[02468ace] <[^>]*> 0c18[ \t]+cmplt \.L1 a0,a0,a1
[0-9a-f]+[02468ace] <[^>]*> 2c09[ \t]+cmplt \.L2 b1,b0,b0
[0-9a-f]+[02468ace] <[^>]*> 4c08[ \t]+cmplt \.L1 a2,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 6c19[ \t]+cmplt \.L2 b3,b0,b1
[0-9a-f]+[02468ace] <[^>]*> 9c18[ \t]+cmplt \.L1X a4,b0,a1
[0-9a-f]+[02468ace] <[^>]*> bc09[ \t]+cmplt \.L2X b5,a0,b0
[0-9a-f]+[02468ace] <[^>]*> dc08[ \t]+cmplt \.L1X a6,b0,a0
[0-9a-f]+[02468ace] <[^>]*> fc39[ \t]+cmpgt \.L2X b7,a0,b1
[0-9a-f]+[02468ace] <[^>]*> 2c28[ \t]+cmpgt \.L1 a1,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 4c29[ \t]+cmpgt \.L2 b2,b0,b0
[0-9a-f]+[02468ace] <[^>]*> 6c38[ \t]+cmpgt \.L1 a3,a0,a1
[0-9a-f]+[02468ace] <[^>]*> 8c39[ \t]+cmpgt \.L2 b4,b0,b1
[0-9a-f]+[02468ace] <[^>]*> bc28[ \t]+cmpgt \.L1X a5,b0,a0
[0-9a-f]+[02468ace] <[^>]*> dc29[ \t]+cmpgt \.L2X b6,a0,b0
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0c58[ \t]+cmpltu \.L1 a16,a16,a1
[0-9a-f]+[02468ace] <[^>]*> 2c49[ \t]+cmpltu \.L2 b17,b16,b0
[0-9a-f]+[02468ace] <[^>]*> 4c48[ \t]+cmpltu \.L1 a18,a16,a0
[0-9a-f]+[02468ace] <[^>]*> 6c59[ \t]+cmpltu \.L2 b19,b16,b1
[0-9a-f]+[02468ace] <[^>]*> 9c58[ \t]+cmpltu \.L1X a20,b16,a1
[0-9a-f]+[02468ace] <[^>]*> bc49[ \t]+cmpltu \.L2X b21,a16,b0
[0-9a-f]+[02468ace] <[^>]*> dc48[ \t]+cmpltu \.L1X a22,b16,a0
[0-9a-f]+[02468ace] <[^>]*> fc79[ \t]+cmpgtu \.L2X b23,a16,b1
[0-9a-f]+[02468ace] <[^>]*> 2c68[ \t]+cmpgtu \.L1 a17,a16,a0
[0-9a-f]+[02468ace] <[^>]*> 4c69[ \t]+cmpgtu \.L2 b18,b16,b0
[0-9a-f]+[02468ace] <[^>]*> 6c78[ \t]+cmpgtu \.L1 a19,a16,a1
[0-9a-f]+[02468ace] <[^>]*> 8c79[ \t]+cmpgtu \.L2 b20,b16,b1
[0-9a-f]+[02468ace] <[^>]*> bc68[ \t]+cmpgtu \.L1X a21,b16,a0
[0-9a-f]+[02468ace] <[^>]*> dc69[ \t]+cmpgtu \.L2X b22,a16,b0
[0-9a-f]+[02468ace] <[^>]*> efe80000[ \t]+<fetch packet header 0xefe80000>
[0-9a-f]+[02468ace] <[^>]*> 0426[ \t]+mvk \.L1 0,a0
[0-9a-f]+[02468ace] <[^>]*> 2527[ \t]+mvk \.L2 1,b2
[0-9a-f]+[02468ace] <[^>]*> 46a6[ \t]+mvk \.L1 2,a5
[0-9a-f]+[02468ace] <[^>]*> 67a7[ \t]+mvk \.L2 3,b7
[0-9a-f]+[02468ace] <[^>]*> 8626[ \t]+mvk \.L1 4,a4
[0-9a-f]+[02468ace] <[^>]*> a527[ \t]+mvk \.L2 5,b2
[0-9a-f]+[02468ace] <[^>]*> c4a6[ \t]+mvk \.L1 6,a1
[0-9a-f]+[02468ace] <[^>]*> f5a7[ \t]+mvk \.L2 -9,b3
[0-9a-f]+[02468ace] <[^>]*> 1626[ \t]+mvk \.L1 -16,a4
[0-9a-f]+[02468ace] <[^>]*> 3727[ \t]+mvk \.L2 -15,b6
[0-9a-f]+[02468ace] <[^>]*> 56a6[ \t]+mvk \.L1 -14,a5
[0-9a-f]+[02468ace] <[^>]*> 75a7[ \t]+mvk \.L2 -13,b3
[0-9a-f]+[02468ace] <[^>]*> 9426[ \t]+mvk \.L1 -12,a0
[0-9a-f]+[02468ace] <[^>]*> b527[ \t]+mvk \.L2 -11,b2
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0026[ \t]+cmpeq \.L1 0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 2127[ \t]+cmpeq \.L2 1,b2,b0
[0-9a-f]+[02468ace] <[^>]*> 42a6[ \t]+cmpeq \.L1 2,a5,a0
[0-9a-f]+[02468ace] <[^>]*> 63a7[ \t]+cmpeq \.L2 3,b7,b0
[0-9a-f]+[02468ace] <[^>]*> 8226[ \t]+cmpeq \.L1 4,a4,a0
[0-9a-f]+[02468ace] <[^>]*> a127[ \t]+cmpeq \.L2 5,b2,b0
[0-9a-f]+[02468ace] <[^>]*> c0a6[ \t]+cmpeq \.L1 6,a1,a0
[0-9a-f]+[02468ace] <[^>]*> e1a7[ \t]+cmpeq \.L2 7,b3,b0
[0-9a-f]+[02468ace] <[^>]*> 0226[ \t]+cmpeq \.L1 0,a4,a0
[0-9a-f]+[02468ace] <[^>]*> 2327[ \t]+cmpeq \.L2 1,b6,b0
[0-9a-f]+[02468ace] <[^>]*> 42a6[ \t]+cmpeq \.L1 2,a5,a0
[0-9a-f]+[02468ace] <[^>]*> 61a7[ \t]+cmpeq \.L2 3,b3,b0
[0-9a-f]+[02468ace] <[^>]*> 8026[ \t]+cmpeq \.L1 4,a0,a0
[0-9a-f]+[02468ace] <[^>]*> a127[ \t]+cmpeq \.L2 5,b2,b0
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 1026[ \t]+cmplt \.L1 0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 3127[ \t]+cmplt \.L2 1,b2,b0
[0-9a-f]+[02468ace] <[^>]*> 52a6[ \t]+cmpgt \.L1 0,a5,a0
[0-9a-f]+[02468ace] <[^>]*> 73a7[ \t]+cmpgt \.L2 1,b7,b0
[0-9a-f]+[02468ace] <[^>]*> 9226[ \t]+cmpltu \.L1 0,a4,a0
[0-9a-f]+[02468ace] <[^>]*> b127[ \t]+cmpltu \.L2 1,b2,b0
[0-9a-f]+[02468ace] <[^>]*> d0a6[ \t]+cmpgtu \.L1 0,a1,a0
[0-9a-f]+[02468ace] <[^>]*> f1a7[ \t]+cmpgtu \.L2 1,b3,b0
[0-9a-f]+[02468ace] <[^>]*> 1226[ \t]+cmplt \.L1 0,a4,a0
[0-9a-f]+[02468ace] <[^>]*> 3327[ \t]+cmplt \.L2 1,b6,b0
[0-9a-f]+[02468ace] <[^>]*> 52a6[ \t]+cmpgt \.L1 0,a5,a0
[0-9a-f]+[02468ace] <[^>]*> 71a7[ \t]+cmpgt \.L2 1,b3,b0
[0-9a-f]+[02468ace] <[^>]*> 9026[ \t]+cmpltu \.L1 0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> b127[ \t]+cmpltu \.L2 1,b2,b0
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 5b66[ \t]+sub \.L1 0,a6,a6
[0-9a-f]+[02468ace] <[^>]*> 5a67[ \t]+sub \.L2 0,b4,b4
[0-9a-f]+[02468ace] <[^>]*> 59e6[ \t]+sub \.L1 0,a3,a3
[0-9a-f]+[02468ace] <[^>]*> 58e7[ \t]+sub \.L2 0,b1,b1
[0-9a-f]+[02468ace] <[^>]*> 5866[ \t]+sub \.L1 0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 5967[ \t]+sub \.L2 0,b2,b2
[0-9a-f]+[02468ace] <[^>]*> 5ae6[ \t]+sub \.L1 0,a5,a5
[0-9a-f]+[02468ace] <[^>]*> 7be7[ \t]+add \.L2 -1,b7,b7
[0-9a-f]+[02468ace] <[^>]*> 7b66[ \t]+add \.L1 -1,a6,a6
[0-9a-f]+[02468ace] <[^>]*> 7a67[ \t]+add \.L2 -1,b4,b4
[0-9a-f]+[02468ace] <[^>]*> 79e6[ \t]+add \.L1 -1,a3,a3
[0-9a-f]+[02468ace] <[^>]*> 78e7[ \t]+add \.L2 -1,b1,b1
[0-9a-f]+[02468ace] <[^>]*> 7866[ \t]+add \.L1 -1,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 7967[ \t]+add \.L2 -1,b2,b2
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[ \t]*\.\.\.
