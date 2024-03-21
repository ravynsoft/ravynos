#as:
#objdump: -dw
#name: x86_64 AVX512DQ/VL insns
#source: x86-64-avx512dq_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1a b4 f0 23 01 00 00[ 	]*vbroadcastf64x2 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 5a b4 f0 23 01 00 00[ 	]*vbroadcasti64x2 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 19 f7[ 	]*vbroadcastf32x2 %xmm31,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 19 f7[ 	]*vbroadcastf32x2 %xmm31,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 19 f7[ 	]*vbroadcastf32x2 %xmm31,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 31[ 	]*vbroadcastf32x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 19 b4 f0 23 01 00 00[ 	]*vbroadcastf32x2 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7b f5[ 	]*vcvtpd2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7b f5[ 	]*vcvtpd2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7b f5[ 	]*vcvtpd2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 31[ 	]*vcvtpd2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7b b4 f0 23 01 00 00[ 	]*vcvtpd2qq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 31[ 	]*vcvtpd2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 7f[ 	]*vcvtpd2qq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 00 08 00 00[ 	]*vcvtpd2qq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 80[ 	]*vcvtpd2qq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 80[ 	]*vcvtpd2qq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7b f5[ 	]*vcvtpd2qq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7b f5[ 	]*vcvtpd2qq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7b f5[ 	]*vcvtpd2qq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 31[ 	]*vcvtpd2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7b b4 f0 23 01 00 00[ 	]*vcvtpd2qq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 31[ 	]*vcvtpd2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 7f[ 	]*vcvtpd2qq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 00 10 00 00[ 	]*vcvtpd2qq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 80[ 	]*vcvtpd2qq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 80[ 	]*vcvtpd2qq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 79 f5[ 	]*vcvtpd2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 79 f5[ 	]*vcvtpd2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 79 f5[ 	]*vcvtpd2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 31[ 	]*vcvtpd2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 79 b4 f0 23 01 00 00[ 	]*vcvtpd2uqq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 31[ 	]*vcvtpd2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 7f[ 	]*vcvtpd2uqq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 00 08 00 00[ 	]*vcvtpd2uqq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 80[ 	]*vcvtpd2uqq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 80[ 	]*vcvtpd2uqq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 79 f5[ 	]*vcvtpd2uqq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 79 f5[ 	]*vcvtpd2uqq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 79 f5[ 	]*vcvtpd2uqq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 31[ 	]*vcvtpd2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 79 b4 f0 23 01 00 00[ 	]*vcvtpd2uqq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 31[ 	]*vcvtpd2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 7f[ 	]*vcvtpd2uqq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 00 10 00 00[ 	]*vcvtpd2uqq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 80[ 	]*vcvtpd2uqq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 80[ 	]*vcvtpd2uqq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7b f5[ 	]*vcvtps2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7b f5[ 	]*vcvtps2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7b f5[ 	]*vcvtps2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 31[ 	]*vcvtps2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7b b4 f0 23 01 00 00[ 	]*vcvtps2qq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 31[ 	]*vcvtps2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 7f[ 	]*vcvtps2qq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 00 04 00 00[ 	]*vcvtps2qq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 80[ 	]*vcvtps2qq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 f8 fb ff ff[ 	]*vcvtps2qq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 80[ 	]*vcvtps2qq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7b f5[ 	]*vcvtps2qq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7b f5[ 	]*vcvtps2qq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7b f5[ 	]*vcvtps2qq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 31[ 	]*vcvtps2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7b b4 f0 23 01 00 00[ 	]*vcvtps2qq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 31[ 	]*vcvtps2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 7f[ 	]*vcvtps2qq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 00 08 00 00[ 	]*vcvtps2qq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 80[ 	]*vcvtps2qq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 80[ 	]*vcvtps2qq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 79 f5[ 	]*vcvtps2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 79 f5[ 	]*vcvtps2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 79 f5[ 	]*vcvtps2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 31[ 	]*vcvtps2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 79 b4 f0 23 01 00 00[ 	]*vcvtps2uqq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 31[ 	]*vcvtps2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 7f[ 	]*vcvtps2uqq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 00 04 00 00[ 	]*vcvtps2uqq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 80[ 	]*vcvtps2uqq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 80[ 	]*vcvtps2uqq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 79 f5[ 	]*vcvtps2uqq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 79 f5[ 	]*vcvtps2uqq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 79 f5[ 	]*vcvtps2uqq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 31[ 	]*vcvtps2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 79 b4 f0 23 01 00 00[ 	]*vcvtps2uqq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 31[ 	]*vcvtps2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 7f[ 	]*vcvtps2uqq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 00 08 00 00[ 	]*vcvtps2uqq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 80[ 	]*vcvtps2uqq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 80[ 	]*vcvtps2uqq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 e6 f5[ 	]*vcvtqq2pd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f e6 f5[ 	]*vcvtqq2pd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f e6 f5[ 	]*vcvtqq2pd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 31[ 	]*vcvtqq2pd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 e6 b4 f0 23 01 00 00[ 	]*vcvtqq2pd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 31[ 	]*vcvtqq2pd \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 7f[ 	]*vcvtqq2pd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 00 08 00 00[ 	]*vcvtqq2pd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 80[ 	]*vcvtqq2pd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 80[ 	]*vcvtqq2pd -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 e6 f5[ 	]*vcvtqq2pd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f e6 f5[ 	]*vcvtqq2pd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af e6 f5[ 	]*vcvtqq2pd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 31[ 	]*vcvtqq2pd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 e6 b4 f0 23 01 00 00[ 	]*vcvtqq2pd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 31[ 	]*vcvtqq2pd \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 7f[ 	]*vcvtqq2pd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 00 10 00 00[ 	]*vcvtqq2pd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 80[ 	]*vcvtqq2pd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 80[ 	]*vcvtqq2pd -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 08 5b f5[ 	]*vcvtqq2ps %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 0f 5b f5[ 	]*vcvtqq2ps %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 8f 5b f5[ 	]*vcvtqq2ps %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 31[ 	]*vcvtqq2psx \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 08 5b b4 f0 23 01 00 00[ 	]*vcvtqq2psx 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 31[ 	]*vcvtqq2ps \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 7f[ 	]*vcvtqq2psx 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 00 08 00 00[ 	]*vcvtqq2psx 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 80[ 	]*vcvtqq2psx -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 f0 f7 ff ff[ 	]*vcvtqq2psx -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 80[ 	]*vcvtqq2ps -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 28 5b f5[ 	]*vcvtqq2ps %ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 2f 5b f5[ 	]*vcvtqq2ps %ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc af 5b f5[ 	]*vcvtqq2ps %ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 31[ 	]*vcvtqq2psy \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 28 5b b4 f0 23 01 00 00[ 	]*vcvtqq2psy 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 31[ 	]*vcvtqq2ps \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 7f[ 	]*vcvtqq2psy 0xfe0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 00 10 00 00[ 	]*vcvtqq2psy 0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 80[ 	]*vcvtqq2psy -0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 e0 ef ff ff[ 	]*vcvtqq2psy -0x1020\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 80[ 	]*vcvtqq2ps -0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 7a f5[ 	]*vcvtuqq2pd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f 7a f5[ 	]*vcvtuqq2pd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f 7a f5[ 	]*vcvtuqq2pd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 31[ 	]*vcvtuqq2pd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2pd 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 31[ 	]*vcvtuqq2pd \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 7f[ 	]*vcvtuqq2pd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 00 08 00 00[ 	]*vcvtuqq2pd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 80[ 	]*vcvtuqq2pd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 7a f5[ 	]*vcvtuqq2pd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f 7a f5[ 	]*vcvtuqq2pd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af 7a f5[ 	]*vcvtuqq2pd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 31[ 	]*vcvtuqq2pd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2pd 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 31[ 	]*vcvtuqq2pd \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 7f[ 	]*vcvtuqq2pd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 00 10 00 00[ 	]*vcvtuqq2pd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 80[ 	]*vcvtuqq2pd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7a f5[ 	]*vcvtuqq2ps %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7a f5[ 	]*vcvtuqq2ps %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7a f5[ 	]*vcvtuqq2ps %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 31[ 	]*vcvtuqq2psx \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2psx 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 31[ 	]*vcvtuqq2ps \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 7f[ 	]*vcvtuqq2psx 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 00 08 00 00[ 	]*vcvtuqq2psx 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 80[ 	]*vcvtuqq2psx -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2psx -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7a f5[ 	]*vcvtuqq2ps %ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7a f5[ 	]*vcvtuqq2ps %ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7a f5[ 	]*vcvtuqq2ps %ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 31[ 	]*vcvtuqq2psy \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2psy 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 31[ 	]*vcvtuqq2ps \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 7f[ 	]*vcvtuqq2psy 0xfe0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 00 10 00 00[ 	]*vcvtuqq2psy 0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 80[ 	]*vcvtuqq2psy -0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2psy -0x1020\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee ab[ 	]*vfpclasspd \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 66 ee ab[ 	]*vfpclasspd \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee 7b[ 	]*vfpclasspd \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 29 7b[ 	]*vfpclasspdx \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspdx \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 29 7b[ 	]*vfpclasspd \$0x7b,\(%rcx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 7f 7b[ 	]*vfpclasspdx \$0x7b,0x7f0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa 00 08 00 00 7b[ 	]*vfpclasspdx \$0x7b,0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 80 7b[ 	]*vfpclasspdx \$0x7b,-0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspdx \$0x7b,-0x810\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee ab[ 	]*vfpclasspd \$0xab,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 2f 66 ee ab[ 	]*vfpclasspd \$0xab,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee 7b[ 	]*vfpclasspd \$0x7b,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 29 7b[ 	]*vfpclasspdy \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 28 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspdy \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 29 7b[ 	]*vfpclasspd \$0x7b,\(%rcx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 7f 7b[ 	]*vfpclasspdy \$0x7b,0xfe0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa 00 10 00 00 7b[ 	]*vfpclasspdy \$0x7b,0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 80 7b[ 	]*vfpclasspdy \$0x7b,-0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa e0 ef ff ff 7b[ 	]*vfpclasspdy \$0x7b,-0x1020\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee ab[ 	]*vfpclassps \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 66 ee ab[ 	]*vfpclassps \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee 7b[ 	]*vfpclassps \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 29 7b[ 	]*vfpclasspsx \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspsx \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 29 7b[ 	]*vfpclassps \$0x7b,\(%rcx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 7f 7b[ 	]*vfpclasspsx \$0x7b,0x7f0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa 00 08 00 00 7b[ 	]*vfpclasspsx \$0x7b,0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 80 7b[ 	]*vfpclasspsx \$0x7b,-0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspsx \$0x7b,-0x810\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee ab[ 	]*vfpclassps \$0xab,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 2f 66 ee ab[ 	]*vfpclassps \$0xab,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee 7b[ 	]*vfpclassps \$0x7b,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 29 7b[ 	]*vfpclasspsy \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 28 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspsy \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 29 7b[ 	]*vfpclassps \$0x7b,\(%rcx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 7f 7b[ 	]*vfpclasspsy \$0x7b,0xfe0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa 00 10 00 00 7b[ 	]*vfpclasspsy \$0x7b,0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 80 7b[ 	]*vfpclasspsy \$0x7b,-0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa e0 ef ff ff 7b[ 	]*vfpclasspsy \$0x7b,-0x1020\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 18 b4 f0 23 01 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 38 b4 f0 23 01 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 59 f7[ 	]*vbroadcasti32x2 %xmm31,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 31[ 	]*vbroadcasti32x2 \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 59 b4 f0 23 01 00 00[ 	]*vbroadcasti32x2 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 59 f7[ 	]*vbroadcasti32x2 %xmm31,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 59 f7[ 	]*vbroadcasti32x2 %xmm31,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 31[ 	]*vbroadcasti32x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 59 b4 f0 23 01 00 00[ 	]*vbroadcasti32x2 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 40 f4[ 	]*vpmullq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 40 f4[ 	]*vpmullq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 40 f4[ 	]*vpmullq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 31[ 	]*vpmullq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 40 b4 f0 23 01 00 00[ 	]*vpmullq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 31[ 	]*vpmullq \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 7f[ 	]*vpmullq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 00 08 00 00[ 	]*vpmullq 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 80[ 	]*vpmullq -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 f0 f7 ff ff[ 	]*vpmullq -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 7f[ 	]*vpmullq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 80[ 	]*vpmullq -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 40 f4[ 	]*vpmullq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 40 f4[ 	]*vpmullq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 40 f4[ 	]*vpmullq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 31[ 	]*vpmullq \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 40 b4 f0 23 01 00 00[ 	]*vpmullq 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 31[ 	]*vpmullq \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 7f[ 	]*vpmullq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 00 10 00 00[ 	]*vpmullq 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 80[ 	]*vpmullq -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 e0 ef ff ff[ 	]*vpmullq -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 7f[ 	]*vpmullq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 80[ 	]*vpmullq -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 ab[ 	]*vrangepd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 50 f4 ab[ 	]*vrangepd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 50 f4 ab[ 	]*vrangepd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 7b[ 	]*vrangepd \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 50 b4 f0 23 01 00 00 7b[ 	]*vrangepd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 00 08 00 00 7b[ 	]*vrangepd \$0x7b,0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd \$0x7b,-0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 ab[ 	]*vrangepd \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 50 f4 ab[ 	]*vrangepd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 50 f4 ab[ 	]*vrangepd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 7b[ 	]*vrangepd \$0x7b,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 50 b4 f0 23 01 00 00 7b[ 	]*vrangepd \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 7f 7b[ 	]*vrangepd \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 00 10 00 00 7b[ 	]*vrangepd \$0x7b,0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 e0 ef ff ff 7b[ 	]*vrangepd \$0x7b,-0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 ab[ 	]*vrangeps \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 50 f4 ab[ 	]*vrangeps \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 50 f4 ab[ 	]*vrangeps \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 7b[ 	]*vrangeps \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 50 b4 f0 23 01 00 00 7b[ 	]*vrangeps \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 00 08 00 00 7b[ 	]*vrangeps \$0x7b,0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps \$0x7b,-0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 ab[ 	]*vrangeps \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 50 f4 ab[ 	]*vrangeps \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 50 f4 ab[ 	]*vrangeps \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 7b[ 	]*vrangeps \$0x7b,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 50 b4 f0 23 01 00 00 7b[ 	]*vrangeps \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 7f 7b[ 	]*vrangeps \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 00 10 00 00 7b[ 	]*vrangeps \$0x7b,0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 e0 ef ff ff 7b[ 	]*vrangeps \$0x7b,-0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 54 f4[ 	]*vandpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 54 f4[ 	]*vandpd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 54 f4[ 	]*vandpd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 31[ 	]*vandpd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 54 b4 f0 23 01 00 00[ 	]*vandpd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 31[ 	]*vandpd \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 7f[ 	]*vandpd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 00 08 00 00[ 	]*vandpd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 80[ 	]*vandpd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 f0 f7 ff ff[ 	]*vandpd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 7f[ 	]*vandpd 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 80[ 	]*vandpd -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 54 f4[ 	]*vandpd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 54 f4[ 	]*vandpd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 54 f4[ 	]*vandpd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 31[ 	]*vandpd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 54 b4 f0 23 01 00 00[ 	]*vandpd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 31[ 	]*vandpd \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 7f[ 	]*vandpd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 00 10 00 00[ 	]*vandpd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 80[ 	]*vandpd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 e0 ef ff ff[ 	]*vandpd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 7f[ 	]*vandpd 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 80[ 	]*vandpd -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 54 f4[ 	]*vandps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 54 f4[ 	]*vandps %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 54 f4[ 	]*vandps %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 31[ 	]*vandps \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 54 b4 f0 23 01 00 00[ 	]*vandps 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 31[ 	]*vandps \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 7f[ 	]*vandps 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 00 08 00 00[ 	]*vandps 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 80[ 	]*vandps -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 f0 f7 ff ff[ 	]*vandps -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 7f[ 	]*vandps 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 00 02 00 00[ 	]*vandps 0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 80[ 	]*vandps -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 54 f4[ 	]*vandps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 54 f4[ 	]*vandps %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 54 f4[ 	]*vandps %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 31[ 	]*vandps \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 54 b4 f0 23 01 00 00[ 	]*vandps 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 31[ 	]*vandps \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 7f[ 	]*vandps 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 00 10 00 00[ 	]*vandps 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 80[ 	]*vandps -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 e0 ef ff ff[ 	]*vandps -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 7f[ 	]*vandps 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 00 02 00 00[ 	]*vandps 0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 80[ 	]*vandps -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 55 f4[ 	]*vandnpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 55 f4[ 	]*vandnpd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 55 f4[ 	]*vandnpd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 31[ 	]*vandnpd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 55 b4 f0 23 01 00 00[ 	]*vandnpd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 31[ 	]*vandnpd \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 7f[ 	]*vandnpd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 00 08 00 00[ 	]*vandnpd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 80[ 	]*vandnpd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 f0 f7 ff ff[ 	]*vandnpd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 7f[ 	]*vandnpd 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 80[ 	]*vandnpd -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 55 f4[ 	]*vandnpd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 55 f4[ 	]*vandnpd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 55 f4[ 	]*vandnpd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 31[ 	]*vandnpd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 55 b4 f0 23 01 00 00[ 	]*vandnpd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 31[ 	]*vandnpd \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 7f[ 	]*vandnpd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 00 10 00 00[ 	]*vandnpd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 80[ 	]*vandnpd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 e0 ef ff ff[ 	]*vandnpd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 7f[ 	]*vandnpd 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 80[ 	]*vandnpd -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 55 f4[ 	]*vandnps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 55 f4[ 	]*vandnps %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 55 f4[ 	]*vandnps %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 31[ 	]*vandnps \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 55 b4 f0 23 01 00 00[ 	]*vandnps 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 31[ 	]*vandnps \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 7f[ 	]*vandnps 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 00 08 00 00[ 	]*vandnps 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 80[ 	]*vandnps -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 f0 f7 ff ff[ 	]*vandnps -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 7f[ 	]*vandnps 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 80[ 	]*vandnps -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 55 f4[ 	]*vandnps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 55 f4[ 	]*vandnps %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 55 f4[ 	]*vandnps %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 31[ 	]*vandnps \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 55 b4 f0 23 01 00 00[ 	]*vandnps 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 31[ 	]*vandnps \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 7f[ 	]*vandnps 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 00 10 00 00[ 	]*vandnps 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 80[ 	]*vandnps -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 e0 ef ff ff[ 	]*vandnps -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 7f[ 	]*vandnps 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 80[ 	]*vandnps -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 56 f4[ 	]*vorpd  %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 56 f4[ 	]*vorpd  %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 56 f4[ 	]*vorpd  %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 31[ 	]*vorpd  \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 56 b4 f0 23 01 00 00[ 	]*vorpd  0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 31[ 	]*vorpd  \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 7f[ 	]*vorpd  0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 00 08 00 00[ 	]*vorpd  0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 80[ 	]*vorpd  -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 f0 f7 ff ff[ 	]*vorpd  -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 7f[ 	]*vorpd  0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 80[ 	]*vorpd  -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 56 f4[ 	]*vorpd  %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 56 f4[ 	]*vorpd  %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 56 f4[ 	]*vorpd  %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 31[ 	]*vorpd  \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 56 b4 f0 23 01 00 00[ 	]*vorpd  0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 31[ 	]*vorpd  \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 7f[ 	]*vorpd  0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 00 10 00 00[ 	]*vorpd  0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 80[ 	]*vorpd  -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 e0 ef ff ff[ 	]*vorpd  -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 7f[ 	]*vorpd  0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 80[ 	]*vorpd  -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 56 f4[ 	]*vorps  %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 56 f4[ 	]*vorps  %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 56 f4[ 	]*vorps  %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 31[ 	]*vorps  \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 56 b4 f0 23 01 00 00[ 	]*vorps  0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 31[ 	]*vorps  \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 7f[ 	]*vorps  0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 00 08 00 00[ 	]*vorps  0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 80[ 	]*vorps  -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 f0 f7 ff ff[ 	]*vorps  -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 7f[ 	]*vorps  0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 00 02 00 00[ 	]*vorps  0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 80[ 	]*vorps  -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 56 f4[ 	]*vorps  %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 56 f4[ 	]*vorps  %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 56 f4[ 	]*vorps  %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 31[ 	]*vorps  \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 56 b4 f0 23 01 00 00[ 	]*vorps  0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 31[ 	]*vorps  \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 7f[ 	]*vorps  0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 00 10 00 00[ 	]*vorps  0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 80[ 	]*vorps  -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 e0 ef ff ff[ 	]*vorps  -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 7f[ 	]*vorps  0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 00 02 00 00[ 	]*vorps  0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 80[ 	]*vorps  -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 57 f4[ 	]*vxorpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 57 f4[ 	]*vxorpd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 57 f4[ 	]*vxorpd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 31[ 	]*vxorpd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 57 b4 f0 23 01 00 00[ 	]*vxorpd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 31[ 	]*vxorpd \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 7f[ 	]*vxorpd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 00 08 00 00[ 	]*vxorpd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 80[ 	]*vxorpd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 f0 f7 ff ff[ 	]*vxorpd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 7f[ 	]*vxorpd 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 80[ 	]*vxorpd -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 57 f4[ 	]*vxorpd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 57 f4[ 	]*vxorpd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 57 f4[ 	]*vxorpd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 31[ 	]*vxorpd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 57 b4 f0 23 01 00 00[ 	]*vxorpd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 31[ 	]*vxorpd \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 7f[ 	]*vxorpd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 00 10 00 00[ 	]*vxorpd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 80[ 	]*vxorpd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 e0 ef ff ff[ 	]*vxorpd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 7f[ 	]*vxorpd 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 80[ 	]*vxorpd -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 57 f4[ 	]*vxorps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 57 f4[ 	]*vxorps %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 57 f4[ 	]*vxorps %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 31[ 	]*vxorps \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 57 b4 f0 23 01 00 00[ 	]*vxorps 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 31[ 	]*vxorps \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 7f[ 	]*vxorps 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 00 08 00 00[ 	]*vxorps 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 80[ 	]*vxorps -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 f0 f7 ff ff[ 	]*vxorps -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 7f[ 	]*vxorps 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 80[ 	]*vxorps -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 57 f4[ 	]*vxorps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 57 f4[ 	]*vxorps %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 57 f4[ 	]*vxorps %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 31[ 	]*vxorps \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 57 b4 f0 23 01 00 00[ 	]*vxorps 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 31[ 	]*vxorps \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 7f[ 	]*vxorps 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 00 10 00 00[ 	]*vxorps 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 80[ 	]*vxorps -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 e0 ef ff ff[ 	]*vxorps -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 7f[ 	]*vxorps 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 80[ 	]*vxorps -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 ab[ 	]*vreducepd \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 0f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 8f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 7b[ 	]*vreducepd \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 56 b4 f0 23 01 00 00 7b[ 	]*vreducepd \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 00 08 00 00 7b[ 	]*vreducepd \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 ab[ 	]*vreducepd \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 56 f5 ab[ 	]*vreducepd \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 56 f5 ab[ 	]*vreducepd \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 7b[ 	]*vreducepd \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 56 b4 f0 23 01 00 00 7b[ 	]*vreducepd \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 7f 7b[ 	]*vreducepd \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 00 10 00 00 7b[ 	]*vreducepd \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 e0 ef ff ff 7b[ 	]*vreducepd \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 ab[ 	]*vreduceps \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 0f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 8f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 7b[ 	]*vreduceps \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 56 b4 f0 23 01 00 00 7b[ 	]*vreduceps \$0x7b,0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 00 08 00 00 7b[ 	]*vreduceps \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 ab[ 	]*vreduceps \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 2f 56 f5 ab[ 	]*vreduceps \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d af 56 f5 ab[ 	]*vreduceps \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 7b[ 	]*vreduceps \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 28 56 b4 f0 23 01 00 00 7b[ 	]*vreduceps \$0x7b,0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 7f 7b[ 	]*vreduceps \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 00 10 00 00 7b[ 	]*vreduceps \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 e0 ef ff ff 7b[ 	]*vreduceps \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 ab[ 	]*vextractf64x2 \$0xab,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 19 29 ab[ 	]*vextractf64x2 \$0xab,%ymm29,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 19 ac f0 23 01 00 00 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 7f 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 80 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 ab[ 	]*vextracti64x2 \$0xab,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 39 29 ab[ 	]*vextracti64x2 \$0xab,%ymm29,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 39 ac f0 23 01 00 00 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 7f 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 80 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7a f5[ 	]*vcvttpd2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7a f5[ 	]*vcvttpd2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7a f5[ 	]*vcvttpd2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 31[ 	]*vcvttpd2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7a b4 f0 23 01 00 00[ 	]*vcvttpd2qq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 31[ 	]*vcvttpd2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 7f[ 	]*vcvttpd2qq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 00 08 00 00[ 	]*vcvttpd2qq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 80[ 	]*vcvttpd2qq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 80[ 	]*vcvttpd2qq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7a f5[ 	]*vcvttpd2qq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7a f5[ 	]*vcvttpd2qq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7a f5[ 	]*vcvttpd2qq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 31[ 	]*vcvttpd2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7a b4 f0 23 01 00 00[ 	]*vcvttpd2qq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 31[ 	]*vcvttpd2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 7f[ 	]*vcvttpd2qq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 00 10 00 00[ 	]*vcvttpd2qq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 80[ 	]*vcvttpd2qq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 80[ 	]*vcvttpd2qq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 78 f5[ 	]*vcvttpd2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 78 f5[ 	]*vcvttpd2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 78 f5[ 	]*vcvttpd2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 31[ 	]*vcvttpd2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 78 b4 f0 23 01 00 00[ 	]*vcvttpd2uqq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 31[ 	]*vcvttpd2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 7f[ 	]*vcvttpd2uqq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 00 08 00 00[ 	]*vcvttpd2uqq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 80[ 	]*vcvttpd2uqq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 80[ 	]*vcvttpd2uqq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 78 f5[ 	]*vcvttpd2uqq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 78 f5[ 	]*vcvttpd2uqq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 78 f5[ 	]*vcvttpd2uqq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 31[ 	]*vcvttpd2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 78 b4 f0 23 01 00 00[ 	]*vcvttpd2uqq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 31[ 	]*vcvttpd2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 7f[ 	]*vcvttpd2uqq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 00 10 00 00[ 	]*vcvttpd2uqq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 80[ 	]*vcvttpd2uqq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 80[ 	]*vcvttpd2uqq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7a f5[ 	]*vcvttps2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7a f5[ 	]*vcvttps2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7a f5[ 	]*vcvttps2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 31[ 	]*vcvttps2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7a b4 f0 23 01 00 00[ 	]*vcvttps2qq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 31[ 	]*vcvttps2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 7f[ 	]*vcvttps2qq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 00 04 00 00[ 	]*vcvttps2qq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 80[ 	]*vcvttps2qq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 f8 fb ff ff[ 	]*vcvttps2qq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 80[ 	]*vcvttps2qq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7a f5[ 	]*vcvttps2qq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7a f5[ 	]*vcvttps2qq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7a f5[ 	]*vcvttps2qq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 31[ 	]*vcvttps2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7a b4 f0 23 01 00 00[ 	]*vcvttps2qq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 31[ 	]*vcvttps2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 7f[ 	]*vcvttps2qq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 00 08 00 00[ 	]*vcvttps2qq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 80[ 	]*vcvttps2qq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 80[ 	]*vcvttps2qq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 78 f5[ 	]*vcvttps2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 78 f5[ 	]*vcvttps2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 78 f5[ 	]*vcvttps2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 31[ 	]*vcvttps2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 78 b4 f0 23 01 00 00[ 	]*vcvttps2uqq 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 31[ 	]*vcvttps2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 7f[ 	]*vcvttps2uqq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 00 04 00 00[ 	]*vcvttps2uqq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 80[ 	]*vcvttps2uqq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 80[ 	]*vcvttps2uqq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 78 f5[ 	]*vcvttps2uqq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 78 f5[ 	]*vcvttps2uqq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 78 f5[ 	]*vcvttps2uqq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 31[ 	]*vcvttps2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 78 b4 f0 23 01 00 00[ 	]*vcvttps2uqq 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 31[ 	]*vcvttps2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 7f[ 	]*vcvttps2uqq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 00 08 00 00[ 	]*vcvttps2uqq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 80[ 	]*vcvttps2uqq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 80[ 	]*vcvttps2uqq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 08 39 ee[ 	]*vpmovd2m %xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 28 39 ee[ 	]*vpmovd2m %ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 08 39 ee[ 	]*vpmovq2m %xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 28 39 ee[ 	]*vpmovq2m %ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 38 f5[ 	]*vpmovm2d %k5,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 38 f5[ 	]*vpmovm2d %k5,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 38 f5[ 	]*vpmovm2q %k5,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 38 f5[ 	]*vpmovm2q %k5,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1a b4 f0 34 12 00 00[ 	]*vbroadcastf64x2 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 5a b4 f0 34 12 00 00[ 	]*vbroadcasti64x2 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 19 f7[ 	]*vbroadcastf32x2 %xmm31,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 19 f7[ 	]*vbroadcastf32x2 %xmm31,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 19 f7[ 	]*vbroadcastf32x2 %xmm31,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 31[ 	]*vbroadcastf32x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 19 b4 f0 34 12 00 00[ 	]*vbroadcastf32x2 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7b f5[ 	]*vcvtpd2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7b f5[ 	]*vcvtpd2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7b f5[ 	]*vcvtpd2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 31[ 	]*vcvtpd2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7b b4 f0 34 12 00 00[ 	]*vcvtpd2qq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 31[ 	]*vcvtpd2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 7f[ 	]*vcvtpd2qq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 00 08 00 00[ 	]*vcvtpd2qq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 80[ 	]*vcvtpd2qq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 80[ 	]*vcvtpd2qq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7b f5[ 	]*vcvtpd2qq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7b f5[ 	]*vcvtpd2qq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7b f5[ 	]*vcvtpd2qq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 31[ 	]*vcvtpd2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7b b4 f0 34 12 00 00[ 	]*vcvtpd2qq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 31[ 	]*vcvtpd2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 7f[ 	]*vcvtpd2qq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 00 10 00 00[ 	]*vcvtpd2qq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 80[ 	]*vcvtpd2qq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 80[ 	]*vcvtpd2qq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 79 f5[ 	]*vcvtpd2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 79 f5[ 	]*vcvtpd2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 79 f5[ 	]*vcvtpd2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 31[ 	]*vcvtpd2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 79 b4 f0 34 12 00 00[ 	]*vcvtpd2uqq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 31[ 	]*vcvtpd2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 7f[ 	]*vcvtpd2uqq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 00 08 00 00[ 	]*vcvtpd2uqq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 80[ 	]*vcvtpd2uqq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 80[ 	]*vcvtpd2uqq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 79 f5[ 	]*vcvtpd2uqq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 79 f5[ 	]*vcvtpd2uqq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 79 f5[ 	]*vcvtpd2uqq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 31[ 	]*vcvtpd2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 79 b4 f0 34 12 00 00[ 	]*vcvtpd2uqq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 31[ 	]*vcvtpd2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 7f[ 	]*vcvtpd2uqq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 00 10 00 00[ 	]*vcvtpd2uqq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 80[ 	]*vcvtpd2uqq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 80[ 	]*vcvtpd2uqq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7b f5[ 	]*vcvtps2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7b f5[ 	]*vcvtps2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7b f5[ 	]*vcvtps2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 31[ 	]*vcvtps2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7b b4 f0 34 12 00 00[ 	]*vcvtps2qq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 31[ 	]*vcvtps2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 7f[ 	]*vcvtps2qq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 00 04 00 00[ 	]*vcvtps2qq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 80[ 	]*vcvtps2qq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 f8 fb ff ff[ 	]*vcvtps2qq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 80[ 	]*vcvtps2qq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7b f5[ 	]*vcvtps2qq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7b f5[ 	]*vcvtps2qq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7b f5[ 	]*vcvtps2qq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 31[ 	]*vcvtps2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7b b4 f0 34 12 00 00[ 	]*vcvtps2qq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 31[ 	]*vcvtps2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 7f[ 	]*vcvtps2qq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 00 08 00 00[ 	]*vcvtps2qq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 80[ 	]*vcvtps2qq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 80[ 	]*vcvtps2qq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 79 f5[ 	]*vcvtps2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 79 f5[ 	]*vcvtps2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 79 f5[ 	]*vcvtps2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 31[ 	]*vcvtps2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 79 b4 f0 34 12 00 00[ 	]*vcvtps2uqq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 31[ 	]*vcvtps2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 7f[ 	]*vcvtps2uqq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 00 04 00 00[ 	]*vcvtps2uqq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 80[ 	]*vcvtps2uqq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 80[ 	]*vcvtps2uqq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 79 f5[ 	]*vcvtps2uqq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 79 f5[ 	]*vcvtps2uqq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 79 f5[ 	]*vcvtps2uqq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 31[ 	]*vcvtps2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 79 b4 f0 34 12 00 00[ 	]*vcvtps2uqq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 31[ 	]*vcvtps2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 7f[ 	]*vcvtps2uqq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 00 08 00 00[ 	]*vcvtps2uqq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 80[ 	]*vcvtps2uqq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 80[ 	]*vcvtps2uqq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 e6 f5[ 	]*vcvtqq2pd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f e6 f5[ 	]*vcvtqq2pd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f e6 f5[ 	]*vcvtqq2pd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 31[ 	]*vcvtqq2pd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 e6 b4 f0 34 12 00 00[ 	]*vcvtqq2pd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 31[ 	]*vcvtqq2pd \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 7f[ 	]*vcvtqq2pd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 00 08 00 00[ 	]*vcvtqq2pd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 80[ 	]*vcvtqq2pd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 80[ 	]*vcvtqq2pd -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 e6 f5[ 	]*vcvtqq2pd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f e6 f5[ 	]*vcvtqq2pd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af e6 f5[ 	]*vcvtqq2pd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 31[ 	]*vcvtqq2pd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 e6 b4 f0 34 12 00 00[ 	]*vcvtqq2pd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 31[ 	]*vcvtqq2pd \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 7f[ 	]*vcvtqq2pd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 00 10 00 00[ 	]*vcvtqq2pd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 80[ 	]*vcvtqq2pd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 80[ 	]*vcvtqq2pd -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 08 5b f5[ 	]*vcvtqq2ps %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 0f 5b f5[ 	]*vcvtqq2ps %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 8f 5b f5[ 	]*vcvtqq2ps %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 31[ 	]*vcvtqq2psx \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 08 5b b4 f0 34 12 00 00[ 	]*vcvtqq2psx 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 31[ 	]*vcvtqq2ps \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 7f[ 	]*vcvtqq2psx 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 00 08 00 00[ 	]*vcvtqq2psx 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 80[ 	]*vcvtqq2psx -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 f0 f7 ff ff[ 	]*vcvtqq2psx -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 80[ 	]*vcvtqq2ps -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 28 5b f5[ 	]*vcvtqq2ps %ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 2f 5b f5[ 	]*vcvtqq2ps %ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc af 5b f5[ 	]*vcvtqq2ps %ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 31[ 	]*vcvtqq2psy \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 28 5b b4 f0 34 12 00 00[ 	]*vcvtqq2psy 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 31[ 	]*vcvtqq2ps \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 7f[ 	]*vcvtqq2psy 0xfe0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 00 10 00 00[ 	]*vcvtqq2psy 0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 80[ 	]*vcvtqq2psy -0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 e0 ef ff ff[ 	]*vcvtqq2psy -0x1020\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 80[ 	]*vcvtqq2ps -0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 7a f5[ 	]*vcvtuqq2pd %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f 7a f5[ 	]*vcvtuqq2pd %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f 7a f5[ 	]*vcvtuqq2pd %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 31[ 	]*vcvtuqq2pd \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2pd 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 31[ 	]*vcvtuqq2pd \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 7f[ 	]*vcvtuqq2pd 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 00 08 00 00[ 	]*vcvtuqq2pd 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 80[ 	]*vcvtuqq2pd -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 7a f5[ 	]*vcvtuqq2pd %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f 7a f5[ 	]*vcvtuqq2pd %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af 7a f5[ 	]*vcvtuqq2pd %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 31[ 	]*vcvtuqq2pd \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2pd 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 31[ 	]*vcvtuqq2pd \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 7f[ 	]*vcvtuqq2pd 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 00 10 00 00[ 	]*vcvtuqq2pd 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 80[ 	]*vcvtuqq2pd -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7a f5[ 	]*vcvtuqq2ps %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7a f5[ 	]*vcvtuqq2ps %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7a f5[ 	]*vcvtuqq2ps %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 31[ 	]*vcvtuqq2psx \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2psx 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 31[ 	]*vcvtuqq2ps \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 7f[ 	]*vcvtuqq2psx 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 00 08 00 00[ 	]*vcvtuqq2psx 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 80[ 	]*vcvtuqq2psx -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2psx -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7a f5[ 	]*vcvtuqq2ps %ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7a f5[ 	]*vcvtuqq2ps %ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7a f5[ 	]*vcvtuqq2ps %ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 31[ 	]*vcvtuqq2psy \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2psy 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 31[ 	]*vcvtuqq2ps \(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 7f[ 	]*vcvtuqq2psy 0xfe0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 00 10 00 00[ 	]*vcvtuqq2psy 0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 80[ 	]*vcvtuqq2psy -0x1000\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2psy -0x1020\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee ab[ 	]*vfpclasspd \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 66 ee ab[ 	]*vfpclasspd \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee 7b[ 	]*vfpclasspd \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 29 7b[ 	]*vfpclasspdx \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspdx \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 29 7b[ 	]*vfpclasspd \$0x7b,\(%rcx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 7f 7b[ 	]*vfpclasspdx \$0x7b,0x7f0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa 00 08 00 00 7b[ 	]*vfpclasspdx \$0x7b,0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 80 7b[ 	]*vfpclasspdx \$0x7b,-0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspdx \$0x7b,-0x810\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%rdx\)\{1to2\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee ab[ 	]*vfpclasspd \$0xab,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 2f 66 ee ab[ 	]*vfpclasspd \$0xab,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee 7b[ 	]*vfpclasspd \$0x7b,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 29 7b[ 	]*vfpclasspdy \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 28 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspdy \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 29 7b[ 	]*vfpclasspd \$0x7b,\(%rcx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 7f 7b[ 	]*vfpclasspdy \$0x7b,0xfe0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa 00 10 00 00 7b[ 	]*vfpclasspdy \$0x7b,0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 80 7b[ 	]*vfpclasspdy \$0x7b,-0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa e0 ef ff ff 7b[ 	]*vfpclasspdy \$0x7b,-0x1020\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee ab[ 	]*vfpclassps \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 66 ee ab[ 	]*vfpclassps \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee 7b[ 	]*vfpclassps \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 29 7b[ 	]*vfpclasspsx \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspsx \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 29 7b[ 	]*vfpclassps \$0x7b,\(%rcx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 7f 7b[ 	]*vfpclasspsx \$0x7b,0x7f0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa 00 08 00 00 7b[ 	]*vfpclasspsx \$0x7b,0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 80 7b[ 	]*vfpclasspsx \$0x7b,-0x800\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspsx \$0x7b,-0x810\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%rdx\)\{1to4\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee ab[ 	]*vfpclassps \$0xab,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 2f 66 ee ab[ 	]*vfpclassps \$0xab,%ymm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee 7b[ 	]*vfpclassps \$0x7b,%ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 29 7b[ 	]*vfpclasspsy \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 28 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspsy \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 29 7b[ 	]*vfpclassps \$0x7b,\(%rcx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 7f 7b[ 	]*vfpclasspsy \$0x7b,0xfe0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa 00 10 00 00 7b[ 	]*vfpclasspsy \$0x7b,0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 80 7b[ 	]*vfpclasspsy \$0x7b,-0x1000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa e0 ef ff ff 7b[ 	]*vfpclasspsy \$0x7b,-0x1020\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 18 b4 f0 34 12 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 38 b4 f0 34 12 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 59 f7[ 	]*vbroadcasti32x2 %xmm31,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 31[ 	]*vbroadcasti32x2 \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 59 b4 f0 34 12 00 00[ 	]*vbroadcasti32x2 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 59 f7[ 	]*vbroadcasti32x2 %xmm31,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 59 f7[ 	]*vbroadcasti32x2 %xmm31,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 31[ 	]*vbroadcasti32x2 \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 59 b4 f0 34 12 00 00[ 	]*vbroadcasti32x2 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 40 f4[ 	]*vpmullq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 40 f4[ 	]*vpmullq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 40 f4[ 	]*vpmullq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 31[ 	]*vpmullq \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 40 b4 f0 34 12 00 00[ 	]*vpmullq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 31[ 	]*vpmullq \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 7f[ 	]*vpmullq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 00 08 00 00[ 	]*vpmullq 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 80[ 	]*vpmullq -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 f0 f7 ff ff[ 	]*vpmullq -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 7f[ 	]*vpmullq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 80[ 	]*vpmullq -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 40 f4[ 	]*vpmullq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 40 f4[ 	]*vpmullq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 40 f4[ 	]*vpmullq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 31[ 	]*vpmullq \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 40 b4 f0 34 12 00 00[ 	]*vpmullq 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 31[ 	]*vpmullq \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 7f[ 	]*vpmullq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 00 10 00 00[ 	]*vpmullq 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 80[ 	]*vpmullq -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 e0 ef ff ff[ 	]*vpmullq -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 7f[ 	]*vpmullq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 80[ 	]*vpmullq -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 ab[ 	]*vrangepd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 50 f4 ab[ 	]*vrangepd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 50 f4 ab[ 	]*vrangepd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 7b[ 	]*vrangepd \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 50 b4 f0 34 12 00 00 7b[ 	]*vrangepd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 00 08 00 00 7b[ 	]*vrangepd \$0x7b,0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd \$0x7b,-0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 ab[ 	]*vrangepd \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 50 f4 ab[ 	]*vrangepd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 50 f4 ab[ 	]*vrangepd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 7b[ 	]*vrangepd \$0x7b,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 50 b4 f0 34 12 00 00 7b[ 	]*vrangepd \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 7f 7b[ 	]*vrangepd \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 00 10 00 00 7b[ 	]*vrangepd \$0x7b,0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 e0 ef ff ff 7b[ 	]*vrangepd \$0x7b,-0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 ab[ 	]*vrangeps \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 50 f4 ab[ 	]*vrangeps \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 50 f4 ab[ 	]*vrangeps \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 7b[ 	]*vrangeps \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 50 b4 f0 34 12 00 00 7b[ 	]*vrangeps \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 00 08 00 00 7b[ 	]*vrangeps \$0x7b,0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps \$0x7b,-0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 ab[ 	]*vrangeps \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 50 f4 ab[ 	]*vrangeps \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 50 f4 ab[ 	]*vrangeps \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 7b[ 	]*vrangeps \$0x7b,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 50 b4 f0 34 12 00 00 7b[ 	]*vrangeps \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 7f 7b[ 	]*vrangeps \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 00 10 00 00 7b[ 	]*vrangeps \$0x7b,0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 e0 ef ff ff 7b[ 	]*vrangeps \$0x7b,-0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 54 f4[ 	]*vandpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 54 f4[ 	]*vandpd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 54 f4[ 	]*vandpd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 31[ 	]*vandpd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 54 b4 f0 34 12 00 00[ 	]*vandpd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 31[ 	]*vandpd \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 7f[ 	]*vandpd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 00 08 00 00[ 	]*vandpd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 80[ 	]*vandpd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 f0 f7 ff ff[ 	]*vandpd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 7f[ 	]*vandpd 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 80[ 	]*vandpd -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 54 f4[ 	]*vandpd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 54 f4[ 	]*vandpd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 54 f4[ 	]*vandpd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 31[ 	]*vandpd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 54 b4 f0 34 12 00 00[ 	]*vandpd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 31[ 	]*vandpd \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 7f[ 	]*vandpd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 00 10 00 00[ 	]*vandpd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 80[ 	]*vandpd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 e0 ef ff ff[ 	]*vandpd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 7f[ 	]*vandpd 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 80[ 	]*vandpd -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 54 f4[ 	]*vandps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 54 f4[ 	]*vandps %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 54 f4[ 	]*vandps %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 31[ 	]*vandps \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 54 b4 f0 34 12 00 00[ 	]*vandps 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 31[ 	]*vandps \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 7f[ 	]*vandps 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 00 08 00 00[ 	]*vandps 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 80[ 	]*vandps -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 f0 f7 ff ff[ 	]*vandps -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 7f[ 	]*vandps 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 00 02 00 00[ 	]*vandps 0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 80[ 	]*vandps -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 54 f4[ 	]*vandps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 54 f4[ 	]*vandps %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 54 f4[ 	]*vandps %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 31[ 	]*vandps \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 54 b4 f0 34 12 00 00[ 	]*vandps 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 31[ 	]*vandps \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 7f[ 	]*vandps 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 00 10 00 00[ 	]*vandps 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 80[ 	]*vandps -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 e0 ef ff ff[ 	]*vandps -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 7f[ 	]*vandps 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 00 02 00 00[ 	]*vandps 0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 80[ 	]*vandps -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 55 f4[ 	]*vandnpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 55 f4[ 	]*vandnpd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 55 f4[ 	]*vandnpd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 31[ 	]*vandnpd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 55 b4 f0 34 12 00 00[ 	]*vandnpd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 31[ 	]*vandnpd \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 7f[ 	]*vandnpd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 00 08 00 00[ 	]*vandnpd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 80[ 	]*vandnpd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 f0 f7 ff ff[ 	]*vandnpd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 7f[ 	]*vandnpd 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 80[ 	]*vandnpd -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 55 f4[ 	]*vandnpd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 55 f4[ 	]*vandnpd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 55 f4[ 	]*vandnpd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 31[ 	]*vandnpd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 55 b4 f0 34 12 00 00[ 	]*vandnpd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 31[ 	]*vandnpd \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 7f[ 	]*vandnpd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 00 10 00 00[ 	]*vandnpd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 80[ 	]*vandnpd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 e0 ef ff ff[ 	]*vandnpd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 7f[ 	]*vandnpd 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 80[ 	]*vandnpd -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 55 f4[ 	]*vandnps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 55 f4[ 	]*vandnps %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 55 f4[ 	]*vandnps %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 31[ 	]*vandnps \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 55 b4 f0 34 12 00 00[ 	]*vandnps 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 31[ 	]*vandnps \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 7f[ 	]*vandnps 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 00 08 00 00[ 	]*vandnps 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 80[ 	]*vandnps -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 f0 f7 ff ff[ 	]*vandnps -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 7f[ 	]*vandnps 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 80[ 	]*vandnps -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 55 f4[ 	]*vandnps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 55 f4[ 	]*vandnps %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 55 f4[ 	]*vandnps %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 31[ 	]*vandnps \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 55 b4 f0 34 12 00 00[ 	]*vandnps 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 31[ 	]*vandnps \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 7f[ 	]*vandnps 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 00 10 00 00[ 	]*vandnps 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 80[ 	]*vandnps -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 e0 ef ff ff[ 	]*vandnps -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 7f[ 	]*vandnps 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 80[ 	]*vandnps -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 56 f4[ 	]*vorpd  %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 56 f4[ 	]*vorpd  %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 56 f4[ 	]*vorpd  %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 31[ 	]*vorpd  \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 56 b4 f0 34 12 00 00[ 	]*vorpd  0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 31[ 	]*vorpd  \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 7f[ 	]*vorpd  0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 00 08 00 00[ 	]*vorpd  0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 80[ 	]*vorpd  -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 f0 f7 ff ff[ 	]*vorpd  -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 7f[ 	]*vorpd  0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 80[ 	]*vorpd  -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 56 f4[ 	]*vorpd  %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 56 f4[ 	]*vorpd  %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 56 f4[ 	]*vorpd  %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 31[ 	]*vorpd  \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 56 b4 f0 34 12 00 00[ 	]*vorpd  0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 31[ 	]*vorpd  \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 7f[ 	]*vorpd  0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 00 10 00 00[ 	]*vorpd  0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 80[ 	]*vorpd  -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 e0 ef ff ff[ 	]*vorpd  -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 7f[ 	]*vorpd  0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 80[ 	]*vorpd  -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 56 f4[ 	]*vorps  %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 56 f4[ 	]*vorps  %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 56 f4[ 	]*vorps  %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 31[ 	]*vorps  \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 56 b4 f0 34 12 00 00[ 	]*vorps  0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 31[ 	]*vorps  \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 7f[ 	]*vorps  0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 00 08 00 00[ 	]*vorps  0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 80[ 	]*vorps  -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 f0 f7 ff ff[ 	]*vorps  -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 7f[ 	]*vorps  0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 00 02 00 00[ 	]*vorps  0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 80[ 	]*vorps  -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 56 f4[ 	]*vorps  %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 56 f4[ 	]*vorps  %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 56 f4[ 	]*vorps  %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 31[ 	]*vorps  \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 56 b4 f0 34 12 00 00[ 	]*vorps  0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 31[ 	]*vorps  \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 7f[ 	]*vorps  0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 00 10 00 00[ 	]*vorps  0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 80[ 	]*vorps  -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 e0 ef ff ff[ 	]*vorps  -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 7f[ 	]*vorps  0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 00 02 00 00[ 	]*vorps  0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 80[ 	]*vorps  -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 57 f4[ 	]*vxorpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 57 f4[ 	]*vxorpd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 57 f4[ 	]*vxorpd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 31[ 	]*vxorpd \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 57 b4 f0 34 12 00 00[ 	]*vxorpd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 31[ 	]*vxorpd \(%rcx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 7f[ 	]*vxorpd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 00 08 00 00[ 	]*vxorpd 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 80[ 	]*vxorpd -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 f0 f7 ff ff[ 	]*vxorpd -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 7f[ 	]*vxorpd 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 80[ 	]*vxorpd -0x400\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 57 f4[ 	]*vxorpd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 57 f4[ 	]*vxorpd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 57 f4[ 	]*vxorpd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 31[ 	]*vxorpd \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 57 b4 f0 34 12 00 00[ 	]*vxorpd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 31[ 	]*vxorpd \(%rcx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 7f[ 	]*vxorpd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 00 10 00 00[ 	]*vxorpd 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 80[ 	]*vxorpd -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 e0 ef ff ff[ 	]*vxorpd -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 7f[ 	]*vxorpd 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 80[ 	]*vxorpd -0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 57 f4[ 	]*vxorps %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 57 f4[ 	]*vxorps %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 57 f4[ 	]*vxorps %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 31[ 	]*vxorps \(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 57 b4 f0 34 12 00 00[ 	]*vxorps 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 31[ 	]*vxorps \(%rcx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 7f[ 	]*vxorps 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 00 08 00 00[ 	]*vxorps 0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 80[ 	]*vxorps -0x800\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 f0 f7 ff ff[ 	]*vxorps -0x810\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 7f[ 	]*vxorps 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 80[ 	]*vxorps -0x200\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 57 f4[ 	]*vxorps %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 57 f4[ 	]*vxorps %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 57 f4[ 	]*vxorps %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 31[ 	]*vxorps \(%rcx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 57 b4 f0 34 12 00 00[ 	]*vxorps 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 31[ 	]*vxorps \(%rcx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 7f[ 	]*vxorps 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 00 10 00 00[ 	]*vxorps 0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 80[ 	]*vxorps -0x1000\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 e0 ef ff ff[ 	]*vxorps -0x1020\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 7f[ 	]*vxorps 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 80[ 	]*vxorps -0x200\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 ab[ 	]*vreducepd \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 0f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 8f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 7b[ 	]*vreducepd \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 56 b4 f0 34 12 00 00 7b[ 	]*vreducepd \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 00 08 00 00 7b[ 	]*vreducepd \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 ab[ 	]*vreducepd \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 56 f5 ab[ 	]*vreducepd \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 56 f5 ab[ 	]*vreducepd \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 7b[ 	]*vreducepd \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 56 b4 f0 34 12 00 00 7b[ 	]*vreducepd \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 7f 7b[ 	]*vreducepd \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 00 10 00 00 7b[ 	]*vreducepd \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 e0 ef ff ff 7b[ 	]*vreducepd \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 ab[ 	]*vreduceps \$0xab,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 0f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 8f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 7b[ 	]*vreduceps \$0x7b,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 56 b4 f0 34 12 00 00 7b[ 	]*vreduceps \$0x7b,0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 00 08 00 00 7b[ 	]*vreduceps \$0x7b,0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps \$0x7b,-0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%rdx\)\{1to4\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 ab[ 	]*vreduceps \$0xab,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 2f 56 f5 ab[ 	]*vreduceps \$0xab,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d af 56 f5 ab[ 	]*vreduceps \$0xab,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 7b[ 	]*vreduceps \$0x7b,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 28 56 b4 f0 34 12 00 00 7b[ 	]*vreduceps \$0x7b,0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 7f 7b[ 	]*vreduceps \$0x7b,0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 00 10 00 00 7b[ 	]*vreduceps \$0x7b,0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 e0 ef ff ff 7b[ 	]*vreduceps \$0x7b,-0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 ab[ 	]*vextractf64x2 \$0xab,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 19 29 ab[ 	]*vextractf64x2 \$0xab,%ymm29,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 19 ac f0 34 12 00 00 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 7f 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 80 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%ymm29,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 ab[ 	]*vextracti64x2 \$0xab,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 39 29 ab[ 	]*vextracti64x2 \$0xab,%ymm29,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 39 ac f0 34 12 00 00 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 7f 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 80 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%ymm29,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7a f5[ 	]*vcvttpd2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7a f5[ 	]*vcvttpd2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7a f5[ 	]*vcvttpd2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 31[ 	]*vcvttpd2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7a b4 f0 34 12 00 00[ 	]*vcvttpd2qq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 31[ 	]*vcvttpd2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 7f[ 	]*vcvttpd2qq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 00 08 00 00[ 	]*vcvttpd2qq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 80[ 	]*vcvttpd2qq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 80[ 	]*vcvttpd2qq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7a f5[ 	]*vcvttpd2qq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7a f5[ 	]*vcvttpd2qq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7a f5[ 	]*vcvttpd2qq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 31[ 	]*vcvttpd2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7a b4 f0 34 12 00 00[ 	]*vcvttpd2qq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 31[ 	]*vcvttpd2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 7f[ 	]*vcvttpd2qq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 00 10 00 00[ 	]*vcvttpd2qq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 80[ 	]*vcvttpd2qq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 80[ 	]*vcvttpd2qq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 78 f5[ 	]*vcvttpd2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 78 f5[ 	]*vcvttpd2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 78 f5[ 	]*vcvttpd2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 31[ 	]*vcvttpd2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 78 b4 f0 34 12 00 00[ 	]*vcvttpd2uqq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 31[ 	]*vcvttpd2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 7f[ 	]*vcvttpd2uqq 0x7f0\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 00 08 00 00[ 	]*vcvttpd2uqq 0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 80[ 	]*vcvttpd2uqq -0x800\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq -0x810\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 80[ 	]*vcvttpd2uqq -0x400\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 78 f5[ 	]*vcvttpd2uqq %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 78 f5[ 	]*vcvttpd2uqq %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 78 f5[ 	]*vcvttpd2uqq %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 31[ 	]*vcvttpd2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 78 b4 f0 34 12 00 00[ 	]*vcvttpd2uqq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 31[ 	]*vcvttpd2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 7f[ 	]*vcvttpd2uqq 0xfe0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 00 10 00 00[ 	]*vcvttpd2uqq 0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 80[ 	]*vcvttpd2uqq -0x1000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq -0x1020\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 80[ 	]*vcvttpd2uqq -0x400\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7a f5[ 	]*vcvttps2qq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7a f5[ 	]*vcvttps2qq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7a f5[ 	]*vcvttps2qq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 31[ 	]*vcvttps2qq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7a b4 f0 34 12 00 00[ 	]*vcvttps2qq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 31[ 	]*vcvttps2qq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 7f[ 	]*vcvttps2qq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 00 04 00 00[ 	]*vcvttps2qq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 80[ 	]*vcvttps2qq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 f8 fb ff ff[ 	]*vcvttps2qq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 80[ 	]*vcvttps2qq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7a f5[ 	]*vcvttps2qq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7a f5[ 	]*vcvttps2qq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7a f5[ 	]*vcvttps2qq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 31[ 	]*vcvttps2qq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7a b4 f0 34 12 00 00[ 	]*vcvttps2qq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 31[ 	]*vcvttps2qq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 7f[ 	]*vcvttps2qq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 00 08 00 00[ 	]*vcvttps2qq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 80[ 	]*vcvttps2qq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 80[ 	]*vcvttps2qq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 78 f5[ 	]*vcvttps2uqq %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 78 f5[ 	]*vcvttps2uqq %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 78 f5[ 	]*vcvttps2uqq %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 31[ 	]*vcvttps2uqq \(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 78 b4 f0 34 12 00 00[ 	]*vcvttps2uqq 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 31[ 	]*vcvttps2uqq \(%rcx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 7f[ 	]*vcvttps2uqq 0x3f8\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 00 04 00 00[ 	]*vcvttps2uqq 0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 80[ 	]*vcvttps2uqq -0x400\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq -0x408\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 80[ 	]*vcvttps2uqq -0x200\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to2\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 78 f5[ 	]*vcvttps2uqq %xmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 78 f5[ 	]*vcvttps2uqq %xmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 78 f5[ 	]*vcvttps2uqq %xmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 31[ 	]*vcvttps2uqq \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 78 b4 f0 34 12 00 00[ 	]*vcvttps2uqq 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 31[ 	]*vcvttps2uqq \(%rcx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 7f[ 	]*vcvttps2uqq 0x7f0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 00 08 00 00[ 	]*vcvttps2uqq 0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 80[ 	]*vcvttps2uqq -0x800\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq -0x810\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 80[ 	]*vcvttps2uqq -0x200\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to4\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 08 39 ee[ 	]*vpmovd2m %xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 28 39 ee[ 	]*vpmovd2m %ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 08 39 ee[ 	]*vpmovq2m %xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 28 39 ee[ 	]*vpmovq2m %ymm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 38 f5[ 	]*vpmovm2d %k5,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 38 f5[ 	]*vpmovm2d %k5,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 38 f5[ 	]*vpmovm2q %k5,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 38 f5[ 	]*vpmovm2q %k5,%ymm30
#pass
