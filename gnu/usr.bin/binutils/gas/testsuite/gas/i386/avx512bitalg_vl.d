#as:
#objdump: -dw
#name: i386 AVX512BITALG/VL insns
#source: avx512bitalg_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ec[ 	]*vpshufbitqmb %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f 6a 7f[ 	]*vpshufbitqmb 0x7f0\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ec[ 	]*vpshufbitqmb %ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb -0x1e240\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f 6a 7f[ 	]*vpshufbitqmb 0xfe0\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 f5[ 	]*vpopcntb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 54 f5[ 	]*vpopcntb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 72 7f[ 	]*vpopcntb 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 f5[ 	]*vpopcntb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 54 f5[ 	]*vpopcntb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 72 7f[ 	]*vpopcntb 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 f5[ 	]*vpopcntw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 54 f5[ 	]*vpopcntw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 72 7f[ 	]*vpopcntw 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 f5[ 	]*vpopcntw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 54 f5[ 	]*vpopcntw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 72 7f[ 	]*vpopcntw 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 f5[ 	]*vpopcntd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 55 f5[ 	]*vpopcntd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 72 7f[ 	]*vpopcntd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 f5[ 	]*vpopcntd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 55 f5[ 	]*vpopcntd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 72 7f[ 	]*vpopcntd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 f5[ 	]*vpopcntq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 55 f5[ 	]*vpopcntq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 72 7f[ 	]*vpopcntq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 f5[ 	]*vpopcntq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 55 f5[ 	]*vpopcntq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 72 7f[ 	]*vpopcntq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ec[ 	]*vpshufbitqmb %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f 6a 7f[ 	]*vpshufbitqmb 0x7f0\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ec[ 	]*vpshufbitqmb %ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb -0x1e240\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f 6a 7f[ 	]*vpshufbitqmb 0xfe0\(%edx\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 f5[ 	]*vpopcntb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 54 f5[ 	]*vpopcntb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 72 7f[ 	]*vpopcntb 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 f5[ 	]*vpopcntb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 54 f5[ 	]*vpopcntb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 72 7f[ 	]*vpopcntb 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 f5[ 	]*vpopcntw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 54 f5[ 	]*vpopcntw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 72 7f[ 	]*vpopcntw 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 f5[ 	]*vpopcntw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 54 f5[ 	]*vpopcntw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 72 7f[ 	]*vpopcntw 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 f5[ 	]*vpopcntd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 55 f5[ 	]*vpopcntd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 72 7f[ 	]*vpopcntd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 55 32[ 	]*vpopcntd \(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 f5[ 	]*vpopcntd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 55 f5[ 	]*vpopcntd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 72 7f[ 	]*vpopcntd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 55 32[ 	]*vpopcntd \(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 f5[ 	]*vpopcntq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 55 f5[ 	]*vpopcntq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 72 7f[ 	]*vpopcntq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 55 32[ 	]*vpopcntq \(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 f5[ 	]*vpopcntq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 55 f5[ 	]*vpopcntq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 72 7f[ 	]*vpopcntq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 55 32[ 	]*vpopcntq \(%edx\)\{1to4\},%ymm6\{%k7\}
#pass
