#objdump: -dwMintel
#name: i386 AVX2 insns (Intel disassembly)
#source: avx2.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 5d 8c 31       	vpmaskmovd ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 8e 21       	vpmaskmovd YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 dd 8c 31       	vpmaskmovq ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 8e 21       	vpmaskmovq YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 fd 01 d6 07    	vpermpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 01 31 07    	vpermpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 00 d6 07    	vpermq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 00 31 07    	vpermq ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 4d 36 d4       	vpermd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 36 11       	vpermd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 16 d4       	vpermps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 16 11       	vpermps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 47 d4       	vpsllvd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 47 11       	vpsllvd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 47 d4       	vpsllvq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 47 11       	vpsllvq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 46 d4       	vpsravd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 46 11       	vpsravd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 45 d4       	vpsrlvd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 45 11       	vpsrlvd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 45 d4       	vpsrlvq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 45 11       	vpsrlvq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 2a 21       	vmovntdqa ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 19 f4       	vbroadcastsd ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 18 f4       	vbroadcastss ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 4d 02 d4 07    	vpblendd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 02 11 07    	vpblendd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 46 d4 07    	vperm2i128 ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 46 11 07    	vperm2i128 ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 38 f4 07    	vinserti128 ymm6,ymm4,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 38 31 07    	vinserti128 ymm6,ymm4,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 7d 5a 21       	vbroadcasti128 ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 47 d4       	vpsllvd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 47 39       	vpsllvd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 47 d4       	vpsllvq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 47 39       	vpsllvq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 46 d4       	vpsravd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 46 39       	vpsravd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 45 d4       	vpsrlvd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 45 39       	vpsrlvd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 45 d4       	vpsrlvq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 45 39       	vpsrlvq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 8c 31       	vpmaskmovd xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 d9 8c 31       	vpmaskmovq xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 7d 39 e6 07    	vextracti128 xmm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 39 21 07    	vextracti128 XMMWORD PTR \[ecx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e2 49 8e 21       	vpmaskmovd XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 8e 21       	vpmaskmovq XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 02 d4 07    	vpblendd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 02 11 07    	vpblendd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 79 59 f4       	vpbroadcastq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 59 21       	vpbroadcastq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 59 f4       	vpbroadcastq ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 59 21       	vpbroadcastq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 58 e4       	vpbroadcastd ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 21       	vpbroadcastd ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 58 f4       	vpbroadcastd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 58 21       	vpbroadcastd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 79 f4       	vpbroadcastw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 79 21       	vpbroadcastw xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 79 f4       	vpbroadcastw ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 79 21       	vpbroadcastw ymm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 78 f4       	vpbroadcastb xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 78 21       	vpbroadcastb xmm4,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 78 f4       	vpbroadcastb ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 78 21       	vpbroadcastb ymm4,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 f4       	vbroadcastss xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 5d 8c 31       	vpmaskmovd ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 8e 21       	vpmaskmovd YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 8c 31       	vpmaskmovd ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 8e 21       	vpmaskmovd YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 dd 8c 31       	vpmaskmovq ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 8e 21       	vpmaskmovq YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 dd 8c 31       	vpmaskmovq ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 8e 21       	vpmaskmovq YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 fd 01 d6 07    	vpermpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 01 31 07    	vpermpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 01 31 07    	vpermpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 00 d6 07    	vpermq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 00 31 07    	vpermq ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 fd 00 31 07    	vpermq ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 4d 36 d4       	vpermd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 36 11       	vpermd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 36 11       	vpermd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 16 d4       	vpermps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 16 11       	vpermps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 16 11       	vpermps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 47 d4       	vpsllvd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 47 11       	vpsllvd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 47 11       	vpsllvd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 47 d4       	vpsllvq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 47 11       	vpsllvq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 47 11       	vpsllvq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 46 d4       	vpsravd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 46 11       	vpsravd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 46 11       	vpsravd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 45 d4       	vpsrlvd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 45 11       	vpsrlvd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 45 11       	vpsrlvd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 45 d4       	vpsrlvq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 45 11       	vpsrlvq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 45 11       	vpsrlvq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 2a 21       	vmovntdqa ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 2a 21       	vmovntdqa ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 19 f4       	vbroadcastsd ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 18 f4       	vbroadcastss ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 4d 02 d4 07    	vpblendd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 02 11 07    	vpblendd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 02 11 07    	vpblendd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 46 d4 07    	vperm2i128 ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 46 11 07    	vperm2i128 ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 46 11 07    	vperm2i128 ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 38 f4 07    	vinserti128 ymm6,ymm4,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 38 31 07    	vinserti128 ymm6,ymm4,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 38 31 07    	vinserti128 ymm6,ymm4,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 7d 5a 21       	vbroadcasti128 ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 5a 21       	vbroadcasti128 ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 47 d4       	vpsllvd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 47 39       	vpsllvd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 47 39       	vpsllvd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 47 d4       	vpsllvq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 47 39       	vpsllvq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 47 39       	vpsllvq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 46 d4       	vpsravd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 46 39       	vpsravd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 46 39       	vpsravd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 45 d4       	vpsrlvd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 45 39       	vpsrlvd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 45 39       	vpsrlvd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 45 d4       	vpsrlvq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 45 39       	vpsrlvq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 45 39       	vpsrlvq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 8c 31       	vpmaskmovd xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 8c 31       	vpmaskmovd xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 d9 8c 31       	vpmaskmovq xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 d9 8c 31       	vpmaskmovq xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 7d 39 e6 07    	vextracti128 xmm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 39 21 07    	vextracti128 XMMWORD PTR \[ecx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 39 21 07    	vextracti128 XMMWORD PTR \[ecx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e2 49 8e 21       	vpmaskmovd XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 8e 21       	vpmaskmovd XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 8e 21       	vpmaskmovq XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 8e 21       	vpmaskmovq XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 02 d4 07    	vpblendd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 02 11 07    	vpblendd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 02 11 07    	vpblendd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 79 59 f4       	vpbroadcastq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 59 21       	vpbroadcastq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 59 21       	vpbroadcastq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 59 f4       	vpbroadcastq ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 59 21       	vpbroadcastq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 59 21       	vpbroadcastq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 58 e4       	vpbroadcastd ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 21       	vpbroadcastd ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 58 21       	vpbroadcastd ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 58 f4       	vpbroadcastd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 58 21       	vpbroadcastd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 58 21       	vpbroadcastd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 79 f4       	vpbroadcastw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 79 21       	vpbroadcastw xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 79 21       	vpbroadcastw xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 79 f4       	vpbroadcastw ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 79 21       	vpbroadcastw ymm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 79 21       	vpbroadcastw ymm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 78 f4       	vpbroadcastb xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 78 21       	vpbroadcastb xmm4,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 78 21       	vpbroadcastb xmm4,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 78 f4       	vpbroadcastb ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 78 21       	vpbroadcastb ymm4,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 78 21       	vpbroadcastb ymm4,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 f4       	vbroadcastss xmm6,xmm4
#pass
