#as:
#objdump: -dwMintel
#name: i386 AVX512ER insns (Intel disassembly)
#source: avx512er.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 f5    	vexp2ps zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 18 c8 f5    	vexp2ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 31    	vexp2ps zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b4 f4 c0 1d fe ff 	vexp2ps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 30    	vexp2ps zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 7f 	vexp2ps zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 00 20 00 00 	vexp2ps zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 80 	vexp2ps zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 c0 df ff ff 	vexp2ps zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 7f 	vexp2ps zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 00 02 00 00 	vexp2ps zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 80 	vexp2ps zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 fc fd ff ff 	vexp2ps zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 f5    	vexp2pd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 18 c8 f5    	vexp2pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 31    	vexp2pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b4 f4 c0 1d fe ff 	vexp2pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 30    	vexp2pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 7f 	vexp2pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 00 20 00 00 	vexp2pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 80 	vexp2pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 c0 df ff ff 	vexp2pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 7f 	vexp2pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 00 04 00 00 	vexp2pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 80 	vexp2pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 f8 fb ff ff 	vexp2pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca f5    	vrcp28ps zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f ca f5    	vrcp28ps zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf ca f5    	vrcp28ps zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 18 ca f5    	vrcp28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 31    	vrcp28ps zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b4 f4 c0 1d fe ff 	vrcp28ps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 30    	vrcp28ps zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 7f 	vrcp28ps zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 00 20 00 00 	vrcp28ps zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 80 	vrcp28ps zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 c0 df ff ff 	vrcp28ps zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 7f 	vrcp28ps zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 00 02 00 00 	vrcp28ps zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 80 	vrcp28ps zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 fc fd ff ff 	vrcp28ps zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca f5    	vrcp28pd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f ca f5    	vrcp28pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf ca f5    	vrcp28pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 18 ca f5    	vrcp28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 31    	vrcp28pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b4 f4 c0 1d fe ff 	vrcp28pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 30    	vrcp28pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 7f 	vrcp28pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 00 20 00 00 	vrcp28pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 80 	vrcp28pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 c0 df ff ff 	vrcp28pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 7f 	vrcp28pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 00 04 00 00 	vrcp28pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 80 	vrcp28pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 f8 fb ff ff 	vrcp28pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 8f cb f4    	vrcp28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 31    	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b4 f4 c0 1d fe ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 7f 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 00 02 00 00 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 80 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 fc fd ff ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 8f cb f4    	vrcp28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 31    	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b4 f4 c0 1d fe ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 7f 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 00 04 00 00 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 80 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 f8 fb ff ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc f5    	vrsqrt28ps zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f cc f5    	vrsqrt28ps zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf cc f5    	vrsqrt28ps zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 18 cc f5    	vrsqrt28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 31    	vrsqrt28ps zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b4 f4 c0 1d fe ff 	vrsqrt28ps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 30    	vrsqrt28ps zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 7f 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 00 20 00 00 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 80 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 c0 df ff ff 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 7f 	vrsqrt28ps zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 00 02 00 00 	vrsqrt28ps zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 80 	vrsqrt28ps zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 fc fd ff ff 	vrsqrt28ps zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc f5    	vrsqrt28pd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f cc f5    	vrsqrt28pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf cc f5    	vrsqrt28pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 18 cc f5    	vrsqrt28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 31    	vrsqrt28pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b4 f4 c0 1d fe ff 	vrsqrt28pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 30    	vrsqrt28pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 7f 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 00 20 00 00 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 80 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 c0 df ff ff 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 7f 	vrsqrt28pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 00 04 00 00 	vrsqrt28pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 80 	vrsqrt28pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 f8 fb ff ff 	vrsqrt28pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 8f cd f4    	vrsqrt28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 31    	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b4 f4 c0 1d fe ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 7f 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 00 02 00 00 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 80 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 fc fd ff ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 8f cd f4    	vrsqrt28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 31    	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b4 f4 c0 1d fe ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 7f 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 00 04 00 00 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 80 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 f8 fb ff ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 f5    	vexp2ps zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 18 c8 f5    	vexp2ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 31    	vexp2ps zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b4 f4 c0 1d fe ff 	vexp2ps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 30    	vexp2ps zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 7f 	vexp2ps zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 00 20 00 00 	vexp2ps zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 80 	vexp2ps zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 c0 df ff ff 	vexp2ps zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 7f 	vexp2ps zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 00 02 00 00 	vexp2ps zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 80 	vexp2ps zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 fc fd ff ff 	vexp2ps zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 f5    	vexp2pd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 18 c8 f5    	vexp2pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 31    	vexp2pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b4 f4 c0 1d fe ff 	vexp2pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 30    	vexp2pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 7f 	vexp2pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 00 20 00 00 	vexp2pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 80 	vexp2pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 c0 df ff ff 	vexp2pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 7f 	vexp2pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 00 04 00 00 	vexp2pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 80 	vexp2pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 f8 fb ff ff 	vexp2pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca f5    	vrcp28ps zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f ca f5    	vrcp28ps zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf ca f5    	vrcp28ps zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 18 ca f5    	vrcp28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 31    	vrcp28ps zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b4 f4 c0 1d fe ff 	vrcp28ps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 30    	vrcp28ps zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 7f 	vrcp28ps zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 00 20 00 00 	vrcp28ps zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 80 	vrcp28ps zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 c0 df ff ff 	vrcp28ps zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 7f 	vrcp28ps zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 00 02 00 00 	vrcp28ps zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 80 	vrcp28ps zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 fc fd ff ff 	vrcp28ps zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca f5    	vrcp28pd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f ca f5    	vrcp28pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf ca f5    	vrcp28pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 18 ca f5    	vrcp28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 31    	vrcp28pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b4 f4 c0 1d fe ff 	vrcp28pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 30    	vrcp28pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 7f 	vrcp28pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 00 20 00 00 	vrcp28pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 80 	vrcp28pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 c0 df ff ff 	vrcp28pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 7f 	vrcp28pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 00 04 00 00 	vrcp28pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 80 	vrcp28pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 f8 fb ff ff 	vrcp28pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 8f cb f4    	vrcp28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 31    	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b4 f4 c0 1d fe ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 7f 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 00 02 00 00 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 80 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 fc fd ff ff 	vrcp28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 8f cb f4    	vrcp28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 31    	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b4 f4 c0 1d fe ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 7f 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 00 04 00 00 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 80 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 f8 fb ff ff 	vrcp28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc f5    	vrsqrt28ps zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f cc f5    	vrsqrt28ps zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf cc f5    	vrsqrt28ps zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 18 cc f5    	vrsqrt28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 31    	vrsqrt28ps zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b4 f4 c0 1d fe ff 	vrsqrt28ps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 30    	vrsqrt28ps zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 7f 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 00 20 00 00 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 80 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 c0 df ff ff 	vrsqrt28ps zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 7f 	vrsqrt28ps zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 00 02 00 00 	vrsqrt28ps zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 80 	vrsqrt28ps zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 fc fd ff ff 	vrsqrt28ps zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc f5    	vrsqrt28pd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f cc f5    	vrsqrt28pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf cc f5    	vrsqrt28pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 18 cc f5    	vrsqrt28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 31    	vrsqrt28pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b4 f4 c0 1d fe ff 	vrsqrt28pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 30    	vrsqrt28pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 7f 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 00 20 00 00 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 80 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 c0 df ff ff 	vrsqrt28pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 7f 	vrsqrt28pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 00 04 00 00 	vrsqrt28pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 80 	vrsqrt28pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 f8 fb ff ff 	vrsqrt28pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 8f cd f4    	vrsqrt28ss xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 31    	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b4 f4 c0 1d fe ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 7f 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 00 02 00 00 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 80 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 fc fd ff ff 	vrsqrt28ss xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 8f cd f4    	vrsqrt28sd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 31    	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b4 f4 c0 1d fe ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 7f 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 00 04 00 00 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 80 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 f8 fb ff ff 	vrsqrt28sd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\]
#pass
