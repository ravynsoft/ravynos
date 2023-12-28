#as:
#objdump: -dw -Mintel
#name: i386 VP2INTERSECT insns (Intel disassembly)
#source: vp2intersect.s

.*: +file format .*


Disassembly of section \.text:

00000000 <\.text>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 d9[ 	]*vp2intersectd k3,zmm2,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 58 01[ 	]*vp2intersectd k3,zmm2,ZMMWORD PTR \[eax\+0x40\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 58 68 58 02[ 	]*vp2intersectd k3,zmm2,DWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 d9[ 	]*vp2intersectd k3,ymm2,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 58 01[ 	]*vp2intersectd k3,ymm2,YMMWORD PTR \[eax\+0x20\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 38 68 58 02[ 	]*vp2intersectd k3,ymm2,DWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 d9[ 	]*vp2intersectd k3,xmm2,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 58 01[ 	]*vp2intersectd k3,xmm2,XMMWORD PTR \[eax\+0x10\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 18 68 58 02[ 	]*vp2intersectd k3,xmm2,DWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 d9[ 	]*vp2intersectq k3,zmm2,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 58 01[ 	]*vp2intersectq k3,zmm2,ZMMWORD PTR \[eax\+0x40\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 58 68 58 01[ 	]*vp2intersectq k3,zmm2,QWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 d9[ 	]*vp2intersectq k3,ymm2,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 58 01[ 	]*vp2intersectq k3,ymm2,YMMWORD PTR \[eax\+0x20\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 38 68 58 01[ 	]*vp2intersectq k3,ymm2,QWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 d9[ 	]*vp2intersectq k3,xmm2,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 58 01[ 	]*vp2intersectq k3,xmm2,XMMWORD PTR \[eax\+0x10\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 18 68 58 01[ 	]*vp2intersectq k3,xmm2,QWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 d9[ 	]*vp2intersectd k3,zmm2,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 58 01[ 	]*vp2intersectd k3,zmm2,ZMMWORD PTR \[eax\+0x40\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 58 68 58 02[ 	]*vp2intersectd k3,zmm2,DWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 d9[ 	]*vp2intersectd k3,ymm2,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 58 01[ 	]*vp2intersectd k3,ymm2,YMMWORD PTR \[eax\+0x20\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 38 68 58 02[ 	]*vp2intersectd k3,ymm2,DWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 d9[ 	]*vp2intersectd k3,xmm2,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 58 01[ 	]*vp2intersectd k3,xmm2,XMMWORD PTR \[eax\+0x10\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 18 68 58 02[ 	]*vp2intersectd k3,xmm2,DWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 d9[ 	]*vp2intersectq k3,zmm2,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 58 01[ 	]*vp2intersectq k3,zmm2,ZMMWORD PTR \[eax\+0x40\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 58 68 58 01[ 	]*vp2intersectq k3,zmm2,QWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 d9[ 	]*vp2intersectq k3,ymm2,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 58 01[ 	]*vp2intersectq k3,ymm2,YMMWORD PTR \[eax\+0x20\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 38 68 58 01[ 	]*vp2intersectq k3,ymm2,QWORD BCST \[eax\+0x8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 d9[ 	]*vp2intersectq k3,xmm2,xmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 58 01[ 	]*vp2intersectq k3,xmm2,XMMWORD PTR \[eax\+0x10\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 18 68 58 01[ 	]*vp2intersectq k3,xmm2,QWORD BCST \[eax\+0x8\]
#pass
