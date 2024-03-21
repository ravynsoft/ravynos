#as:
#objdump: -dw
#name: x86_64 AVX512IFMA/VL insns
#source: x86-64-avx512ifma_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b4 f4[ 	]*vpmadd52luq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b4 f4[ 	]*vpmadd52luq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b4 f4[ 	]*vpmadd52luq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 31[ 	]*vpmadd52luq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b4 b4 f0 23 01 00 00[ 	]*vpmadd52luq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 31[ 	]*vpmadd52luq \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 7f[ 	]*vpmadd52luq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 00 08 00 00[ 	]*vpmadd52luq 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 80[ 	]*vpmadd52luq -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 80[ 	]*vpmadd52luq -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b4 f4[ 	]*vpmadd52luq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b4 f4[ 	]*vpmadd52luq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b4 f4[ 	]*vpmadd52luq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 31[ 	]*vpmadd52luq \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b4 b4 f0 23 01 00 00[ 	]*vpmadd52luq 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 31[ 	]*vpmadd52luq \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 7f[ 	]*vpmadd52luq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 00 10 00 00[ 	]*vpmadd52luq 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 80[ 	]*vpmadd52luq -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 e0 ef ff ff[ 	]*vpmadd52luq -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 80[ 	]*vpmadd52luq -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b5 f4[ 	]*vpmadd52huq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b5 f4[ 	]*vpmadd52huq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b5 f4[ 	]*vpmadd52huq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 31[ 	]*vpmadd52huq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b5 b4 f0 23 01 00 00[ 	]*vpmadd52huq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 31[ 	]*vpmadd52huq \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 7f[ 	]*vpmadd52huq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 00 08 00 00[ 	]*vpmadd52huq 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 80[ 	]*vpmadd52huq -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 80[ 	]*vpmadd52huq -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b5 f4[ 	]*vpmadd52huq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b5 f4[ 	]*vpmadd52huq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b5 f4[ 	]*vpmadd52huq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 31[ 	]*vpmadd52huq \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b5 b4 f0 23 01 00 00[ 	]*vpmadd52huq 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 31[ 	]*vpmadd52huq \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 7f[ 	]*vpmadd52huq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 00 10 00 00[ 	]*vpmadd52huq 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 80[ 	]*vpmadd52huq -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 e0 ef ff ff[ 	]*vpmadd52huq -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 80[ 	]*vpmadd52huq -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b4 f4[ 	]*vpmadd52luq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b4 f4[ 	]*vpmadd52luq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b4 f4[ 	]*vpmadd52luq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 31[ 	]*vpmadd52luq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b4 b4 f0 34 12 00 00[ 	]*vpmadd52luq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 31[ 	]*vpmadd52luq \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 7f[ 	]*vpmadd52luq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 00 08 00 00[ 	]*vpmadd52luq 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 80[ 	]*vpmadd52luq -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 80[ 	]*vpmadd52luq -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b4 f4[ 	]*vpmadd52luq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b4 f4[ 	]*vpmadd52luq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b4 f4[ 	]*vpmadd52luq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 31[ 	]*vpmadd52luq \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b4 b4 f0 34 12 00 00[ 	]*vpmadd52luq 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 31[ 	]*vpmadd52luq \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 7f[ 	]*vpmadd52luq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 00 10 00 00[ 	]*vpmadd52luq 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 80[ 	]*vpmadd52luq -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 e0 ef ff ff[ 	]*vpmadd52luq -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 80[ 	]*vpmadd52luq -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b5 f4[ 	]*vpmadd52huq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b5 f4[ 	]*vpmadd52huq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b5 f4[ 	]*vpmadd52huq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 31[ 	]*vpmadd52huq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b5 b4 f0 34 12 00 00[ 	]*vpmadd52huq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 31[ 	]*vpmadd52huq \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 7f[ 	]*vpmadd52huq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 00 08 00 00[ 	]*vpmadd52huq 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 80[ 	]*vpmadd52huq -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 80[ 	]*vpmadd52huq -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b5 f4[ 	]*vpmadd52huq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b5 f4[ 	]*vpmadd52huq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b5 f4[ 	]*vpmadd52huq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 31[ 	]*vpmadd52huq \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b5 b4 f0 34 12 00 00[ 	]*vpmadd52huq 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 31[ 	]*vpmadd52huq \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 7f[ 	]*vpmadd52huq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 00 10 00 00[ 	]*vpmadd52huq 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 80[ 	]*vpmadd52huq -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 e0 ef ff ff[ 	]*vpmadd52huq -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 80[ 	]*vpmadd52huq -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
#pass
