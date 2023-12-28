#objdump: -dw -Mintel
#name: x86_64 AVX512/4VNNIW insns (Intel disassembly)
#source: x86-64-avx512_4vnniw.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 09[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f 52 09[ 	]*vp4dpwssd zmm1\{k7\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf 52 09[ 	]*vp4dpwssd zmm1\{k7\}\{z\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 52 8c f0 c0 1d fe ff[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a e0 0f 00 00[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a 00 10 00 00[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a 00 f0 ff ff[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a e0 ef ff ff[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 09[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f 53 09[ 	]*vp4dpwssds zmm1\{k7\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf 53 09[ 	]*vp4dpwssds zmm1\{k7\}\{z\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 53 8c f0 c0 1d fe ff[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a e0 0f 00 00[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a 00 10 00 00[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a 00 f0 ff ff[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a e0 ef ff ff[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 09[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 09[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f 52 09[ 	]*vp4dpwssd zmm1\{k7\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf 52 09[ 	]*vp4dpwssd zmm1\{k7\}\{z\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 52 8c f0 c0 1d fe ff[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a e0 0f 00 00[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a 00 10 00 00[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a 00 f0 ff ff[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 52 8a e0 ef ff ff[ 	]*vp4dpwssd zmm1,zmm8,XMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 09[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 09[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 4f 53 09[ 	]*vp4dpwssds zmm1\{k7\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f cf 53 09[ 	]*vp4dpwssds zmm1\{k7\}\{z\},zmm8,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 3f 48 53 8c f0 c0 1d fe ff[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a e0 0f 00 00[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a 00 10 00 00[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a 00 f0 ff ff[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 3f 48 53 8a e0 ef ff ff[ 	]*vp4dpwssds zmm1,zmm8,XMMWORD PTR \[rdx-0x1020\]
#pass
