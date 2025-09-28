#as: -mevexwig=1
#objdump: -dwMintel
#name: i386 AVX512 wig insns (Intel disassembly)
#source: evex-wig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f1 fe 08 2a c0    	\{evex\} vcvtsi2ss xmm0,xmm0,eax
[ 	]*[a-f0-9]+:	62 f1 fe 08 2a 40 01 	\{evex\} vcvtsi2ss xmm0,xmm0,DWORD PTR \[eax\+0x4\]
[ 	]*[a-f0-9]+:	62 f1 ff 08 2a c0    	\{evex\} vcvtsi2sd xmm0,xmm0,eax
[ 	]*[a-f0-9]+:	62 f1 ff 08 2a 40 01 	\{evex\} vcvtsi2sd xmm0,xmm0,DWORD PTR \[eax\+0x4\]
[ 	]*[a-f0-9]+:	62 f1 fe 08 2d c0    	\{evex\} vcvtss2si eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 2d c0    	\{evex\} vcvtsd2si eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 2c c0    	\{evex\} vcvttss2si eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 2c c0    	\{evex\} vcvttsd2si eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 7b c0    	vcvtusi2ss xmm0,xmm0,eax
[ 	]*[a-f0-9]+:	62 f1 fe 08 7b 40 01 	vcvtusi2ss xmm0,xmm0,DWORD PTR \[eax\+0x4\]
[ 	]*[a-f0-9]+:	62 f1 ff 08 7b c0    	vcvtusi2sd xmm0,xmm0,eax
[ 	]*[a-f0-9]+:	62 f1 ff 08 7b 40 01 	vcvtusi2sd xmm0,xmm0,DWORD PTR \[eax\+0x4\]
[ 	]*[a-f0-9]+:	62 f1 fe 08 79 c0    	vcvtss2usi eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 79 c0    	vcvtsd2usi eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 78 c0    	vcvttss2usi eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 78 c0    	vcvttsd2usi eax,xmm0
[ 	]*[a-f0-9]+:	62 f3 fd 08 17 c0 00 	\{evex\} vextractps eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 17 40 01 00 	\{evex\} vextractps DWORD PTR \[eax\+0x4\],xmm0,0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 6e c0    	\{evex\} vmovd xmm0,eax
[ 	]*[a-f0-9]+:	62 f1 fd 08 6e 40 01 	\{evex\} vmovd xmm0,DWORD PTR \[eax\+0x4\]
[ 	]*[a-f0-9]+:	62 f1 fd 08 7e c0    	\{evex\} vmovd eax,xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 7e 40 01 	\{evex\} vmovd DWORD PTR \[eax\+0x4\],xmm0
[ 	]*[a-f0-9]+:	62 f2 fd 08 7c c0    	vpbroadcastd xmm0,eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 c0 00 	\{evex\} vpextrb eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 40 01 00 	\{evex\} vpextrb BYTE PTR \[eax\+0x1\],xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 16 c0 00 	\{evex\} vpextrd eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 16 40 01 00 	\{evex\} vpextrd DWORD PTR \[eax\+0x4\],xmm0,0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c5 c0 00 	\{evex\} vpextrw eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 c0 00 	\{evex\} vpextrw eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 40 01 00 	\{evex\} vpextrw WORD PTR \[eax\+0x2\],xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 c0 00 	\{evex\} vpinsrb xmm0,xmm0,eax,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 40 01 00 	\{evex\} vpinsrb xmm0,xmm0,BYTE PTR \[eax\+0x1\],0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 22 c0 00 	\{evex\} vpinsrd xmm0,xmm0,eax,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 22 40 01 00 	\{evex\} vpinsrd xmm0,xmm0,DWORD PTR \[eax\+0x4\],0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 c0 00 	\{evex\} vpinsrw xmm0,xmm0,eax,0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 40 01 00 	\{evex\} vpinsrw xmm0,xmm0,WORD PTR \[eax\+0x2\],0x0
[ 	]*[a-f0-9]+:	62 f1 7e 0f 10 c0    	vmovss xmm0\{k7\},xmm0,xmm0
[ 	]*[a-f0-9]+:	62 f1 7e 0f 10 00    	vmovss xmm0\{k7\},DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	62 f1 7e 0f 11 00    	vmovss DWORD PTR \[eax\]\{k7\},xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 0f 10 c0    	vmovsd xmm0\{k7\},xmm0,xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 0f 10 00    	vmovsd xmm0\{k7\},QWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	62 f1 ff 0f 11 00    	vmovsd QWORD PTR \[eax\]\{k7\},xmm0
[ 	]*[a-f0-9]+:	62 f5 7e 0f 10 c0    	vmovsh xmm0\{k7\},xmm0,xmm0
[ 	]*[a-f0-9]+:	62 f5 7e 0f 10 00    	vmovsh xmm0\{k7\},WORD PTR \[eax\]
[ 	]*[a-f0-9]+:	62 f5 7e 0f 11 00    	vmovsh WORD PTR \[eax\]\{k7\},xmm0
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 f5    	vpmovsxbd zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 21 f5    	vpmovsxbd zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 31    	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b4 f4 c0 1d fe ff 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 7f 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 00 08 00 00 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 80 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 f5    	vpmovsxbq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 22 f5    	vpmovsxbq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 31    	vpmovsxbq zmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b4 f4 c0 1d fe ff 	vpmovsxbq zmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 7f 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 00 04 00 00 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 80 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 f5    	vpmovsxwd zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 23 f5    	vpmovsxwd zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 31    	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b4 f4 c0 1d fe ff 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 7f 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 00 10 00 00 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 80 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 f5    	vpmovsxwq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 24 f5    	vpmovsxwq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 31    	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b4 f4 c0 1d fe ff 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 7f 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 00 08 00 00 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 80 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 f5    	vpmovzxbd zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 31 f5    	vpmovzxbd zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 31    	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b4 f4 c0 1d fe ff 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 7f 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 00 08 00 00 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 80 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 f5    	vpmovzxbq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 32 f5    	vpmovzxbq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 31    	vpmovzxbq zmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b4 f4 c0 1d fe ff 	vpmovzxbq zmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 7f 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 00 04 00 00 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 80 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 f5    	vpmovzxwd zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 33 f5    	vpmovzxwd zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 31    	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b4 f4 c0 1d fe ff 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 7f 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 00 10 00 00 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 80 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 f5    	vpmovzxwq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 34 f5    	vpmovzxwq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 31    	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b4 f4 c0 1d fe ff 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 7f 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 00 08 00 00 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 80 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 f5    	vpmovsxbd zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 21 f5    	vpmovsxbd zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 31    	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b4 f4 c0 1d fe ff 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 7f 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 00 08 00 00 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 80 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 f5    	vpmovsxbq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 22 f5    	vpmovsxbq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 31    	vpmovsxbq zmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b4 f4 c0 1d fe ff 	vpmovsxbq zmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 7f 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 00 04 00 00 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 80 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq zmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 f5    	vpmovsxwd zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 23 f5    	vpmovsxwd zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 31    	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b4 f4 c0 1d fe ff 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 7f 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 00 10 00 00 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 80 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 f5    	vpmovsxwq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 24 f5    	vpmovsxwq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 31    	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b4 f4 c0 1d fe ff 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 7f 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 00 08 00 00 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 80 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 f5    	vpmovzxbd zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 31 f5    	vpmovzxbd zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 31    	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b4 f4 c0 1d fe ff 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 7f 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 00 08 00 00 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 80 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 f5    	vpmovzxbq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 32 f5    	vpmovzxbq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 31    	vpmovzxbq zmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b4 f4 c0 1d fe ff 	vpmovzxbq zmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 7f 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 00 04 00 00 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 80 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq zmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 f5    	vpmovzxwd zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 33 f5    	vpmovzxwd zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 31    	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b4 f4 c0 1d fe ff 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 7f 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 00 10 00 00 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 80 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 f5    	vpmovzxwq zmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 34 f5    	vpmovzxwq zmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 31    	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b4 f4 c0 1d fe ff 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 7f 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 00 08 00 00 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 80 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq zmm6\{k7\},XMMWORD PTR \[edx-0x810\]
#pass
