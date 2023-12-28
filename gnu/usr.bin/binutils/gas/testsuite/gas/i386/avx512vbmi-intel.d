#as:
#objdump: -dw -Mintel
#name: i386 AVX512VBMI insns (Intel disassembly)
#source: avx512vbmi.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d f4[ 	]*vpermb zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8d f4[ 	]*vpermb zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 8d f4[ 	]*vpermb zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 31[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b4 f4 c0 1d fe ff[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 7f[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 00 20 00 00[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 80[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 c0 df ff ff[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 f4[ 	]*vpermi2b zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 75 f4[ 	]*vpermi2b zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 75 f4[ 	]*vpermi2b zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 31[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 7f[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 00 20 00 00[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 80[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 c0 df ff ff[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d f4[ 	]*vpermt2b zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 7d f4[ 	]*vpermt2b zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 7d f4[ 	]*vpermt2b zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 31[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 7f[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 00 20 00 00[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 80[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 c0 df ff ff[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 f4[ 	]*vpmultishiftqb zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 83 f4[ 	]*vpmultishiftqb zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 83 f4[ 	]*vpmultishiftqb zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 31[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 30[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 7f[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 00 20 00 00[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 80[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 c0 df ff ff[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 7f[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 00 04 00 00[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 80[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d f4[ 	]*vpermb zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8d f4[ 	]*vpermb zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 8d f4[ 	]*vpermb zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 31[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b4 f4 c0 1d fe ff[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 7f[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 00 20 00 00[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 80[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 c0 df ff ff[ 	]*vpermb zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 f4[ 	]*vpermi2b zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 75 f4[ 	]*vpermi2b zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 75 f4[ 	]*vpermi2b zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 31[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 7f[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 00 20 00 00[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 80[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 c0 df ff ff[ 	]*vpermi2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d f4[ 	]*vpermt2b zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 7d f4[ 	]*vpermt2b zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 7d f4[ 	]*vpermt2b zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 31[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 7f[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 00 20 00 00[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 80[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 c0 df ff ff[ 	]*vpermt2b zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 f4[ 	]*vpmultishiftqb zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 83 f4[ 	]*vpmultishiftqb zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 83 f4[ 	]*vpmultishiftqb zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 31[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 30[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 7f[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 00 20 00 00[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 80[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 c0 df ff ff[ 	]*vpmultishiftqb zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 7f[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 00 04 00 00[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 80[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb zmm6,zmm5,QWORD BCST \[edx-0x408\]
#pass
