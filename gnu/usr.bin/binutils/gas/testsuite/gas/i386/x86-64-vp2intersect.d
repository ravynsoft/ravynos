#as:
#objdump: -dw
#name: x86_64 VP2INTERSECT insns
#source: x86-64-vp2intersect.s

.*: +file format .*


Disassembly of section \.text:

0+ <\.text>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 d9[ 	]*vp2intersectd %zmm1,%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 58 01[ 	]*vp2intersectd 0x40\(%rax\),%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 58 68 58 02[ 	]*vp2intersectd 0x8\(%rax\)\{1to16\},%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 d9[ 	]*vp2intersectd %ymm1,%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 58 01[ 	]*vp2intersectd 0x20\(%rax\),%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 38 68 58 02[ 	]*vp2intersectd 0x8\(%rax\)\{1to8\},%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 d9[ 	]*vp2intersectd %xmm1,%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 58 01[ 	]*vp2intersectd 0x10\(%rax\),%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 18 68 58 02[ 	]*vp2intersectd 0x8\(%rax\)\{1to4\},%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 d9[ 	]*vp2intersectq %zmm1,%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 58 01[ 	]*vp2intersectq 0x40\(%rax\),%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 58 68 58 01[ 	]*vp2intersectq 0x8\(%rax\)\{1to8\},%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 d9[ 	]*vp2intersectq %ymm1,%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 58 01[ 	]*vp2intersectq 0x20\(%rax\),%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 38 68 58 01[ 	]*vp2intersectq 0x8\(%rax\)\{1to4\},%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 d9[ 	]*vp2intersectq %xmm1,%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 58 01[ 	]*vp2intersectq 0x10\(%rax\),%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 18 68 58 01[ 	]*vp2intersectq 0x8\(%rax\)\{1to2\},%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 d9[ 	]*vp2intersectd %zmm1,%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 48 68 58 01[ 	]*vp2intersectd 0x40\(%rax\),%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 58 68 58 02[ 	]*vp2intersectd 0x8\(%rax\)\{1to16\},%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 d9[ 	]*vp2intersectd %ymm1,%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 28 68 58 01[ 	]*vp2intersectd 0x20\(%rax\),%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 38 68 58 02[ 	]*vp2intersectd 0x8\(%rax\)\{1to8\},%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 d9[ 	]*vp2intersectd %xmm1,%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 08 68 58 01[ 	]*vp2intersectd 0x10\(%rax\),%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 6f 18 68 58 02[ 	]*vp2intersectd 0x8\(%rax\)\{1to4\},%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 d9[ 	]*vp2intersectq %zmm1,%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 48 68 58 01[ 	]*vp2intersectq 0x40\(%rax\),%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 58 68 58 01[ 	]*vp2intersectq 0x8\(%rax\)\{1to8\},%zmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 d9[ 	]*vp2intersectq %ymm1,%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 28 68 58 01[ 	]*vp2intersectq 0x20\(%rax\),%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 38 68 58 01[ 	]*vp2intersectq 0x8\(%rax\)\{1to4\},%ymm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 d9[ 	]*vp2intersectq %xmm1,%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 08 68 58 01[ 	]*vp2intersectq 0x10\(%rax\),%xmm2,%k3
[ 	]*[a-f0-9]+:[ 	]*62 f2 ef 18 68 58 01[ 	]*vp2intersectq 0x8\(%rax\)\{1to2\},%xmm2,%k3
#pass
