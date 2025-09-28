#as:
#objdump: -dw -Mintel
#name: i386 AVX512IFMA insns (Intel disassembly)
#source: avx512ifma.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 f4[ 	]*vpmadd52luq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b4 f4[ 	]*vpmadd52luq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b4 f4[ 	]*vpmadd52luq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 31[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 30[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 7f[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 00 20 00 00[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 80[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 c0 df ff ff[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 7f[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 00 04 00 00[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 80[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 f4[ 	]*vpmadd52huq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b5 f4[ 	]*vpmadd52huq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b5 f4[ 	]*vpmadd52huq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 31[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 30[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 7f[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 00 20 00 00[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 80[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 c0 df ff ff[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 7f[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 00 04 00 00[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 80[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 f4[ 	]*vpmadd52luq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b4 f4[ 	]*vpmadd52luq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b4 f4[ 	]*vpmadd52luq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 31[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 30[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 7f[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 00 20 00 00[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 80[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 c0 df ff ff[ 	]*vpmadd52luq zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 7f[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 00 04 00 00[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 80[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 f4[ 	]*vpmadd52huq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b5 f4[ 	]*vpmadd52huq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b5 f4[ 	]*vpmadd52huq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 31[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 30[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 7f[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 00 20 00 00[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 80[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 c0 df ff ff[ 	]*vpmadd52huq zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 7f[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 00 04 00 00[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 80[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq zmm6,zmm5,QWORD BCST \[edx-0x408\]
#pass
