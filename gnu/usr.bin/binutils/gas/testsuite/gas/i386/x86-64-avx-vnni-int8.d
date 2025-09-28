#as:
#objdump: -dw
#name: x86_64 AVX-VNNI-INT8 insns
#source: x86-64-avx-vnni-int8.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 42 37 50 d0\s+vpdpbssd %ymm8,%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 33 50 d0\s+vpdpbssd %xmm8,%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 22 37 50 94 f5 00 00 00 10\s+vpdpbssd 0x10000000\(%rbp,%r14,8\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 37 50 11\s+vpdpbssd \(%r9\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 37 50 91 e0 0f 00 00\s+vpdpbssd 0xfe0\(%rcx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 37 50 92 00 f0 ff ff\s+vpdpbssd -0x1000\(%rdx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 22 33 50 94 f5 00 00 00 10\s+vpdpbssd 0x10000000\(%rbp,%r14,8\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 33 50 11\s+vpdpbssd \(%r9\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 33 50 91 f0 07 00 00\s+vpdpbssd 0x7f0\(%rcx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 33 50 92 00 f8 ff ff\s+vpdpbssd -0x800\(%rdx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 37 51 d0\s+vpdpbssds %ymm8,%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 33 51 d0\s+vpdpbssds %xmm8,%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 22 37 51 94 f5 00 00 00 10\s+vpdpbssds 0x10000000\(%rbp,%r14,8\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 37 51 11\s+vpdpbssds \(%r9\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 37 51 91 e0 0f 00 00\s+vpdpbssds 0xfe0\(%rcx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 37 51 92 00 f0 ff ff\s+vpdpbssds -0x1000\(%rdx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 22 33 51 94 f5 00 00 00 10\s+vpdpbssds 0x10000000\(%rbp,%r14,8\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 33 51 11\s+vpdpbssds \(%r9\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 33 51 91 f0 07 00 00\s+vpdpbssds 0x7f0\(%rcx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 33 51 92 00 f8 ff ff\s+vpdpbssds -0x800\(%rdx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 36 50 d0\s+vpdpbsud %ymm8,%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 32 50 d0\s+vpdpbsud %xmm8,%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 22 36 50 94 f5 00 00 00 10\s+vpdpbsud 0x10000000\(%rbp,%r14,8\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 36 50 11\s+vpdpbsud \(%r9\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 36 50 91 e0 0f 00 00\s+vpdpbsud 0xfe0\(%rcx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 36 50 92 00 f0 ff ff\s+vpdpbsud -0x1000\(%rdx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 22 32 50 94 f5 00 00 00 10\s+vpdpbsud 0x10000000\(%rbp,%r14,8\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 32 50 11\s+vpdpbsud \(%r9\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 32 50 91 f0 07 00 00\s+vpdpbsud 0x7f0\(%rcx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 32 50 92 00 f8 ff ff\s+vpdpbsud -0x800\(%rdx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 36 51 d0\s+vpdpbsuds %ymm8,%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 32 51 d0\s+vpdpbsuds %xmm8,%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 22 36 51 94 f5 00 00 00 10\s+vpdpbsuds 0x10000000\(%rbp,%r14,8\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 36 51 11\s+vpdpbsuds \(%r9\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 36 51 91 e0 0f 00 00\s+vpdpbsuds 0xfe0\(%rcx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 36 51 92 00 f0 ff ff\s+vpdpbsuds -0x1000\(%rdx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 22 32 51 94 f5 00 00 00 10\s+vpdpbsuds 0x10000000\(%rbp,%r14,8\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 32 51 11\s+vpdpbsuds \(%r9\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 32 51 91 f0 07 00 00\s+vpdpbsuds 0x7f0\(%rcx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 32 51 92 00 f8 ff ff\s+vpdpbsuds -0x800\(%rdx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 34 50 d0\s+vpdpbuud %ymm8,%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 30 50 d0\s+vpdpbuud %xmm8,%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 22 34 50 94 f5 00 00 00 10\s+vpdpbuud 0x10000000\(%rbp,%r14,8\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 34 50 11\s+vpdpbuud \(%r9\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 34 50 91 e0 0f 00 00\s+vpdpbuud 0xfe0\(%rcx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 34 50 92 00 f0 ff ff\s+vpdpbuud -0x1000\(%rdx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 22 30 50 94 f5 00 00 00 10\s+vpdpbuud 0x10000000\(%rbp,%r14,8\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 30 50 11\s+vpdpbuud \(%r9\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 30 50 91 f0 07 00 00\s+vpdpbuud 0x7f0\(%rcx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 30 50 92 00 f8 ff ff\s+vpdpbuud -0x800\(%rdx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 34 51 d0\s+vpdpbuuds %ymm8,%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 30 51 d0\s+vpdpbuuds %xmm8,%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 22 34 51 94 f5 00 00 00 10\s+vpdpbuuds 0x10000000\(%rbp,%r14,8\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 42 34 51 11\s+vpdpbuuds \(%r9\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 34 51 91 e0 0f 00 00\s+vpdpbuuds 0xfe0\(%rcx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 62 34 51 92 00 f0 ff ff\s+vpdpbuuds -0x1000\(%rdx\),%ymm9,%ymm10
\s*[a-f0-9]+:\s*c4 22 30 51 94 f5 00 00 00 10\s+vpdpbuuds 0x10000000\(%rbp,%r14,8\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 42 30 51 11\s+vpdpbuuds \(%r9\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 30 51 91 f0 07 00 00\s+vpdpbuuds 0x7f0\(%rcx\),%xmm9,%xmm10
\s*[a-f0-9]+:\s*c4 62 30 51 92 00 f8 ff ff\s+vpdpbuuds -0x800\(%rdx\),%xmm9,%xmm10
#pass
