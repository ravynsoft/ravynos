#as:
#objdump: -dwMintel
#name: x86_64 AVX512CD insns (Intel disassembly)
#source: x86-64-avx512cd.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 02 7d 48 c4 f5    	vpconflictd zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 7d 4f c4 f5    	vpconflictd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 7d cf c4 f5    	vpconflictd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 31    	vpconflictd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 7d 48 c4 b4 f0 23 01 00 00 	vpconflictd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 31    	vpconflictd zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 7f 	vpconflictd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 00 20 00 00 	vpconflictd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 80 	vpconflictd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 c0 df ff ff 	vpconflictd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 7f 	vpconflictd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 00 02 00 00 	vpconflictd zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 80 	vpconflictd zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 fc fd ff ff 	vpconflictd zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 fd 48 c4 f5    	vpconflictq zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 fd 4f c4 f5    	vpconflictq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 fd cf c4 f5    	vpconflictq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 31    	vpconflictq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 48 c4 b4 f0 23 01 00 00 	vpconflictq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 31    	vpconflictq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 7f 	vpconflictq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 00 20 00 00 	vpconflictq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 80 	vpconflictq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 c0 df ff ff 	vpconflictq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 7f 	vpconflictq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 00 04 00 00 	vpconflictq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 80 	vpconflictq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 f8 fb ff ff 	vpconflictq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 7d 48 44 f5    	vplzcntd zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 7d 4f 44 f5    	vplzcntd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 7d cf 44 f5    	vplzcntd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 7d 48 44 31    	vplzcntd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 7d 48 44 b4 f0 23 01 00 00 	vplzcntd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 31    	vplzcntd zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 7f 	vplzcntd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 00 20 00 00 	vplzcntd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 80 	vplzcntd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 c0 df ff ff 	vplzcntd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 7f 	vplzcntd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 00 02 00 00 	vplzcntd zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 80 	vplzcntd zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 fc fd ff ff 	vplzcntd zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 fd 48 44 f5    	vplzcntq zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 fd 4f 44 f5    	vplzcntq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 44 f5    	vplzcntq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 fd 48 44 31    	vplzcntq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 48 44 b4 f0 23 01 00 00 	vplzcntq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 31    	vplzcntq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 7f 	vplzcntq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 00 20 00 00 	vplzcntq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 80 	vplzcntq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 c0 df ff ff 	vplzcntq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 7f 	vplzcntq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 00 04 00 00 	vplzcntq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 80 	vplzcntq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 f8 fb ff ff 	vplzcntq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 62 7e 48 3a f6    	vpbroadcastmw2d zmm30,k6
[ 	]*[a-f0-9]+:	62 62 fe 48 2a f6    	vpbroadcastmb2q zmm30,k6
[ 	]*[a-f0-9]+:	62 02 7d 48 c4 f5    	vpconflictd zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 7d 4f c4 f5    	vpconflictd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 7d cf c4 f5    	vpconflictd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 31    	vpconflictd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 7d 48 c4 b4 f0 34 12 00 00 	vpconflictd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 31    	vpconflictd zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 7f 	vpconflictd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 00 20 00 00 	vpconflictd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 80 	vpconflictd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 c0 df ff ff 	vpconflictd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 7f 	vpconflictd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 00 02 00 00 	vpconflictd zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 80 	vpconflictd zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 fc fd ff ff 	vpconflictd zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 fd 48 c4 f5    	vpconflictq zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 fd 4f c4 f5    	vpconflictq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 fd cf c4 f5    	vpconflictq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 31    	vpconflictq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 48 c4 b4 f0 34 12 00 00 	vpconflictq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 31    	vpconflictq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 7f 	vpconflictq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 00 20 00 00 	vpconflictq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 80 	vpconflictq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 c0 df ff ff 	vpconflictq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 7f 	vpconflictq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 00 04 00 00 	vpconflictq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 80 	vpconflictq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 f8 fb ff ff 	vpconflictq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 02 7d 48 44 f5    	vplzcntd zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 7d 4f 44 f5    	vplzcntd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 7d cf 44 f5    	vplzcntd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 7d 48 44 31    	vplzcntd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 7d 48 44 b4 f0 34 12 00 00 	vplzcntd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 31    	vplzcntd zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 7f 	vplzcntd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 00 20 00 00 	vplzcntd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 80 	vplzcntd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 c0 df ff ff 	vplzcntd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 7f 	vplzcntd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 00 02 00 00 	vplzcntd zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 80 	vplzcntd zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 fc fd ff ff 	vplzcntd zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:	62 02 fd 48 44 f5    	vplzcntq zmm30,zmm29
[ 	]*[a-f0-9]+:	62 02 fd 4f 44 f5    	vplzcntq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:	62 02 fd cf 44 f5    	vplzcntq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:	62 62 fd 48 44 31    	vplzcntq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	62 22 fd 48 44 b4 f0 34 12 00 00 	vplzcntq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 31    	vplzcntq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 7f 	vplzcntq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 00 20 00 00 	vplzcntq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 80 	vplzcntq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 c0 df ff ff 	vplzcntq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 7f 	vplzcntq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 00 04 00 00 	vplzcntq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 80 	vplzcntq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 f8 fb ff ff 	vplzcntq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:	62 62 7e 48 3a f6    	vpbroadcastmw2d zmm30,k6
[ 	]*[a-f0-9]+:	62 62 fe 48 2a f6    	vpbroadcastmb2q zmm30,k6
#pass
