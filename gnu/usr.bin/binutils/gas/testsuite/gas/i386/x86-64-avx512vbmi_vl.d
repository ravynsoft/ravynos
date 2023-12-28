#as:
#objdump: -dw
#name: x86_64 AVX512VBMI/VL insns
#source: x86-64-avx512vbmi_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 8d f4[ 	]*vpermb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 8d f4[ 	]*vpermb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 8d f4[ 	]*vpermb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 31[ 	]*vpermb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 8d b4 f0 23 01 00 00[ 	]*vpermb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 7f[ 	]*vpermb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 00 08 00 00[ 	]*vpermb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 80[ 	]*vpermb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 f0 f7 ff ff[ 	]*vpermb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 8d f4[ 	]*vpermb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 8d f4[ 	]*vpermb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 8d f4[ 	]*vpermb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 31[ 	]*vpermb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 8d b4 f0 23 01 00 00[ 	]*vpermb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 7f[ 	]*vpermb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 00 10 00 00[ 	]*vpermb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 80[ 	]*vpermb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 e0 ef ff ff[ 	]*vpermb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 75 f4[ 	]*vpermi2b %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 75 f4[ 	]*vpermi2b %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 75 f4[ 	]*vpermi2b %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 31[ 	]*vpermi2b \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 75 b4 f0 23 01 00 00[ 	]*vpermi2b 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 7f[ 	]*vpermi2b 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 00 08 00 00[ 	]*vpermi2b 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 80[ 	]*vpermi2b -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 f0 f7 ff ff[ 	]*vpermi2b -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 75 f4[ 	]*vpermi2b %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 75 f4[ 	]*vpermi2b %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 75 f4[ 	]*vpermi2b %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 31[ 	]*vpermi2b \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 75 b4 f0 23 01 00 00[ 	]*vpermi2b 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 7f[ 	]*vpermi2b 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 00 10 00 00[ 	]*vpermi2b 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 80[ 	]*vpermi2b -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 e0 ef ff ff[ 	]*vpermi2b -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 7d f4[ 	]*vpermt2b %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 7d f4[ 	]*vpermt2b %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 7d f4[ 	]*vpermt2b %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 31[ 	]*vpermt2b \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 7d b4 f0 23 01 00 00[ 	]*vpermt2b 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 7f[ 	]*vpermt2b 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 00 08 00 00[ 	]*vpermt2b 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 80[ 	]*vpermt2b -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 f0 f7 ff ff[ 	]*vpermt2b -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 7d f4[ 	]*vpermt2b %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 7d f4[ 	]*vpermt2b %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 7d f4[ 	]*vpermt2b %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 31[ 	]*vpermt2b \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 7d b4 f0 23 01 00 00[ 	]*vpermt2b 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 7f[ 	]*vpermt2b 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 00 10 00 00[ 	]*vpermt2b 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 80[ 	]*vpermt2b -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 e0 ef ff ff[ 	]*vpermt2b -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 83 f4[ 	]*vpmultishiftqb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 83 f4[ 	]*vpmultishiftqb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 83 f4[ 	]*vpmultishiftqb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 31[ 	]*vpmultishiftqb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 83 b4 f0 23 01 00 00[ 	]*vpmultishiftqb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 31[ 	]*vpmultishiftqb \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 7f[ 	]*vpmultishiftqb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 00 08 00 00[ 	]*vpmultishiftqb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 80[ 	]*vpmultishiftqb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 80[ 	]*vpmultishiftqb -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 83 f4[ 	]*vpmultishiftqb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 83 f4[ 	]*vpmultishiftqb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 83 f4[ 	]*vpmultishiftqb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 31[ 	]*vpmultishiftqb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 83 b4 f0 23 01 00 00[ 	]*vpmultishiftqb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 31[ 	]*vpmultishiftqb \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 7f[ 	]*vpmultishiftqb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 00 10 00 00[ 	]*vpmultishiftqb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 80[ 	]*vpmultishiftqb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 80[ 	]*vpmultishiftqb -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 8d f4[ 	]*vpermb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 8d f4[ 	]*vpermb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 8d f4[ 	]*vpermb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 31[ 	]*vpermb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 8d b4 f0 34 12 00 00[ 	]*vpermb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 7f[ 	]*vpermb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 00 08 00 00[ 	]*vpermb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 80[ 	]*vpermb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 f0 f7 ff ff[ 	]*vpermb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 8d f4[ 	]*vpermb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 8d f4[ 	]*vpermb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 8d f4[ 	]*vpermb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 31[ 	]*vpermb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 8d b4 f0 34 12 00 00[ 	]*vpermb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 7f[ 	]*vpermb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 00 10 00 00[ 	]*vpermb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 80[ 	]*vpermb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 e0 ef ff ff[ 	]*vpermb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 75 f4[ 	]*vpermi2b %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 75 f4[ 	]*vpermi2b %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 75 f4[ 	]*vpermi2b %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 31[ 	]*vpermi2b \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 75 b4 f0 34 12 00 00[ 	]*vpermi2b 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 7f[ 	]*vpermi2b 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 00 08 00 00[ 	]*vpermi2b 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 80[ 	]*vpermi2b -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 f0 f7 ff ff[ 	]*vpermi2b -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 75 f4[ 	]*vpermi2b %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 75 f4[ 	]*vpermi2b %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 75 f4[ 	]*vpermi2b %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 31[ 	]*vpermi2b \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 75 b4 f0 34 12 00 00[ 	]*vpermi2b 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 7f[ 	]*vpermi2b 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 00 10 00 00[ 	]*vpermi2b 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 80[ 	]*vpermi2b -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 e0 ef ff ff[ 	]*vpermi2b -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 7d f4[ 	]*vpermt2b %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 7d f4[ 	]*vpermt2b %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 7d f4[ 	]*vpermt2b %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 31[ 	]*vpermt2b \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 7d b4 f0 34 12 00 00[ 	]*vpermt2b 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 7f[ 	]*vpermt2b 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 00 08 00 00[ 	]*vpermt2b 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 80[ 	]*vpermt2b -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 f0 f7 ff ff[ 	]*vpermt2b -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 7d f4[ 	]*vpermt2b %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 7d f4[ 	]*vpermt2b %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 7d f4[ 	]*vpermt2b %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 31[ 	]*vpermt2b \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 7d b4 f0 34 12 00 00[ 	]*vpermt2b 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 7f[ 	]*vpermt2b 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 00 10 00 00[ 	]*vpermt2b 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 80[ 	]*vpermt2b -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 e0 ef ff ff[ 	]*vpermt2b -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 83 f4[ 	]*vpmultishiftqb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 83 f4[ 	]*vpmultishiftqb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 83 f4[ 	]*vpmultishiftqb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 31[ 	]*vpmultishiftqb \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 83 b4 f0 34 12 00 00[ 	]*vpmultishiftqb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 31[ 	]*vpmultishiftqb \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 7f[ 	]*vpmultishiftqb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 00 08 00 00[ 	]*vpmultishiftqb 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 80[ 	]*vpmultishiftqb -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 80[ 	]*vpmultishiftqb -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 83 f4[ 	]*vpmultishiftqb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 83 f4[ 	]*vpmultishiftqb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 83 f4[ 	]*vpmultishiftqb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 31[ 	]*vpmultishiftqb \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 83 b4 f0 34 12 00 00[ 	]*vpmultishiftqb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 31[ 	]*vpmultishiftqb \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 7f[ 	]*vpmultishiftqb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 00 10 00 00[ 	]*vpmultishiftqb 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 80[ 	]*vpmultishiftqb -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 80[ 	]*vpmultishiftqb -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
#pass
