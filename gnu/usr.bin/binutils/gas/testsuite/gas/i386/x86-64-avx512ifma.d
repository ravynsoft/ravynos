#as:
#objdump: -dw
#name: x86_64 AVX512IFMA insns
#source: x86-64-avx512ifma.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 31[ 	]*vpmadd52luq \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b4 b4 f0 23 01 00 00[ 	]*vpmadd52luq 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 31[ 	]*vpmadd52luq \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 7f[ 	]*vpmadd52luq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 00 20 00 00[ 	]*vpmadd52luq 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 80[ 	]*vpmadd52luq -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 c0 df ff ff[ 	]*vpmadd52luq -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 80[ 	]*vpmadd52luq -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b5 f4[ 	]*vpmadd52huq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b5 f4[ 	]*vpmadd52huq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b5 f4[ 	]*vpmadd52huq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 31[ 	]*vpmadd52huq \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b5 b4 f0 23 01 00 00[ 	]*vpmadd52huq 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 31[ 	]*vpmadd52huq \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 7f[ 	]*vpmadd52huq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 00 20 00 00[ 	]*vpmadd52huq 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 80[ 	]*vpmadd52huq -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 c0 df ff ff[ 	]*vpmadd52huq -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 80[ 	]*vpmadd52huq -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 31[ 	]*vpmadd52luq \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b4 b4 f0 34 12 00 00[ 	]*vpmadd52luq 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 31[ 	]*vpmadd52luq \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 7f[ 	]*vpmadd52luq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 00 20 00 00[ 	]*vpmadd52luq 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 80[ 	]*vpmadd52luq -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 c0 df ff ff[ 	]*vpmadd52luq -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 80[ 	]*vpmadd52luq -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b5 f4[ 	]*vpmadd52huq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b5 f4[ 	]*vpmadd52huq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b5 f4[ 	]*vpmadd52huq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 31[ 	]*vpmadd52huq \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b5 b4 f0 34 12 00 00[ 	]*vpmadd52huq 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 31[ 	]*vpmadd52huq \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 7f[ 	]*vpmadd52huq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 00 20 00 00[ 	]*vpmadd52huq 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 80[ 	]*vpmadd52huq -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 c0 df ff ff[ 	]*vpmadd52huq -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 80[ 	]*vpmadd52huq -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
#pass
