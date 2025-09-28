#as:
#objdump: -dw
#name: i386 AVX-VNNI-INT8 insns
#source: avx-vnni-int8.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 e2 57 50 f4\s+vpdpbssd %ymm4,%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 53 50 f4\s+vpdpbssd %xmm4,%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 57 50 b4 f4 00 00 00 10\s+vpdpbssd 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 57 50 31\s+vpdpbssd \(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 57 50 b1 e0 0f 00 00\s+vpdpbssd 0xfe0\(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 57 50 b2 00 f0 ff ff\s+vpdpbssd -0x1000\(%edx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 53 50 b4 f4 00 00 00 10\s+vpdpbssd 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 53 50 31\s+vpdpbssd \(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 53 50 b1 f0 07 00 00\s+vpdpbssd 0x7f0\(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 53 50 b2 00 f8 ff ff\s+vpdpbssd -0x800\(%edx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 57 51 f4\s+vpdpbssds %ymm4,%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 53 51 f4\s+vpdpbssds %xmm4,%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 57 51 b4 f4 00 00 00 10\s+vpdpbssds 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 57 51 31\s+vpdpbssds \(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 57 51 b1 e0 0f 00 00\s+vpdpbssds 0xfe0\(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 57 51 b2 00 f0 ff ff\s+vpdpbssds -0x1000\(%edx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 53 51 b4 f4 00 00 00 10\s+vpdpbssds 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 53 51 31\s+vpdpbssds \(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 53 51 b1 f0 07 00 00\s+vpdpbssds 0x7f0\(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 53 51 b2 00 f8 ff ff\s+vpdpbssds -0x800\(%edx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 56 50 f4\s+vpdpbsud %ymm4,%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 52 50 f4\s+vpdpbsud %xmm4,%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 56 50 b4 f4 00 00 00 10\s+vpdpbsud 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 56 50 31\s+vpdpbsud \(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 56 50 b1 e0 0f 00 00\s+vpdpbsud 0xfe0\(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 56 50 b2 00 f0 ff ff\s+vpdpbsud -0x1000\(%edx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 52 50 b4 f4 00 00 00 10\s+vpdpbsud 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 52 50 31\s+vpdpbsud \(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 52 50 b1 f0 07 00 00\s+vpdpbsud 0x7f0\(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 52 50 b2 00 f8 ff ff\s+vpdpbsud -0x800\(%edx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 56 51 f4\s+vpdpbsuds %ymm4,%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 52 51 f4\s+vpdpbsuds %xmm4,%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 56 51 b4 f4 00 00 00 10\s+vpdpbsuds 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 56 51 31\s+vpdpbsuds \(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 56 51 b1 e0 0f 00 00\s+vpdpbsuds 0xfe0\(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 56 51 b2 00 f0 ff ff\s+vpdpbsuds -0x1000\(%edx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 52 51 b4 f4 00 00 00 10\s+vpdpbsuds 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 52 51 31\s+vpdpbsuds \(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 52 51 b1 f0 07 00 00\s+vpdpbsuds 0x7f0\(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 52 51 b2 00 f8 ff ff\s+vpdpbsuds -0x800\(%edx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 54 50 f4\s+vpdpbuud %ymm4,%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 50 50 f4\s+vpdpbuud %xmm4,%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 54 50 b4 f4 00 00 00 10\s+vpdpbuud 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 54 50 31\s+vpdpbuud \(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 54 50 b1 e0 0f 00 00\s+vpdpbuud 0xfe0\(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 54 50 b2 00 f0 ff ff\s+vpdpbuud -0x1000\(%edx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 50 50 b4 f4 00 00 00 10\s+vpdpbuud 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 50 50 31\s+vpdpbuud \(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 50 50 b1 f0 07 00 00\s+vpdpbuud 0x7f0\(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 50 50 b2 00 f8 ff ff\s+vpdpbuud -0x800\(%edx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 54 51 f4\s+vpdpbuuds %ymm4,%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 50 51 f4\s+vpdpbuuds %xmm4,%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 54 51 b4 f4 00 00 00 10\s+vpdpbuuds 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 54 51 31\s+vpdpbuuds \(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 54 51 b1 e0 0f 00 00\s+vpdpbuuds 0xfe0\(%ecx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 54 51 b2 00 f0 ff ff\s+vpdpbuuds -0x1000\(%edx\),%ymm5,%ymm6
\s*[a-f0-9]+:\s*c4 e2 50 51 b4 f4 00 00 00 10\s+vpdpbuuds 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 50 51 31\s+vpdpbuuds \(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 50 51 b1 f0 07 00 00\s+vpdpbuuds 0x7f0\(%ecx\),%xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 50 51 b2 00 f8 ff ff\s+vpdpbuuds -0x800\(%edx\),%xmm5,%xmm6
#pass
