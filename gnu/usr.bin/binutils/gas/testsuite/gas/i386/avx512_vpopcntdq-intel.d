#as:
#objdump: -dw -Mintel
#name: i386 AVX512/VPOPCNTDQ insns (Intel disassembly)
#source: avx512_vpopcntdq.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 31[ 	]*vpopcntd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 30[ 	]*vpopcntd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 80[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 80[ 	]*vpopcntd zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 31[ 	]*vpopcntq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 30[ 	]*vpopcntq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 80[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 80[ 	]*vpopcntq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 31[ 	]*vpopcntd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 30[ 	]*vpopcntd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 30[ 	]*vpopcntd zmm6,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 80[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd zmm6,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 80[ 	]*vpopcntd zmm6,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd zmm6,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 31[ 	]*vpopcntq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 30[ 	]*vpopcntq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 30[ 	]*vpopcntq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 80[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 80[ 	]*vpopcntq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq zmm6,QWORD BCST \[edx-0x408\]
#pass
