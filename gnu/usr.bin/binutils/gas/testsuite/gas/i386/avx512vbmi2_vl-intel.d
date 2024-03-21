#as:
#objdump: -dw -Mintel
#name: i386 AVX512VBMI2/VL insns (Intel disassembly)
#source: avx512vbmi2_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 72 7e[ 	]*vpcompressb XMMWORD PTR \[edx\+0x7e\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 72 7e[ 	]*vpcompressb YMMWORD PTR \[edx\+0x7e\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 ee[ 	]*vpcompressb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 63 ee[ 	]*vpcompressb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 ee[ 	]*vpcompressb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 63 ee[ 	]*vpcompressb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 72 40[ 	]*vpcompressw XMMWORD PTR \[edx\+0x80\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 72 40[ 	]*vpcompressw YMMWORD PTR \[edx\+0x80\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 ee[ 	]*vpcompressw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 63 ee[ 	]*vpcompressw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 ee[ 	]*vpcompressw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 63 ee[ 	]*vpcompressw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 31[ 	]*vpexpandb xmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 72 7e[ 	]*vpexpandb xmm6\{k7\},XMMWORD PTR \[edx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 31[ 	]*vpexpandb ymm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 72 7e[ 	]*vpexpandb ymm6\{k7\},YMMWORD PTR \[edx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 f5[ 	]*vpexpandb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 f5[ 	]*vpexpandb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 f5[ 	]*vpexpandb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 f5[ 	]*vpexpandb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 31[ 	]*vpexpandw xmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 72 40[ 	]*vpexpandw xmm6\{k7\},XMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 31[ 	]*vpexpandw ymm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 72 40[ 	]*vpexpandw ymm6\{k7\},YMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 f5[ 	]*vpexpandw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 f5[ 	]*vpexpandw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 f5[ 	]*vpexpandw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 f5[ 	]*vpexpandw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 f4[ 	]*vpshldvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 70 f4[ 	]*vpshldvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 72 7f[ 	]*vpshldvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 f4[ 	]*vpshldvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 70 f4[ 	]*vpshldvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 72 7f[ 	]*vpshldvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 f4[ 	]*vpshldvd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 71 f4[ 	]*vpshldvd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 72 7f[ 	]*vpshldvd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 71 72 7f[ 	]*vpshldvd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 f4[ 	]*vpshldvd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 71 f4[ 	]*vpshldvd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 72 7f[ 	]*vpshldvd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 71 72 7f[ 	]*vpshldvd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 f4[ 	]*vpshldvq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 71 f4[ 	]*vpshldvq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 72 7f[ 	]*vpshldvq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 71 72 7f[ 	]*vpshldvq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 f4[ 	]*vpshldvq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 71 f4[ 	]*vpshldvq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 72 7f[ 	]*vpshldvq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 71 72 7f[ 	]*vpshldvq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 f4[ 	]*vpshrdvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 72 f4[ 	]*vpshrdvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 72 7f[ 	]*vpshrdvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 f4[ 	]*vpshrdvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 72 f4[ 	]*vpshrdvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 72 7f[ 	]*vpshrdvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 f4[ 	]*vpshrdvd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 73 f4[ 	]*vpshrdvd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 72 7f[ 	]*vpshrdvd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 73 72 7f[ 	]*vpshrdvd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 f4[ 	]*vpshrdvd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 73 f4[ 	]*vpshrdvd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 72 7f[ 	]*vpshrdvd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 73 72 7f[ 	]*vpshrdvd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 f4[ 	]*vpshrdvq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 73 f4[ 	]*vpshrdvq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 72 7f[ 	]*vpshrdvq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 73 72 7f[ 	]*vpshrdvq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 f4[ 	]*vpshrdvq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 73 f4[ 	]*vpshrdvq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 72 7f[ 	]*vpshrdvq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 73 72 7f[ 	]*vpshrdvq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 f4 ab[ 	]*vpshldw xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 70 f4 ab[ 	]*vpshldw xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 72 7f 7b[ 	]*vpshldw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 f4 ab[ 	]*vpshldw ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 70 f4 ab[ 	]*vpshldw ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 72 7f 7b[ 	]*vpshldw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 f4 ab[ 	]*vpshldd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 71 f4 ab[ 	]*vpshldd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 72 7f 7b[ 	]*vpshldd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 71 72 7f 7b[ 	]*vpshldd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 f4 ab[ 	]*vpshldd ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 71 f4 ab[ 	]*vpshldd ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 72 7f 7b[ 	]*vpshldd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 71 72 7f 7b[ 	]*vpshldd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 f4 ab[ 	]*vpshldq xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 71 f4 ab[ 	]*vpshldq xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 72 7f 7b[ 	]*vpshldq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 71 72 7f 7b[ 	]*vpshldq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 f4 ab[ 	]*vpshldq ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 71 f4 ab[ 	]*vpshldq ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 72 7f 7b[ 	]*vpshldq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 71 72 7f 7b[ 	]*vpshldq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 f4 ab[ 	]*vpshrdw xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 72 f4 ab[ 	]*vpshrdw xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 72 7f 7b[ 	]*vpshrdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 f4 ab[ 	]*vpshrdw ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 72 f4 ab[ 	]*vpshrdw ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 72 7f 7b[ 	]*vpshrdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 f4 ab[ 	]*vpshrdd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 73 f4 ab[ 	]*vpshrdd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 72 7f 7b[ 	]*vpshrdd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 73 72 7f 7b[ 	]*vpshrdd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 f4 ab[ 	]*vpshrdd ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 73 f4 ab[ 	]*vpshrdd ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 72 7f 7b[ 	]*vpshrdd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 73 72 7f 7b[ 	]*vpshrdd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 f4 ab[ 	]*vpshrdq xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 73 f4 ab[ 	]*vpshrdq xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 72 7f 7b[ 	]*vpshrdq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 73 72 7f 7b[ 	]*vpshrdq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 f4 ab[ 	]*vpshrdq ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 73 f4 ab[ 	]*vpshrdq ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 72 7f 7b[ 	]*vpshrdq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 73 72 7f 7b[ 	]*vpshrdq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 72 7e[ 	]*vpcompressb XMMWORD PTR \[edx\+0x7e\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 72 7e[ 	]*vpcompressb YMMWORD PTR \[edx\+0x7e\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 ee[ 	]*vpcompressb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 63 ee[ 	]*vpcompressb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 ee[ 	]*vpcompressb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 63 ee[ 	]*vpcompressb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 72 40[ 	]*vpcompressw XMMWORD PTR \[edx\+0x80\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 72 40[ 	]*vpcompressw YMMWORD PTR \[edx\+0x80\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 ee[ 	]*vpcompressw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 63 ee[ 	]*vpcompressw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 ee[ 	]*vpcompressw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 63 ee[ 	]*vpcompressw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 31[ 	]*vpexpandb xmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 72 7e[ 	]*vpexpandb xmm6\{k7\},XMMWORD PTR \[edx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 31[ 	]*vpexpandb ymm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 72 7e[ 	]*vpexpandb ymm6\{k7\},YMMWORD PTR \[edx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 f5[ 	]*vpexpandb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 f5[ 	]*vpexpandb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 f5[ 	]*vpexpandb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 f5[ 	]*vpexpandb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 31[ 	]*vpexpandw xmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 72 40[ 	]*vpexpandw xmm6\{k7\},XMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 31[ 	]*vpexpandw ymm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 72 40[ 	]*vpexpandw ymm6\{k7\},YMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 f5[ 	]*vpexpandw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 f5[ 	]*vpexpandw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 f5[ 	]*vpexpandw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 f5[ 	]*vpexpandw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 f4[ 	]*vpshldvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 70 f4[ 	]*vpshldvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 72 7f[ 	]*vpshldvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 f4[ 	]*vpshldvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 70 f4[ 	]*vpshldvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 72 7f[ 	]*vpshldvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 f4[ 	]*vpshldvd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 71 f4[ 	]*vpshldvd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 72 7f[ 	]*vpshldvd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 71 72 7f[ 	]*vpshldvd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 f4[ 	]*vpshldvd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 71 f4[ 	]*vpshldvd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 72 7f[ 	]*vpshldvd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 71 72 7f[ 	]*vpshldvd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 f4[ 	]*vpshldvq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 71 f4[ 	]*vpshldvq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 72 7f[ 	]*vpshldvq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 71 72 7f[ 	]*vpshldvq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 f4[ 	]*vpshldvq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 71 f4[ 	]*vpshldvq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 72 7f[ 	]*vpshldvq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 71 72 7f[ 	]*vpshldvq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 f4[ 	]*vpshrdvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 72 f4[ 	]*vpshrdvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 72 7f[ 	]*vpshrdvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 f4[ 	]*vpshrdvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 72 f4[ 	]*vpshrdvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 72 7f[ 	]*vpshrdvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 f4[ 	]*vpshrdvd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 73 f4[ 	]*vpshrdvd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 72 7f[ 	]*vpshrdvd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 73 72 7f[ 	]*vpshrdvd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 f4[ 	]*vpshrdvd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 73 f4[ 	]*vpshrdvd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 72 7f[ 	]*vpshrdvd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 73 72 7f[ 	]*vpshrdvd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 f4[ 	]*vpshrdvq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 73 f4[ 	]*vpshrdvq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 72 7f[ 	]*vpshrdvq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 73 72 7f[ 	]*vpshrdvq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 f4[ 	]*vpshrdvq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 73 f4[ 	]*vpshrdvq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 72 7f[ 	]*vpshrdvq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 73 72 7f[ 	]*vpshrdvq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 f4 ab[ 	]*vpshldw xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 70 f4 ab[ 	]*vpshldw xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 72 7f 7b[ 	]*vpshldw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 f4 ab[ 	]*vpshldw ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 70 f4 ab[ 	]*vpshldw ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 72 7f 7b[ 	]*vpshldw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 f4 ab[ 	]*vpshldd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 71 f4 ab[ 	]*vpshldd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 72 7f 7b[ 	]*vpshldd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 71 72 7f 7b[ 	]*vpshldd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 f4 ab[ 	]*vpshldd ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 71 f4 ab[ 	]*vpshldd ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 72 7f 7b[ 	]*vpshldd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 71 72 7f 7b[ 	]*vpshldd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 f4 ab[ 	]*vpshldq xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 71 f4 ab[ 	]*vpshldq xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 72 7f 7b[ 	]*vpshldq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 71 72 7f 7b[ 	]*vpshldq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 f4 ab[ 	]*vpshldq ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 71 f4 ab[ 	]*vpshldq ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 72 7f 7b[ 	]*vpshldq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 71 72 7f 7b[ 	]*vpshldq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 f4 ab[ 	]*vpshrdw xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 72 f4 ab[ 	]*vpshrdw xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 72 7f 7b[ 	]*vpshrdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 f4 ab[ 	]*vpshrdw ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 72 f4 ab[ 	]*vpshrdw ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 72 7f 7b[ 	]*vpshrdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 f4 ab[ 	]*vpshrdd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 73 f4 ab[ 	]*vpshrdd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 72 7f 7b[ 	]*vpshrdd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 73 72 7f 7b[ 	]*vpshrdd xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 f4 ab[ 	]*vpshrdd ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 73 f4 ab[ 	]*vpshrdd ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 72 7f 7b[ 	]*vpshrdd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 73 72 7f 7b[ 	]*vpshrdd ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 f4 ab[ 	]*vpshrdq xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 73 f4 ab[ 	]*vpshrdq xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 72 7f 7b[ 	]*vpshrdq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 73 72 7f 7b[ 	]*vpshrdq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 f4 ab[ 	]*vpshrdq ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 73 f4 ab[ 	]*vpshrdq ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 72 7f 7b[ 	]*vpshrdq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 73 72 7f 7b[ 	]*vpshrdq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
#pass
