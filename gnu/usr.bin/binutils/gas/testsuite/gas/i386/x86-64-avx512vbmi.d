#as:
#objdump: -dw
#name: x86_64 AVX512VBMI insns
#source: x86-64-avx512vbmi.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 31[ 	]*vpermb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 8d b4 f0 23 01 00 00[ 	]*vpermb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 7f[ 	]*vpermb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 00 20 00 00[ 	]*vpermb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 80[ 	]*vpermb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 c0 df ff ff[ 	]*vpermb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 75 f4[ 	]*vpermi2b %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 75 f4[ 	]*vpermi2b %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 75 f4[ 	]*vpermi2b %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 31[ 	]*vpermi2b \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 75 b4 f0 23 01 00 00[ 	]*vpermi2b 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 7f[ 	]*vpermi2b 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 00 20 00 00[ 	]*vpermi2b 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 80[ 	]*vpermi2b -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 c0 df ff ff[ 	]*vpermi2b -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 7d f4[ 	]*vpermt2b %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 7d f4[ 	]*vpermt2b %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 7d f4[ 	]*vpermt2b %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 31[ 	]*vpermt2b \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 7d b4 f0 23 01 00 00[ 	]*vpermt2b 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 7f[ 	]*vpermt2b 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 00 20 00 00[ 	]*vpermt2b 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 80[ 	]*vpermt2b -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 c0 df ff ff[ 	]*vpermt2b -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 83 f4[ 	]*vpmultishiftqb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 83 f4[ 	]*vpmultishiftqb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 83 f4[ 	]*vpmultishiftqb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 31[ 	]*vpmultishiftqb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 83 b4 f0 23 01 00 00[ 	]*vpmultishiftqb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 31[ 	]*vpmultishiftqb \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 7f[ 	]*vpmultishiftqb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 00 20 00 00[ 	]*vpmultishiftqb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 80[ 	]*vpmultishiftqb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 c0 df ff ff[ 	]*vpmultishiftqb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 80[ 	]*vpmultishiftqb -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 31[ 	]*vpermb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 8d b4 f0 34 12 00 00[ 	]*vpermb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 7f[ 	]*vpermb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 00 20 00 00[ 	]*vpermb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 80[ 	]*vpermb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 c0 df ff ff[ 	]*vpermb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 75 f4[ 	]*vpermi2b %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 75 f4[ 	]*vpermi2b %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 75 f4[ 	]*vpermi2b %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 31[ 	]*vpermi2b \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 75 b4 f0 34 12 00 00[ 	]*vpermi2b 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 7f[ 	]*vpermi2b 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 00 20 00 00[ 	]*vpermi2b 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 80[ 	]*vpermi2b -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 c0 df ff ff[ 	]*vpermi2b -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 7d f4[ 	]*vpermt2b %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 7d f4[ 	]*vpermt2b %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 7d f4[ 	]*vpermt2b %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 31[ 	]*vpermt2b \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 7d b4 f0 34 12 00 00[ 	]*vpermt2b 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 7f[ 	]*vpermt2b 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 00 20 00 00[ 	]*vpermt2b 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 80[ 	]*vpermt2b -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 c0 df ff ff[ 	]*vpermt2b -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 83 f4[ 	]*vpmultishiftqb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 83 f4[ 	]*vpmultishiftqb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 83 f4[ 	]*vpmultishiftqb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 31[ 	]*vpmultishiftqb \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 83 b4 f0 34 12 00 00[ 	]*vpmultishiftqb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 31[ 	]*vpmultishiftqb \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 7f[ 	]*vpmultishiftqb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 00 20 00 00[ 	]*vpmultishiftqb 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 80[ 	]*vpmultishiftqb -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 c0 df ff ff[ 	]*vpmultishiftqb -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 80[ 	]*vpmultishiftqb -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
#pass
