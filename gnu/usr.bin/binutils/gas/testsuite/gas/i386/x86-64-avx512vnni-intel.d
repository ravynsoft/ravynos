#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512VNNI insns (Intel disassembly)
#source: x86-64-avx512vnni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 40 52 d1[ 	]*vpdpwssd zmm18,zmm18,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 45 52 d1[ 	]*vpdpwssd zmm18\{k5\},zmm18,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d c5 52 d1[ 	]*vpdpwssd zmm18\{k5\}\{z\},zmm18,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 40 52 94 f0 23 01 00 00[ 	]*vpdpwssd zmm18,zmm18,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 40 52 52 7f[ 	]*vpdpwssd zmm18,zmm18,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 50 52 52 7f[ 	]*vpdpwssd zmm18,zmm18,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 53 e9[ 	]*vpdpwssds zmm21,zmm21,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 44 53 e9[ 	]*vpdpwssds zmm21\{k4\},zmm21,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 c4 53 e9[ 	]*vpdpwssds zmm21\{k4\}\{z\},zmm21,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 53 ac f0 23 01 00 00[ 	]*vpdpwssds zmm21,zmm21,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 40 53 6a 7f[ 	]*vpdpwssds zmm21,zmm21,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 50 53 6a 7f[ 	]*vpdpwssds zmm21,zmm21,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 50 fa[ 	]*vpdpbusd zmm23,zmm21,zmm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 44 50 fa[ 	]*vpdpbusd zmm23\{k4\},zmm21,zmm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 c4 50 fa[ 	]*vpdpbusd zmm23\{k4\}\{z\},zmm21,zmm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 50 bc f0 23 01 00 00[ 	]*vpdpbusd zmm23,zmm21,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 40 50 7a 7f[ 	]*vpdpbusd zmm23,zmm21,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 50 50 7a 7f[ 	]*vpdpbusd zmm23,zmm21,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 3d 40 51 c1[ 	]*vpdpbusds zmm24,zmm24,zmm25
[ 	]*[a-f0-9]+:[ 	]*62 02 3d 47 51 c1[ 	]*vpdpbusds zmm24\{k7\},zmm24,zmm25
[ 	]*[a-f0-9]+:[ 	]*62 02 3d c7 51 c1[ 	]*vpdpbusds zmm24\{k7\}\{z\},zmm24,zmm25
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 40 51 84 f0 23 01 00 00[ 	]*vpdpbusds zmm24,zmm24,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 40 51 42 7f[ 	]*vpdpbusds zmm24,zmm24,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 50 51 42 7f[ 	]*vpdpbusds zmm24,zmm24,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 22 25 40 52 e1[ 	]*vpdpwssd zmm28,zmm27,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 22 25 47 52 e1[ 	]*vpdpwssd zmm28\{k7\},zmm27,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 22 25 c7 52 e1[ 	]*vpdpwssd zmm28\{k7\}\{z\},zmm27,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 22 25 40 52 a4 f0 34 12 00 00[ 	]*vpdpwssd zmm28,zmm27,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 25 40 52 62 7f[ 	]*vpdpwssd zmm28,zmm27,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 25 50 52 62 7f[ 	]*vpdpwssd zmm28,zmm27,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 22 1d 40 53 e9[ 	]*vpdpwssds zmm29,zmm28,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 22 1d 43 53 e9[ 	]*vpdpwssds zmm29\{k3\},zmm28,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 22 1d c3 53 e9[ 	]*vpdpwssds zmm29\{k3\}\{z\},zmm28,zmm17
[ 	]*[a-f0-9]+:[ 	]*62 22 1d 40 53 ac f0 34 12 00 00[ 	]*vpdpwssds zmm29,zmm28,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 1d 40 53 6a 7f[ 	]*vpdpwssds zmm29,zmm28,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 1d 50 53 6a 7f[ 	]*vpdpwssds zmm29,zmm28,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 40 50 e5[ 	]*vpdpbusd zmm28,zmm24,zmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 46 50 e5[ 	]*vpdpbusd zmm28\{k6\},zmm24,zmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 3d c6 50 e5[ 	]*vpdpbusd zmm28\{k6\}\{z\},zmm24,zmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 40 50 a4 f0 34 12 00 00[ 	]*vpdpbusd zmm28,zmm24,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 40 50 62 7f[ 	]*vpdpbusd zmm28,zmm24,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 50 50 62 7f[ 	]*vpdpbusd zmm28,zmm24,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 40 51 e4[ 	]*vpdpbusds zmm20,zmm17,zmm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 42 51 e4[ 	]*vpdpbusds zmm20\{k2\},zmm17,zmm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 c2 51 e4[ 	]*vpdpbusds zmm20\{k2\}\{z\},zmm17,zmm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 40 51 a4 f0 34 12 00 00[ 	]*vpdpbusds zmm20,zmm17,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 40 51 62 7f[ 	]*vpdpbusds zmm20,zmm17,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 50 51 62 7f[ 	]*vpdpbusds zmm20,zmm17,DWORD BCST \[rdx\+0x1fc\]
#pass
