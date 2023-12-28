#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512VBMI2/VL insns (Intel disassembly)
#source: x86-64-avx512vbmi2_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 63 31[ 	]*vpcompressb XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 63 b4 f0 23 01 00 00[ 	]*vpcompressb XMMWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 63 72 7f[ 	]*vpcompressb XMMWORD PTR \[rdx\+0x7f\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 63 31[ 	]*vpcompressb YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 63 b4 f0 23 01 00 00[ 	]*vpcompressb YMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 63 72 7f[ 	]*vpcompressb YMMWORD PTR \[rdx\+0x7f\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 63 ee[ 	]*vpcompressb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 63 ee[ 	]*vpcompressb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 63 ee[ 	]*vpcompressb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 63 ee[ 	]*vpcompressb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 63 ee[ 	]*vpcompressb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 63 ee[ 	]*vpcompressb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 63 31[ 	]*vpcompressw XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 63 b4 f0 23 01 00 00[ 	]*vpcompressw XMMWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 63 72 7f[ 	]*vpcompressw XMMWORD PTR \[rdx\+0xfe\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 63 31[ 	]*vpcompressw YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 63 b4 f0 23 01 00 00[ 	]*vpcompressw YMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 63 72 7f[ 	]*vpcompressw YMMWORD PTR \[rdx\+0xfe\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 63 ee[ 	]*vpcompressw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 63 ee[ 	]*vpcompressw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 63 ee[ 	]*vpcompressw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 63 ee[ 	]*vpcompressw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 63 ee[ 	]*vpcompressw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 63 ee[ 	]*vpcompressw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 62 31[ 	]*vpexpandb xmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 62 31[ 	]*vpexpandb xmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 62 b4 f0 23 01 00 00[ 	]*vpexpandb xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 62 72 7f[ 	]*vpexpandb xmm30,XMMWORD PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 62 31[ 	]*vpexpandb ymm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 62 31[ 	]*vpexpandb ymm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 62 b4 f0 23 01 00 00[ 	]*vpexpandb ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 62 72 7f[ 	]*vpexpandb ymm30,YMMWORD PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 62 f5[ 	]*vpexpandb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 62 f5[ 	]*vpexpandb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 62 f5[ 	]*vpexpandb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 62 f5[ 	]*vpexpandb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 62 f5[ 	]*vpexpandb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 62 f5[ 	]*vpexpandb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 62 31[ 	]*vpexpandw xmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 8f 62 31[ 	]*vpexpandw xmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 62 b4 f0 23 01 00 00[ 	]*vpexpandw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 62 72 7f[ 	]*vpexpandw xmm30,XMMWORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 62 31[ 	]*vpexpandw ymm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 62 31[ 	]*vpexpandw ymm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 62 b4 f0 23 01 00 00[ 	]*vpexpandw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 62 72 7f[ 	]*vpexpandw ymm30,YMMWORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 62 f5[ 	]*vpexpandw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 62 f5[ 	]*vpexpandw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 62 f5[ 	]*vpexpandw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 62 f5[ 	]*vpexpandw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 62 f5[ 	]*vpexpandw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 62 f5[ 	]*vpexpandw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 70 f4[ 	]*vpshldvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 70 f4[ 	]*vpshldvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 70 f4[ 	]*vpshldvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 70 b4 f0 23 01 00 00[ 	]*vpshldvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 70 72 7f[ 	]*vpshldvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 70 f4[ 	]*vpshldvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 70 f4[ 	]*vpshldvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 70 f4[ 	]*vpshldvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 70 b4 f0 23 01 00 00[ 	]*vpshldvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 70 72 7f[ 	]*vpshldvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 71 f4[ 	]*vpshldvd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 71 f4[ 	]*vpshldvd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 71 f4[ 	]*vpshldvd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 71 b4 f0 23 01 00 00[ 	]*vpshldvd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 71 72 7f[ 	]*vpshldvd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 71 72 7f[ 	]*vpshldvd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 71 f4[ 	]*vpshldvd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 71 f4[ 	]*vpshldvd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 71 f4[ 	]*vpshldvd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 71 b4 f0 23 01 00 00[ 	]*vpshldvd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 71 72 7f[ 	]*vpshldvd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 71 72 7f[ 	]*vpshldvd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 71 f4[ 	]*vpshldvq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 71 f4[ 	]*vpshldvq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 71 f4[ 	]*vpshldvq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 71 b4 f0 23 01 00 00[ 	]*vpshldvq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 71 72 7f[ 	]*vpshldvq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 71 72 7f[ 	]*vpshldvq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 71 f4[ 	]*vpshldvq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 71 f4[ 	]*vpshldvq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 71 f4[ 	]*vpshldvq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 71 b4 f0 23 01 00 00[ 	]*vpshldvq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 71 72 7f[ 	]*vpshldvq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 71 72 7f[ 	]*vpshldvq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 72 f4[ 	]*vpshrdvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 72 f4[ 	]*vpshrdvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 72 f4[ 	]*vpshrdvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 72 b4 f0 23 01 00 00[ 	]*vpshrdvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 72 72 7f[ 	]*vpshrdvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 72 f4[ 	]*vpshrdvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 72 f4[ 	]*vpshrdvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 72 f4[ 	]*vpshrdvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 72 b4 f0 23 01 00 00[ 	]*vpshrdvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 72 72 7f[ 	]*vpshrdvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 73 f4[ 	]*vpshrdvd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 73 f4[ 	]*vpshrdvd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 73 f4[ 	]*vpshrdvd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 73 b4 f0 23 01 00 00[ 	]*vpshrdvd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 73 72 7f[ 	]*vpshrdvd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 73 72 7f[ 	]*vpshrdvd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 73 f4[ 	]*vpshrdvd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 73 f4[ 	]*vpshrdvd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 73 f4[ 	]*vpshrdvd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 73 b4 f0 23 01 00 00[ 	]*vpshrdvd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 73 72 7f[ 	]*vpshrdvd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 73 72 7f[ 	]*vpshrdvd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 73 f4[ 	]*vpshrdvq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 73 f4[ 	]*vpshrdvq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 73 f4[ 	]*vpshrdvq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 73 b4 f0 23 01 00 00[ 	]*vpshrdvq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 73 72 7f[ 	]*vpshrdvq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 73 72 7f[ 	]*vpshrdvq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 73 f4[ 	]*vpshrdvq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 73 f4[ 	]*vpshrdvq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 73 f4[ 	]*vpshrdvq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 73 b4 f0 23 01 00 00[ 	]*vpshrdvq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 73 72 7f[ 	]*vpshrdvq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 73 72 7f[ 	]*vpshrdvq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 70 f4 ab[ 	]*vpshldw xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 70 f4 ab[ 	]*vpshldw xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 70 f4 ab[ 	]*vpshldw xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 70 b4 f0 23 01 00 00 7b[ 	]*vpshldw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 70 72 7f 7b[ 	]*vpshldw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 70 f4 ab[ 	]*vpshldw ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 70 f4 ab[ 	]*vpshldw ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 70 f4 ab[ 	]*vpshldw ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 70 b4 f0 23 01 00 00 7b[ 	]*vpshldw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 70 72 7f 7b[ 	]*vpshldw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 71 f4 ab[ 	]*vpshldd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 71 f4 ab[ 	]*vpshldd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 71 f4 ab[ 	]*vpshldd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 71 b4 f0 23 01 00 00 7b[ 	]*vpshldd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 71 72 7f 7b[ 	]*vpshldd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 71 72 7f 7b[ 	]*vpshldd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 71 f4 ab[ 	]*vpshldd ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 71 f4 ab[ 	]*vpshldd ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 71 f4 ab[ 	]*vpshldd ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 71 b4 f0 23 01 00 00 7b[ 	]*vpshldd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 71 72 7f 7b[ 	]*vpshldd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 71 72 7f 7b[ 	]*vpshldd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 71 f4 ab[ 	]*vpshldq xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 71 f4 ab[ 	]*vpshldq xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 71 f4 ab[ 	]*vpshldq xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 71 b4 f0 23 01 00 00 7b[ 	]*vpshldq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 71 72 7f 7b[ 	]*vpshldq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 71 72 7f 7b[ 	]*vpshldq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 71 f4 ab[ 	]*vpshldq ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 71 f4 ab[ 	]*vpshldq ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 71 f4 ab[ 	]*vpshldq ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 71 b4 f0 23 01 00 00 7b[ 	]*vpshldq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 71 72 7f 7b[ 	]*vpshldq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 71 72 7f 7b[ 	]*vpshldq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 72 f4 ab[ 	]*vpshrdw xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 72 f4 ab[ 	]*vpshrdw xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 72 f4 ab[ 	]*vpshrdw xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 72 b4 f0 23 01 00 00 7b[ 	]*vpshrdw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 72 72 7f 7b[ 	]*vpshrdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 72 f4 ab[ 	]*vpshrdw ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 72 f4 ab[ 	]*vpshrdw ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 72 f4 ab[ 	]*vpshrdw ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 72 b4 f0 23 01 00 00 7b[ 	]*vpshrdw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 72 72 7f 7b[ 	]*vpshrdw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 73 f4 ab[ 	]*vpshrdd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 73 f4 ab[ 	]*vpshrdd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 73 f4 ab[ 	]*vpshrdd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 73 72 7f 7b[ 	]*vpshrdd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 73 72 7f 7b[ 	]*vpshrdd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 73 f4 ab[ 	]*vpshrdd ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 73 f4 ab[ 	]*vpshrdd ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 73 f4 ab[ 	]*vpshrdd ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 73 72 7f 7b[ 	]*vpshrdd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 73 72 7f 7b[ 	]*vpshrdd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 73 f4 ab[ 	]*vpshrdq xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 73 f4 ab[ 	]*vpshrdq xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 73 f4 ab[ 	]*vpshrdq xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 73 72 7f 7b[ 	]*vpshrdq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 73 72 7f 7b[ 	]*vpshrdq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 73 f4 ab[ 	]*vpshrdq ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 73 f4 ab[ 	]*vpshrdq ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 73 f4 ab[ 	]*vpshrdq ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 73 72 7f 7b[ 	]*vpshrdq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 73 72 7f 7b[ 	]*vpshrdq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 63 31[ 	]*vpcompressb XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 63 b4 f0 34 12 00 00[ 	]*vpcompressb XMMWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 63 72 7f[ 	]*vpcompressb XMMWORD PTR \[rdx\+0x7f\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 63 31[ 	]*vpcompressb YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 63 b4 f0 34 12 00 00[ 	]*vpcompressb YMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 63 72 7f[ 	]*vpcompressb YMMWORD PTR \[rdx\+0x7f\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 63 ee[ 	]*vpcompressb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 63 ee[ 	]*vpcompressb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 63 ee[ 	]*vpcompressb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 63 ee[ 	]*vpcompressb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 63 ee[ 	]*vpcompressb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 63 ee[ 	]*vpcompressb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 63 31[ 	]*vpcompressw XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 63 b4 f0 34 12 00 00[ 	]*vpcompressw XMMWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 63 72 7f[ 	]*vpcompressw XMMWORD PTR \[rdx\+0xfe\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 63 31[ 	]*vpcompressw YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 63 b4 f0 34 12 00 00[ 	]*vpcompressw YMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 63 72 7f[ 	]*vpcompressw YMMWORD PTR \[rdx\+0xfe\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 63 ee[ 	]*vpcompressw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 63 ee[ 	]*vpcompressw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 63 ee[ 	]*vpcompressw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 63 ee[ 	]*vpcompressw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 63 ee[ 	]*vpcompressw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 63 ee[ 	]*vpcompressw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 62 31[ 	]*vpexpandb xmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 62 31[ 	]*vpexpandb xmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 62 b4 f0 34 12 00 00[ 	]*vpexpandb xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 62 72 7f[ 	]*vpexpandb xmm30,XMMWORD PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 62 31[ 	]*vpexpandb ymm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 62 31[ 	]*vpexpandb ymm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 62 b4 f0 34 12 00 00[ 	]*vpexpandb ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 62 72 7f[ 	]*vpexpandb ymm30,YMMWORD PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 62 f5[ 	]*vpexpandb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 62 f5[ 	]*vpexpandb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 62 f5[ 	]*vpexpandb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 62 f5[ 	]*vpexpandb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 62 f5[ 	]*vpexpandb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 62 f5[ 	]*vpexpandb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 62 31[ 	]*vpexpandw xmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 8f 62 31[ 	]*vpexpandw xmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 62 b4 f0 34 12 00 00[ 	]*vpexpandw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 62 72 7f[ 	]*vpexpandw xmm30,XMMWORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 62 31[ 	]*vpexpandw ymm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 62 31[ 	]*vpexpandw ymm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 62 b4 f0 34 12 00 00[ 	]*vpexpandw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 62 72 7f[ 	]*vpexpandw ymm30,YMMWORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 62 f5[ 	]*vpexpandw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 62 f5[ 	]*vpexpandw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 62 f5[ 	]*vpexpandw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 62 f5[ 	]*vpexpandw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 62 f5[ 	]*vpexpandw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 62 f5[ 	]*vpexpandw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 70 f4[ 	]*vpshldvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 70 f4[ 	]*vpshldvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 70 f4[ 	]*vpshldvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 70 b4 f0 34 12 00 00[ 	]*vpshldvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 70 72 7f[ 	]*vpshldvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 70 f4[ 	]*vpshldvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 70 f4[ 	]*vpshldvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 70 f4[ 	]*vpshldvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 70 b4 f0 34 12 00 00[ 	]*vpshldvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 70 72 7f[ 	]*vpshldvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 71 f4[ 	]*vpshldvd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 71 f4[ 	]*vpshldvd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 71 f4[ 	]*vpshldvd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 71 b4 f0 34 12 00 00[ 	]*vpshldvd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 71 72 7f[ 	]*vpshldvd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 71 72 7f[ 	]*vpshldvd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 71 f4[ 	]*vpshldvd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 71 f4[ 	]*vpshldvd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 71 f4[ 	]*vpshldvd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 71 b4 f0 34 12 00 00[ 	]*vpshldvd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 71 72 7f[ 	]*vpshldvd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 71 72 7f[ 	]*vpshldvd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 71 f4[ 	]*vpshldvq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 71 f4[ 	]*vpshldvq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 71 f4[ 	]*vpshldvq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 71 b4 f0 34 12 00 00[ 	]*vpshldvq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 71 72 7f[ 	]*vpshldvq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 71 72 7f[ 	]*vpshldvq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 71 f4[ 	]*vpshldvq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 71 f4[ 	]*vpshldvq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 71 f4[ 	]*vpshldvq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 71 b4 f0 34 12 00 00[ 	]*vpshldvq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 71 72 7f[ 	]*vpshldvq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 71 72 7f[ 	]*vpshldvq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 72 f4[ 	]*vpshrdvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 72 f4[ 	]*vpshrdvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 72 f4[ 	]*vpshrdvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 72 b4 f0 34 12 00 00[ 	]*vpshrdvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 72 72 7f[ 	]*vpshrdvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 72 f4[ 	]*vpshrdvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 72 f4[ 	]*vpshrdvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 72 f4[ 	]*vpshrdvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 72 b4 f0 34 12 00 00[ 	]*vpshrdvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 72 72 7f[ 	]*vpshrdvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 73 f4[ 	]*vpshrdvd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 73 f4[ 	]*vpshrdvd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 73 f4[ 	]*vpshrdvd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 73 b4 f0 34 12 00 00[ 	]*vpshrdvd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 73 72 7f[ 	]*vpshrdvd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 73 72 7f[ 	]*vpshrdvd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 73 f4[ 	]*vpshrdvd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 73 f4[ 	]*vpshrdvd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 73 f4[ 	]*vpshrdvd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 73 b4 f0 34 12 00 00[ 	]*vpshrdvd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 73 72 7f[ 	]*vpshrdvd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 73 72 7f[ 	]*vpshrdvd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 73 f4[ 	]*vpshrdvq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 73 f4[ 	]*vpshrdvq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 73 f4[ 	]*vpshrdvq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 73 b4 f0 34 12 00 00[ 	]*vpshrdvq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 73 72 7f[ 	]*vpshrdvq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 73 72 7f[ 	]*vpshrdvq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 73 f4[ 	]*vpshrdvq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 73 f4[ 	]*vpshrdvq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 73 f4[ 	]*vpshrdvq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 73 b4 f0 34 12 00 00[ 	]*vpshrdvq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 73 72 7f[ 	]*vpshrdvq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 73 72 7f[ 	]*vpshrdvq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 70 f4 ab[ 	]*vpshldw xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 70 f4 ab[ 	]*vpshldw xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 70 f4 ab[ 	]*vpshldw xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 70 b4 f0 34 12 00 00 7b[ 	]*vpshldw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 70 72 7f 7b[ 	]*vpshldw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 70 f4 ab[ 	]*vpshldw ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 70 f4 ab[ 	]*vpshldw ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 70 f4 ab[ 	]*vpshldw ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 70 b4 f0 34 12 00 00 7b[ 	]*vpshldw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 70 72 7f 7b[ 	]*vpshldw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 71 f4 ab[ 	]*vpshldd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 71 f4 ab[ 	]*vpshldd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 71 f4 ab[ 	]*vpshldd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 71 b4 f0 34 12 00 00 7b[ 	]*vpshldd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 71 72 7f 7b[ 	]*vpshldd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 71 72 7f 7b[ 	]*vpshldd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 71 f4 ab[ 	]*vpshldd ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 71 f4 ab[ 	]*vpshldd ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 71 f4 ab[ 	]*vpshldd ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 71 b4 f0 34 12 00 00 7b[ 	]*vpshldd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 71 72 7f 7b[ 	]*vpshldd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 71 72 7f 7b[ 	]*vpshldd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 71 f4 ab[ 	]*vpshldq xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 71 f4 ab[ 	]*vpshldq xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 71 f4 ab[ 	]*vpshldq xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 71 b4 f0 34 12 00 00 7b[ 	]*vpshldq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 71 72 7f 7b[ 	]*vpshldq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 71 72 7f 7b[ 	]*vpshldq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 71 f4 ab[ 	]*vpshldq ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 71 f4 ab[ 	]*vpshldq ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 71 f4 ab[ 	]*vpshldq ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 71 b4 f0 34 12 00 00 7b[ 	]*vpshldq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 71 72 7f 7b[ 	]*vpshldq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 71 72 7f 7b[ 	]*vpshldq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 72 f4 ab[ 	]*vpshrdw xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 72 f4 ab[ 	]*vpshrdw xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 72 f4 ab[ 	]*vpshrdw xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 72 b4 f0 34 12 00 00 7b[ 	]*vpshrdw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 72 72 7f 7b[ 	]*vpshrdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 72 f4 ab[ 	]*vpshrdw ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 72 f4 ab[ 	]*vpshrdw ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 72 f4 ab[ 	]*vpshrdw ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 72 b4 f0 34 12 00 00 7b[ 	]*vpshrdw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 72 72 7f 7b[ 	]*vpshrdw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 73 f4 ab[ 	]*vpshrdd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 73 f4 ab[ 	]*vpshrdd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 73 f4 ab[ 	]*vpshrdd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 73 72 7f 7b[ 	]*vpshrdd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 73 72 7f 7b[ 	]*vpshrdd xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 73 f4 ab[ 	]*vpshrdd ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 73 f4 ab[ 	]*vpshrdd ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 73 f4 ab[ 	]*vpshrdd ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 73 72 7f 7b[ 	]*vpshrdd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 73 72 7f 7b[ 	]*vpshrdd ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 73 f4 ab[ 	]*vpshrdq xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 73 f4 ab[ 	]*vpshrdq xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 73 f4 ab[ 	]*vpshrdq xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 73 72 7f 7b[ 	]*vpshrdq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 73 72 7f 7b[ 	]*vpshrdq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 73 f4 ab[ 	]*vpshrdq ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 73 f4 ab[ 	]*vpshrdq ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 73 f4 ab[ 	]*vpshrdq ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 73 72 7f 7b[ 	]*vpshrdq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 73 72 7f 7b[ 	]*vpshrdq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\],0x7b
#pass
