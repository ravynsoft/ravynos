#as:
#objdump: -dw
#name: x86_64 AVX512DQ insns
#source: x86-64-avx512dq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1b b4 f0 23 01 00 00[ 	]*vbroadcastf32x8 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 80[ 	]*vbroadcastf32x8 -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1a b4 f0 23 01 00 00[ 	]*vbroadcastf64x2 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 31[ 	]*vbroadcasti32x8 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 5b 31[ 	]*vbroadcasti32x8 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 5b 31[ 	]*vbroadcasti32x8 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 5b b4 f0 23 01 00 00[ 	]*vbroadcasti32x8 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 80[ 	]*vbroadcasti32x8 -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 5a b4 f0 23 01 00 00[ 	]*vbroadcasti64x2 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 19 f7[ 	]*vbroadcastf32x2 %xmm31,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 19 f7[ 	]*vbroadcastf32x2 %xmm31,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 19 f7[ 	]*vbroadcastf32x2 %xmm31,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 31[ 	]*vbroadcastf32x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 19 b4 f0 23 01 00 00[ 	]*vbroadcastf32x2 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7b f5[ 	]*vcvtpd2qq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7b f5[ 	]*vcvtpd2qq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7b f5[ 	]*vcvtpd2qq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7b f5[ 	]*vcvtpd2qq \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 7b f5[ 	]*vcvtpd2qq \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 7b f5[ 	]*vcvtpd2qq \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 7b f5[ 	]*vcvtpd2qq \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 31[ 	]*vcvtpd2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7b b4 f0 23 01 00 00[ 	]*vcvtpd2qq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 31[ 	]*vcvtpd2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 7f[ 	]*vcvtpd2qq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 80[ 	]*vcvtpd2qq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 80[ 	]*vcvtpd2qq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 79 f5[ 	]*vcvtpd2uqq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 79 f5[ 	]*vcvtpd2uqq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 79 f5[ 	]*vcvtpd2uqq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 79 f5[ 	]*vcvtpd2uqq \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 79 f5[ 	]*vcvtpd2uqq \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 79 f5[ 	]*vcvtpd2uqq \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 79 f5[ 	]*vcvtpd2uqq \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 31[ 	]*vcvtpd2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 79 b4 f0 23 01 00 00[ 	]*vcvtpd2uqq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 31[ 	]*vcvtpd2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 7f[ 	]*vcvtpd2uqq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 80[ 	]*vcvtpd2uqq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 80[ 	]*vcvtpd2uqq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7b f5[ 	]*vcvtps2qq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7b f5[ 	]*vcvtps2qq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7b f5[ 	]*vcvtps2qq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7b f5[ 	]*vcvtps2qq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 7b f5[ 	]*vcvtps2qq \{ru-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 7b f5[ 	]*vcvtps2qq \{rd-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 7b f5[ 	]*vcvtps2qq \{rz-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 31[ 	]*vcvtps2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7b b4 f0 23 01 00 00[ 	]*vcvtps2qq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 31[ 	]*vcvtps2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 7f[ 	]*vcvtps2qq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 00 10 00 00[ 	]*vcvtps2qq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 80[ 	]*vcvtps2qq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 e0 ef ff ff[ 	]*vcvtps2qq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 80[ 	]*vcvtps2qq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 79 f5[ 	]*vcvtps2uqq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 79 f5[ 	]*vcvtps2uqq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 79 f5[ 	]*vcvtps2uqq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 79 f5[ 	]*vcvtps2uqq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 79 f5[ 	]*vcvtps2uqq \{ru-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 79 f5[ 	]*vcvtps2uqq \{rd-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 79 f5[ 	]*vcvtps2uqq \{rz-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 31[ 	]*vcvtps2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 79 b4 f0 23 01 00 00[ 	]*vcvtps2uqq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 31[ 	]*vcvtps2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 7f[ 	]*vcvtps2uqq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 00 10 00 00[ 	]*vcvtps2uqq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 80[ 	]*vcvtps2uqq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 80[ 	]*vcvtps2uqq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 e6 f5[ 	]*vcvtqq2pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f e6 f5[ 	]*vcvtqq2pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf e6 f5[ 	]*vcvtqq2pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 e6 f5[ 	]*vcvtqq2pd \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 e6 f5[ 	]*vcvtqq2pd \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 e6 f5[ 	]*vcvtqq2pd \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 e6 f5[ 	]*vcvtqq2pd \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 31[ 	]*vcvtqq2pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 e6 b4 f0 23 01 00 00[ 	]*vcvtqq2pd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 31[ 	]*vcvtqq2pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 7f[ 	]*vcvtqq2pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 80[ 	]*vcvtqq2pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 80[ 	]*vcvtqq2pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 48 5b f5[ 	]*vcvtqq2ps %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 4f 5b f5[ 	]*vcvtqq2ps %zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc cf 5b f5[ 	]*vcvtqq2ps %zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 18 5b f5[ 	]*vcvtqq2ps \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 58 5b f5[ 	]*vcvtqq2ps \{ru-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 5b f5[ 	]*vcvtqq2ps \{rd-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 78 5b f5[ 	]*vcvtqq2ps \{rz-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 31[ 	]*vcvtqq2ps \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 48 5b b4 f0 23 01 00 00[ 	]*vcvtqq2ps 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 31[ 	]*vcvtqq2ps \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 7f[ 	]*vcvtqq2ps 0x1fc0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 00 20 00 00[ 	]*vcvtqq2ps 0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 80[ 	]*vcvtqq2ps -0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 c0 df ff ff[ 	]*vcvtqq2ps -0x2040\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 80[ 	]*vcvtqq2ps -0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 7a f5[ 	]*vcvtuqq2pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 7a f5[ 	]*vcvtuqq2pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 7a f5[ 	]*vcvtuqq2pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 7a f5[ 	]*vcvtuqq2pd \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 7a f5[ 	]*vcvtuqq2pd \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 7a f5[ 	]*vcvtuqq2pd \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 7a f5[ 	]*vcvtuqq2pd \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 31[ 	]*vcvtuqq2pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2pd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 31[ 	]*vcvtuqq2pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 7f[ 	]*vcvtuqq2pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 80[ 	]*vcvtuqq2pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 7a f5[ 	]*vcvtuqq2ps %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 7a f5[ 	]*vcvtuqq2ps %zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 7a f5[ 	]*vcvtuqq2ps %zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 18 7a f5[ 	]*vcvtuqq2ps \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 58 7a f5[ 	]*vcvtuqq2ps \{ru-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 38 7a f5[ 	]*vcvtuqq2ps \{rd-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 78 7a f5[ 	]*vcvtuqq2ps \{rz-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 31[ 	]*vcvtuqq2ps \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2ps 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 31[ 	]*vcvtuqq2ps \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 7f[ 	]*vcvtuqq2ps 0x1fc0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 00 20 00 00[ 	]*vcvtuqq2ps 0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 80[ 	]*vcvtuqq2ps -0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps -0x2040\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee 7b[ 	]*vextractf32x8 \$0x7b,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee 7b[ 	]*vextracti32x8 \$0x7b,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 4f 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee 7b[ 	]*vfpclasspd \$0x7b,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspdz \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 48 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspdz \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 29 7b[ 	]*vfpclasspd \$0x7b,\(%rcx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspdz \$0x7b,0x1fc0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspdz \$0x7b,0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspdz \$0x7b,-0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspdz \$0x7b,-0x2040\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee ab[ 	]*vfpclassps \$0xab,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 4f 66 ee ab[ 	]*vfpclassps \$0xab,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee 7b[ 	]*vfpclassps \$0x7b,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclasspsz \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 48 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspsz \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 29 7b[ 	]*vfpclassps \$0x7b,\(%rcx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclasspsz \$0x7b,0x1fc0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspsz \$0x7b,0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclasspsz \$0x7b,-0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspsz \$0x7b,-0x2040\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee ab[ 	]*vfpclasssd \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 67 ee ab[ 	]*vfpclasssd \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee 7b[ 	]*vfpclasssd \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 29 7b[ 	]*vfpclasssd \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 67 ac f0 23 01 00 00 7b[ 	]*vfpclasssd \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 7f 7b[ 	]*vfpclasssd \$0x7b,0x3f8\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa 00 04 00 00 7b[ 	]*vfpclasssd \$0x7b,0x400\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 80 7b[ 	]*vfpclasssd \$0x7b,-0x400\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd \$0x7b,-0x408\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee ab[ 	]*vfpclassss \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 67 ee ab[ 	]*vfpclassss \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee 7b[ 	]*vfpclassss \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 29 7b[ 	]*vfpclassss \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 67 ac f0 23 01 00 00 7b[ 	]*vfpclassss \$0x7b,0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 7f 7b[ 	]*vfpclassss \$0x7b,0x1fc\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa 00 02 00 00 7b[ 	]*vfpclassss \$0x7b,0x200\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 80 7b[ 	]*vfpclassss \$0x7b,-0x200\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa fc fd ff ff 7b[ 	]*vfpclassss \$0x7b,-0x204\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 18 b4 f0 23 01 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 7b[ 	]*vinsertf32x8 \$0x7b,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 31 7b[ 	]*vinsertf32x8 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 1a b4 f0 23 01 00 00 7b[ 	]*vinsertf32x8 \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 7f 7b[ 	]*vinsertf32x8 \$0x7b,0xfe0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 \$0x7b,0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 80 7b[ 	]*vinsertf32x8 \$0x7b,-0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 \$0x7b,-0x1020\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 38 b4 f0 23 01 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 7b[ 	]*vinserti32x8 \$0x7b,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 31 7b[ 	]*vinserti32x8 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 3a b4 f0 23 01 00 00 7b[ 	]*vinserti32x8 \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 7f 7b[ 	]*vinserti32x8 \$0x7b,0xfe0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 \$0x7b,0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 80 7b[ 	]*vinserti32x8 \$0x7b,-0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 \$0x7b,-0x1020\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 59 f7[ 	]*vbroadcasti32x2 %xmm31,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 59 f7[ 	]*vbroadcasti32x2 %xmm31,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 31[ 	]*vbroadcasti32x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 59 b4 f0 23 01 00 00[ 	]*vbroadcasti32x2 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 ab[ 	]*vpextrd \$0xab,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 7b[ 	]*vpextrd \$0x7b,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 ed 7b[ 	]*vpextrd \$0x7b,%xmm29,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 43 7d 08 16 ed 7b[ 	]*vpextrd \$0x7b,%xmm29,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 29 7b[ 	]*vpextrd \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 16 ac f0 23 01 00 00 7b[ 	]*vpextrd \$0x7b,%xmm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 7f 7b[ 	]*vpextrd \$0x7b,%xmm29,0x1fc\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa 00 02 00 00 7b[ 	]*vpextrd \$0x7b,%xmm29,0x200\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 80 7b[ 	]*vpextrd \$0x7b,%xmm29,-0x200\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa fc fd ff ff 7b[ 	]*vpextrd \$0x7b,%xmm29,-0x204\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 ab[ 	]*vpextrq \$0xab,%xmm29,%rax
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 7b[ 	]*vpextrq \$0x7b,%xmm29,%rax
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 16 e8 7b[ 	]*vpextrq \$0x7b,%xmm29,%r8
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 29 7b[ 	]*vpextrq \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 16 ac f0 23 01 00 00 7b[ 	]*vpextrq \$0x7b,%xmm29,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 7f 7b[ 	]*vpextrq \$0x7b,%xmm29,0x3f8\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa 00 04 00 00 7b[ 	]*vpextrq \$0x7b,%xmm29,0x400\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 80 7b[ 	]*vpextrq \$0x7b,%xmm29,-0x400\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa f8 fb ff ff 7b[ 	]*vpextrq \$0x7b,%xmm29,-0x408\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 ab[ 	]*vpinsrd \$0xab,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 7b[ 	]*vpinsrd \$0x7b,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f5 7b[ 	]*vpinsrd \$0x7b,%ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 43 15 00 22 f5 7b[ 	]*vpinsrd \$0x7b,%r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 31 7b[ 	]*vpinsrd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 22 b4 f0 23 01 00 00 7b[ 	]*vpinsrd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 7f 7b[ 	]*vpinsrd \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 00 02 00 00 7b[ 	]*vpinsrd \$0x7b,0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 80 7b[ 	]*vpinsrd \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 fc fd ff ff 7b[ 	]*vpinsrd \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 ab[ 	]*vpinsrq \$0xab,%rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 7b[ 	]*vpinsrq \$0x7b,%rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 22 f0 7b[ 	]*vpinsrq \$0x7b,%r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 31 7b[ 	]*vpinsrq \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 22 b4 f0 23 01 00 00 7b[ 	]*vpinsrq \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 7f 7b[ 	]*vpinsrq \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 00 04 00 00 7b[ 	]*vpinsrq \$0x7b,0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 80 7b[ 	]*vpinsrq \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 f8 fb ff ff 7b[ 	]*vpinsrq \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 40 f4[ 	]*vpmullq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 40 f4[ 	]*vpmullq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 40 f4[ 	]*vpmullq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 31[ 	]*vpmullq \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 40 b4 f0 23 01 00 00[ 	]*vpmullq 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 31[ 	]*vpmullq \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 7f[ 	]*vpmullq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 00 20 00 00[ 	]*vpmullq 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 80[ 	]*vpmullq -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 c0 df ff ff[ 	]*vpmullq -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 7f[ 	]*vpmullq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 80[ 	]*vpmullq -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 ab[ 	]*vrangepd \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 50 f4 ab[ 	]*vrangepd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 50 f4 ab[ 	]*vrangepd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 7b[ 	]*vrangepd \$0x7b,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 50 b4 f0 23 01 00 00 7b[ 	]*vrangepd \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 00 20 00 00 7b[ 	]*vrangepd \$0x7b,0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 c0 df ff ff 7b[ 	]*vrangepd \$0x7b,-0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 ab[ 	]*vrangeps \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 50 f4 ab[ 	]*vrangeps \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 50 f4 ab[ 	]*vrangeps \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 7b[ 	]*vrangeps \$0x7b,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 50 b4 f0 23 01 00 00 7b[ 	]*vrangeps \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 00 20 00 00 7b[ 	]*vrangeps \$0x7b,0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 c0 df ff ff 7b[ 	]*vrangeps \$0x7b,-0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 ab[ 	]*vrangesd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 51 f4 ab[ 	]*vrangesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 51 f4 ab[ 	]*vrangesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 7b[ 	]*vrangesd \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 31 7b[ 	]*vrangesd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 51 b4 f0 23 01 00 00 7b[ 	]*vrangesd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 7f 7b[ 	]*vrangesd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 00 04 00 00 7b[ 	]*vrangesd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 80 7b[ 	]*vrangesd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 f8 fb ff ff 7b[ 	]*vrangesd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 ab[ 	]*vrangess \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 51 f4 ab[ 	]*vrangess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 51 f4 ab[ 	]*vrangess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 7b[ 	]*vrangess \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 31 7b[ 	]*vrangess \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 51 b4 f0 23 01 00 00 7b[ 	]*vrangess \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 7f 7b[ 	]*vrangess \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 00 02 00 00 7b[ 	]*vrangess \$0x7b,0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 80 7b[ 	]*vrangess \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 fc fd ff ff 7b[ 	]*vrangess \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 54 f4[ 	]*vandpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 54 f4[ 	]*vandpd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 54 f4[ 	]*vandpd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 31[ 	]*vandpd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 54 b4 f0 23 01 00 00[ 	]*vandpd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 31[ 	]*vandpd \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 7f[ 	]*vandpd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 00 20 00 00[ 	]*vandpd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 80[ 	]*vandpd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 c0 df ff ff[ 	]*vandpd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 7f[ 	]*vandpd 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 80[ 	]*vandpd -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 54 f4[ 	]*vandps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 54 f4[ 	]*vandps %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 54 f4[ 	]*vandps %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 31[ 	]*vandps \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 54 b4 f0 23 01 00 00[ 	]*vandps 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 31[ 	]*vandps \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 7f[ 	]*vandps 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 00 20 00 00[ 	]*vandps 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 80[ 	]*vandps -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 c0 df ff ff[ 	]*vandps -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 7f[ 	]*vandps 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 00 02 00 00[ 	]*vandps 0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 80[ 	]*vandps -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 55 f4[ 	]*vandnpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 55 f4[ 	]*vandnpd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 55 f4[ 	]*vandnpd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 31[ 	]*vandnpd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 55 b4 f0 23 01 00 00[ 	]*vandnpd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 31[ 	]*vandnpd \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 7f[ 	]*vandnpd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 00 20 00 00[ 	]*vandnpd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 80[ 	]*vandnpd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 c0 df ff ff[ 	]*vandnpd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 7f[ 	]*vandnpd 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 80[ 	]*vandnpd -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 55 f4[ 	]*vandnps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 55 f4[ 	]*vandnps %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 55 f4[ 	]*vandnps %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 31[ 	]*vandnps \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 55 b4 f0 23 01 00 00[ 	]*vandnps 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 31[ 	]*vandnps \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 7f[ 	]*vandnps 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 00 20 00 00[ 	]*vandnps 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 80[ 	]*vandnps -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 c0 df ff ff[ 	]*vandnps -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 7f[ 	]*vandnps 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 80[ 	]*vandnps -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 56 f4[ 	]*vorpd  %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 56 f4[ 	]*vorpd  %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 56 f4[ 	]*vorpd  %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 31[ 	]*vorpd  \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 56 b4 f0 23 01 00 00[ 	]*vorpd  0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 31[ 	]*vorpd  \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 7f[ 	]*vorpd  0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 00 20 00 00[ 	]*vorpd  0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 80[ 	]*vorpd  -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 c0 df ff ff[ 	]*vorpd  -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 7f[ 	]*vorpd  0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 80[ 	]*vorpd  -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 56 f4[ 	]*vorps  %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 56 f4[ 	]*vorps  %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 56 f4[ 	]*vorps  %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 31[ 	]*vorps  \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 56 b4 f0 23 01 00 00[ 	]*vorps  0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 31[ 	]*vorps  \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 7f[ 	]*vorps  0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 00 20 00 00[ 	]*vorps  0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 80[ 	]*vorps  -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 c0 df ff ff[ 	]*vorps  -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 7f[ 	]*vorps  0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 00 02 00 00[ 	]*vorps  0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 80[ 	]*vorps  -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 57 f4[ 	]*vxorpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 57 f4[ 	]*vxorpd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 57 f4[ 	]*vxorpd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 31[ 	]*vxorpd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 57 b4 f0 23 01 00 00[ 	]*vxorpd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 31[ 	]*vxorpd \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 7f[ 	]*vxorpd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 00 20 00 00[ 	]*vxorpd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 80[ 	]*vxorpd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 c0 df ff ff[ 	]*vxorpd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 7f[ 	]*vxorpd 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 80[ 	]*vxorpd -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 57 f4[ 	]*vxorps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 57 f4[ 	]*vxorps %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 57 f4[ 	]*vxorps %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 31[ 	]*vxorps \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 57 b4 f0 23 01 00 00[ 	]*vxorps 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 31[ 	]*vxorps \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 7f[ 	]*vxorps 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 00 20 00 00[ 	]*vxorps 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 80[ 	]*vxorps -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 c0 df ff ff[ 	]*vxorps -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 7f[ 	]*vxorps 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 80[ 	]*vxorps -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 ab[ 	]*vreducepd \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 56 f5 ab[ 	]*vreducepd \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 56 f5 ab[ 	]*vreducepd \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 7b[ 	]*vreducepd \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 56 b4 f0 23 01 00 00 7b[ 	]*vreducepd \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 ab[ 	]*vreduceps \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 56 f5 ab[ 	]*vreduceps \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 56 f5 ab[ 	]*vreduceps \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 7b[ 	]*vreduceps \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 56 b4 f0 23 01 00 00 7b[ 	]*vreduceps \$0x7b,0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 ab[ 	]*vreducesd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 57 f4 ab[ 	]*vreducesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 57 f4 ab[ 	]*vreducesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 7b[ 	]*vreducesd \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 31 7b[ 	]*vreducesd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 57 b4 f0 23 01 00 00 7b[ 	]*vreducesd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 7f 7b[ 	]*vreducesd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 00 04 00 00 7b[ 	]*vreducesd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 80 7b[ 	]*vreducesd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 f8 fb ff ff 7b[ 	]*vreducesd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 ab[ 	]*vreducess \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 57 f4 ab[ 	]*vreducess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 57 f4 ab[ 	]*vreducess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 7b[ 	]*vreducess \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 31 7b[ 	]*vreducess \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 57 b4 f0 23 01 00 00 7b[ 	]*vreducess \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 7f 7b[ 	]*vreducess \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 00 02 00 00 7b[ 	]*vreducess \$0x7b,0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 80 7b[ 	]*vreducess \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 fc fd ff ff 7b[ 	]*vreducess \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*c5 cd 41 ef[ 	]*kandb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 42 ef[ 	]*kandnb %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 45 ef[ 	]*korb   %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 46 ef[ 	]*kxnorb %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 47 ef[ 	]*kxorb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 44 ee[ 	]*knotb  %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 98 ee[ 	]*kortestb %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f8 99 ee[ 	]*ktestw %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 99 ee[ 	]*ktestb %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee ab[ 	]*kshiftrb \$0xab,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee 7b[ 	]*kshiftrb \$0x7b,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee ab[ 	]*kshiftlb \$0xab,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee 7b[ 	]*kshiftlb \$0x7b,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ee[ 	]*kmovb  %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  \(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 90 ac f0 23 01 00 00[ 	]*kmovb  0x123\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  %k5,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 91 ac f0 23 01 00 00[ 	]*kmovb  %k5,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  %eax,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  %ebp,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 c1 79 92 ed[ 	]*kmovb  %r13d,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  %k5,%eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  %k5,%ebp
[ 	]*[a-f0-9]+:[ 	]*c5 79 93 ed[ 	]*kmovb  %k5,%r13d
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 19 b4 f0 23 01 00 00 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 7f 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 80 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 1b b4 f0 23 01 00 00 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,0xfe0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 80 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,-0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,-0x1020\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 39 b4 f0 23 01 00 00 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 7f 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 80 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 3b b4 f0 23 01 00 00 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,0xfe0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 80 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,-0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,-0x1020\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7a f5[ 	]*vcvttpd2qq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7a f5[ 	]*vcvttpd2qq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7a f5[ 	]*vcvttpd2qq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 31[ 	]*vcvttpd2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7a b4 f0 23 01 00 00[ 	]*vcvttpd2qq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 31[ 	]*vcvttpd2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 7f[ 	]*vcvttpd2qq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 80[ 	]*vcvttpd2qq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 80[ 	]*vcvttpd2qq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 78 f5[ 	]*vcvttpd2uqq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 78 f5[ 	]*vcvttpd2uqq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 78 f5[ 	]*vcvttpd2uqq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 31[ 	]*vcvttpd2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 78 b4 f0 23 01 00 00[ 	]*vcvttpd2uqq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 31[ 	]*vcvttpd2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 7f[ 	]*vcvttpd2uqq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 80[ 	]*vcvttpd2uqq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 80[ 	]*vcvttpd2uqq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7a f5[ 	]*vcvttps2qq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7a f5[ 	]*vcvttps2qq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7a f5[ 	]*vcvttps2qq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7a f5[ 	]*vcvttps2qq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 31[ 	]*vcvttps2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7a b4 f0 23 01 00 00[ 	]*vcvttps2qq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 31[ 	]*vcvttps2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 7f[ 	]*vcvttps2qq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 00 10 00 00[ 	]*vcvttps2qq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 80[ 	]*vcvttps2qq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 e0 ef ff ff[ 	]*vcvttps2qq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 80[ 	]*vcvttps2qq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 78 f5[ 	]*vcvttps2uqq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 78 f5[ 	]*vcvttps2uqq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 78 f5[ 	]*vcvttps2uqq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 31[ 	]*vcvttps2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 78 b4 f0 23 01 00 00[ 	]*vcvttps2uqq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 31[ 	]*vcvttps2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 7f[ 	]*vcvttps2uqq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 00 10 00 00[ 	]*vcvttps2uqq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 80[ 	]*vcvttps2uqq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 80[ 	]*vcvttps2uqq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 48 39 ee[ 	]*vpmovd2m %zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 48 39 ee[ 	]*vpmovq2m %zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 38 f5[ 	]*vpmovm2d %k5,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 48 38 f5[ 	]*vpmovm2q %k5,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1b b4 f0 34 12 00 00[ 	]*vbroadcastf32x8 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 80[ 	]*vbroadcastf32x8 -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 1a 31[ 	]*vbroadcastf64x2 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1a b4 f0 34 12 00 00[ 	]*vbroadcastf64x2 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 31[ 	]*vbroadcasti32x8 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 5b 31[ 	]*vbroadcasti32x8 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 5b 31[ 	]*vbroadcasti32x8 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 5b b4 f0 34 12 00 00[ 	]*vbroadcasti32x8 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 80[ 	]*vbroadcasti32x8 -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 5a 31[ 	]*vbroadcasti64x2 \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 5a b4 f0 34 12 00 00[ 	]*vbroadcasti64x2 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 19 f7[ 	]*vbroadcastf32x2 %xmm31,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 19 f7[ 	]*vbroadcastf32x2 %xmm31,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 19 f7[ 	]*vbroadcastf32x2 %xmm31,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 31[ 	]*vbroadcastf32x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 19 b4 f0 34 12 00 00[ 	]*vbroadcastf32x2 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7b f5[ 	]*vcvtpd2qq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7b f5[ 	]*vcvtpd2qq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7b f5[ 	]*vcvtpd2qq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7b f5[ 	]*vcvtpd2qq \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 7b f5[ 	]*vcvtpd2qq \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 7b f5[ 	]*vcvtpd2qq \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 7b f5[ 	]*vcvtpd2qq \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 31[ 	]*vcvtpd2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7b b4 f0 34 12 00 00[ 	]*vcvtpd2qq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 31[ 	]*vcvtpd2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 7f[ 	]*vcvtpd2qq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 80[ 	]*vcvtpd2qq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 80[ 	]*vcvtpd2qq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 79 f5[ 	]*vcvtpd2uqq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 79 f5[ 	]*vcvtpd2uqq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 79 f5[ 	]*vcvtpd2uqq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 79 f5[ 	]*vcvtpd2uqq \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 79 f5[ 	]*vcvtpd2uqq \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 79 f5[ 	]*vcvtpd2uqq \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 79 f5[ 	]*vcvtpd2uqq \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 31[ 	]*vcvtpd2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 79 b4 f0 34 12 00 00[ 	]*vcvtpd2uqq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 31[ 	]*vcvtpd2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 7f[ 	]*vcvtpd2uqq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 80[ 	]*vcvtpd2uqq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 80[ 	]*vcvtpd2uqq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7b f5[ 	]*vcvtps2qq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7b f5[ 	]*vcvtps2qq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7b f5[ 	]*vcvtps2qq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7b f5[ 	]*vcvtps2qq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 7b f5[ 	]*vcvtps2qq \{ru-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 7b f5[ 	]*vcvtps2qq \{rd-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 7b f5[ 	]*vcvtps2qq \{rz-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 31[ 	]*vcvtps2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7b b4 f0 34 12 00 00[ 	]*vcvtps2qq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 31[ 	]*vcvtps2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 7f[ 	]*vcvtps2qq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 00 10 00 00[ 	]*vcvtps2qq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 80[ 	]*vcvtps2qq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 e0 ef ff ff[ 	]*vcvtps2qq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 80[ 	]*vcvtps2qq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 79 f5[ 	]*vcvtps2uqq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 79 f5[ 	]*vcvtps2uqq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 79 f5[ 	]*vcvtps2uqq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 79 f5[ 	]*vcvtps2uqq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 79 f5[ 	]*vcvtps2uqq \{ru-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 79 f5[ 	]*vcvtps2uqq \{rd-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 79 f5[ 	]*vcvtps2uqq \{rz-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 31[ 	]*vcvtps2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 79 b4 f0 34 12 00 00[ 	]*vcvtps2uqq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 31[ 	]*vcvtps2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 7f[ 	]*vcvtps2uqq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 00 10 00 00[ 	]*vcvtps2uqq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 80[ 	]*vcvtps2uqq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 80[ 	]*vcvtps2uqq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 e6 f5[ 	]*vcvtqq2pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f e6 f5[ 	]*vcvtqq2pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf e6 f5[ 	]*vcvtqq2pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 e6 f5[ 	]*vcvtqq2pd \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 e6 f5[ 	]*vcvtqq2pd \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 e6 f5[ 	]*vcvtqq2pd \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 e6 f5[ 	]*vcvtqq2pd \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 31[ 	]*vcvtqq2pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 e6 b4 f0 34 12 00 00[ 	]*vcvtqq2pd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 31[ 	]*vcvtqq2pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 7f[ 	]*vcvtqq2pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 80[ 	]*vcvtqq2pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 80[ 	]*vcvtqq2pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 48 5b f5[ 	]*vcvtqq2ps %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 4f 5b f5[ 	]*vcvtqq2ps %zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc cf 5b f5[ 	]*vcvtqq2ps %zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 18 5b f5[ 	]*vcvtqq2ps \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 58 5b f5[ 	]*vcvtqq2ps \{ru-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 5b f5[ 	]*vcvtqq2ps \{rd-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 78 5b f5[ 	]*vcvtqq2ps \{rz-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 31[ 	]*vcvtqq2ps \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 48 5b b4 f0 34 12 00 00[ 	]*vcvtqq2ps 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 31[ 	]*vcvtqq2ps \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 7f[ 	]*vcvtqq2ps 0x1fc0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 00 20 00 00[ 	]*vcvtqq2ps 0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 80[ 	]*vcvtqq2ps -0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 c0 df ff ff[ 	]*vcvtqq2ps -0x2040\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 80[ 	]*vcvtqq2ps -0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 7a f5[ 	]*vcvtuqq2pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 7a f5[ 	]*vcvtuqq2pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 7a f5[ 	]*vcvtuqq2pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 7a f5[ 	]*vcvtuqq2pd \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 7a f5[ 	]*vcvtuqq2pd \{ru-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 7a f5[ 	]*vcvtuqq2pd \{rd-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 7a f5[ 	]*vcvtuqq2pd \{rz-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 31[ 	]*vcvtuqq2pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2pd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 31[ 	]*vcvtuqq2pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 7f[ 	]*vcvtuqq2pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 80[ 	]*vcvtuqq2pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 7a f5[ 	]*vcvtuqq2ps %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 7a f5[ 	]*vcvtuqq2ps %zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 7a f5[ 	]*vcvtuqq2ps %zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 18 7a f5[ 	]*vcvtuqq2ps \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 58 7a f5[ 	]*vcvtuqq2ps \{ru-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 38 7a f5[ 	]*vcvtuqq2ps \{rd-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 78 7a f5[ 	]*vcvtuqq2ps \{rz-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 31[ 	]*vcvtuqq2ps \(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2ps 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 31[ 	]*vcvtuqq2ps \(%rcx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 7f[ 	]*vcvtuqq2ps 0x1fc0\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 00 20 00 00[ 	]*vcvtuqq2ps 0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 80[ 	]*vcvtuqq2ps -0x2000\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps -0x2040\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%rdx\)\{1to8\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee 7b[ 	]*vextractf32x8 \$0x7b,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee 7b[ 	]*vextracti32x8 \$0x7b,%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 4f 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee 7b[ 	]*vfpclasspd \$0x7b,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspdz \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 48 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspdz \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 29 7b[ 	]*vfpclasspd \$0x7b,\(%rcx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspdz \$0x7b,0x1fc0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspdz \$0x7b,0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspdz \$0x7b,-0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspdz \$0x7b,-0x2040\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%rdx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee ab[ 	]*vfpclassps \$0xab,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 4f 66 ee ab[ 	]*vfpclassps \$0xab,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee 7b[ 	]*vfpclassps \$0x7b,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclasspsz \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 48 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspsz \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 29 7b[ 	]*vfpclassps \$0x7b,\(%rcx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclasspsz \$0x7b,0x1fc0\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspsz \$0x7b,0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclasspsz \$0x7b,-0x2000\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspsz \$0x7b,-0x2040\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%rdx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee ab[ 	]*vfpclasssd \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 67 ee ab[ 	]*vfpclasssd \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee 7b[ 	]*vfpclasssd \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 29 7b[ 	]*vfpclasssd \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 67 ac f0 34 12 00 00 7b[ 	]*vfpclasssd \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 7f 7b[ 	]*vfpclasssd \$0x7b,0x3f8\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa 00 04 00 00 7b[ 	]*vfpclasssd \$0x7b,0x400\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 80 7b[ 	]*vfpclasssd \$0x7b,-0x400\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd \$0x7b,-0x408\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee ab[ 	]*vfpclassss \$0xab,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 67 ee ab[ 	]*vfpclassss \$0xab,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee 7b[ 	]*vfpclassss \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 29 7b[ 	]*vfpclassss \$0x7b,\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 67 ac f0 34 12 00 00 7b[ 	]*vfpclassss \$0x7b,0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 7f 7b[ 	]*vfpclassss \$0x7b,0x1fc\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa 00 02 00 00 7b[ 	]*vfpclassss \$0x7b,0x200\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 80 7b[ 	]*vfpclassss \$0x7b,-0x200\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa fc fd ff ff 7b[ 	]*vfpclassss \$0x7b,-0x204\(%rdx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 18 b4 f0 34 12 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 7b[ 	]*vinsertf32x8 \$0x7b,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 31 7b[ 	]*vinsertf32x8 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 1a b4 f0 34 12 00 00 7b[ 	]*vinsertf32x8 \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 7f 7b[ 	]*vinsertf32x8 \$0x7b,0xfe0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 \$0x7b,0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 80 7b[ 	]*vinsertf32x8 \$0x7b,-0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 \$0x7b,-0x1020\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 38 b4 f0 34 12 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 7b[ 	]*vinserti32x8 \$0x7b,%ymm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 31 7b[ 	]*vinserti32x8 \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 3a b4 f0 34 12 00 00 7b[ 	]*vinserti32x8 \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 7f 7b[ 	]*vinserti32x8 \$0x7b,0xfe0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 \$0x7b,0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 80 7b[ 	]*vinserti32x8 \$0x7b,-0x1000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 \$0x7b,-0x1020\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 59 f7[ 	]*vbroadcasti32x2 %xmm31,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 59 f7[ 	]*vbroadcasti32x2 %xmm31,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 59 f7[ 	]*vbroadcasti32x2 %xmm31,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 31[ 	]*vbroadcasti32x2 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 59 b4 f0 34 12 00 00[ 	]*vbroadcasti32x2 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 ab[ 	]*vpextrd \$0xab,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 7b[ 	]*vpextrd \$0x7b,%xmm29,%eax
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 ed 7b[ 	]*vpextrd \$0x7b,%xmm29,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 43 7d 08 16 ed 7b[ 	]*vpextrd \$0x7b,%xmm29,%r13d
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 29 7b[ 	]*vpextrd \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 16 ac f0 34 12 00 00 7b[ 	]*vpextrd \$0x7b,%xmm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 7f 7b[ 	]*vpextrd \$0x7b,%xmm29,0x1fc\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa 00 02 00 00 7b[ 	]*vpextrd \$0x7b,%xmm29,0x200\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 80 7b[ 	]*vpextrd \$0x7b,%xmm29,-0x200\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa fc fd ff ff 7b[ 	]*vpextrd \$0x7b,%xmm29,-0x204\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 ab[ 	]*vpextrq \$0xab,%xmm29,%rax
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 7b[ 	]*vpextrq \$0x7b,%xmm29,%rax
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 16 e8 7b[ 	]*vpextrq \$0x7b,%xmm29,%r8
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 29 7b[ 	]*vpextrq \$0x7b,%xmm29,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 16 ac f0 34 12 00 00 7b[ 	]*vpextrq \$0x7b,%xmm29,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 7f 7b[ 	]*vpextrq \$0x7b,%xmm29,0x3f8\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa 00 04 00 00 7b[ 	]*vpextrq \$0x7b,%xmm29,0x400\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 80 7b[ 	]*vpextrq \$0x7b,%xmm29,-0x400\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa f8 fb ff ff 7b[ 	]*vpextrq \$0x7b,%xmm29,-0x408\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 ab[ 	]*vpinsrd \$0xab,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 7b[ 	]*vpinsrd \$0x7b,%eax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f5 7b[ 	]*vpinsrd \$0x7b,%ebp,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 43 15 00 22 f5 7b[ 	]*vpinsrd \$0x7b,%r13d,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 31 7b[ 	]*vpinsrd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 22 b4 f0 34 12 00 00 7b[ 	]*vpinsrd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 7f 7b[ 	]*vpinsrd \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 00 02 00 00 7b[ 	]*vpinsrd \$0x7b,0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 80 7b[ 	]*vpinsrd \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 fc fd ff ff 7b[ 	]*vpinsrd \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 ab[ 	]*vpinsrq \$0xab,%rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 7b[ 	]*vpinsrq \$0x7b,%rax,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 22 f0 7b[ 	]*vpinsrq \$0x7b,%r8,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 31 7b[ 	]*vpinsrq \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 22 b4 f0 34 12 00 00 7b[ 	]*vpinsrq \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 7f 7b[ 	]*vpinsrq \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 00 04 00 00 7b[ 	]*vpinsrq \$0x7b,0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 80 7b[ 	]*vpinsrq \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 f8 fb ff ff 7b[ 	]*vpinsrq \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 40 f4[ 	]*vpmullq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 40 f4[ 	]*vpmullq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 40 f4[ 	]*vpmullq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 31[ 	]*vpmullq \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 40 b4 f0 34 12 00 00[ 	]*vpmullq 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 31[ 	]*vpmullq \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 7f[ 	]*vpmullq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 00 20 00 00[ 	]*vpmullq 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 80[ 	]*vpmullq -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 c0 df ff ff[ 	]*vpmullq -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 7f[ 	]*vpmullq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 80[ 	]*vpmullq -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 ab[ 	]*vrangepd \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 50 f4 ab[ 	]*vrangepd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 50 f4 ab[ 	]*vrangepd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 7b[ 	]*vrangepd \$0x7b,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 50 b4 f0 34 12 00 00 7b[ 	]*vrangepd \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 31 7b[ 	]*vrangepd \$0x7b,\(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 00 20 00 00 7b[ 	]*vrangepd \$0x7b,0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 c0 df ff ff 7b[ 	]*vrangepd \$0x7b,-0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 ab[ 	]*vrangeps \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 50 f4 ab[ 	]*vrangeps \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 50 f4 ab[ 	]*vrangeps \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 7b[ 	]*vrangeps \$0x7b,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 50 b4 f0 34 12 00 00 7b[ 	]*vrangeps \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 31 7b[ 	]*vrangeps \$0x7b,\(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 00 20 00 00 7b[ 	]*vrangeps \$0x7b,0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 c0 df ff ff 7b[ 	]*vrangeps \$0x7b,-0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 ab[ 	]*vrangesd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 51 f4 ab[ 	]*vrangesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 51 f4 ab[ 	]*vrangesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 7b[ 	]*vrangesd \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 31 7b[ 	]*vrangesd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 51 b4 f0 34 12 00 00 7b[ 	]*vrangesd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 7f 7b[ 	]*vrangesd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 00 04 00 00 7b[ 	]*vrangesd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 80 7b[ 	]*vrangesd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 f8 fb ff ff 7b[ 	]*vrangesd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 ab[ 	]*vrangess \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 51 f4 ab[ 	]*vrangess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 51 f4 ab[ 	]*vrangess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 7b[ 	]*vrangess \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 31 7b[ 	]*vrangess \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 51 b4 f0 34 12 00 00 7b[ 	]*vrangess \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 7f 7b[ 	]*vrangess \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 00 02 00 00 7b[ 	]*vrangess \$0x7b,0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 80 7b[ 	]*vrangess \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 fc fd ff ff 7b[ 	]*vrangess \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 54 f4[ 	]*vandpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 54 f4[ 	]*vandpd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 54 f4[ 	]*vandpd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 31[ 	]*vandpd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 54 b4 f0 34 12 00 00[ 	]*vandpd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 31[ 	]*vandpd \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 7f[ 	]*vandpd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 00 20 00 00[ 	]*vandpd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 80[ 	]*vandpd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 c0 df ff ff[ 	]*vandpd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 7f[ 	]*vandpd 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 80[ 	]*vandpd -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 54 f4[ 	]*vandps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 54 f4[ 	]*vandps %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 54 f4[ 	]*vandps %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 31[ 	]*vandps \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 54 b4 f0 34 12 00 00[ 	]*vandps 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 31[ 	]*vandps \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 7f[ 	]*vandps 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 00 20 00 00[ 	]*vandps 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 80[ 	]*vandps -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 c0 df ff ff[ 	]*vandps -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 7f[ 	]*vandps 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 00 02 00 00[ 	]*vandps 0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 80[ 	]*vandps -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 55 f4[ 	]*vandnpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 55 f4[ 	]*vandnpd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 55 f4[ 	]*vandnpd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 31[ 	]*vandnpd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 55 b4 f0 34 12 00 00[ 	]*vandnpd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 31[ 	]*vandnpd \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 7f[ 	]*vandnpd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 00 20 00 00[ 	]*vandnpd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 80[ 	]*vandnpd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 c0 df ff ff[ 	]*vandnpd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 7f[ 	]*vandnpd 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 80[ 	]*vandnpd -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 55 f4[ 	]*vandnps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 55 f4[ 	]*vandnps %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 55 f4[ 	]*vandnps %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 31[ 	]*vandnps \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 55 b4 f0 34 12 00 00[ 	]*vandnps 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 31[ 	]*vandnps \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 7f[ 	]*vandnps 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 00 20 00 00[ 	]*vandnps 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 80[ 	]*vandnps -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 c0 df ff ff[ 	]*vandnps -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 7f[ 	]*vandnps 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 80[ 	]*vandnps -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 56 f4[ 	]*vorpd  %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 56 f4[ 	]*vorpd  %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 56 f4[ 	]*vorpd  %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 31[ 	]*vorpd  \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 56 b4 f0 34 12 00 00[ 	]*vorpd  0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 31[ 	]*vorpd  \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 7f[ 	]*vorpd  0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 00 20 00 00[ 	]*vorpd  0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 80[ 	]*vorpd  -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 c0 df ff ff[ 	]*vorpd  -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 7f[ 	]*vorpd  0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 80[ 	]*vorpd  -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 56 f4[ 	]*vorps  %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 56 f4[ 	]*vorps  %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 56 f4[ 	]*vorps  %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 31[ 	]*vorps  \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 56 b4 f0 34 12 00 00[ 	]*vorps  0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 31[ 	]*vorps  \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 7f[ 	]*vorps  0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 00 20 00 00[ 	]*vorps  0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 80[ 	]*vorps  -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 c0 df ff ff[ 	]*vorps  -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 7f[ 	]*vorps  0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 00 02 00 00[ 	]*vorps  0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 80[ 	]*vorps  -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 57 f4[ 	]*vxorpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 57 f4[ 	]*vxorpd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 57 f4[ 	]*vxorpd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 31[ 	]*vxorpd \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 57 b4 f0 34 12 00 00[ 	]*vxorpd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 31[ 	]*vxorpd \(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 7f[ 	]*vxorpd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 00 20 00 00[ 	]*vxorpd 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 80[ 	]*vxorpd -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 c0 df ff ff[ 	]*vxorpd -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 7f[ 	]*vxorpd 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 80[ 	]*vxorpd -0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 57 f4[ 	]*vxorps %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 57 f4[ 	]*vxorps %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 57 f4[ 	]*vxorps %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 31[ 	]*vxorps \(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 57 b4 f0 34 12 00 00[ 	]*vxorps 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 31[ 	]*vxorps \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 7f[ 	]*vxorps 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 00 20 00 00[ 	]*vxorps 0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 80[ 	]*vxorps -0x2000\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 c0 df ff ff[ 	]*vxorps -0x2040\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 7f[ 	]*vxorps 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 80[ 	]*vxorps -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 ab[ 	]*vreducepd \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 56 f5 ab[ 	]*vreducepd \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 56 f5 ab[ 	]*vreducepd \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 7b[ 	]*vreducepd \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 56 b4 f0 34 12 00 00 7b[ 	]*vreducepd \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 31 7b[ 	]*vreducepd \$0x7b,\(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 ab[ 	]*vreduceps \$0xab,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 56 f5 ab[ 	]*vreduceps \$0xab,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 56 f5 ab[ 	]*vreduceps \$0xab,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 7b[ 	]*vreduceps \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 56 b4 f0 34 12 00 00 7b[ 	]*vreduceps \$0x7b,0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 31 7b[ 	]*vreduceps \$0x7b,\(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps \$0x7b,0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps \$0x7b,-0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 ab[ 	]*vreducesd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 57 f4 ab[ 	]*vreducesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 57 f4 ab[ 	]*vreducesd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 7b[ 	]*vreducesd \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 31 7b[ 	]*vreducesd \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 57 b4 f0 34 12 00 00 7b[ 	]*vreducesd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 7f 7b[ 	]*vreducesd \$0x7b,0x3f8\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 00 04 00 00 7b[ 	]*vreducesd \$0x7b,0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 80 7b[ 	]*vreducesd \$0x7b,-0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 f8 fb ff ff 7b[ 	]*vreducesd \$0x7b,-0x408\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 ab[ 	]*vreducess \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 57 f4 ab[ 	]*vreducess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 57 f4 ab[ 	]*vreducess \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 7b[ 	]*vreducess \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 31 7b[ 	]*vreducess \$0x7b,\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 57 b4 f0 34 12 00 00 7b[ 	]*vreducess \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 7f 7b[ 	]*vreducess \$0x7b,0x1fc\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 00 02 00 00 7b[ 	]*vreducess \$0x7b,0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 80 7b[ 	]*vreducess \$0x7b,-0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 fc fd ff ff 7b[ 	]*vreducess \$0x7b,-0x204\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*c5 cd 41 ef[ 	]*kandb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 42 ef[ 	]*kandnb %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 45 ef[ 	]*korb   %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 46 ef[ 	]*kxnorb %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 47 ef[ 	]*kxorb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 44 ee[ 	]*knotb  %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 98 ee[ 	]*kortestb %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f8 99 ee[ 	]*ktestw %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 99 ee[ 	]*ktestb %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee ab[ 	]*kshiftrb \$0xab,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee 7b[ 	]*kshiftrb \$0x7b,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee ab[ 	]*kshiftlb \$0xab,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee 7b[ 	]*kshiftlb \$0x7b,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ee[ 	]*kmovb  %k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  \(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 90 ac f0 34 12 00 00[ 	]*kmovb  0x1234\(%rax,%r14,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  %k5,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 91 ac f0 34 12 00 00[ 	]*kmovb  %k5,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  %eax,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  %ebp,%k5
[ 	]*[a-f0-9]+:[ 	]*c4 c1 79 92 ed[ 	]*kmovb  %r13d,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  %k5,%eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  %k5,%ebp
[ 	]*[a-f0-9]+:[ 	]*c5 79 93 ed[ 	]*kmovb  %k5,%r13d
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 19 b4 f0 34 12 00 00 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 7f 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 80 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%zmm30,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 1b b4 f0 34 12 00 00 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,0xfe0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 80 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,-0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 \$0x7b,%zmm30,-0x1020\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 39 b4 f0 34 12 00 00 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 7f 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,0x7f0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 80 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,-0x800\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%zmm30,-0x810\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 3b b4 f0 34 12 00 00 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,0xfe0\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 80 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,-0x1000\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 \$0x7b,%zmm30,-0x1020\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7a f5[ 	]*vcvttpd2qq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7a f5[ 	]*vcvttpd2qq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7a f5[ 	]*vcvttpd2qq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 31[ 	]*vcvttpd2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7a b4 f0 34 12 00 00[ 	]*vcvttpd2qq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 31[ 	]*vcvttpd2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 7f[ 	]*vcvttpd2qq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 80[ 	]*vcvttpd2qq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 80[ 	]*vcvttpd2qq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 78 f5[ 	]*vcvttpd2uqq %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 78 f5[ 	]*vcvttpd2uqq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 78 f5[ 	]*vcvttpd2uqq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 31[ 	]*vcvttpd2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 78 b4 f0 34 12 00 00[ 	]*vcvttpd2uqq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 31[ 	]*vcvttpd2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 7f[ 	]*vcvttpd2uqq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 80[ 	]*vcvttpd2uqq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 80[ 	]*vcvttpd2uqq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7a f5[ 	]*vcvttps2qq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7a f5[ 	]*vcvttps2qq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7a f5[ 	]*vcvttps2qq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7a f5[ 	]*vcvttps2qq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 31[ 	]*vcvttps2qq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7a b4 f0 34 12 00 00[ 	]*vcvttps2qq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 31[ 	]*vcvttps2qq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 7f[ 	]*vcvttps2qq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 00 10 00 00[ 	]*vcvttps2qq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 80[ 	]*vcvttps2qq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 e0 ef ff ff[ 	]*vcvttps2qq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 80[ 	]*vcvttps2qq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 78 f5[ 	]*vcvttps2uqq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 78 f5[ 	]*vcvttps2uqq %ymm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 78 f5[ 	]*vcvttps2uqq %ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 31[ 	]*vcvttps2uqq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 78 b4 f0 34 12 00 00[ 	]*vcvttps2uqq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 31[ 	]*vcvttps2uqq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 7f[ 	]*vcvttps2uqq 0xfe0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 00 10 00 00[ 	]*vcvttps2uqq 0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 80[ 	]*vcvttps2uqq -0x1000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq -0x1020\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 80[ 	]*vcvttps2uqq -0x200\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 48 39 ee[ 	]*vpmovd2m %zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 48 39 ee[ 	]*vpmovq2m %zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 38 f5[ 	]*vpmovm2d %k5,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 48 38 f5[ 	]*vpmovm2q %k5,%zmm30
#pass
