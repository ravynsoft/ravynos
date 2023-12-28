#as:
#objdump: -dw
#name: x86_64 AVX512BITALG insns
#source: x86-64-avx512bitalg.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 92 15 40 8f ec[ 	]*vpshufbitqmb %zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 47 8f ec[ 	]*vpshufbitqmb %zmm28,%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 40 8f ac f0 23 01 00 00[ 	]*vpshufbitqmb 0x123\(%rax,%r14,8\),%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 40 8f 6a 7f[ 	]*vpshufbitqmb 0x1fc0\(%rdx\),%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 54 f5[ 	]*vpopcntb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 54 f5[ 	]*vpopcntb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 54 f5[ 	]*vpopcntb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 54 b4 f0 23 01 00 00[ 	]*vpopcntb 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 54 72 7f[ 	]*vpopcntb 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 54 f5[ 	]*vpopcntw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 54 f5[ 	]*vpopcntw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 54 f5[ 	]*vpopcntw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 54 b4 f0 23 01 00 00[ 	]*vpopcntw 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 54 72 7f[ 	]*vpopcntw 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 23 01 00 00[ 	]*vpopcntd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 23 01 00 00[ 	]*vpopcntq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 15 40 8f ec[ 	]*vpshufbitqmb %zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 15 47 8f ec[ 	]*vpshufbitqmb %zmm28,%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 40 8f ac f0 34 12 00 00[ 	]*vpshufbitqmb 0x1234\(%rax,%r14,8\),%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 40 8f 6a 7f[ 	]*vpshufbitqmb 0x1fc0\(%rdx\),%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 54 f5[ 	]*vpopcntb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 54 f5[ 	]*vpopcntb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 54 f5[ 	]*vpopcntb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 54 b4 f0 34 12 00 00[ 	]*vpopcntb 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 54 72 7f[ 	]*vpopcntb 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 54 f5[ 	]*vpopcntw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 54 f5[ 	]*vpopcntw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 54 f5[ 	]*vpopcntw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 54 b4 f0 34 12 00 00[ 	]*vpopcntw 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 54 72 7f[ 	]*vpopcntw 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 34 12 00 00[ 	]*vpopcntd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 34 12 00 00[ 	]*vpopcntq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to8\},%zmm30
#pass
