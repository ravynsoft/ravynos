#as: -mevexwig=1
#objdump: -dwMintel
#name: x86_64 AVX512 wig insns (Intel disassembly)
#source: x86-64-evex-wig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 ab 	vextractps eax,xmm29,0xab
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 7b 	vextractps eax,xmm29,0x7b
[ 	]*[a-f0-9]+:	62 43 fd 08 17 e8 7b 	vextractps r8d,xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 29 7b 	vextractps DWORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 23 fd 08 17 ac f0 23 01 00 00 7b 	vextractps DWORD PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 7f 7b 	vextractps DWORD PTR \[rdx\+0x1fc\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa 00 02 00 00 7b 	vextractps DWORD PTR \[rdx\+0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 80 7b 	vextractps DWORD PTR \[rdx-0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa fc fd ff ff 7b 	vextractps DWORD PTR \[rdx-0x204\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 c0 00 	\{evex\} vpextrb eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 00 00 	\{evex\} vpextrb BYTE PTR \[rax\],xmm0,0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c5 c0 00 	\{evex\} vpextrw eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 c0 00 	\{evex\} vpextrw eax,xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 00 00 	\{evex\} vpextrw WORD PTR \[rax\],xmm0,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 c0 00 	\{evex\} vpinsrb xmm0,xmm0,eax,0x0
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 00 00 	\{evex\} vpinsrb xmm0,xmm0,BYTE PTR \[rax\],0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 c0 00 	\{evex\} vpinsrw xmm0,xmm0,eax,0x0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 00 00 	\{evex\} vpinsrw xmm0,xmm0,WORD PTR \[rax\],0x0
[ 	]*[a-f0-9]+:	62 02 fd 4f 21 f5    	vpmovsxbd zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 21 f5    	vpmovsxbd zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 31    	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 21 b4 f0 23 01 00 00 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 7f 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 00 08 00 00 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 80 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 22 f5    	vpmovsxbq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 22 f5    	vpmovsxbq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 31    	vpmovsxbq zmm30\{k7\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 22 b4 f0 23 01 00 00 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 7f 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 00 04 00 00 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 80 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 23 f5    	vpmovsxwd zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:	62 02 fd cf 23 f5    	vpmovsxwd zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 31    	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 23 b4 f0 23 01 00 00 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 7f 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 00 10 00 00 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 80 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 24 f5    	vpmovsxwq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 24 f5    	vpmovsxwq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 31    	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 24 b4 f0 23 01 00 00 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 7f 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 00 08 00 00 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 80 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 31 f5    	vpmovzxbd zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 31 f5    	vpmovzxbd zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 31    	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 31 b4 f0 23 01 00 00 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 7f 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 00 08 00 00 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 80 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 32 f5    	vpmovzxbq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 32 f5    	vpmovzxbq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 31    	vpmovzxbq zmm30\{k7\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 32 b4 f0 23 01 00 00 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 7f 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 00 04 00 00 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 80 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 33 f5    	vpmovzxwd zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:	62 02 fd cf 33 f5    	vpmovzxwd zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 31    	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 33 b4 f0 23 01 00 00 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 7f 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 00 10 00 00 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 80 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 34 f5    	vpmovzxwq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 34 f5    	vpmovzxwq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 31    	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 34 b4 f0 23 01 00 00 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 7f 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 00 08 00 00 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 80 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 ab 	vextractps eax,xmm29,0xab
[ 	]*[a-f0-9]+:	62 63 fd 08 17 e8 7b 	vextractps eax,xmm29,0x7b
[ 	]*[a-f0-9]+:	62 43 fd 08 17 e8 7b 	vextractps r8d,xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 29 7b 	vextractps DWORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 23 fd 08 17 ac f0 34 12 00 00 7b 	vextractps DWORD PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 7f 7b 	vextractps DWORD PTR \[rdx\+0x1fc\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa 00 02 00 00 7b 	vextractps DWORD PTR \[rdx\+0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 6a 80 7b 	vextractps DWORD PTR \[rdx-0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 63 fd 08 17 aa fc fd ff ff 7b 	vextractps DWORD PTR \[rdx-0x204\],xmm29,0x7b
[ 	]*[a-f0-9]+:	62 02 fd 4f 21 f5    	vpmovsxbd zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 21 f5    	vpmovsxbd zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 31    	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 21 b4 f0 34 12 00 00 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 7f 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 00 08 00 00 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 72 80 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 22 f5    	vpmovsxbq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 22 f5    	vpmovsxbq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 31    	vpmovsxbq zmm30\{k7\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 22 b4 f0 34 12 00 00 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 7f 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 00 04 00 00 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 72 80 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq zmm30\{k7\},QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 23 f5    	vpmovsxwd zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:	62 02 fd cf 23 f5    	vpmovsxwd zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 31    	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 23 b4 f0 34 12 00 00 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 7f 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 00 10 00 00 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 72 80 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 24 f5    	vpmovsxwq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 24 f5    	vpmovsxwq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 31    	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 24 b4 f0 34 12 00 00 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 7f 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 00 08 00 00 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 72 80 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 31 f5    	vpmovzxbd zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 31 f5    	vpmovzxbd zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 31    	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 31 b4 f0 34 12 00 00 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 7f 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 00 08 00 00 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 72 80 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 32 f5    	vpmovzxbq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 32 f5    	vpmovzxbq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 31    	vpmovzxbq zmm30\{k7\},QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 32 b4 f0 34 12 00 00 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 7f 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 00 04 00 00 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 72 80 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq zmm30\{k7\},QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 33 f5    	vpmovzxwd zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:	62 02 fd cf 33 f5    	vpmovzxwd zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 31    	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 33 b4 f0 34 12 00 00 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 7f 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 00 10 00 00 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 72 80 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd zmm30\{k7\},YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:	62 02 fd 4f 34 f5    	vpmovzxwq zmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 34 f5    	vpmovzxwq zmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 31    	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 4f 34 b4 f0 34 12 00 00 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 7f 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 00 08 00 00 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 72 80 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	62 62 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq zmm30\{k7\},XMMWORD PTR \[rdx-0x810\]
#pass
