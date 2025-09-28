#as:
#objdump: -dw -Mintel
#name: x86_64 AVX-VNNI-INT8 insns (Intel disassembly)
#source: x86-64-avx-vnni-int8.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 42 37 50 d0\s+vpdpbssd ymm10,ymm9,ymm8
\s*[a-f0-9]+:\s*c4 42 33 50 d0\s+vpdpbssd xmm10,xmm9,xmm8
\s*[a-f0-9]+:\s*c4 22 37 50 94 f5 00 00 00 10\s+vpdpbssd ymm10,ymm9,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 37 50 11\s+vpdpbssd ymm10,ymm9,YMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 37 50 91 e0 0f 00 00\s+vpdpbssd ymm10,ymm9,YMMWORD PTR \[rcx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 62 37 50 92 00 f0 ff ff\s+vpdpbssd ymm10,ymm9,YMMWORD PTR \[rdx-0x1000\]
\s*[a-f0-9]+:\s*c4 22 33 50 94 f5 00 00 00 10\s+vpdpbssd xmm10,xmm9,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 33 50 11\s+vpdpbssd xmm10,xmm9,XMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 33 50 91 f0 07 00 00\s+vpdpbssd xmm10,xmm9,XMMWORD PTR \[rcx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 62 33 50 92 00 f8 ff ff\s+vpdpbssd xmm10,xmm9,XMMWORD PTR \[rdx-0x800\]
\s*[a-f0-9]+:\s*c4 42 37 51 d0\s+vpdpbssds ymm10,ymm9,ymm8
\s*[a-f0-9]+:\s*c4 42 33 51 d0\s+vpdpbssds xmm10,xmm9,xmm8
\s*[a-f0-9]+:\s*c4 22 37 51 94 f5 00 00 00 10\s+vpdpbssds ymm10,ymm9,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 37 51 11\s+vpdpbssds ymm10,ymm9,YMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 37 51 91 e0 0f 00 00\s+vpdpbssds ymm10,ymm9,YMMWORD PTR \[rcx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 62 37 51 92 00 f0 ff ff\s+vpdpbssds ymm10,ymm9,YMMWORD PTR \[rdx-0x1000\]
\s*[a-f0-9]+:\s*c4 22 33 51 94 f5 00 00 00 10\s+vpdpbssds xmm10,xmm9,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 33 51 11\s+vpdpbssds xmm10,xmm9,XMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 33 51 91 f0 07 00 00\s+vpdpbssds xmm10,xmm9,XMMWORD PTR \[rcx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 62 33 51 92 00 f8 ff ff\s+vpdpbssds xmm10,xmm9,XMMWORD PTR \[rdx-0x800\]
\s*[a-f0-9]+:\s*c4 42 36 50 d0\s+vpdpbsud ymm10,ymm9,ymm8
\s*[a-f0-9]+:\s*c4 42 32 50 d0\s+vpdpbsud xmm10,xmm9,xmm8
\s*[a-f0-9]+:\s*c4 22 36 50 94 f5 00 00 00 10\s+vpdpbsud ymm10,ymm9,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 36 50 11\s+vpdpbsud ymm10,ymm9,YMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 36 50 91 e0 0f 00 00\s+vpdpbsud ymm10,ymm9,YMMWORD PTR \[rcx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 62 36 50 92 00 f0 ff ff\s+vpdpbsud ymm10,ymm9,YMMWORD PTR \[rdx-0x1000\]
\s*[a-f0-9]+:\s*c4 22 32 50 94 f5 00 00 00 10\s+vpdpbsud xmm10,xmm9,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 32 50 11\s+vpdpbsud xmm10,xmm9,XMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 32 50 91 f0 07 00 00\s+vpdpbsud xmm10,xmm9,XMMWORD PTR \[rcx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 62 32 50 92 00 f8 ff ff\s+vpdpbsud xmm10,xmm9,XMMWORD PTR \[rdx-0x800\]
\s*[a-f0-9]+:\s*c4 42 36 51 d0\s+vpdpbsuds ymm10,ymm9,ymm8
\s*[a-f0-9]+:\s*c4 42 32 51 d0\s+vpdpbsuds xmm10,xmm9,xmm8
\s*[a-f0-9]+:\s*c4 22 36 51 94 f5 00 00 00 10\s+vpdpbsuds ymm10,ymm9,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 36 51 11\s+vpdpbsuds ymm10,ymm9,YMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 36 51 91 e0 0f 00 00\s+vpdpbsuds ymm10,ymm9,YMMWORD PTR \[rcx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 62 36 51 92 00 f0 ff ff\s+vpdpbsuds ymm10,ymm9,YMMWORD PTR \[rdx-0x1000\]
\s*[a-f0-9]+:\s*c4 22 32 51 94 f5 00 00 00 10\s+vpdpbsuds xmm10,xmm9,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 32 51 11\s+vpdpbsuds xmm10,xmm9,XMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 32 51 91 f0 07 00 00\s+vpdpbsuds xmm10,xmm9,XMMWORD PTR \[rcx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 62 32 51 92 00 f8 ff ff\s+vpdpbsuds xmm10,xmm9,XMMWORD PTR \[rdx-0x800\]
\s*[a-f0-9]+:\s*c4 42 34 50 d0\s+vpdpbuud ymm10,ymm9,ymm8
\s*[a-f0-9]+:\s*c4 42 30 50 d0\s+vpdpbuud xmm10,xmm9,xmm8
\s*[a-f0-9]+:\s*c4 22 34 50 94 f5 00 00 00 10\s+vpdpbuud ymm10,ymm9,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 34 50 11\s+vpdpbuud ymm10,ymm9,YMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 34 50 91 e0 0f 00 00\s+vpdpbuud ymm10,ymm9,YMMWORD PTR \[rcx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 62 34 50 92 00 f0 ff ff\s+vpdpbuud ymm10,ymm9,YMMWORD PTR \[rdx-0x1000\]
\s*[a-f0-9]+:\s*c4 22 30 50 94 f5 00 00 00 10\s+vpdpbuud xmm10,xmm9,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 30 50 11\s+vpdpbuud xmm10,xmm9,XMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 30 50 91 f0 07 00 00\s+vpdpbuud xmm10,xmm9,XMMWORD PTR \[rcx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 62 30 50 92 00 f8 ff ff\s+vpdpbuud xmm10,xmm9,XMMWORD PTR \[rdx-0x800\]
\s*[a-f0-9]+:\s*c4 42 34 51 d0\s+vpdpbuuds ymm10,ymm9,ymm8
\s*[a-f0-9]+:\s*c4 42 30 51 d0\s+vpdpbuuds xmm10,xmm9,xmm8
\s*[a-f0-9]+:\s*c4 22 34 51 94 f5 00 00 00 10\s+vpdpbuuds ymm10,ymm9,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 34 51 11\s+vpdpbuuds ymm10,ymm9,YMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 34 51 91 e0 0f 00 00\s+vpdpbuuds ymm10,ymm9,YMMWORD PTR \[rcx\+0xfe0\]
\s*[a-f0-9]+:\s*c4 62 34 51 92 00 f0 ff ff\s+vpdpbuuds ymm10,ymm9,YMMWORD PTR \[rdx-0x1000\]
\s*[a-f0-9]+:\s*c4 22 30 51 94 f5 00 00 00 10\s+vpdpbuuds xmm10,xmm9,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*c4 42 30 51 11\s+vpdpbuuds xmm10,xmm9,XMMWORD PTR \[r9\]
\s*[a-f0-9]+:\s*c4 62 30 51 91 f0 07 00 00\s+vpdpbuuds xmm10,xmm9,XMMWORD PTR \[rcx\+0x7f0\]
\s*[a-f0-9]+:\s*c4 62 30 51 92 00 f8 ff ff\s+vpdpbuuds xmm10,xmm9,XMMWORD PTR \[rdx-0x800\]
#pass
