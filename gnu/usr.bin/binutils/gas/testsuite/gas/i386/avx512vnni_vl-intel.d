#as:
#objdump: -dw -Mintel
#name: i386 AVX512VNNI/VL insns (Intel disassembly)
#source: avx512vnni_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0b 52 d2[ 	]*vpdpwssd xmm2\{k3\},xmm4,xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 8b 52 d2[ 	]*vpdpwssd xmm2\{k3\}\{z\},xmm4,xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 09 52 94 f4 c0 1d fe ff[ 	]*vpdpwssd xmm2\{k1\},xmm4,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 09 52 52 7f[ 	]*vpdpwssd xmm2\{k1\},xmm4,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 19 52 52 7f[ 	]*vpdpwssd xmm2\{k1\},xmm4,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 29 52 d9[ 	]*vpdpwssd ymm3\{k1\},ymm3,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 a9 52 d9[ 	]*vpdpwssd ymm3\{k1\}\{z\},ymm3,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 2c 52 9c f4 c0 1d fe ff[ 	]*vpdpwssd ymm3\{k4\},ymm3,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 2c 52 5a 7f[ 	]*vpdpwssd ymm3\{k4\},ymm3,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 3c 52 5a 7f[ 	]*vpdpwssd ymm3\{k4\},ymm3,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 09 53 d1[ 	]*vpdpwssds xmm2\{k1\},xmm4,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 89 53 d1[ 	]*vpdpwssds xmm2\{k1\}\{z\},xmm4,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0c 53 94 f4 c0 1d fe ff[ 	]*vpdpwssds xmm2\{k4\},xmm4,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0c 53 52 7f[ 	]*vpdpwssds xmm2\{k4\},xmm4,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 1c 53 52 7f[ 	]*vpdpwssds xmm2\{k4\},xmm4,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 2f 53 e4[ 	]*vpdpwssds ymm4\{k7\},ymm1,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 af 53 e4[ 	]*vpdpwssds ymm4\{k7\}\{z\},ymm1,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 2b 53 a4 f4 c0 1d fe ff[ 	]*vpdpwssds ymm4\{k3\},ymm1,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 2b 53 62 7f[ 	]*vpdpwssds ymm4\{k3\},ymm1,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 3b 53 62 7f[ 	]*vpdpwssds ymm4\{k3\},ymm1,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 0c 50 d1[ 	]*vpdpbusd xmm2\{k4\},xmm3,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 8c 50 d1[ 	]*vpdpbusd xmm2\{k4\}\{z\},xmm3,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 0a 50 94 f4 c0 1d fe ff[ 	]*vpdpbusd xmm2\{k2\},xmm3,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 0a 50 52 7f[ 	]*vpdpbusd xmm2\{k2\},xmm3,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 1a 50 52 7f[ 	]*vpdpbusd xmm2\{k2\},xmm3,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2d 50 d2[ 	]*vpdpbusd ymm2\{k5\},ymm2,ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d ad 50 d2[ 	]*vpdpbusd ymm2\{k5\}\{z\},ymm2,ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2f 50 94 f4 c0 1d fe ff[ 	]*vpdpbusd ymm2\{k7\},ymm2,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2f 50 52 7f[ 	]*vpdpbusd ymm2\{k7\},ymm2,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 3f 50 52 7f[ 	]*vpdpbusd ymm2\{k7\},ymm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 0e 51 f4[ 	]*vpdpbusds xmm6\{k6\},xmm2,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 8e 51 f4[ 	]*vpdpbusds xmm6\{k6\}\{z\},xmm2,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 0c 51 b4 f4 c0 1d fe ff[ 	]*vpdpbusds xmm6\{k4\},xmm2,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 0c 51 72 7f[ 	]*vpdpbusds xmm6\{k4\},xmm2,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 1c 51 72 7f[ 	]*vpdpbusds xmm6\{k4\},xmm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 2f 51 e1[ 	]*vpdpbusds ymm4\{k7\},ymm3,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 af 51 e1[ 	]*vpdpbusds ymm4\{k7\}\{z\},ymm3,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 29 51 a4 f4 c0 1d fe ff[ 	]*vpdpbusds ymm4\{k1\},ymm3,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 29 51 62 7f[ 	]*vpdpbusds ymm4\{k1\},ymm3,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 39 51 62 7f[ 	]*vpdpbusds ymm4\{k1\},ymm3,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 09 52 ea[ 	]*vpdpwssd xmm5\{k1\},xmm2,xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 89 52 ea[ 	]*vpdpwssd xmm5\{k1\}\{z\},xmm2,xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 0e 52 ac f4 c0 1d fe ff[ 	]*vpdpwssd xmm5\{k6\},xmm2,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 0e 52 6a 7f[ 	]*vpdpwssd xmm5\{k6\},xmm2,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 1e 52 6a 7f[ 	]*vpdpwssd xmm5\{k6\},xmm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2f 52 cc[ 	]*vpdpwssd ymm1\{k7\},ymm2,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d af 52 cc[ 	]*vpdpwssd ymm1\{k7\}\{z\},ymm2,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2e 52 8c f4 c0 1d fe ff[ 	]*vpdpwssd ymm1\{k6\},ymm2,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2e 52 4a 7f[ 	]*vpdpwssd ymm1\{k6\},ymm2,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 3e 52 4a 7f[ 	]*vpdpwssd ymm1\{k6\},ymm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0a 53 c9[ 	]*vpdpwssds xmm1\{k2\},xmm4,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 8a 53 c9[ 	]*vpdpwssds xmm1\{k2\}\{z\},xmm4,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0e 53 8c f4 c0 1d fe ff[ 	]*vpdpwssds xmm1\{k6\},xmm4,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0e 53 4a 7f[ 	]*vpdpwssds xmm1\{k6\},xmm4,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 1e 53 4a 7f[ 	]*vpdpwssds xmm1\{k6\},xmm4,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2c 53 dc[ 	]*vpdpwssds ymm3\{k4\},ymm2,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d ac 53 dc[ 	]*vpdpwssds ymm3\{k4\}\{z\},ymm2,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2d 53 9c f4 c0 1d fe ff[ 	]*vpdpwssds ymm3\{k5\},ymm2,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2d 53 5a 7f[ 	]*vpdpwssds ymm3\{k5\},ymm2,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 3d 53 5a 7f[ 	]*vpdpwssds ymm3\{k5\},ymm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 0f 50 dc[ 	]*vpdpbusd xmm3\{k7\},xmm4,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 8f 50 dc[ 	]*vpdpbusd xmm3\{k7\}\{z\},xmm4,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 09 50 9c f4 c0 1d fe ff[ 	]*vpdpbusd xmm3\{k1\},xmm4,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 09 50 5a 7f[ 	]*vpdpbusd xmm3\{k1\},xmm4,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 19 50 5a 7f[ 	]*vpdpbusd xmm3\{k1\},xmm4,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2d 50 f4[ 	]*vpdpbusd ymm6\{k5\},ymm2,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d ad 50 f4[ 	]*vpdpbusd ymm6\{k5\}\{z\},ymm2,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2d 50 b4 f4 c0 1d fe ff[ 	]*vpdpbusd ymm6\{k5\},ymm2,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 2d 50 72 7f[ 	]*vpdpbusd ymm6\{k5\},ymm2,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 3d 50 72 7f[ 	]*vpdpbusd ymm6\{k5\},ymm2,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 0d 51 dc[ 	]*vpdpbusds xmm3\{k5\},xmm3,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 8d 51 dc[ 	]*vpdpbusds xmm3\{k5\}\{z\},xmm3,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 0c 51 9c f4 c0 1d fe ff[ 	]*vpdpbusds xmm3\{k4\},xmm3,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 0c 51 5a 7f[ 	]*vpdpbusds xmm3\{k4\},xmm3,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 1c 51 5a 7f[ 	]*vpdpbusds xmm3\{k4\},xmm3,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 2c 51 d4[ 	]*vpdpbusds ymm2\{k4\},ymm3,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 ac 51 d4[ 	]*vpdpbusds ymm2\{k4\}\{z\},ymm3,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 29 51 94 f4 c0 1d fe ff[ 	]*vpdpbusds ymm2\{k1\},ymm3,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 29 51 52 7f[ 	]*vpdpbusds ymm2\{k1\},ymm3,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 39 51 52 7f[ 	]*vpdpbusds ymm2\{k1\},ymm3,DWORD BCST \[edx\+0x1fc\]
#pass
