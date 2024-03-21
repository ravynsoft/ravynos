#as:
#objdump: -dw -Mintel
#name: i386 AVX512BITALG insns (Intel disassembly)
#source: avx512bitalg.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ec[ 	]*vpshufbitqmb k5,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8f ec[ 	]*vpshufbitqmb k5\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb k5,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f 6a 7f[ 	]*vpshufbitqmb k5,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 f5[ 	]*vpopcntb zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 54 f5[ 	]*vpopcntb zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 54 f5[ 	]*vpopcntb zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 72 7f[ 	]*vpopcntb zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 f5[ 	]*vpopcntw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 54 f5[ 	]*vpopcntw zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 54 f5[ 	]*vpopcntw zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 72 7f[ 	]*vpopcntw zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ec[ 	]*vpshufbitqmb k5,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8f ec[ 	]*vpshufbitqmb k5\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb k5,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f 6a 7f[ 	]*vpshufbitqmb k5,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 f5[ 	]*vpopcntb zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 54 f5[ 	]*vpopcntb zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 54 f5[ 	]*vpopcntb zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 72 7f[ 	]*vpopcntb zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 f5[ 	]*vpopcntw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 54 f5[ 	]*vpopcntw zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 54 f5[ 	]*vpopcntw zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 72 7f[ 	]*vpopcntw zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd zmm6,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq zmm6,QWORD BCST \[edx\+0x3f8\]
#pass
