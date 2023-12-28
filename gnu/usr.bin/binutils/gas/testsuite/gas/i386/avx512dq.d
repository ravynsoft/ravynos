#as:
#objdump: -dw
#name: i386 AVX512DQ insns
#source: avx512dq.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x8 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 0xfe0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 80[ 	]*vbroadcastf32x8 -0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 -0x1020\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 31[ 	]*vbroadcasti32x8 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 5b 31[ 	]*vbroadcasti32x8 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 5b 31[ 	]*vbroadcasti32x8 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x8 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 0xfe0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 80[ 	]*vbroadcasti32x8 -0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 -0x1020\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 f7[ 	]*vbroadcastf32x2 %xmm7,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 19 f7[ 	]*vbroadcastf32x2 %xmm7,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 19 f7[ 	]*vbroadcastf32x2 %xmm7,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 31[ 	]*vbroadcastf32x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b f5[ 	]*vcvtpd2qq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7b f5[ 	]*vcvtpd2qq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7b f5[ 	]*vcvtpd2qq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7b f5[ 	]*vcvtpd2qq \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b f5[ 	]*vcvtpd2qq \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 7b f5[ 	]*vcvtpd2qq \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 7b f5[ 	]*vcvtpd2qq \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 31[ 	]*vcvtpd2qq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 30[ 	]*vcvtpd2qq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 7f[ 	]*vcvtpd2qq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 80[ 	]*vcvtpd2qq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 80[ 	]*vcvtpd2qq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 f5[ 	]*vcvtpd2uqq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 79 f5[ 	]*vcvtpd2uqq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 79 f5[ 	]*vcvtpd2uqq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 79 f5[ 	]*vcvtpd2uqq \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 f5[ 	]*vcvtpd2uqq \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 79 f5[ 	]*vcvtpd2uqq \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 79 f5[ 	]*vcvtpd2uqq \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 31[ 	]*vcvtpd2uqq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 30[ 	]*vcvtpd2uqq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 7f[ 	]*vcvtpd2uqq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 80[ 	]*vcvtpd2uqq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 80[ 	]*vcvtpd2uqq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b f5[ 	]*vcvtps2qq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7b f5[ 	]*vcvtps2qq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b f5[ 	]*vcvtps2qq \{rn-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b f5[ 	]*vcvtps2qq \{ru-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b f5[ 	]*vcvtps2qq \{rd-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 7b f5[ 	]*vcvtps2qq \{rz-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 31[ 	]*vcvtps2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 30[ 	]*vcvtps2qq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 7f[ 	]*vcvtps2qq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 00 10 00 00[ 	]*vcvtps2qq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 80[ 	]*vcvtps2qq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 e0 ef ff ff[ 	]*vcvtps2qq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 80[ 	]*vcvtps2qq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 f5[ 	]*vcvtps2uqq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 79 f5[ 	]*vcvtps2uqq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 f5[ 	]*vcvtps2uqq \{rn-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 f5[ 	]*vcvtps2uqq \{ru-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 f5[ 	]*vcvtps2uqq \{rd-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 79 f5[ 	]*vcvtps2uqq \{rz-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 31[ 	]*vcvtps2uqq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 30[ 	]*vcvtps2uqq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 7f[ 	]*vcvtps2uqq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 00 10 00 00[ 	]*vcvtps2uqq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 80[ 	]*vcvtps2uqq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 80[ 	]*vcvtps2uqq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 f5[ 	]*vcvtqq2pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f e6 f5[ 	]*vcvtqq2pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf e6 f5[ 	]*vcvtqq2pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 e6 f5[ 	]*vcvtqq2pd \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 f5[ 	]*vcvtqq2pd \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 e6 f5[ 	]*vcvtqq2pd \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 e6 f5[ 	]*vcvtqq2pd \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 31[ 	]*vcvtqq2pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 30[ 	]*vcvtqq2pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 7f[ 	]*vcvtqq2pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 80[ 	]*vcvtqq2pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 80[ 	]*vcvtqq2pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b f5[ 	]*vcvtqq2ps %zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc cf 5b f5[ 	]*vcvtqq2ps %zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b f5[ 	]*vcvtqq2ps \{rn-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b f5[ 	]*vcvtqq2ps \{ru-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b f5[ 	]*vcvtqq2ps \{rd-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 7f 5b f5[ 	]*vcvtqq2ps \{rz-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 31[ 	]*vcvtqq2ps \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 30[ 	]*vcvtqq2ps \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 7f[ 	]*vcvtqq2ps 0x1fc0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 00 20 00 00[ 	]*vcvtqq2ps 0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 80[ 	]*vcvtqq2ps -0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 c0 df ff ff[ 	]*vcvtqq2ps -0x2040\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 80[ 	]*vcvtqq2ps -0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a f5[ 	]*vcvtuqq2pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f 7a f5[ 	]*vcvtuqq2pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf 7a f5[ 	]*vcvtuqq2pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 7a f5[ 	]*vcvtuqq2pd \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a f5[ 	]*vcvtuqq2pd \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 7a f5[ 	]*vcvtuqq2pd \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 7a f5[ 	]*vcvtuqq2pd \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 31[ 	]*vcvtuqq2pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 30[ 	]*vcvtuqq2pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 7f[ 	]*vcvtuqq2pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 80[ 	]*vcvtuqq2pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a f5[ 	]*vcvtuqq2ps %zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7a f5[ 	]*vcvtuqq2ps %zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a f5[ 	]*vcvtuqq2ps \{rn-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a f5[ 	]*vcvtuqq2ps \{ru-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a f5[ 	]*vcvtuqq2ps \{rd-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 7f 7a f5[ 	]*vcvtuqq2ps \{rz-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 31[ 	]*vcvtuqq2ps \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 30[ 	]*vcvtuqq2ps \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 7f[ 	]*vcvtuqq2ps 0x1fc0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 00 20 00 00[ 	]*vcvtuqq2ps 0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 80[ 	]*vcvtuqq2ps -0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps -0x2040\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee 7b[ 	]*vextractf32x8 \$0x7b,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee 7b[ 	]*vextracti32x8 \$0x7b,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee 7b[ 	]*vfpclasspd \$0x7b,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspdz \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspdz \$0x7b,-0x1e240\(%esp,%esi,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 28 7b[ 	]*vfpclasspd \$0x7b,\(%eax\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspdz \$0x7b,0x1fc0\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspdz \$0x7b,0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspdz \$0x7b,-0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspdz \$0x7b,-0x2040\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee ab[ 	]*vfpclassps \$0xab,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 66 ee ab[ 	]*vfpclassps \$0xab,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee 7b[ 	]*vfpclassps \$0x7b,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclasspsz \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspsz \$0x7b,-0x1e240\(%esp,%esi,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 28 7b[ 	]*vfpclassps \$0x7b,\(%eax\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclasspsz \$0x7b,0x1fc0\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspsz \$0x7b,0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclasspsz \$0x7b,-0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspsz \$0x7b,-0x2040\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee ab[ 	]*vfpclasssd \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee 7b[ 	]*vfpclasssd \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 29 7b[ 	]*vfpclasssd \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclasssd \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 7f 7b[ 	]*vfpclasssd \$0x7b,0x3f8\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa 00 04 00 00 7b[ 	]*vfpclasssd \$0x7b,0x400\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 80 7b[ 	]*vfpclasssd \$0x7b,-0x400\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd \$0x7b,-0x408\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee ab[ 	]*vfpclassss \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee 7b[ 	]*vfpclassss \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 29 7b[ 	]*vfpclassss \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclassss \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 7f 7b[ 	]*vfpclassss \$0x7b,0x1fc\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa 00 02 00 00 7b[ 	]*vfpclassss \$0x7b,0x200\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 80 7b[ 	]*vfpclassss \$0x7b,-0x200\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa fc fd ff ff 7b[ 	]*vfpclassss \$0x7b,-0x204\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 7b[ 	]*vinsertf32x8 \$0x7b,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 31 7b[ 	]*vinsertf32x8 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b4 f4 c0 1d fe ff 7b[ 	]*vinsertf32x8 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 7f 7b[ 	]*vinsertf32x8 \$0x7b,0xfe0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 \$0x7b,0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 80 7b[ 	]*vinsertf32x8 \$0x7b,-0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 \$0x7b,-0x1020\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 7b[ 	]*vinserti32x8 \$0x7b,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 31 7b[ 	]*vinserti32x8 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b4 f4 c0 1d fe ff 7b[ 	]*vinserti32x8 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 7f 7b[ 	]*vinserti32x8 \$0x7b,0xfe0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 \$0x7b,0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 80 7b[ 	]*vinserti32x8 \$0x7b,-0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 \$0x7b,-0x1020\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 f7[ 	]*vbroadcasti32x2 %xmm7,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 59 f7[ 	]*vbroadcasti32x2 %xmm7,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 31[ 	]*vbroadcasti32x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 f4[ 	]*vpmullq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 40 f4[ 	]*vpmullq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 40 f4[ 	]*vpmullq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 31[ 	]*vpmullq \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b4 f4 c0 1d fe ff[ 	]*vpmullq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 30[ 	]*vpmullq \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 7f[ 	]*vpmullq 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 00 20 00 00[ 	]*vpmullq 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 80[ 	]*vpmullq -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 c0 df ff ff[ 	]*vpmullq -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 7f[ 	]*vpmullq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 80[ 	]*vpmullq -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 ab[ 	]*vrangepd \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 50 f4 ab[ 	]*vrangepd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 50 f4 ab[ 	]*vrangepd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 7b[ 	]*vrangepd \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 31 7b[ 	]*vrangepd \$0x7b,\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 30 7b[ 	]*vrangepd \$0x7b,\(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 00 20 00 00 7b[ 	]*vrangepd \$0x7b,0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 c0 df ff ff 7b[ 	]*vrangepd \$0x7b,-0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 ab[ 	]*vrangeps \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 50 f4 ab[ 	]*vrangeps \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 50 f4 ab[ 	]*vrangeps \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 7b[ 	]*vrangeps \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 31 7b[ 	]*vrangeps \$0x7b,\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 30 7b[ 	]*vrangeps \$0x7b,\(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 00 20 00 00 7b[ 	]*vrangeps \$0x7b,0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 c0 df ff ff 7b[ 	]*vrangeps \$0x7b,-0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 ab[ 	]*vrangesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 51 f4 ab[ 	]*vrangesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 7b[ 	]*vrangesd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 31 7b[ 	]*vrangesd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangesd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 7f 7b[ 	]*vrangesd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 00 04 00 00 7b[ 	]*vrangesd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 80 7b[ 	]*vrangesd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 f8 fb ff ff 7b[ 	]*vrangesd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 ab[ 	]*vrangess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 51 f4 ab[ 	]*vrangess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 7b[ 	]*vrangess \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 31 7b[ 	]*vrangess \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangess \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 7f 7b[ 	]*vrangess \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 00 02 00 00 7b[ 	]*vrangess \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 80 7b[ 	]*vrangess \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 fc fd ff ff 7b[ 	]*vrangess \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 f4[ 	]*vandpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 54 f4[ 	]*vandpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 54 f4[ 	]*vandpd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 31[ 	]*vandpd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b4 f4 c0 1d fe ff[ 	]*vandpd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 30[ 	]*vandpd \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 7f[ 	]*vandpd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 00 20 00 00[ 	]*vandpd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 80[ 	]*vandpd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 c0 df ff ff[ 	]*vandpd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 7f[ 	]*vandpd 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 80[ 	]*vandpd -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 f4[ 	]*vandps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 54 f4[ 	]*vandps %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 54 f4[ 	]*vandps %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 31[ 	]*vandps \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b4 f4 c0 1d fe ff[ 	]*vandps -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 30[ 	]*vandps \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 7f[ 	]*vandps 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 00 20 00 00[ 	]*vandps 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 80[ 	]*vandps -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 c0 df ff ff[ 	]*vandps -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 7f[ 	]*vandps 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 00 02 00 00[ 	]*vandps 0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 80[ 	]*vandps -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 f4[ 	]*vandnpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 55 f4[ 	]*vandnpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 55 f4[ 	]*vandnpd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 31[ 	]*vandnpd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b4 f4 c0 1d fe ff[ 	]*vandnpd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 30[ 	]*vandnpd \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 7f[ 	]*vandnpd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 00 20 00 00[ 	]*vandnpd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 80[ 	]*vandnpd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 c0 df ff ff[ 	]*vandnpd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 7f[ 	]*vandnpd 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 80[ 	]*vandnpd -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 f4[ 	]*vandnps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 55 f4[ 	]*vandnps %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 55 f4[ 	]*vandnps %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 31[ 	]*vandnps \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b4 f4 c0 1d fe ff[ 	]*vandnps -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 30[ 	]*vandnps \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 7f[ 	]*vandnps 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 00 20 00 00[ 	]*vandnps 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 80[ 	]*vandnps -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 c0 df ff ff[ 	]*vandnps -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 7f[ 	]*vandnps 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 80[ 	]*vandnps -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 f4[ 	]*vorpd  %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 56 f4[ 	]*vorpd  %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 56 f4[ 	]*vorpd  %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 31[ 	]*vorpd  \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b4 f4 c0 1d fe ff[ 	]*vorpd  -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 30[ 	]*vorpd  \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 7f[ 	]*vorpd  0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 00 20 00 00[ 	]*vorpd  0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 80[ 	]*vorpd  -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 c0 df ff ff[ 	]*vorpd  -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 7f[ 	]*vorpd  0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 80[ 	]*vorpd  -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 f4[ 	]*vorps  %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 56 f4[ 	]*vorps  %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 56 f4[ 	]*vorps  %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 31[ 	]*vorps  \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b4 f4 c0 1d fe ff[ 	]*vorps  -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 30[ 	]*vorps  \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 7f[ 	]*vorps  0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 00 20 00 00[ 	]*vorps  0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 80[ 	]*vorps  -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 c0 df ff ff[ 	]*vorps  -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 7f[ 	]*vorps  0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 00 02 00 00[ 	]*vorps  0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 80[ 	]*vorps  -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 f4[ 	]*vxorpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 57 f4[ 	]*vxorpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 57 f4[ 	]*vxorpd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 31[ 	]*vxorpd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b4 f4 c0 1d fe ff[ 	]*vxorpd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 30[ 	]*vxorpd \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 7f[ 	]*vxorpd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 00 20 00 00[ 	]*vxorpd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 80[ 	]*vxorpd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 c0 df ff ff[ 	]*vxorpd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 7f[ 	]*vxorpd 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 80[ 	]*vxorpd -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 f4[ 	]*vxorps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 57 f4[ 	]*vxorps %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 57 f4[ 	]*vxorps %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 31[ 	]*vxorps \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b4 f4 c0 1d fe ff[ 	]*vxorps -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 30[ 	]*vxorps \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 7f[ 	]*vxorps 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 00 20 00 00[ 	]*vxorps 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 80[ 	]*vxorps -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 c0 df ff ff[ 	]*vxorps -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 7f[ 	]*vxorps 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 80[ 	]*vxorps -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 ab[ 	]*vreducepd \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 56 f5 ab[ 	]*vreducepd \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 56 f5 ab[ 	]*vreducepd \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 7b[ 	]*vreducepd \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 31 7b[ 	]*vreducepd \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 30 7b[ 	]*vreducepd \$0x7b,\(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 ab[ 	]*vreduceps \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 56 f5 ab[ 	]*vreduceps \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 56 f5 ab[ 	]*vreduceps \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 7b[ 	]*vreduceps \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 31 7b[ 	]*vreduceps \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 30 7b[ 	]*vreduceps \$0x7b,\(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 ab[ 	]*vreducesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 57 f4 ab[ 	]*vreducesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 7b[ 	]*vreducesd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 31 7b[ 	]*vreducesd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducesd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 7f 7b[ 	]*vreducesd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 00 04 00 00 7b[ 	]*vreducesd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 80 7b[ 	]*vreducesd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 f8 fb ff ff 7b[ 	]*vreducesd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 ab[ 	]*vreducess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 57 f4 ab[ 	]*vreducess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 7b[ 	]*vreducess \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 31 7b[ 	]*vreducess \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducess \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 7f 7b[ 	]*vreducess \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 00 02 00 00 7b[ 	]*vreducess \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 80 7b[ 	]*vreducess \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 fc fd ff ff 7b[ 	]*vreducess \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
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
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  \(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ac f4 c0 1d fe ff[ 	]*kmovb  -0x1e240\(%esp,%esi,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  %k5,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 ac f4 c0 1d fe ff[ 	]*kmovb  %k5,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  %eax,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  %ebp,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  %k5,%eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  %k5,%ebp
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b4 f4 c0 1d fe ff 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 7f 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,0x7f0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 80 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,-0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,-0x810\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b4 f4 c0 1d fe ff 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,0xfe0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 80 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,-0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,-0x1020\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b4 f4 c0 1d fe ff 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 7f 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,0x7f0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 80 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,-0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,-0x810\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b4 f4 c0 1d fe ff 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,0xfe0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 80 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,-0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,-0x1020\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a f5[ 	]*vcvttpd2qq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7a f5[ 	]*vcvttpd2qq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7a f5[ 	]*vcvttpd2qq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 31[ 	]*vcvttpd2qq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 30[ 	]*vcvttpd2qq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 7f[ 	]*vcvttpd2qq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 80[ 	]*vcvttpd2qq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 80[ 	]*vcvttpd2qq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 f5[ 	]*vcvttpd2uqq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 78 f5[ 	]*vcvttpd2uqq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 78 f5[ 	]*vcvttpd2uqq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 31[ 	]*vcvttpd2uqq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 30[ 	]*vcvttpd2uqq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 7f[ 	]*vcvttpd2uqq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 80[ 	]*vcvttpd2uqq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 80[ 	]*vcvttpd2uqq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a f5[ 	]*vcvttps2qq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7a f5[ 	]*vcvttps2qq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a f5[ 	]*vcvttps2qq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 31[ 	]*vcvttps2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 30[ 	]*vcvttps2qq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 7f[ 	]*vcvttps2qq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 00 10 00 00[ 	]*vcvttps2qq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 80[ 	]*vcvttps2qq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 e0 ef ff ff[ 	]*vcvttps2qq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 80[ 	]*vcvttps2qq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 f5[ 	]*vcvttps2uqq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 78 f5[ 	]*vcvttps2uqq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 31[ 	]*vcvttps2uqq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 30[ 	]*vcvttps2uqq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 7f[ 	]*vcvttps2uqq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 00 10 00 00[ 	]*vcvttps2uqq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 80[ 	]*vcvttps2uqq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 80[ 	]*vcvttps2uqq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 39 ee[ 	]*vpmovd2m %zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 39 ee[ 	]*vpmovq2m %zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 38 f5[ 	]*vpmovm2d %k5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 38 f5[ 	]*vpmovm2q %k5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x8 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 0xfe0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 80[ 	]*vbroadcastf32x8 -0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 -0x1020\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 31[ 	]*vbroadcasti32x8 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 5b 31[ 	]*vbroadcasti32x8 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 5b 31[ 	]*vbroadcasti32x8 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x8 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 0xfe0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 80[ 	]*vbroadcasti32x8 -0x1000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 -0x1020\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 f7[ 	]*vbroadcastf32x2 %xmm7,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 19 f7[ 	]*vbroadcastf32x2 %xmm7,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 19 f7[ 	]*vbroadcastf32x2 %xmm7,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 31[ 	]*vbroadcastf32x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b f5[ 	]*vcvtpd2qq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7b f5[ 	]*vcvtpd2qq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7b f5[ 	]*vcvtpd2qq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7b f5[ 	]*vcvtpd2qq \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b f5[ 	]*vcvtpd2qq \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 7b f5[ 	]*vcvtpd2qq \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 7b f5[ 	]*vcvtpd2qq \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 31[ 	]*vcvtpd2qq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 30[ 	]*vcvtpd2qq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 7f[ 	]*vcvtpd2qq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 80[ 	]*vcvtpd2qq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 80[ 	]*vcvtpd2qq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 f5[ 	]*vcvtpd2uqq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 79 f5[ 	]*vcvtpd2uqq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 79 f5[ 	]*vcvtpd2uqq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 79 f5[ 	]*vcvtpd2uqq \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 f5[ 	]*vcvtpd2uqq \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 79 f5[ 	]*vcvtpd2uqq \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 79 f5[ 	]*vcvtpd2uqq \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 31[ 	]*vcvtpd2uqq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 30[ 	]*vcvtpd2uqq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 7f[ 	]*vcvtpd2uqq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 80[ 	]*vcvtpd2uqq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 80[ 	]*vcvtpd2uqq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b f5[ 	]*vcvtps2qq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7b f5[ 	]*vcvtps2qq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b f5[ 	]*vcvtps2qq \{rn-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b f5[ 	]*vcvtps2qq \{ru-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b f5[ 	]*vcvtps2qq \{rd-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 7b f5[ 	]*vcvtps2qq \{rz-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 31[ 	]*vcvtps2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 30[ 	]*vcvtps2qq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 7f[ 	]*vcvtps2qq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 00 10 00 00[ 	]*vcvtps2qq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 80[ 	]*vcvtps2qq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 e0 ef ff ff[ 	]*vcvtps2qq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 80[ 	]*vcvtps2qq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 f5[ 	]*vcvtps2uqq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 79 f5[ 	]*vcvtps2uqq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 f5[ 	]*vcvtps2uqq \{rn-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 f5[ 	]*vcvtps2uqq \{ru-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 f5[ 	]*vcvtps2uqq \{rd-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 79 f5[ 	]*vcvtps2uqq \{rz-sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 31[ 	]*vcvtps2uqq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 30[ 	]*vcvtps2uqq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 7f[ 	]*vcvtps2uqq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 00 10 00 00[ 	]*vcvtps2uqq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 80[ 	]*vcvtps2uqq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 80[ 	]*vcvtps2uqq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 f5[ 	]*vcvtqq2pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f e6 f5[ 	]*vcvtqq2pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf e6 f5[ 	]*vcvtqq2pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 e6 f5[ 	]*vcvtqq2pd \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 f5[ 	]*vcvtqq2pd \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 e6 f5[ 	]*vcvtqq2pd \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 e6 f5[ 	]*vcvtqq2pd \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 31[ 	]*vcvtqq2pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 30[ 	]*vcvtqq2pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 7f[ 	]*vcvtqq2pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 80[ 	]*vcvtqq2pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 80[ 	]*vcvtqq2pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b f5[ 	]*vcvtqq2ps %zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc cf 5b f5[ 	]*vcvtqq2ps %zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b f5[ 	]*vcvtqq2ps \{rn-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b f5[ 	]*vcvtqq2ps \{ru-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b f5[ 	]*vcvtqq2ps \{rd-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 7f 5b f5[ 	]*vcvtqq2ps \{rz-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 31[ 	]*vcvtqq2ps \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 30[ 	]*vcvtqq2ps \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 7f[ 	]*vcvtqq2ps 0x1fc0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 00 20 00 00[ 	]*vcvtqq2ps 0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 80[ 	]*vcvtqq2ps -0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 c0 df ff ff[ 	]*vcvtqq2ps -0x2040\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 80[ 	]*vcvtqq2ps -0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a f5[ 	]*vcvtuqq2pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f 7a f5[ 	]*vcvtuqq2pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf 7a f5[ 	]*vcvtuqq2pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 7a f5[ 	]*vcvtuqq2pd \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a f5[ 	]*vcvtuqq2pd \{ru-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 7a f5[ 	]*vcvtuqq2pd \{rd-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 7a f5[ 	]*vcvtuqq2pd \{rz-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 31[ 	]*vcvtuqq2pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 30[ 	]*vcvtuqq2pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 7f[ 	]*vcvtuqq2pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 80[ 	]*vcvtuqq2pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a f5[ 	]*vcvtuqq2ps %zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7a f5[ 	]*vcvtuqq2ps %zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a f5[ 	]*vcvtuqq2ps \{rn-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a f5[ 	]*vcvtuqq2ps \{ru-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a f5[ 	]*vcvtuqq2ps \{rd-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 7f 7a f5[ 	]*vcvtuqq2ps \{rz-sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 31[ 	]*vcvtuqq2ps \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 30[ 	]*vcvtuqq2ps \(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 7f[ 	]*vcvtuqq2ps 0x1fc0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 00 20 00 00[ 	]*vcvtuqq2ps 0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 80[ 	]*vcvtuqq2ps -0x2000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps -0x2040\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 19 ee ab[ 	]*vextractf64x2 \$0xab,%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 1b ee ab[ 	]*vextractf32x8 \$0xab,%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee 7b[ 	]*vextractf32x8 \$0x7b,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 39 ee ab[ 	]*vextracti64x2 \$0xab,%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%zmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 3b ee ab[ 	]*vextracti32x8 \$0xab,%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee 7b[ 	]*vextracti32x8 \$0x7b,%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 66 ee ab[ 	]*vfpclasspd \$0xab,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee 7b[ 	]*vfpclasspd \$0x7b,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspdz \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspdz \$0x7b,-0x1e240\(%esp,%esi,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 28 7b[ 	]*vfpclasspd \$0x7b,\(%eax\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspdz \$0x7b,0x1fc0\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspdz \$0x7b,0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspdz \$0x7b,-0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspdz \$0x7b,-0x2040\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%edx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee ab[ 	]*vfpclassps \$0xab,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 66 ee ab[ 	]*vfpclassps \$0xab,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee 7b[ 	]*vfpclassps \$0x7b,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclasspsz \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspsz \$0x7b,-0x1e240\(%esp,%esi,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 28 7b[ 	]*vfpclassps \$0x7b,\(%eax\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclasspsz \$0x7b,0x1fc0\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspsz \$0x7b,0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclasspsz \$0x7b,-0x2000\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspsz \$0x7b,-0x2040\(%edx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%edx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee ab[ 	]*vfpclasssd \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee 7b[ 	]*vfpclasssd \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 29 7b[ 	]*vfpclasssd \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclasssd \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 7f 7b[ 	]*vfpclasssd \$0x7b,0x3f8\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa 00 04 00 00 7b[ 	]*vfpclasssd \$0x7b,0x400\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 80 7b[ 	]*vfpclasssd \$0x7b,-0x400\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd \$0x7b,-0x408\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee ab[ 	]*vfpclassss \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee 7b[ 	]*vfpclassss \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 29 7b[ 	]*vfpclassss \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclassss \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 7f 7b[ 	]*vfpclassss \$0x7b,0x1fc\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa 00 02 00 00 7b[ 	]*vfpclassss \$0x7b,0x200\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 80 7b[ 	]*vfpclassss \$0x7b,-0x200\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa fc fd ff ff 7b[ 	]*vfpclassss \$0x7b,-0x204\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 1a f4 ab[ 	]*vinsertf32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 7b[ 	]*vinsertf32x8 \$0x7b,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 31 7b[ 	]*vinsertf32x8 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b4 f4 c0 1d fe ff 7b[ 	]*vinsertf32x8 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 7f 7b[ 	]*vinsertf32x8 \$0x7b,0xfe0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 \$0x7b,0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 80 7b[ 	]*vinsertf32x8 \$0x7b,-0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 \$0x7b,-0x1020\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 3a f4 ab[ 	]*vinserti32x8 \$0xab,%ymm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 7b[ 	]*vinserti32x8 \$0x7b,%ymm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 31 7b[ 	]*vinserti32x8 \$0x7b,\(%ecx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b4 f4 c0 1d fe ff 7b[ 	]*vinserti32x8 \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 7f 7b[ 	]*vinserti32x8 \$0x7b,0xfe0\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 \$0x7b,0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 80 7b[ 	]*vinserti32x8 \$0x7b,-0x1000\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 \$0x7b,-0x1020\(%edx\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 f7[ 	]*vbroadcasti32x2 %xmm7,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 59 f7[ 	]*vbroadcasti32x2 %xmm7,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 31[ 	]*vbroadcasti32x2 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 f4[ 	]*vpmullq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 40 f4[ 	]*vpmullq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 40 f4[ 	]*vpmullq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 31[ 	]*vpmullq \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b4 f4 c0 1d fe ff[ 	]*vpmullq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 30[ 	]*vpmullq \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 7f[ 	]*vpmullq 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 00 20 00 00[ 	]*vpmullq 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 80[ 	]*vpmullq -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 c0 df ff ff[ 	]*vpmullq -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 7f[ 	]*vpmullq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 80[ 	]*vpmullq -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 ab[ 	]*vrangepd \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 50 f4 ab[ 	]*vrangepd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 50 f4 ab[ 	]*vrangepd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 7b[ 	]*vrangepd \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 31 7b[ 	]*vrangepd \$0x7b,\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 30 7b[ 	]*vrangepd \$0x7b,\(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 00 20 00 00 7b[ 	]*vrangepd \$0x7b,0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 c0 df ff ff 7b[ 	]*vrangepd \$0x7b,-0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 ab[ 	]*vrangeps \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 50 f4 ab[ 	]*vrangeps \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 50 f4 ab[ 	]*vrangeps \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 7b[ 	]*vrangeps \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 31 7b[ 	]*vrangeps \$0x7b,\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 30 7b[ 	]*vrangeps \$0x7b,\(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 00 20 00 00 7b[ 	]*vrangeps \$0x7b,0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 c0 df ff ff 7b[ 	]*vrangeps \$0x7b,-0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 ab[ 	]*vrangesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 51 f4 ab[ 	]*vrangesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 7b[ 	]*vrangesd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 31 7b[ 	]*vrangesd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangesd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 7f 7b[ 	]*vrangesd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 00 04 00 00 7b[ 	]*vrangesd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 80 7b[ 	]*vrangesd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 f8 fb ff ff 7b[ 	]*vrangesd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 ab[ 	]*vrangess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 51 f4 ab[ 	]*vrangess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 7b[ 	]*vrangess \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 31 7b[ 	]*vrangess \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangess \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 7f 7b[ 	]*vrangess \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 00 02 00 00 7b[ 	]*vrangess \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 80 7b[ 	]*vrangess \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 fc fd ff ff 7b[ 	]*vrangess \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 f4[ 	]*vandpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 54 f4[ 	]*vandpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 54 f4[ 	]*vandpd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 31[ 	]*vandpd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b4 f4 c0 1d fe ff[ 	]*vandpd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 30[ 	]*vandpd \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 7f[ 	]*vandpd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 00 20 00 00[ 	]*vandpd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 80[ 	]*vandpd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 c0 df ff ff[ 	]*vandpd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 7f[ 	]*vandpd 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 80[ 	]*vandpd -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 f4[ 	]*vandps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 54 f4[ 	]*vandps %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 54 f4[ 	]*vandps %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 31[ 	]*vandps \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b4 f4 c0 1d fe ff[ 	]*vandps -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 30[ 	]*vandps \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 7f[ 	]*vandps 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 00 20 00 00[ 	]*vandps 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 80[ 	]*vandps -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 c0 df ff ff[ 	]*vandps -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 7f[ 	]*vandps 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 00 02 00 00[ 	]*vandps 0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 80[ 	]*vandps -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 f4[ 	]*vandnpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 55 f4[ 	]*vandnpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 55 f4[ 	]*vandnpd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 31[ 	]*vandnpd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b4 f4 c0 1d fe ff[ 	]*vandnpd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 30[ 	]*vandnpd \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 7f[ 	]*vandnpd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 00 20 00 00[ 	]*vandnpd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 80[ 	]*vandnpd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 c0 df ff ff[ 	]*vandnpd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 7f[ 	]*vandnpd 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 80[ 	]*vandnpd -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 f4[ 	]*vandnps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 55 f4[ 	]*vandnps %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 55 f4[ 	]*vandnps %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 31[ 	]*vandnps \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b4 f4 c0 1d fe ff[ 	]*vandnps -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 30[ 	]*vandnps \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 7f[ 	]*vandnps 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 00 20 00 00[ 	]*vandnps 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 80[ 	]*vandnps -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 c0 df ff ff[ 	]*vandnps -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 7f[ 	]*vandnps 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 80[ 	]*vandnps -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 f4[ 	]*vorpd  %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 56 f4[ 	]*vorpd  %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 56 f4[ 	]*vorpd  %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 31[ 	]*vorpd  \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b4 f4 c0 1d fe ff[ 	]*vorpd  -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 30[ 	]*vorpd  \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 7f[ 	]*vorpd  0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 00 20 00 00[ 	]*vorpd  0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 80[ 	]*vorpd  -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 c0 df ff ff[ 	]*vorpd  -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 7f[ 	]*vorpd  0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 80[ 	]*vorpd  -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 f4[ 	]*vorps  %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 56 f4[ 	]*vorps  %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 56 f4[ 	]*vorps  %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 31[ 	]*vorps  \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b4 f4 c0 1d fe ff[ 	]*vorps  -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 30[ 	]*vorps  \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 7f[ 	]*vorps  0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 00 20 00 00[ 	]*vorps  0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 80[ 	]*vorps  -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 c0 df ff ff[ 	]*vorps  -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 7f[ 	]*vorps  0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 00 02 00 00[ 	]*vorps  0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 80[ 	]*vorps  -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 f4[ 	]*vxorpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 57 f4[ 	]*vxorpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 57 f4[ 	]*vxorpd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 31[ 	]*vxorpd \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b4 f4 c0 1d fe ff[ 	]*vxorpd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 30[ 	]*vxorpd \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 7f[ 	]*vxorpd 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 00 20 00 00[ 	]*vxorpd 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 80[ 	]*vxorpd -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 c0 df ff ff[ 	]*vxorpd -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 7f[ 	]*vxorpd 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 80[ 	]*vxorpd -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 f4[ 	]*vxorps %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 57 f4[ 	]*vxorps %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 57 f4[ 	]*vxorps %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 31[ 	]*vxorps \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b4 f4 c0 1d fe ff[ 	]*vxorps -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 30[ 	]*vxorps \(%eax\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 7f[ 	]*vxorps 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 00 20 00 00[ 	]*vxorps 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 80[ 	]*vxorps -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 c0 df ff ff[ 	]*vxorps -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 7f[ 	]*vxorps 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 80[ 	]*vxorps -0x200\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 ab[ 	]*vreducepd \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 56 f5 ab[ 	]*vreducepd \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 56 f5 ab[ 	]*vreducepd \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 7b[ 	]*vreducepd \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 31 7b[ 	]*vreducepd \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 30 7b[ 	]*vreducepd \$0x7b,\(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 ab[ 	]*vreduceps \$0xab,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 56 f5 ab[ 	]*vreduceps \$0xab,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 56 f5 ab[ 	]*vreduceps \$0xab,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 7b[ 	]*vreduceps \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 31 7b[ 	]*vreduceps \$0x7b,\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 30 7b[ 	]*vreduceps \$0x7b,\(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps \$0x7b,0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps \$0x7b,-0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 ab[ 	]*vreducesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 57 f4 ab[ 	]*vreducesd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 7b[ 	]*vreducesd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 31 7b[ 	]*vreducesd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducesd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 7f 7b[ 	]*vreducesd \$0x7b,0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 00 04 00 00 7b[ 	]*vreducesd \$0x7b,0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 80 7b[ 	]*vreducesd \$0x7b,-0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 f8 fb ff ff 7b[ 	]*vreducesd \$0x7b,-0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 ab[ 	]*vreducess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 57 f4 ab[ 	]*vreducess \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 7b[ 	]*vreducess \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 31 7b[ 	]*vreducess \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducess \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 7f 7b[ 	]*vreducess \$0x7b,0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 00 02 00 00 7b[ 	]*vreducess \$0x7b,0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 80 7b[ 	]*vreducess \$0x7b,-0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 fc fd ff ff 7b[ 	]*vreducess \$0x7b,-0x204\(%edx\),%xmm5,%xmm6\{%k7\}
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
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  \(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ac f4 c0 1d fe ff[ 	]*kmovb  -0x1e240\(%esp,%esi,8\),%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  %k5,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 ac f4 c0 1d fe ff[ 	]*kmovb  %k5,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  %eax,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  %ebp,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  %k5,%eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  %k5,%ebp
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  %k7,%k6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 31 ab[ 	]*vextractf64x2 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b4 f4 c0 1d fe ff 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 7f 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,0x7f0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 80 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,-0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%zmm6,-0x810\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b 31 ab[ 	]*vextractf32x8 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b4 f4 c0 1d fe ff 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,0xfe0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 80 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,-0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 \$0x7b,%zmm6,-0x1020\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 31 ab[ 	]*vextracti64x2 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b4 f4 c0 1d fe ff 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 7f 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,0x7f0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 80 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,-0x800\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%zmm6,-0x810\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b 31 ab[ 	]*vextracti32x8 \$0xab,%zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b4 f4 c0 1d fe ff 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,0xfe0\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 80 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,-0x1000\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 \$0x7b,%zmm6,-0x1020\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a f5[ 	]*vcvttpd2qq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7a f5[ 	]*vcvttpd2qq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7a f5[ 	]*vcvttpd2qq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 31[ 	]*vcvttpd2qq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 30[ 	]*vcvttpd2qq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 7f[ 	]*vcvttpd2qq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 80[ 	]*vcvttpd2qq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 80[ 	]*vcvttpd2qq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 f5[ 	]*vcvttpd2uqq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 78 f5[ 	]*vcvttpd2uqq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 78 f5[ 	]*vcvttpd2uqq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 31[ 	]*vcvttpd2uqq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 30[ 	]*vcvttpd2uqq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 7f[ 	]*vcvttpd2uqq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 80[ 	]*vcvttpd2uqq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 80[ 	]*vcvttpd2uqq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a f5[ 	]*vcvttps2qq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7a f5[ 	]*vcvttps2qq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a f5[ 	]*vcvttps2qq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 31[ 	]*vcvttps2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 30[ 	]*vcvttps2qq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 7f[ 	]*vcvttps2qq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 00 10 00 00[ 	]*vcvttps2qq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 80[ 	]*vcvttps2qq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 e0 ef ff ff[ 	]*vcvttps2qq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 80[ 	]*vcvttps2qq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 f5[ 	]*vcvttps2uqq %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 78 f5[ 	]*vcvttps2uqq %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 31[ 	]*vcvttps2uqq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 30[ 	]*vcvttps2uqq \(%eax\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 7f[ 	]*vcvttps2uqq 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 00 10 00 00[ 	]*vcvttps2uqq 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 80[ 	]*vcvttps2uqq -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 80[ 	]*vcvttps2uqq -0x200\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to8\},%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 39 ee[ 	]*vpmovd2m %zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 39 ee[ 	]*vpmovq2m %zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 38 f5[ 	]*vpmovm2d %k5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 38 f5[ 	]*vpmovm2q %k5,%zmm6
#pass
