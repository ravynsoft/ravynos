#as:
#objdump: -dw -Mintel
#name: i386 AVX-VNNI-INT8 insns (Intel disassembly)
#source: avx-vnni-int8.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 e2 57 50 f4\s+vpdpbssd ymm6,ymm5,ymm4
\s*[a-f0-9]+:\s*c4 e2 53 50 f4\s+vpdpbssd xmm6,xmm5,xmm4
\s*[a-f0-9]+:\s*c4 e2 57 50 b4 f4 00 00 00 10\s+vpdpbssd ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 57 50 31\s+vpdpbssd ymm6,ymm5,YMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 57 50 b1 e0 0f 00 00\s+vpdpbssd ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 e2 57 50 b2 00 f0 ff ff\s+vpdpbssd ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
\s*[a-f0-9]+:\s*c4 e2 53 50 b4 f4 00 00 00 10\s+vpdpbssd xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 53 50 31\s+vpdpbssd xmm6,xmm5,XMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 53 50 b1 f0 07 00 00\s+vpdpbssd xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 e2 53 50 b2 00 f8 ff ff\s+vpdpbssd xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
\s*[a-f0-9]+:\s*c4 e2 57 51 f4\s+vpdpbssds ymm6,ymm5,ymm4
\s*[a-f0-9]+:\s*c4 e2 53 51 f4\s+vpdpbssds xmm6,xmm5,xmm4
\s*[a-f0-9]+:\s*c4 e2 57 51 b4 f4 00 00 00 10\s+vpdpbssds ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 57 51 31\s+vpdpbssds ymm6,ymm5,YMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 57 51 b1 e0 0f 00 00\s+vpdpbssds ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 e2 57 51 b2 00 f0 ff ff\s+vpdpbssds ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
\s*[a-f0-9]+:\s*c4 e2 53 51 b4 f4 00 00 00 10\s+vpdpbssds xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 53 51 31\s+vpdpbssds xmm6,xmm5,XMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 53 51 b1 f0 07 00 00\s+vpdpbssds xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 e2 53 51 b2 00 f8 ff ff\s+vpdpbssds xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
\s*[a-f0-9]+:\s*c4 e2 56 50 f4\s+vpdpbsud ymm6,ymm5,ymm4
\s*[a-f0-9]+:\s*c4 e2 52 50 f4\s+vpdpbsud xmm6,xmm5,xmm4
\s*[a-f0-9]+:\s*c4 e2 56 50 b4 f4 00 00 00 10\s+vpdpbsud ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 56 50 31\s+vpdpbsud ymm6,ymm5,YMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 56 50 b1 e0 0f 00 00\s+vpdpbsud ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 e2 56 50 b2 00 f0 ff ff\s+vpdpbsud ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
\s*[a-f0-9]+:\s*c4 e2 52 50 b4 f4 00 00 00 10\s+vpdpbsud xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 52 50 31\s+vpdpbsud xmm6,xmm5,XMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 52 50 b1 f0 07 00 00\s+vpdpbsud xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 e2 52 50 b2 00 f8 ff ff\s+vpdpbsud xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
\s*[a-f0-9]+:\s*c4 e2 56 51 f4\s+vpdpbsuds ymm6,ymm5,ymm4
\s*[a-f0-9]+:\s*c4 e2 52 51 f4\s+vpdpbsuds xmm6,xmm5,xmm4
\s*[a-f0-9]+:\s*c4 e2 56 51 b4 f4 00 00 00 10\s+vpdpbsuds ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 56 51 31\s+vpdpbsuds ymm6,ymm5,YMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 56 51 b1 e0 0f 00 00\s+vpdpbsuds ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 e2 56 51 b2 00 f0 ff ff\s+vpdpbsuds ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
\s*[a-f0-9]+:\s*c4 e2 52 51 b4 f4 00 00 00 10\s+vpdpbsuds xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 52 51 31\s+vpdpbsuds xmm6,xmm5,XMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 52 51 b1 f0 07 00 00\s+vpdpbsuds xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 e2 52 51 b2 00 f8 ff ff\s+vpdpbsuds xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
\s*[a-f0-9]+:\s*c4 e2 54 50 f4\s+vpdpbuud ymm6,ymm5,ymm4
\s*[a-f0-9]+:\s*c4 e2 50 50 f4\s+vpdpbuud xmm6,xmm5,xmm4
\s*[a-f0-9]+:\s*c4 e2 54 50 b4 f4 00 00 00 10\s+vpdpbuud ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 54 50 31\s+vpdpbuud ymm6,ymm5,YMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 54 50 b1 e0 0f 00 00\s+vpdpbuud ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 e2 54 50 b2 00 f0 ff ff\s+vpdpbuud ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
\s*[a-f0-9]+:\s*c4 e2 50 50 b4 f4 00 00 00 10\s+vpdpbuud xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 50 50 31\s+vpdpbuud xmm6,xmm5,XMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 50 50 b1 f0 07 00 00\s+vpdpbuud xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 e2 50 50 b2 00 f8 ff ff\s+vpdpbuud xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
\s*[a-f0-9]+:\s*c4 e2 54 51 f4\s+vpdpbuuds ymm6,ymm5,ymm4
\s*[a-f0-9]+:\s*c4 e2 50 51 f4\s+vpdpbuuds xmm6,xmm5,xmm4
\s*[a-f0-9]+:\s*c4 e2 54 51 b4 f4 00 00 00 10\s+vpdpbuuds ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 54 51 31\s+vpdpbuuds ymm6,ymm5,YMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 54 51 b1 e0 0f 00 00\s+vpdpbuuds ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 e2 54 51 b2 00 f0 ff ff\s+vpdpbuuds ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
\s*[a-f0-9]+:\s*c4 e2 50 51 b4 f4 00 00 00 10\s+vpdpbuuds xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 e2 50 51 31\s+vpdpbuuds xmm6,xmm5,XMMWORD PTR \[ecx\]
\s*[a-f0-9]+:\s*c4 e2 50 51 b1 f0 07 00 00\s+vpdpbuuds xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 e2 50 51 b2 00 f8 ff ff\s+vpdpbuuds xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
#pass
