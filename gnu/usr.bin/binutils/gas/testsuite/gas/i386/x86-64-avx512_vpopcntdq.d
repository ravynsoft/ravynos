#as:
#objdump: -dw
#name: x86_64 AVX512/VPOPCNTDQ insns
#source: x86-64-avx512_vpopcntdq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 31[ 	]*vpopcntd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 23 01 00 00[ 	]*vpopcntd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 31[ 	]*vpopcntd \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 80[ 	]*vpopcntd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 80[ 	]*vpopcntd -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 31[ 	]*vpopcntq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 23 01 00 00[ 	]*vpopcntq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 31[ 	]*vpopcntq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 80[ 	]*vpopcntq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 80[ 	]*vpopcntq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 31[ 	]*vpopcntd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 34 12 00 00[ 	]*vpopcntd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 31[ 	]*vpopcntd \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 80[ 	]*vpopcntd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 80[ 	]*vpopcntd -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 31[ 	]*vpopcntq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 34 12 00 00[ 	]*vpopcntq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 31[ 	]*vpopcntq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 80[ 	]*vpopcntq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 80[ 	]*vpopcntq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq -0x408\(%rdx\)\{1to8\},%zmm30
#pass
