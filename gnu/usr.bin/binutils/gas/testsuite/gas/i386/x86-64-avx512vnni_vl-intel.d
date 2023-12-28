#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512VNNI/VL insns (Intel disassembly)
#source: x86-64-avx512vnni_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 00 52 d4[ 	]*vpdpwssd xmm26,xmm22,xmm20
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 03 52 d4[ 	]*vpdpwssd xmm26\{k3\},xmm22,xmm20
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 83 52 d4[ 	]*vpdpwssd xmm26\{k3\}\{z\},xmm22,xmm20
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 00 52 94 f0 23 01 00 00[ 	]*vpdpwssd xmm26,xmm22,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 4d 00 52 52 7f[ 	]*vpdpwssd xmm26,xmm22,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 4d 10 52 52 7f[ 	]*vpdpwssd xmm26,xmm22,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 20 52 e2[ 	]*vpdpwssd ymm20,ymm20,ymm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 25 52 e2[ 	]*vpdpwssd ymm20\{k5\},ymm20,ymm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d a5 52 e2[ 	]*vpdpwssd ymm20\{k5\}\{z\},ymm20,ymm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 20 52 a4 f0 23 01 00 00[ 	]*vpdpwssd ymm20,ymm20,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 20 52 62 7f[ 	]*vpdpwssd ymm20,ymm20,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 30 52 62 7f[ 	]*vpdpwssd ymm20,ymm20,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 00 53 f7[ 	]*vpdpwssds xmm22,xmm19,xmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 07 53 f7[ 	]*vpdpwssds xmm22\{k7\},xmm19,xmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 87 53 f7[ 	]*vpdpwssds xmm22\{k7\}\{z\},xmm19,xmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 00 53 b4 f0 23 01 00 00[ 	]*vpdpwssds xmm22,xmm19,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 65 00 53 72 7f[ 	]*vpdpwssds xmm22,xmm19,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 65 10 53 72 7f[ 	]*vpdpwssds xmm22,xmm19,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 82 45 20 53 fc[ 	]*vpdpwssds ymm23,ymm23,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 82 45 23 53 fc[ 	]*vpdpwssds ymm23\{k3\},ymm23,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 82 45 a3 53 fc[ 	]*vpdpwssds ymm23\{k3\}\{z\},ymm23,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 a2 45 20 53 bc f0 23 01 00 00[ 	]*vpdpwssds ymm23,ymm23,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 45 20 53 7a 7f[ 	]*vpdpwssds ymm23,ymm23,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 45 30 53 7a 7f[ 	]*vpdpwssds ymm23,ymm23,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 82 15 00 50 d4[ 	]*vpdpbusd xmm18,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 82 15 03 50 d4[ 	]*vpdpbusd xmm18\{k3\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 82 15 83 50 d4[ 	]*vpdpbusd xmm18\{k3\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 a2 15 00 50 94 f0 23 01 00 00[ 	]*vpdpbusd xmm18,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 15 00 50 52 7f[ 	]*vpdpbusd xmm18,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 15 10 50 52 7f[ 	]*vpdpbusd xmm18,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 20 50 e1[ 	]*vpdpbusd ymm20,ymm18,ymm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 22 50 e1[ 	]*vpdpbusd ymm20\{k2\},ymm18,ymm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d a2 50 e1[ 	]*vpdpbusd ymm20\{k2\}\{z\},ymm18,ymm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 20 50 a4 f0 23 01 00 00[ 	]*vpdpbusd ymm20,ymm18,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 20 50 62 7f[ 	]*vpdpbusd ymm20,ymm18,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 30 50 62 7f[ 	]*vpdpbusd ymm20,ymm18,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 00 51 c3[ 	]*vpdpbusds xmm24,xmm26,xmm27
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 04 51 c3[ 	]*vpdpbusds xmm24\{k4\},xmm26,xmm27
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 84 51 c3[ 	]*vpdpbusds xmm24\{k4\}\{z\},xmm26,xmm27
[ 	]*[a-f0-9]+:[ 	]*62 22 2d 00 51 84 f0 23 01 00 00[ 	]*vpdpbusds xmm24,xmm26,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 00 51 42 7f[ 	]*vpdpbusds xmm24,xmm26,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 10 51 42 7f[ 	]*vpdpbusds xmm24,xmm26,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 51 f1[ 	]*vpdpbusds ymm30,ymm29,ymm25
[ 	]*[a-f0-9]+:[ 	]*62 02 15 21 51 f1[ 	]*vpdpbusds ymm30\{k1\},ymm29,ymm25
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a1 51 f1[ 	]*vpdpbusds ymm30\{k1\}\{z\},ymm29,ymm25
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 51 b4 f0 23 01 00 00[ 	]*vpdpbusds ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 51 72 7f[ 	]*vpdpbusds ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 51 72 7f[ 	]*vpdpbusds ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 00 52 ef[ 	]*vpdpwssd xmm21,xmm20,xmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 06 52 ef[ 	]*vpdpwssd xmm21\{k6\},xmm20,xmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 86 52 ef[ 	]*vpdpwssd xmm21\{k6\}\{z\},xmm20,xmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 00 52 ac f0 34 12 00 00[ 	]*vpdpwssd xmm21,xmm20,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 00 52 6a 7f[ 	]*vpdpwssd xmm21,xmm20,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 10 52 6a 7f[ 	]*vpdpwssd xmm21,xmm20,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 22 25 20 52 c9[ 	]*vpdpwssd ymm25,ymm27,ymm17
[ 	]*[a-f0-9]+:[ 	]*62 22 25 26 52 c9[ 	]*vpdpwssd ymm25\{k6\},ymm27,ymm17
[ 	]*[a-f0-9]+:[ 	]*62 22 25 a6 52 c9[ 	]*vpdpwssd ymm25\{k6\}\{z\},ymm27,ymm17
[ 	]*[a-f0-9]+:[ 	]*62 22 25 20 52 8c f0 34 12 00 00[ 	]*vpdpwssd ymm25,ymm27,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 25 20 52 4a 7f[ 	]*vpdpwssd ymm25,ymm27,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 25 30 52 4a 7f[ 	]*vpdpwssd ymm25,ymm27,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 22 35 00 53 f5[ 	]*vpdpwssds xmm30,xmm25,xmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 35 06 53 f5[ 	]*vpdpwssds xmm30\{k6\},xmm25,xmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 35 86 53 f5[ 	]*vpdpwssds xmm30\{k6\}\{z\},xmm25,xmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 35 00 53 b4 f0 34 12 00 00[ 	]*vpdpwssds xmm30,xmm25,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 35 00 53 72 7f[ 	]*vpdpwssds xmm30,xmm25,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 35 10 53 72 7f[ 	]*vpdpwssds xmm30,xmm25,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 25 20 53 e3[ 	]*vpdpwssds ymm28,ymm27,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 02 25 27 53 e3[ 	]*vpdpwssds ymm28\{k7\},ymm27,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 02 25 a7 53 e3[ 	]*vpdpwssds ymm28\{k7\}\{z\},ymm27,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 22 25 20 53 a4 f0 34 12 00 00[ 	]*vpdpwssds ymm28,ymm27,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 25 20 53 62 7f[ 	]*vpdpwssds ymm28,ymm27,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 25 30 53 62 7f[ 	]*vpdpwssds ymm28,ymm27,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 00 50 d3[ 	]*vpdpbusd xmm26,xmm18,xmm19
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 06 50 d3[ 	]*vpdpbusd xmm26\{k6\},xmm18,xmm19
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 86 50 d3[ 	]*vpdpbusd xmm26\{k6\}\{z\},xmm18,xmm19
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 00 50 94 f0 34 12 00 00[ 	]*vpdpbusd xmm26,xmm18,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 6d 00 50 52 7f[ 	]*vpdpbusd xmm26,xmm18,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 6d 10 50 52 7f[ 	]*vpdpbusd xmm26,xmm18,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 82 75 20 50 eb[ 	]*vpdpbusd ymm21,ymm17,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 82 75 22 50 eb[ 	]*vpdpbusd ymm21\{k2\},ymm17,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 82 75 a2 50 eb[ 	]*vpdpbusd ymm21\{k2\}\{z\},ymm17,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 20 50 ac f0 34 12 00 00[ 	]*vpdpbusd ymm21,ymm17,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 20 50 6a 7f[ 	]*vpdpbusd ymm21,ymm17,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 30 50 6a 7f[ 	]*vpdpbusd ymm21,ymm17,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 00 51 e0[ 	]*vpdpbusds xmm28,xmm26,xmm24
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 01 51 e0[ 	]*vpdpbusds xmm28\{k1\},xmm26,xmm24
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 81 51 e0[ 	]*vpdpbusds xmm28\{k1\}\{z\},xmm26,xmm24
[ 	]*[a-f0-9]+:[ 	]*62 22 2d 00 51 a4 f0 34 12 00 00[ 	]*vpdpbusds xmm28,xmm26,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 00 51 62 7f[ 	]*vpdpbusds xmm28,xmm26,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 10 51 62 7f[ 	]*vpdpbusds xmm28,xmm26,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 82 6d 20 51 fb[ 	]*vpdpbusds ymm23,ymm18,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 82 6d 26 51 fb[ 	]*vpdpbusds ymm23\{k6\},ymm18,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 82 6d a6 51 fb[ 	]*vpdpbusds ymm23\{k6\}\{z\},ymm18,ymm27
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 20 51 bc f0 34 12 00 00[ 	]*vpdpbusds ymm23,ymm18,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 20 51 7a 7f[ 	]*vpdpbusds ymm23,ymm18,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 30 51 7a 7f[ 	]*vpdpbusds ymm23,ymm18,DWORD BCST \[rdx\+0x1fc\]
#pass
