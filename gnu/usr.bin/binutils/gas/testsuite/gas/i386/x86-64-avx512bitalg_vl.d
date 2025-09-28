#as:
#objdump: -dw
#name: x86_64 AVX512BITALG/VL insns
#source: x86-64-avx512bitalg_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 92 15 00 8f ec[ 	]*vpshufbitqmb %xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 07 8f ec[ 	]*vpshufbitqmb %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 00 8f ac f0 23 01 00 00[ 	]*vpshufbitqmb 0x123\(%rax,%r14,8\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 00 8f 6a 7f[ 	]*vpshufbitqmb 0x7f0\(%rdx\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 20 8f ec[ 	]*vpshufbitqmb %ymm28,%ymm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 27 8f ec[ 	]*vpshufbitqmb %ymm28,%ymm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 20 8f ac f0 23 01 00 00[ 	]*vpshufbitqmb 0x123\(%rax,%r14,8\),%ymm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 20 8f 6a 7f[ 	]*vpshufbitqmb 0xfe0\(%rdx\),%ymm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 54 f5[ 	]*vpopcntb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 54 f5[ 	]*vpopcntb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 54 f5[ 	]*vpopcntb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 54 b4 f0 23 01 00 00[ 	]*vpopcntb 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 54 72 7f[ 	]*vpopcntb 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 54 f5[ 	]*vpopcntb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 54 f5[ 	]*vpopcntb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 54 f5[ 	]*vpopcntb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 54 b4 f0 23 01 00 00[ 	]*vpopcntb 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 54 72 7f[ 	]*vpopcntb 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 54 f5[ 	]*vpopcntw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 54 f5[ 	]*vpopcntw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 54 f5[ 	]*vpopcntw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 54 b4 f0 23 01 00 00[ 	]*vpopcntw 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 54 72 7f[ 	]*vpopcntw 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 54 f5[ 	]*vpopcntw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 54 f5[ 	]*vpopcntw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 54 f5[ 	]*vpopcntw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 54 b4 f0 23 01 00 00[ 	]*vpopcntw 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 54 72 7f[ 	]*vpopcntw 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 55 f5[ 	]*vpopcntd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 55 f5[ 	]*vpopcntd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 55 f5[ 	]*vpopcntd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 55 b4 f0 23 01 00 00[ 	]*vpopcntd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 55 72 7f[ 	]*vpopcntd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 55 f5[ 	]*vpopcntd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 55 f5[ 	]*vpopcntd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 55 f5[ 	]*vpopcntd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 55 b4 f0 23 01 00 00[ 	]*vpopcntd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 55 72 7f[ 	]*vpopcntd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 55 f5[ 	]*vpopcntq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 55 f5[ 	]*vpopcntq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 55 f5[ 	]*vpopcntq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 55 b4 f0 23 01 00 00[ 	]*vpopcntq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 55 72 7f[ 	]*vpopcntq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 55 f5[ 	]*vpopcntq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 55 f5[ 	]*vpopcntq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 55 f5[ 	]*vpopcntq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 55 b4 f0 23 01 00 00[ 	]*vpopcntq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 55 72 7f[ 	]*vpopcntq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 15 00 8f ec[ 	]*vpshufbitqmb %xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 07 8f ec[ 	]*vpshufbitqmb %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 00 8f ac f0 34 12 00 00[ 	]*vpshufbitqmb 0x1234\(%rax,%r14,8\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 00 8f 6a 7f[ 	]*vpshufbitqmb 0x7f0\(%rdx\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 20 8f ec[ 	]*vpshufbitqmb %ymm28,%ymm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 27 8f ec[ 	]*vpshufbitqmb %ymm28,%ymm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 20 8f ac f0 34 12 00 00[ 	]*vpshufbitqmb 0x1234\(%rax,%r14,8\),%ymm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 20 8f 6a 7f[ 	]*vpshufbitqmb 0xfe0\(%rdx\),%ymm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 54 f5[ 	]*vpopcntb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 54 f5[ 	]*vpopcntb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 54 f5[ 	]*vpopcntb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 54 b4 f0 34 12 00 00[ 	]*vpopcntb 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 54 72 7f[ 	]*vpopcntb 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 54 f5[ 	]*vpopcntb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 54 f5[ 	]*vpopcntb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 54 f5[ 	]*vpopcntb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 54 b4 f0 34 12 00 00[ 	]*vpopcntb 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 54 72 7f[ 	]*vpopcntb 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 54 f5[ 	]*vpopcntw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 54 f5[ 	]*vpopcntw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 54 f5[ 	]*vpopcntw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 54 b4 f0 34 12 00 00[ 	]*vpopcntw 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 54 72 7f[ 	]*vpopcntw 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 54 f5[ 	]*vpopcntw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 54 f5[ 	]*vpopcntw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 54 f5[ 	]*vpopcntw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 54 b4 f0 34 12 00 00[ 	]*vpopcntw 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 54 72 7f[ 	]*vpopcntw 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 55 f5[ 	]*vpopcntd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 55 f5[ 	]*vpopcntd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 55 f5[ 	]*vpopcntd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 55 b4 f0 34 12 00 00[ 	]*vpopcntd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 55 72 7f[ 	]*vpopcntd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 55 f5[ 	]*vpopcntd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 55 f5[ 	]*vpopcntd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 55 f5[ 	]*vpopcntd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 55 b4 f0 34 12 00 00[ 	]*vpopcntd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 55 72 7f[ 	]*vpopcntd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 55 f5[ 	]*vpopcntq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 55 f5[ 	]*vpopcntq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 55 f5[ 	]*vpopcntq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 55 b4 f0 34 12 00 00[ 	]*vpopcntq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 55 72 7f[ 	]*vpopcntq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 55 f5[ 	]*vpopcntq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 55 f5[ 	]*vpopcntq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 55 f5[ 	]*vpopcntq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 55 b4 f0 34 12 00 00[ 	]*vpopcntq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 55 72 7f[ 	]*vpopcntq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to4\},%ymm30
#pass
