#as:
#objdump: -dw -Mintel
#name: i386 AVX512VNNI insns (Intel disassembly)
#source: avx512vnni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 e3[ 	]*vpdpwssd zmm4,zmm1,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 49 52 e3[ 	]*vpdpwssd zmm4\{k1\},zmm1,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 c9 52 e3[ 	]*vpdpwssd zmm4\{k1\}\{z\},zmm1,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 a4 f4 c0 1d fe ff[ 	]*vpdpwssd zmm4,zmm1,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 62 7f[ 	]*vpdpwssd zmm4,zmm1,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 58 52 62 7f[ 	]*vpdpwssd zmm4,zmm1,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 53 d4[ 	]*vpdpwssds zmm2,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4e 53 d4[ 	]*vpdpwssds zmm2\{k6\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 ce 53 d4[ 	]*vpdpwssds zmm2\{k6\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 53 94 f4 c0 1d fe ff[ 	]*vpdpwssds zmm2,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 53 52 7f[ 	]*vpdpwssds zmm2,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 53 52 7f[ 	]*vpdpwssds zmm2,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 48 50 eb[ 	]*vpdpbusd zmm5,zmm2,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 49 50 eb[ 	]*vpdpbusd zmm5\{k1\},zmm2,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d c9 50 eb[ 	]*vpdpbusd zmm5\{k1\}\{z\},zmm2,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 48 50 ac f4 c0 1d fe ff[ 	]*vpdpbusd zmm5,zmm2,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 48 50 6a 7f[ 	]*vpdpbusd zmm5,zmm2,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 58 50 6a 7f[ 	]*vpdpbusd zmm5,zmm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 48 51 e9[ 	]*vpdpbusds zmm5,zmm3,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 4a 51 e9[ 	]*vpdpbusds zmm5\{k2\},zmm3,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 ca 51 e9[ 	]*vpdpbusds zmm5\{k2\}\{z\},zmm3,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 48 51 ac f4 c0 1d fe ff[ 	]*vpdpbusds zmm5,zmm3,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 48 51 6a 7f[ 	]*vpdpbusds zmm5,zmm3,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 58 51 6a 7f[ 	]*vpdpbusds zmm5,zmm3,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 52 d9[ 	]*vpdpwssd zmm3,zmm4,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 4b 52 d9[ 	]*vpdpwssd zmm3\{k3\},zmm4,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d cb 52 d9[ 	]*vpdpwssd zmm3\{k3\}\{z\},zmm4,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 52 9c f4 c0 1d fe ff[ 	]*vpdpwssd zmm3,zmm4,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 52 5a 7f[ 	]*vpdpwssd zmm3,zmm4,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 58 52 5a 7f[ 	]*vpdpwssd zmm3,zmm4,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 53 da[ 	]*vpdpwssds zmm3,zmm1,zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 4f 53 da[ 	]*vpdpwssds zmm3\{k7\},zmm1,zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 cf 53 da[ 	]*vpdpwssds zmm3\{k7\}\{z\},zmm1,zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 53 9c f4 c0 1d fe ff[ 	]*vpdpwssds zmm3,zmm1,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 53 5a 7f[ 	]*vpdpwssds zmm3,zmm1,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 58 53 5a 7f[ 	]*vpdpwssds zmm3,zmm1,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 50 d9[ 	]*vpdpbusd zmm3,zmm4,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 4e 50 d9[ 	]*vpdpbusd zmm3\{k6\},zmm4,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d ce 50 d9[ 	]*vpdpbusd zmm3\{k6\}\{z\},zmm4,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 50 9c f4 c0 1d fe ff[ 	]*vpdpbusd zmm3,zmm4,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 50 5a 7f[ 	]*vpdpbusd zmm3,zmm4,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 58 50 5a 7f[ 	]*vpdpbusd zmm3,zmm4,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 51 c9[ 	]*vpdpbusds zmm1,zmm1,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 49 51 c9[ 	]*vpdpbusds zmm1\{k1\},zmm1,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 c9 51 c9[ 	]*vpdpbusds zmm1\{k1\}\{z\},zmm1,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 51 8c f4 c0 1d fe ff[ 	]*vpdpbusds zmm1,zmm1,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 51 4a 7f[ 	]*vpdpbusds zmm1,zmm1,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 58 51 4a 7f[ 	]*vpdpbusds zmm1,zmm1,DWORD BCST \[edx\+0x1fc\]
#pass
