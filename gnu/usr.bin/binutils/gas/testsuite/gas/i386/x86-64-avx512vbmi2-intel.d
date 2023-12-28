#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512VBMI2 insns (Intel disassembly)
#source: x86-64-avx512vbmi2.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 63 31[ 	]*vpcompressb ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 63 b4 f0 23 01 00 00[ 	]*vpcompressb ZMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 63 72 7e[ 	]*vpcompressb ZMMWORD PTR \[rdx\+0x7e\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 63 ee[ 	]*vpcompressb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 63 ee[ 	]*vpcompressb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 63 ee[ 	]*vpcompressb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 63 31[ 	]*vpcompressw ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 63 b4 f0 23 01 00 00[ 	]*vpcompressw ZMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 63 72 7f[ 	]*vpcompressw ZMMWORD PTR \[rdx\+0xfe\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 63 ee[ 	]*vpcompressw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 63 ee[ 	]*vpcompressw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 63 ee[ 	]*vpcompressw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 62 31[ 	]*vpexpandb zmm30\{k7\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 62 31[ 	]*vpexpandb zmm30\{k7\}\{z\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 62 b4 f0 23 01 00 00[ 	]*vpexpandb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 62 72 7e[ 	]*vpexpandb zmm30,ZMMWORD PTR \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 62 f5[ 	]*vpexpandb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 62 f5[ 	]*vpexpandb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 62 f5[ 	]*vpexpandb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 62 31[ 	]*vpexpandw zmm30\{k7\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 62 31[ 	]*vpexpandw zmm30\{k7\}\{z\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 62 b4 f0 23 01 00 00[ 	]*vpexpandw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 62 72 7f[ 	]*vpexpandw zmm30,ZMMWORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 62 f5[ 	]*vpexpandw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 62 f5[ 	]*vpexpandw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 62 f5[ 	]*vpexpandw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 70 f4[ 	]*vpshldvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 70 f4[ 	]*vpshldvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 70 f4[ 	]*vpshldvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 70 b4 f0 23 01 00 00[ 	]*vpshldvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 70 72 7f[ 	]*vpshldvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 71 f4[ 	]*vpshldvd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 71 f4[ 	]*vpshldvd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 71 f4[ 	]*vpshldvd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 71 b4 f0 23 01 00 00[ 	]*vpshldvd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 71 72 7f[ 	]*vpshldvd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 71 72 7f[ 	]*vpshldvd zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 71 f4[ 	]*vpshldvq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 71 f4[ 	]*vpshldvq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 71 f4[ 	]*vpshldvq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 71 b4 f0 23 01 00 00[ 	]*vpshldvq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 71 72 7f[ 	]*vpshldvq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 71 72 7f[ 	]*vpshldvq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 72 f4[ 	]*vpshrdvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 72 f4[ 	]*vpshrdvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 72 f4[ 	]*vpshrdvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 72 b4 f0 23 01 00 00[ 	]*vpshrdvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 72 72 7f[ 	]*vpshrdvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 73 f4[ 	]*vpshrdvd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 73 f4[ 	]*vpshrdvd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 73 f4[ 	]*vpshrdvd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 73 b4 f0 23 01 00 00[ 	]*vpshrdvd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 73 72 7f[ 	]*vpshrdvd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 73 f4[ 	]*vpshrdvq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 73 f4[ 	]*vpshrdvq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 73 f4[ 	]*vpshrdvq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 73 b4 f0 23 01 00 00[ 	]*vpshrdvq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 73 72 7f[ 	]*vpshrdvq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 70 f4 ab[ 	]*vpshldw zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 70 f4 ab[ 	]*vpshldw zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 70 f4 ab[ 	]*vpshldw zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 70 b4 f0 23 01 00 00 7b[ 	]*vpshldw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 70 72 7f 7b[ 	]*vpshldw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 71 f4 ab[ 	]*vpshldd zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 71 f4 ab[ 	]*vpshldd zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 71 f4 ab[ 	]*vpshldd zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 71 b4 f0 23 01 00 00 7b[ 	]*vpshldd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 71 72 7f 7b[ 	]*vpshldd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 71 f4 ab[ 	]*vpshldq zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 71 f4 ab[ 	]*vpshldq zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 71 f4 ab[ 	]*vpshldq zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 71 b4 f0 23 01 00 00 7b[ 	]*vpshldq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 71 72 7f 7b[ 	]*vpshldq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 71 72 7f 7b[ 	]*vpshldq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 72 f4 ab[ 	]*vpshrdw zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 72 f4 ab[ 	]*vpshrdw zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 72 f4 ab[ 	]*vpshrdw zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 72 b4 f0 23 01 00 00 7b[ 	]*vpshrdw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 72 72 7f 7b[ 	]*vpshrdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 73 f4 ab[ 	]*vpshrdd zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 73 f4 ab[ 	]*vpshrdd zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 73 f4 ab[ 	]*vpshrdd zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 73 31 7b[ 	]*vpshrdd zmm30,zmm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 73 72 7f 7b[ 	]*vpshrdd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 73 f4 ab[ 	]*vpshrdq zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 73 f4 ab[ 	]*vpshrdq zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 73 f4 ab[ 	]*vpshrdq zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 73 31 7b[ 	]*vpshrdq zmm30,zmm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 73 72 7f 7b[ 	]*vpshrdq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 63 31[ 	]*vpcompressb ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 63 b4 f0 34 12 00 00[ 	]*vpcompressb ZMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 63 72 7e[ 	]*vpcompressb ZMMWORD PTR \[rdx\+0x7e\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 63 ee[ 	]*vpcompressb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 63 ee[ 	]*vpcompressb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 63 ee[ 	]*vpcompressb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 63 31[ 	]*vpcompressw ZMMWORD PTR \[rcx\]\{k7\},zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 63 b4 f0 34 12 00 00[ 	]*vpcompressw ZMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 63 72 7f[ 	]*vpcompressw ZMMWORD PTR \[rdx\+0xfe\],zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 63 ee[ 	]*vpcompressw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 63 ee[ 	]*vpcompressw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 63 ee[ 	]*vpcompressw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 62 31[ 	]*vpexpandb zmm30\{k7\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 62 31[ 	]*vpexpandb zmm30\{k7\}\{z\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 62 b4 f0 34 12 00 00[ 	]*vpexpandb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 62 72 7e[ 	]*vpexpandb zmm30,ZMMWORD PTR \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 62 f5[ 	]*vpexpandb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 62 f5[ 	]*vpexpandb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 62 f5[ 	]*vpexpandb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 62 31[ 	]*vpexpandw zmm30\{k7\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 62 31[ 	]*vpexpandw zmm30\{k7\}\{z\},ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 62 b4 f0 34 12 00 00[ 	]*vpexpandw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 62 72 7f[ 	]*vpexpandw zmm30,ZMMWORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 62 f5[ 	]*vpexpandw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 62 f5[ 	]*vpexpandw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 62 f5[ 	]*vpexpandw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 70 f4[ 	]*vpshldvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 70 f4[ 	]*vpshldvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 70 f4[ 	]*vpshldvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 70 b4 f0 34 12 00 00[ 	]*vpshldvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 70 72 7f[ 	]*vpshldvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 71 f4[ 	]*vpshldvd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 71 f4[ 	]*vpshldvd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 71 f4[ 	]*vpshldvd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 71 b4 f0 34 12 00 00[ 	]*vpshldvd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 71 31[ 	]*vpshldvd zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 71 72 7f[ 	]*vpshldvd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 71 72 7f[ 	]*vpshldvd zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 71 f4[ 	]*vpshldvq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 71 f4[ 	]*vpshldvq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 71 f4[ 	]*vpshldvq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 71 b4 f0 34 12 00 00[ 	]*vpshldvq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 71 72 7f[ 	]*vpshldvq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 71 72 7f[ 	]*vpshldvq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 72 f4[ 	]*vpshrdvw zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 72 f4[ 	]*vpshrdvw zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 72 f4[ 	]*vpshrdvw zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 72 b4 f0 34 12 00 00[ 	]*vpshrdvw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 72 72 7f[ 	]*vpshrdvw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 73 f4[ 	]*vpshrdvd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 73 f4[ 	]*vpshrdvd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 73 f4[ 	]*vpshrdvd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 73 b4 f0 34 12 00 00[ 	]*vpshrdvd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 73 72 7f[ 	]*vpshrdvd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 73 72 7f[ 	]*vpshrdvd zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 73 f4[ 	]*vpshrdvq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 73 f4[ 	]*vpshrdvq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 73 f4[ 	]*vpshrdvq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 73 b4 f0 34 12 00 00[ 	]*vpshrdvq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 73 72 7f[ 	]*vpshrdvq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 73 72 7f[ 	]*vpshrdvq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 70 f4 ab[ 	]*vpshldw zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 70 f4 ab[ 	]*vpshldw zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 70 f4 ab[ 	]*vpshldw zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 70 b4 f0 34 12 00 00 7b[ 	]*vpshldw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 70 72 7f 7b[ 	]*vpshldw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 71 f4 ab[ 	]*vpshldd zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 71 f4 ab[ 	]*vpshldd zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 71 f4 ab[ 	]*vpshldd zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 71 b4 f0 34 12 00 00 7b[ 	]*vpshldd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 71 72 7f 7b[ 	]*vpshldd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 71 72 7f 7b[ 	]*vpshldd zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 71 f4 ab[ 	]*vpshldq zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 71 f4 ab[ 	]*vpshldq zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 71 f4 ab[ 	]*vpshldq zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 71 b4 f0 34 12 00 00 7b[ 	]*vpshldq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 71 72 7f 7b[ 	]*vpshldq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 71 72 7f 7b[ 	]*vpshldq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 72 f4 ab[ 	]*vpshrdw zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 72 f4 ab[ 	]*vpshrdw zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 72 f4 ab[ 	]*vpshrdw zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 72 b4 f0 34 12 00 00 7b[ 	]*vpshrdw zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 72 72 7f 7b[ 	]*vpshrdw zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 73 f4 ab[ 	]*vpshrdd zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 73 f4 ab[ 	]*vpshrdd zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 73 f4 ab[ 	]*vpshrdd zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 73 72 7f 7b[ 	]*vpshrdd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 73 72 7f 7b[ 	]*vpshrdd zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 73 f4 ab[ 	]*vpshrdq zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 73 f4 ab[ 	]*vpshrdq zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 73 f4 ab[ 	]*vpshrdq zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 73 72 7f 7b[ 	]*vpshrdq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 73 72 7f 7b[ 	]*vpshrdq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
#pass
