#as:
#objdump: -dwMintel
#name: i386 AVX512CD insns (Intel disassembly)
#source: avx512cd.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 f5    	vpconflictd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f c4 f5    	vpconflictd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf c4 f5    	vpconflictd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 31    	vpconflictd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b4 f4 c0 1d fe ff 	vpconflictd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 30    	vpconflictd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 7f 	vpconflictd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 00 20 00 00 	vpconflictd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 80 	vpconflictd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 c0 df ff ff 	vpconflictd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 7f 	vpconflictd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 00 02 00 00 	vpconflictd zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 80 	vpconflictd zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 fc fd ff ff 	vpconflictd zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 f5    	vpconflictq zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f c4 f5    	vpconflictq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf c4 f5    	vpconflictq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 31    	vpconflictq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b4 f4 c0 1d fe ff 	vpconflictq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 30    	vpconflictq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 7f 	vpconflictq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 00 20 00 00 	vpconflictq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 80 	vpconflictq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 c0 df ff ff 	vpconflictq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 7f 	vpconflictq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 00 04 00 00 	vpconflictq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 80 	vpconflictq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 f8 fb ff ff 	vpconflictq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 f5    	vplzcntd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f 44 f5    	vplzcntd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf 44 f5    	vplzcntd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 31    	vplzcntd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b4 f4 c0 1d fe ff 	vplzcntd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 30    	vplzcntd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 7f 	vplzcntd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 00 20 00 00 	vplzcntd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 80 	vplzcntd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 c0 df ff ff 	vplzcntd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 7f 	vplzcntd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 00 02 00 00 	vplzcntd zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 80 	vplzcntd zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 fc fd ff ff 	vplzcntd zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 f5    	vplzcntq zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 44 f5    	vplzcntq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 44 f5    	vplzcntq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 31    	vplzcntq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b4 f4 c0 1d fe ff 	vplzcntq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 30    	vplzcntq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 7f 	vplzcntq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 00 20 00 00 	vplzcntq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 80 	vplzcntq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 c0 df ff ff 	vplzcntq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 7f 	vplzcntq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 00 04 00 00 	vplzcntq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 80 	vplzcntq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 f8 fb ff ff 	vplzcntq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7e 48 3a f6    	vpbroadcastmw2d zmm6,k6
[ 	]*[a-f0-9]+:	62 f2 fe 48 2a f6    	vpbroadcastmb2q zmm6,k6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 f5    	vpconflictd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f c4 f5    	vpconflictd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf c4 f5    	vpconflictd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 31    	vpconflictd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b4 f4 c0 1d fe ff 	vpconflictd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 30    	vpconflictd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 7f 	vpconflictd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 00 20 00 00 	vpconflictd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 80 	vpconflictd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 c0 df ff ff 	vpconflictd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 7f 	vpconflictd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 00 02 00 00 	vpconflictd zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 80 	vpconflictd zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 fc fd ff ff 	vpconflictd zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 f5    	vpconflictq zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f c4 f5    	vpconflictq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf c4 f5    	vpconflictq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 31    	vpconflictq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b4 f4 c0 1d fe ff 	vpconflictq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 30    	vpconflictq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 7f 	vpconflictq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 00 20 00 00 	vpconflictq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 80 	vpconflictq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 c0 df ff ff 	vpconflictq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 7f 	vpconflictq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 00 04 00 00 	vpconflictq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 80 	vpconflictq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 f8 fb ff ff 	vpconflictq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 f5    	vplzcntd zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 4f 44 f5    	vplzcntd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d cf 44 f5    	vplzcntd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 31    	vplzcntd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b4 f4 c0 1d fe ff 	vplzcntd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 30    	vplzcntd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 7f 	vplzcntd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 00 20 00 00 	vplzcntd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 80 	vplzcntd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 c0 df ff ff 	vplzcntd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 7f 	vplzcntd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 00 02 00 00 	vplzcntd zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 80 	vplzcntd zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 fc fd ff ff 	vplzcntd zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 f5    	vplzcntq zmm6,zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 4f 44 f5    	vplzcntq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd cf 44 f5    	vplzcntq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 31    	vplzcntq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b4 f4 c0 1d fe ff 	vplzcntq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 30    	vplzcntq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 7f 	vplzcntq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 00 20 00 00 	vplzcntq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 80 	vplzcntq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 c0 df ff ff 	vplzcntq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 7f 	vplzcntq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 00 04 00 00 	vplzcntq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 80 	vplzcntq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 f8 fb ff ff 	vplzcntq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:	62 f2 7e 48 3a f6    	vpbroadcastmw2d zmm6,k6
[ 	]*[a-f0-9]+:	62 f2 fe 48 2a f6    	vpbroadcastmb2q zmm6,k6
#pass
