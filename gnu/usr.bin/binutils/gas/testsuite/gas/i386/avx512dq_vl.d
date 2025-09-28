#as:
#objdump: -dw
#name: i386 AVX512DQ/VL insns
#source: avx512dq_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 f7[ 	]*vbroadcastf32x2 %xmm7,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 19 f7[ 	]*vbroadcastf32x2 %xmm7,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 31[ 	]*vbroadcastf32x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b f5[ 	]*vcvtpd2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7b f5[ 	]*vcvtpd2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 31[ 	]*vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 30[ 	]*vcvtpd2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 7f[ 	]*vcvtpd2qq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 00 08 00 00[ 	]*vcvtpd2qq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 80[ 	]*vcvtpd2qq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 80[ 	]*vcvtpd2qq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b f5[ 	]*vcvtpd2qq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7b f5[ 	]*vcvtpd2qq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 31[ 	]*vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 30[ 	]*vcvtpd2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 7f[ 	]*vcvtpd2qq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 00 10 00 00[ 	]*vcvtpd2qq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 80[ 	]*vcvtpd2qq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 80[ 	]*vcvtpd2qq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 f5[ 	]*vcvtpd2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 79 f5[ 	]*vcvtpd2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 31[ 	]*vcvtpd2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 30[ 	]*vcvtpd2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 7f[ 	]*vcvtpd2uqq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 00 08 00 00[ 	]*vcvtpd2uqq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 80[ 	]*vcvtpd2uqq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 80[ 	]*vcvtpd2uqq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 f5[ 	]*vcvtpd2uqq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 79 f5[ 	]*vcvtpd2uqq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 31[ 	]*vcvtpd2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 30[ 	]*vcvtpd2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 7f[ 	]*vcvtpd2uqq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 00 10 00 00[ 	]*vcvtpd2uqq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 80[ 	]*vcvtpd2uqq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 80[ 	]*vcvtpd2uqq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b f5[ 	]*vcvtps2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7b f5[ 	]*vcvtps2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 31[ 	]*vcvtps2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 30[ 	]*vcvtps2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 7f[ 	]*vcvtps2qq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 00 04 00 00[ 	]*vcvtps2qq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 80[ 	]*vcvtps2qq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 f8 fb ff ff[ 	]*vcvtps2qq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 80[ 	]*vcvtps2qq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b f5[ 	]*vcvtps2qq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7b f5[ 	]*vcvtps2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 31[ 	]*vcvtps2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 30[ 	]*vcvtps2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 7f[ 	]*vcvtps2qq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 00 08 00 00[ 	]*vcvtps2qq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 80[ 	]*vcvtps2qq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 80[ 	]*vcvtps2qq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 f5[ 	]*vcvtps2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 79 f5[ 	]*vcvtps2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 31[ 	]*vcvtps2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 30[ 	]*vcvtps2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 7f[ 	]*vcvtps2uqq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 00 04 00 00[ 	]*vcvtps2uqq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 80[ 	]*vcvtps2uqq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 80[ 	]*vcvtps2uqq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 f5[ 	]*vcvtps2uqq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 79 f5[ 	]*vcvtps2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 31[ 	]*vcvtps2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 30[ 	]*vcvtps2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 7f[ 	]*vcvtps2uqq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 00 08 00 00[ 	]*vcvtps2uqq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 80[ 	]*vcvtps2uqq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 80[ 	]*vcvtps2uqq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 f5[ 	]*vcvtqq2pd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f e6 f5[ 	]*vcvtqq2pd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 31[ 	]*vcvtqq2pd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 30[ 	]*vcvtqq2pd \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 7f[ 	]*vcvtqq2pd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 00 08 00 00[ 	]*vcvtqq2pd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 80[ 	]*vcvtqq2pd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 80[ 	]*vcvtqq2pd -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 f5[ 	]*vcvtqq2pd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af e6 f5[ 	]*vcvtqq2pd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 31[ 	]*vcvtqq2pd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 30[ 	]*vcvtqq2pd \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 7f[ 	]*vcvtqq2pd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 00 10 00 00[ 	]*vcvtqq2pd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 80[ 	]*vcvtqq2pd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 80[ 	]*vcvtqq2pd -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b f5[ 	]*vcvtqq2ps %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 8f 5b f5[ 	]*vcvtqq2ps %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 31[ 	]*vcvtqq2psx \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2psx -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 30[ 	]*vcvtqq2ps \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 7f[ 	]*vcvtqq2psx 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 00 08 00 00[ 	]*vcvtqq2psx 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 80[ 	]*vcvtqq2psx -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 f0 f7 ff ff[ 	]*vcvtqq2psx -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 80[ 	]*vcvtqq2ps -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b f5[ 	]*vcvtqq2ps %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc af 5b f5[ 	]*vcvtqq2ps %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 31[ 	]*vcvtqq2psy \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2psy -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 30[ 	]*vcvtqq2ps \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 7f[ 	]*vcvtqq2psy 0xfe0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 00 10 00 00[ 	]*vcvtqq2psy 0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 80[ 	]*vcvtqq2psy -0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 e0 ef ff ff[ 	]*vcvtqq2psy -0x1020\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 80[ 	]*vcvtqq2ps -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a f5[ 	]*vcvtuqq2pd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f 7a f5[ 	]*vcvtuqq2pd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 31[ 	]*vcvtuqq2pd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 30[ 	]*vcvtuqq2pd \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 7f[ 	]*vcvtuqq2pd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2pd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 80[ 	]*vcvtuqq2pd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a f5[ 	]*vcvtuqq2pd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af 7a f5[ 	]*vcvtuqq2pd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 31[ 	]*vcvtuqq2pd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 30[ 	]*vcvtuqq2pd \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 7f[ 	]*vcvtuqq2pd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2pd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 80[ 	]*vcvtuqq2pd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a f5[ 	]*vcvtuqq2ps %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7a f5[ 	]*vcvtuqq2ps %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 31[ 	]*vcvtuqq2psx \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2psx -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 30[ 	]*vcvtuqq2ps \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 7f[ 	]*vcvtuqq2psx 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2psx 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 80[ 	]*vcvtuqq2psx -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2psx -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a f5[ 	]*vcvtuqq2ps %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7a f5[ 	]*vcvtuqq2ps %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 31[ 	]*vcvtuqq2psy \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2psy -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 30[ 	]*vcvtuqq2ps \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 7f[ 	]*vcvtuqq2psy 0xfe0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2psy 0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 80[ 	]*vcvtuqq2psy -0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2psy -0x1020\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee ab[ 	]*vfpclasspd \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee 7b[ 	]*vfpclasspd \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 29 7b[ 	]*vfpclasspdx \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspdx \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 28 7b[ 	]*vfpclasspd \$0x7b,\(%eax\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 7f 7b[ 	]*vfpclasspdx \$0x7b,0x7f0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa 00 08 00 00 7b[ 	]*vfpclasspdx \$0x7b,0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 80 7b[ 	]*vfpclasspdx \$0x7b,-0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspdx \$0x7b,-0x810\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee ab[ 	]*vfpclasspd \$0xab,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee 7b[ 	]*vfpclasspd \$0x7b,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 29 7b[ 	]*vfpclasspdy \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspdy \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 28 7b[ 	]*vfpclasspd \$0x7b,\(%eax\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 7f 7b[ 	]*vfpclasspdy \$0x7b,0xfe0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa 00 10 00 00 7b[ 	]*vfpclasspdy \$0x7b,0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 80 7b[ 	]*vfpclasspdy \$0x7b,-0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclasspdy \$0x7b,-0x1020\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee ab[ 	]*vfpclassps \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee 7b[ 	]*vfpclassps \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 29 7b[ 	]*vfpclasspsx \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspsx \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 28 7b[ 	]*vfpclassps \$0x7b,\(%eax\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 7f 7b[ 	]*vfpclasspsx \$0x7b,0x7f0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa 00 08 00 00 7b[ 	]*vfpclasspsx \$0x7b,0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 80 7b[ 	]*vfpclasspsx \$0x7b,-0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspsx \$0x7b,-0x810\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee ab[ 	]*vfpclassps \$0xab,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee 7b[ 	]*vfpclassps \$0x7b,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 29 7b[ 	]*vfpclasspsy \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspsy \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 28 7b[ 	]*vfpclassps \$0x7b,\(%eax\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 7f 7b[ 	]*vfpclasspsy \$0x7b,0xfe0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa 00 10 00 00 7b[ 	]*vfpclasspsy \$0x7b,0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 80 7b[ 	]*vfpclasspsy \$0x7b,-0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclasspsy \$0x7b,-0x1020\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 31[ 	]*vbroadcasti32x2 \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 59 f7[ 	]*vbroadcasti32x2 %xmm7,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 31[ 	]*vbroadcasti32x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 f4[ 	]*vpmullq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 40 f4[ 	]*vpmullq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 31[ 	]*vpmullq \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 30[ 	]*vpmullq \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 7f[ 	]*vpmullq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 00 08 00 00[ 	]*vpmullq 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 80[ 	]*vpmullq -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 f0 f7 ff ff[ 	]*vpmullq -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 7f[ 	]*vpmullq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 80[ 	]*vpmullq -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 f4[ 	]*vpmullq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 40 f4[ 	]*vpmullq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 31[ 	]*vpmullq \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 30[ 	]*vpmullq \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 7f[ 	]*vpmullq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 00 10 00 00[ 	]*vpmullq 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 80[ 	]*vpmullq -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 e0 ef ff ff[ 	]*vpmullq -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 7f[ 	]*vpmullq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 80[ 	]*vpmullq -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 ab[ 	]*vrangepd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 50 f4 ab[ 	]*vrangepd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 7b[ 	]*vrangepd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 31 7b[ 	]*vrangepd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 30 7b[ 	]*vrangepd \$0x7b,\(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 00 08 00 00 7b[ 	]*vrangepd \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 ab[ 	]*vrangepd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 50 f4 ab[ 	]*vrangepd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 7b[ 	]*vrangepd \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 31 7b[ 	]*vrangepd \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 30 7b[ 	]*vrangepd \$0x7b,\(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 00 10 00 00 7b[ 	]*vrangepd \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangepd \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 ab[ 	]*vrangeps \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 50 f4 ab[ 	]*vrangeps \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 7b[ 	]*vrangeps \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 31 7b[ 	]*vrangeps \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 30 7b[ 	]*vrangeps \$0x7b,\(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 00 08 00 00 7b[ 	]*vrangeps \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 ab[ 	]*vrangeps \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 50 f4 ab[ 	]*vrangeps \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 7b[ 	]*vrangeps \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 31 7b[ 	]*vrangeps \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 30 7b[ 	]*vrangeps \$0x7b,\(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 00 10 00 00 7b[ 	]*vrangeps \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangeps \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 f4[ 	]*vandpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 54 f4[ 	]*vandpd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 31[ 	]*vandpd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b4 f4 c0 1d fe ff[ 	]*vandpd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 30[ 	]*vandpd \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 7f[ 	]*vandpd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 00 08 00 00[ 	]*vandpd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 80[ 	]*vandpd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 f0 f7 ff ff[ 	]*vandpd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 7f[ 	]*vandpd 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 80[ 	]*vandpd -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 f4[ 	]*vandpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 54 f4[ 	]*vandpd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 31[ 	]*vandpd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b4 f4 c0 1d fe ff[ 	]*vandpd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 30[ 	]*vandpd \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 7f[ 	]*vandpd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 00 10 00 00[ 	]*vandpd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 80[ 	]*vandpd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 e0 ef ff ff[ 	]*vandpd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 7f[ 	]*vandpd 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 80[ 	]*vandpd -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 f4[ 	]*vandps %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 54 f4[ 	]*vandps %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 31[ 	]*vandps \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b4 f4 c0 1d fe ff[ 	]*vandps -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 30[ 	]*vandps \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 7f[ 	]*vandps 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 00 08 00 00[ 	]*vandps 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 80[ 	]*vandps -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 f0 f7 ff ff[ 	]*vandps -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 7f[ 	]*vandps 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 00 02 00 00[ 	]*vandps 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 80[ 	]*vandps -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 f4[ 	]*vandps %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 54 f4[ 	]*vandps %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 31[ 	]*vandps \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b4 f4 c0 1d fe ff[ 	]*vandps -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 30[ 	]*vandps \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 7f[ 	]*vandps 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 00 10 00 00[ 	]*vandps 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 80[ 	]*vandps -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 e0 ef ff ff[ 	]*vandps -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 7f[ 	]*vandps 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 00 02 00 00[ 	]*vandps 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 80[ 	]*vandps -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 f4[ 	]*vandnpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 55 f4[ 	]*vandnpd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 31[ 	]*vandnpd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 30[ 	]*vandnpd \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 7f[ 	]*vandnpd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 00 08 00 00[ 	]*vandnpd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 80[ 	]*vandnpd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 f0 f7 ff ff[ 	]*vandnpd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 7f[ 	]*vandnpd 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 80[ 	]*vandnpd -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 f4[ 	]*vandnpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 55 f4[ 	]*vandnpd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 31[ 	]*vandnpd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 30[ 	]*vandnpd \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 7f[ 	]*vandnpd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 00 10 00 00[ 	]*vandnpd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 80[ 	]*vandnpd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 e0 ef ff ff[ 	]*vandnpd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 7f[ 	]*vandnpd 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 80[ 	]*vandnpd -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 f4[ 	]*vandnps %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 55 f4[ 	]*vandnps %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 31[ 	]*vandnps \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnps -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 30[ 	]*vandnps \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 7f[ 	]*vandnps 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 00 08 00 00[ 	]*vandnps 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 80[ 	]*vandnps -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 f0 f7 ff ff[ 	]*vandnps -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 7f[ 	]*vandnps 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 80[ 	]*vandnps -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 f4[ 	]*vandnps %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 55 f4[ 	]*vandnps %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 31[ 	]*vandnps \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnps -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 30[ 	]*vandnps \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 7f[ 	]*vandnps 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 00 10 00 00[ 	]*vandnps 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 80[ 	]*vandnps -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 e0 ef ff ff[ 	]*vandnps -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 7f[ 	]*vandnps 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 80[ 	]*vandnps -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 f4[ 	]*vorpd  %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 56 f4[ 	]*vorpd  %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 31[ 	]*vorpd  \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 30[ 	]*vorpd  \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 7f[ 	]*vorpd  0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 00 08 00 00[ 	]*vorpd  0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 80[ 	]*vorpd  -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 f0 f7 ff ff[ 	]*vorpd  -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 7f[ 	]*vorpd  0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 80[ 	]*vorpd  -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 f4[ 	]*vorpd  %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 56 f4[ 	]*vorpd  %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 31[ 	]*vorpd  \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 30[ 	]*vorpd  \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 7f[ 	]*vorpd  0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 00 10 00 00[ 	]*vorpd  0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 80[ 	]*vorpd  -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 e0 ef ff ff[ 	]*vorpd  -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 7f[ 	]*vorpd  0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 80[ 	]*vorpd  -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 f4[ 	]*vorps  %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 56 f4[ 	]*vorps  %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 31[ 	]*vorps  \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b4 f4 c0 1d fe ff[ 	]*vorps  -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 30[ 	]*vorps  \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 7f[ 	]*vorps  0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 00 08 00 00[ 	]*vorps  0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 80[ 	]*vorps  -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 f0 f7 ff ff[ 	]*vorps  -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 7f[ 	]*vorps  0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 00 02 00 00[ 	]*vorps  0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 80[ 	]*vorps  -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 f4[ 	]*vorps  %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 56 f4[ 	]*vorps  %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 31[ 	]*vorps  \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b4 f4 c0 1d fe ff[ 	]*vorps  -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 30[ 	]*vorps  \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 7f[ 	]*vorps  0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 00 10 00 00[ 	]*vorps  0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 80[ 	]*vorps  -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 e0 ef ff ff[ 	]*vorps  -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 7f[ 	]*vorps  0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 00 02 00 00[ 	]*vorps  0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 80[ 	]*vorps  -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 f4[ 	]*vxorpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 57 f4[ 	]*vxorpd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 31[ 	]*vxorpd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 30[ 	]*vxorpd \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 7f[ 	]*vxorpd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 00 08 00 00[ 	]*vxorpd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 80[ 	]*vxorpd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 f0 f7 ff ff[ 	]*vxorpd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 7f[ 	]*vxorpd 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 80[ 	]*vxorpd -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 f4[ 	]*vxorpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 57 f4[ 	]*vxorpd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 31[ 	]*vxorpd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 30[ 	]*vxorpd \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 7f[ 	]*vxorpd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 00 10 00 00[ 	]*vxorpd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 80[ 	]*vxorpd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 e0 ef ff ff[ 	]*vxorpd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 7f[ 	]*vxorpd 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 80[ 	]*vxorpd -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 f4[ 	]*vxorps %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 57 f4[ 	]*vxorps %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 31[ 	]*vxorps \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorps -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 30[ 	]*vxorps \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 7f[ 	]*vxorps 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 00 08 00 00[ 	]*vxorps 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 80[ 	]*vxorps -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 f0 f7 ff ff[ 	]*vxorps -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 7f[ 	]*vxorps 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 80[ 	]*vxorps -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 f4[ 	]*vxorps %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 57 f4[ 	]*vxorps %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 31[ 	]*vxorps \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorps -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 30[ 	]*vxorps \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 7f[ 	]*vxorps 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 00 10 00 00[ 	]*vxorps 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 80[ 	]*vxorps -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 e0 ef ff ff[ 	]*vxorps -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 7f[ 	]*vxorps 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 80[ 	]*vxorps -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 8f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 7b[ 	]*vreducepd \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 31 7b[ 	]*vreducepd \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 30 7b[ 	]*vreducepd \$0x7b,\(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 00 08 00 00 7b[ 	]*vreducepd \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 ab[ 	]*vreducepd \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 56 f5 ab[ 	]*vreducepd \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 7b[ 	]*vreducepd \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 31 7b[ 	]*vreducepd \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 30 7b[ 	]*vreducepd \$0x7b,\(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 00 10 00 00 7b[ 	]*vreducepd \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 e0 ef ff ff 7b[ 	]*vreducepd \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 8f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 7b[ 	]*vreduceps \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 31 7b[ 	]*vreduceps \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 30 7b[ 	]*vreduceps \$0x7b,\(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 00 08 00 00 7b[ 	]*vreduceps \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 ab[ 	]*vreduceps \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d af 56 f5 ab[ 	]*vreduceps \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 7b[ 	]*vreduceps \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 31 7b[ 	]*vreduceps \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 30 7b[ 	]*vreduceps \$0x7b,\(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 00 10 00 00 7b[ 	]*vreduceps \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 e0 ef ff ff 7b[ 	]*vreduceps \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 ab[ 	]*vextractf64x2 \$0xab,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ac f4 c0 1d fe ff 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 7f 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 80 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 ab[ 	]*vextracti64x2 \$0xab,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ac f4 c0 1d fe ff 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 7f 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 80 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a f5[ 	]*vcvttpd2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7a f5[ 	]*vcvttpd2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 31[ 	]*vcvttpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 30[ 	]*vcvttpd2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 7f[ 	]*vcvttpd2qq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 00 08 00 00[ 	]*vcvttpd2qq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 80[ 	]*vcvttpd2qq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 80[ 	]*vcvttpd2qq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a f5[ 	]*vcvttpd2qq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7a f5[ 	]*vcvttpd2qq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 31[ 	]*vcvttpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 30[ 	]*vcvttpd2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 7f[ 	]*vcvttpd2qq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 00 10 00 00[ 	]*vcvttpd2qq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 80[ 	]*vcvttpd2qq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 80[ 	]*vcvttpd2qq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 f5[ 	]*vcvttpd2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 78 f5[ 	]*vcvttpd2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 31[ 	]*vcvttpd2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 30[ 	]*vcvttpd2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 7f[ 	]*vcvttpd2uqq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 00 08 00 00[ 	]*vcvttpd2uqq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 80[ 	]*vcvttpd2uqq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 80[ 	]*vcvttpd2uqq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 f5[ 	]*vcvttpd2uqq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 78 f5[ 	]*vcvttpd2uqq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 31[ 	]*vcvttpd2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 30[ 	]*vcvttpd2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 7f[ 	]*vcvttpd2uqq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 00 10 00 00[ 	]*vcvttpd2uqq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 80[ 	]*vcvttpd2uqq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 80[ 	]*vcvttpd2uqq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a f5[ 	]*vcvttps2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7a f5[ 	]*vcvttps2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 31[ 	]*vcvttps2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 30[ 	]*vcvttps2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 7f[ 	]*vcvttps2qq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 00 04 00 00[ 	]*vcvttps2qq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 80[ 	]*vcvttps2qq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 f8 fb ff ff[ 	]*vcvttps2qq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 80[ 	]*vcvttps2qq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a f5[ 	]*vcvttps2qq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7a f5[ 	]*vcvttps2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 31[ 	]*vcvttps2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 30[ 	]*vcvttps2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 7f[ 	]*vcvttps2qq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 00 08 00 00[ 	]*vcvttps2qq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 80[ 	]*vcvttps2qq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 80[ 	]*vcvttps2qq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 f5[ 	]*vcvttps2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 78 f5[ 	]*vcvttps2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 31[ 	]*vcvttps2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 30[ 	]*vcvttps2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 7f[ 	]*vcvttps2uqq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 00 04 00 00[ 	]*vcvttps2uqq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 80[ 	]*vcvttps2uqq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 80[ 	]*vcvttps2uqq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 f5[ 	]*vcvttps2uqq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 78 f5[ 	]*vcvttps2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 31[ 	]*vcvttps2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 30[ 	]*vcvttps2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 7f[ 	]*vcvttps2uqq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 00 08 00 00[ 	]*vcvttps2uqq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 80[ 	]*vcvttps2uqq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 80[ 	]*vcvttps2uqq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 39 ee[ 	]*vpmovd2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 39 ee[ 	]*vpmovd2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 39 ee[ 	]*vpmovq2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 39 ee[ 	]*vpmovq2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 38 f5[ 	]*vpmovm2d %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 38 f5[ 	]*vpmovm2d %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 38 f5[ 	]*vpmovm2q %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 38 f5[ 	]*vpmovm2q %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1a 31[ 	]*vbroadcastf64x2 \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 7f[ 	]*vbroadcastf64x2 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 80[ 	]*vbroadcastf64x2 -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 5a 31[ 	]*vbroadcasti64x2 \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 7f[ 	]*vbroadcasti64x2 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 80[ 	]*vbroadcasti64x2 -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 f7[ 	]*vbroadcastf32x2 %xmm7,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 19 f7[ 	]*vbroadcastf32x2 %xmm7,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 31[ 	]*vbroadcastf32x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 7f[ 	]*vbroadcastf32x2 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 80[ 	]*vbroadcastf32x2 -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b f5[ 	]*vcvtpd2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7b f5[ 	]*vcvtpd2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 31[ 	]*vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 30[ 	]*vcvtpd2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 7f[ 	]*vcvtpd2qq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 00 08 00 00[ 	]*vcvtpd2qq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 80[ 	]*vcvtpd2qq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 80[ 	]*vcvtpd2qq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b f5[ 	]*vcvtpd2qq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7b f5[ 	]*vcvtpd2qq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 31[ 	]*vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 30[ 	]*vcvtpd2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 7f[ 	]*vcvtpd2qq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 00 10 00 00[ 	]*vcvtpd2qq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 80[ 	]*vcvtpd2qq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 7f[ 	]*vcvtpd2qq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 00 04 00 00[ 	]*vcvtpd2qq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 80[ 	]*vcvtpd2qq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 f5[ 	]*vcvtpd2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 79 f5[ 	]*vcvtpd2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 31[ 	]*vcvtpd2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 30[ 	]*vcvtpd2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 7f[ 	]*vcvtpd2uqq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 00 08 00 00[ 	]*vcvtpd2uqq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 80[ 	]*vcvtpd2uqq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 80[ 	]*vcvtpd2uqq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 f5[ 	]*vcvtpd2uqq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 79 f5[ 	]*vcvtpd2uqq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 31[ 	]*vcvtpd2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 30[ 	]*vcvtpd2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 7f[ 	]*vcvtpd2uqq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 00 10 00 00[ 	]*vcvtpd2uqq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 80[ 	]*vcvtpd2uqq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 7f[ 	]*vcvtpd2uqq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 80[ 	]*vcvtpd2uqq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b f5[ 	]*vcvtps2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7b f5[ 	]*vcvtps2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 31[ 	]*vcvtps2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 30[ 	]*vcvtps2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 7f[ 	]*vcvtps2qq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 00 04 00 00[ 	]*vcvtps2qq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 80[ 	]*vcvtps2qq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 f8 fb ff ff[ 	]*vcvtps2qq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 80[ 	]*vcvtps2qq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b f5[ 	]*vcvtps2qq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7b f5[ 	]*vcvtps2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 31[ 	]*vcvtps2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 30[ 	]*vcvtps2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 7f[ 	]*vcvtps2qq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 00 08 00 00[ 	]*vcvtps2qq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 80[ 	]*vcvtps2qq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 00 02 00 00[ 	]*vcvtps2qq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 80[ 	]*vcvtps2qq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 fc fd ff ff[ 	]*vcvtps2qq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 7f[ 	]*vcvtps2qq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 f5[ 	]*vcvtps2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 79 f5[ 	]*vcvtps2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 31[ 	]*vcvtps2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 30[ 	]*vcvtps2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 7f[ 	]*vcvtps2uqq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 00 04 00 00[ 	]*vcvtps2uqq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 80[ 	]*vcvtps2uqq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 80[ 	]*vcvtps2uqq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 f5[ 	]*vcvtps2uqq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 79 f5[ 	]*vcvtps2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 31[ 	]*vcvtps2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 30[ 	]*vcvtps2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 7f[ 	]*vcvtps2uqq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 00 08 00 00[ 	]*vcvtps2uqq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 80[ 	]*vcvtps2uqq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 00 02 00 00[ 	]*vcvtps2uqq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 80[ 	]*vcvtps2uqq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 7f[ 	]*vcvtps2uqq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 f5[ 	]*vcvtqq2pd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f e6 f5[ 	]*vcvtqq2pd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 31[ 	]*vcvtqq2pd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 30[ 	]*vcvtqq2pd \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 7f[ 	]*vcvtqq2pd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 00 08 00 00[ 	]*vcvtqq2pd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 80[ 	]*vcvtqq2pd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 80[ 	]*vcvtqq2pd -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 f5[ 	]*vcvtqq2pd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af e6 f5[ 	]*vcvtqq2pd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 31[ 	]*vcvtqq2pd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 30[ 	]*vcvtqq2pd \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 7f[ 	]*vcvtqq2pd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 00 10 00 00[ 	]*vcvtqq2pd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 80[ 	]*vcvtqq2pd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 7f[ 	]*vcvtqq2pd 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 00 04 00 00[ 	]*vcvtqq2pd 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 80[ 	]*vcvtqq2pd -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b f5[ 	]*vcvtqq2ps %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 8f 5b f5[ 	]*vcvtqq2ps %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 31[ 	]*vcvtqq2psx \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2psx -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 30[ 	]*vcvtqq2ps \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 7f[ 	]*vcvtqq2psx 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 00 08 00 00[ 	]*vcvtqq2psx 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 80[ 	]*vcvtqq2psx -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 f0 f7 ff ff[ 	]*vcvtqq2psx -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 80[ 	]*vcvtqq2ps -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b f5[ 	]*vcvtqq2ps %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc af 5b f5[ 	]*vcvtqq2ps %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 31[ 	]*vcvtqq2psy \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2psy -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 30[ 	]*vcvtqq2ps \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 7f[ 	]*vcvtqq2psy 0xfe0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 00 10 00 00[ 	]*vcvtqq2psy 0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 80[ 	]*vcvtqq2psy -0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 e0 ef ff ff[ 	]*vcvtqq2psy -0x1020\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 7f[ 	]*vcvtqq2ps 0x3f8\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 00 04 00 00[ 	]*vcvtqq2ps 0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 80[ 	]*vcvtqq2ps -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps -0x408\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a f5[ 	]*vcvtuqq2pd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f 7a f5[ 	]*vcvtuqq2pd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 31[ 	]*vcvtuqq2pd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 30[ 	]*vcvtuqq2pd \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 7f[ 	]*vcvtuqq2pd 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2pd 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 80[ 	]*vcvtuqq2pd -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a f5[ 	]*vcvtuqq2pd %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af 7a f5[ 	]*vcvtuqq2pd %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 31[ 	]*vcvtuqq2pd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 30[ 	]*vcvtuqq2pd \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 7f[ 	]*vcvtuqq2pd 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2pd 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 80[ 	]*vcvtuqq2pd -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 7f[ 	]*vcvtuqq2pd 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 80[ 	]*vcvtuqq2pd -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a f5[ 	]*vcvtuqq2ps %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7a f5[ 	]*vcvtuqq2ps %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 31[ 	]*vcvtuqq2psx \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2psx -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 30[ 	]*vcvtuqq2ps \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 7f[ 	]*vcvtuqq2psx 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2psx 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 80[ 	]*vcvtuqq2psx -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2psx -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a f5[ 	]*vcvtuqq2ps %ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7a f5[ 	]*vcvtuqq2ps %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 31[ 	]*vcvtuqq2psy \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2psy -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 30[ 	]*vcvtuqq2ps \(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 7f[ 	]*vcvtuqq2psy 0xfe0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2psy 0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 80[ 	]*vcvtuqq2psy -0x1000\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2psy -0x1020\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 7f[ 	]*vcvtuqq2ps 0x3f8\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps 0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 80[ 	]*vcvtuqq2ps -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps -0x408\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 19 ee ab[ 	]*vextractf64x2 \$0xab,%ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 39 ee ab[ 	]*vextracti64x2 \$0xab,%ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee ab[ 	]*vfpclasspd \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee 7b[ 	]*vfpclasspd \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 29 7b[ 	]*vfpclasspdx \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspdx \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 28 7b[ 	]*vfpclasspd \$0x7b,\(%eax\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 7f 7b[ 	]*vfpclasspdx \$0x7b,0x7f0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa 00 08 00 00 7b[ 	]*vfpclasspdx \$0x7b,0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 80 7b[ 	]*vfpclasspdx \$0x7b,-0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspdx \$0x7b,-0x810\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%edx\)\{1to2\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee ab[ 	]*vfpclasspd \$0xab,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee 7b[ 	]*vfpclasspd \$0x7b,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 29 7b[ 	]*vfpclasspdy \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspdy \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 28 7b[ 	]*vfpclasspd \$0x7b,\(%eax\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 7f 7b[ 	]*vfpclasspdy \$0x7b,0xfe0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa 00 10 00 00 7b[ 	]*vfpclasspdy \$0x7b,0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 80 7b[ 	]*vfpclasspdy \$0x7b,-0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclasspdy \$0x7b,-0x1020\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 7f 7b[ 	]*vfpclasspd \$0x7b,0x3f8\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd \$0x7b,0x400\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 80 7b[ 	]*vfpclasspd \$0x7b,-0x400\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd \$0x7b,-0x408\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee ab[ 	]*vfpclassps \$0xab,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee 7b[ 	]*vfpclassps \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 29 7b[ 	]*vfpclasspsx \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspsx \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 28 7b[ 	]*vfpclassps \$0x7b,\(%eax\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 7f 7b[ 	]*vfpclasspsx \$0x7b,0x7f0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa 00 08 00 00 7b[ 	]*vfpclasspsx \$0x7b,0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 80 7b[ 	]*vfpclasspsx \$0x7b,-0x800\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspsx \$0x7b,-0x810\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%edx\)\{1to4\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee ab[ 	]*vfpclassps \$0xab,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee 7b[ 	]*vfpclassps \$0x7b,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 29 7b[ 	]*vfpclasspsy \$0x7b,\(%ecx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspsy \$0x7b,-0x1e240\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 28 7b[ 	]*vfpclassps \$0x7b,\(%eax\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 7f 7b[ 	]*vfpclasspsy \$0x7b,0xfe0\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa 00 10 00 00 7b[ 	]*vfpclasspsy \$0x7b,0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 80 7b[ 	]*vfpclasspsy \$0x7b,-0x1000\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclasspsy \$0x7b,-0x1020\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 7f 7b[ 	]*vfpclassps \$0x7b,0x1fc\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa 00 02 00 00 7b[ 	]*vfpclassps \$0x7b,0x200\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 80 7b[ 	]*vfpclassps \$0x7b,-0x200\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa fc fd ff ff 7b[ 	]*vfpclassps \$0x7b,-0x204\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 18 f4 ab[ 	]*vinsertf64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 7b[ 	]*vinsertf64x2 \$0x7b,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 31 7b[ 	]*vinsertf64x2 \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 7f 7b[ 	]*vinsertf64x2 \$0x7b,0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 \$0x7b,0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 80 7b[ 	]*vinsertf64x2 \$0x7b,-0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 \$0x7b,-0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 38 f4 ab[ 	]*vinserti64x2 \$0xab,%xmm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 7b[ 	]*vinserti64x2 \$0x7b,%xmm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 31 7b[ 	]*vinserti64x2 \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 7f 7b[ 	]*vinserti64x2 \$0x7b,0x7f0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 \$0x7b,0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 80 7b[ 	]*vinserti64x2 \$0x7b,-0x800\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 \$0x7b,-0x810\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 31[ 	]*vbroadcasti32x2 \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 f7[ 	]*vbroadcasti32x2 %xmm7,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 59 f7[ 	]*vbroadcasti32x2 %xmm7,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 31[ 	]*vbroadcasti32x2 \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 7f[ 	]*vbroadcasti32x2 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 80[ 	]*vbroadcasti32x2 -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 f4[ 	]*vpmullq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 40 f4[ 	]*vpmullq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 31[ 	]*vpmullq \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 30[ 	]*vpmullq \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 7f[ 	]*vpmullq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 00 08 00 00[ 	]*vpmullq 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 80[ 	]*vpmullq -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 f0 f7 ff ff[ 	]*vpmullq -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 7f[ 	]*vpmullq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 80[ 	]*vpmullq -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 f4[ 	]*vpmullq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 40 f4[ 	]*vpmullq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 31[ 	]*vpmullq \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 30[ 	]*vpmullq \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 7f[ 	]*vpmullq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 00 10 00 00[ 	]*vpmullq 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 80[ 	]*vpmullq -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 e0 ef ff ff[ 	]*vpmullq -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 7f[ 	]*vpmullq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 00 04 00 00[ 	]*vpmullq 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 80[ 	]*vpmullq -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 f8 fb ff ff[ 	]*vpmullq -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 ab[ 	]*vrangepd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 50 f4 ab[ 	]*vrangepd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 7b[ 	]*vrangepd \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 31 7b[ 	]*vrangepd \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 30 7b[ 	]*vrangepd \$0x7b,\(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 00 08 00 00 7b[ 	]*vrangepd \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 ab[ 	]*vrangepd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 50 f4 ab[ 	]*vrangepd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 7b[ 	]*vrangepd \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 31 7b[ 	]*vrangepd \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 30 7b[ 	]*vrangepd \$0x7b,\(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 00 10 00 00 7b[ 	]*vrangepd \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangepd \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 7f 7b[ 	]*vrangepd \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 00 04 00 00 7b[ 	]*vrangepd \$0x7b,0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 80 7b[ 	]*vrangepd \$0x7b,-0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd \$0x7b,-0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 ab[ 	]*vrangeps \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 50 f4 ab[ 	]*vrangeps \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 7b[ 	]*vrangeps \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 31 7b[ 	]*vrangeps \$0x7b,\(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 30 7b[ 	]*vrangeps \$0x7b,\(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 00 08 00 00 7b[ 	]*vrangeps \$0x7b,0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps \$0x7b,-0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 ab[ 	]*vrangeps \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 50 f4 ab[ 	]*vrangeps \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 7b[ 	]*vrangeps \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 31 7b[ 	]*vrangeps \$0x7b,\(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 30 7b[ 	]*vrangeps \$0x7b,\(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 00 10 00 00 7b[ 	]*vrangeps \$0x7b,0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangeps \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 7f 7b[ 	]*vrangeps \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 00 02 00 00 7b[ 	]*vrangeps \$0x7b,0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 80 7b[ 	]*vrangeps \$0x7b,-0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 fc fd ff ff 7b[ 	]*vrangeps \$0x7b,-0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 f4[ 	]*vandpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 54 f4[ 	]*vandpd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 31[ 	]*vandpd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b4 f4 c0 1d fe ff[ 	]*vandpd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 30[ 	]*vandpd \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 7f[ 	]*vandpd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 00 08 00 00[ 	]*vandpd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 80[ 	]*vandpd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 f0 f7 ff ff[ 	]*vandpd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 7f[ 	]*vandpd 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 80[ 	]*vandpd -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 f4[ 	]*vandpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 54 f4[ 	]*vandpd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 31[ 	]*vandpd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b4 f4 c0 1d fe ff[ 	]*vandpd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 30[ 	]*vandpd \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 7f[ 	]*vandpd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 00 10 00 00[ 	]*vandpd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 80[ 	]*vandpd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 e0 ef ff ff[ 	]*vandpd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 7f[ 	]*vandpd 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 00 04 00 00[ 	]*vandpd 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 80[ 	]*vandpd -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 f8 fb ff ff[ 	]*vandpd -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 f4[ 	]*vandps %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 54 f4[ 	]*vandps %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 31[ 	]*vandps \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b4 f4 c0 1d fe ff[ 	]*vandps -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 30[ 	]*vandps \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 7f[ 	]*vandps 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 00 08 00 00[ 	]*vandps 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 80[ 	]*vandps -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 f0 f7 ff ff[ 	]*vandps -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 7f[ 	]*vandps 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 00 02 00 00[ 	]*vandps 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 80[ 	]*vandps -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 f4[ 	]*vandps %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 54 f4[ 	]*vandps %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 31[ 	]*vandps \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b4 f4 c0 1d fe ff[ 	]*vandps -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 30[ 	]*vandps \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 7f[ 	]*vandps 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 00 10 00 00[ 	]*vandps 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 80[ 	]*vandps -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 e0 ef ff ff[ 	]*vandps -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 7f[ 	]*vandps 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 00 02 00 00[ 	]*vandps 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 80[ 	]*vandps -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 fc fd ff ff[ 	]*vandps -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 f4[ 	]*vandnpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 55 f4[ 	]*vandnpd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 31[ 	]*vandnpd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 30[ 	]*vandnpd \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 7f[ 	]*vandnpd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 00 08 00 00[ 	]*vandnpd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 80[ 	]*vandnpd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 f0 f7 ff ff[ 	]*vandnpd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 7f[ 	]*vandnpd 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 80[ 	]*vandnpd -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 f4[ 	]*vandnpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 55 f4[ 	]*vandnpd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 31[ 	]*vandnpd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 30[ 	]*vandnpd \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 7f[ 	]*vandnpd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 00 10 00 00[ 	]*vandnpd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 80[ 	]*vandnpd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 e0 ef ff ff[ 	]*vandnpd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 7f[ 	]*vandnpd 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 00 04 00 00[ 	]*vandnpd 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 80[ 	]*vandnpd -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 f8 fb ff ff[ 	]*vandnpd -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 f4[ 	]*vandnps %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 55 f4[ 	]*vandnps %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 31[ 	]*vandnps \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnps -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 30[ 	]*vandnps \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 7f[ 	]*vandnps 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 00 08 00 00[ 	]*vandnps 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 80[ 	]*vandnps -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 f0 f7 ff ff[ 	]*vandnps -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 7f[ 	]*vandnps 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 80[ 	]*vandnps -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 f4[ 	]*vandnps %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 55 f4[ 	]*vandnps %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 31[ 	]*vandnps \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnps -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 30[ 	]*vandnps \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 7f[ 	]*vandnps 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 00 10 00 00[ 	]*vandnps 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 80[ 	]*vandnps -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 e0 ef ff ff[ 	]*vandnps -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 7f[ 	]*vandnps 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 00 02 00 00[ 	]*vandnps 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 80[ 	]*vandnps -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 fc fd ff ff[ 	]*vandnps -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 f4[ 	]*vorpd  %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 56 f4[ 	]*vorpd  %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 31[ 	]*vorpd  \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 30[ 	]*vorpd  \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 7f[ 	]*vorpd  0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 00 08 00 00[ 	]*vorpd  0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 80[ 	]*vorpd  -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 f0 f7 ff ff[ 	]*vorpd  -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 7f[ 	]*vorpd  0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 80[ 	]*vorpd  -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 f4[ 	]*vorpd  %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 56 f4[ 	]*vorpd  %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 31[ 	]*vorpd  \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 30[ 	]*vorpd  \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 7f[ 	]*vorpd  0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 00 10 00 00[ 	]*vorpd  0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 80[ 	]*vorpd  -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 e0 ef ff ff[ 	]*vorpd  -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 7f[ 	]*vorpd  0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 00 04 00 00[ 	]*vorpd  0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 80[ 	]*vorpd  -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 f8 fb ff ff[ 	]*vorpd  -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 f4[ 	]*vorps  %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 56 f4[ 	]*vorps  %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 31[ 	]*vorps  \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b4 f4 c0 1d fe ff[ 	]*vorps  -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 30[ 	]*vorps  \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 7f[ 	]*vorps  0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 00 08 00 00[ 	]*vorps  0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 80[ 	]*vorps  -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 f0 f7 ff ff[ 	]*vorps  -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 7f[ 	]*vorps  0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 00 02 00 00[ 	]*vorps  0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 80[ 	]*vorps  -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 f4[ 	]*vorps  %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 56 f4[ 	]*vorps  %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 31[ 	]*vorps  \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b4 f4 c0 1d fe ff[ 	]*vorps  -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 30[ 	]*vorps  \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 7f[ 	]*vorps  0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 00 10 00 00[ 	]*vorps  0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 80[ 	]*vorps  -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 e0 ef ff ff[ 	]*vorps  -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 7f[ 	]*vorps  0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 00 02 00 00[ 	]*vorps  0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 80[ 	]*vorps  -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 fc fd ff ff[ 	]*vorps  -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 f4[ 	]*vxorpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 57 f4[ 	]*vxorpd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 31[ 	]*vxorpd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 30[ 	]*vxorpd \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 7f[ 	]*vxorpd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 00 08 00 00[ 	]*vxorpd 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 80[ 	]*vxorpd -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 f0 f7 ff ff[ 	]*vxorpd -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 7f[ 	]*vxorpd 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 80[ 	]*vxorpd -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 f4[ 	]*vxorpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 57 f4[ 	]*vxorpd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 31[ 	]*vxorpd \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 30[ 	]*vxorpd \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 7f[ 	]*vxorpd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 00 10 00 00[ 	]*vxorpd 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 80[ 	]*vxorpd -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 e0 ef ff ff[ 	]*vxorpd -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 7f[ 	]*vxorpd 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 00 04 00 00[ 	]*vxorpd 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 80[ 	]*vxorpd -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 f8 fb ff ff[ 	]*vxorpd -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 f4[ 	]*vxorps %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 57 f4[ 	]*vxorps %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 31[ 	]*vxorps \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorps -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 30[ 	]*vxorps \(%eax\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 7f[ 	]*vxorps 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 00 08 00 00[ 	]*vxorps 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 80[ 	]*vxorps -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 f0 f7 ff ff[ 	]*vxorps -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 7f[ 	]*vxorps 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 80[ 	]*vxorps -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 f4[ 	]*vxorps %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 57 f4[ 	]*vxorps %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 31[ 	]*vxorps \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorps -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 30[ 	]*vxorps \(%eax\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 7f[ 	]*vxorps 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 00 10 00 00[ 	]*vxorps 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 80[ 	]*vxorps -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 e0 ef ff ff[ 	]*vxorps -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 7f[ 	]*vxorps 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 00 02 00 00[ 	]*vxorps 0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 80[ 	]*vxorps -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 fc fd ff ff[ 	]*vxorps -0x204\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 8f 56 f5 ab[ 	]*vreducepd \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 7b[ 	]*vreducepd \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 31 7b[ 	]*vreducepd \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 30 7b[ 	]*vreducepd \$0x7b,\(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 00 08 00 00 7b[ 	]*vreducepd \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 ab[ 	]*vreducepd \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 56 f5 ab[ 	]*vreducepd \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 7b[ 	]*vreducepd \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 31 7b[ 	]*vreducepd \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 30 7b[ 	]*vreducepd \$0x7b,\(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 00 10 00 00 7b[ 	]*vreducepd \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 e0 ef ff ff 7b[ 	]*vreducepd \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 7f 7b[ 	]*vreducepd \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 00 04 00 00 7b[ 	]*vreducepd \$0x7b,0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 80 7b[ 	]*vreducepd \$0x7b,-0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd \$0x7b,-0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 8f 56 f5 ab[ 	]*vreduceps \$0xab,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 7b[ 	]*vreduceps \$0x7b,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 31 7b[ 	]*vreduceps \$0x7b,\(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 30 7b[ 	]*vreduceps \$0x7b,\(%eax\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 00 08 00 00 7b[ 	]*vreduceps \$0x7b,0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps \$0x7b,-0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%edx\)\{1to4\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 ab[ 	]*vreduceps \$0xab,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d af 56 f5 ab[ 	]*vreduceps \$0xab,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 7b[ 	]*vreduceps \$0x7b,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 31 7b[ 	]*vreduceps \$0x7b,\(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 30 7b[ 	]*vreduceps \$0x7b,\(%eax\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 00 10 00 00 7b[ 	]*vreduceps \$0x7b,0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 e0 ef ff ff 7b[ 	]*vreduceps \$0x7b,-0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 7f 7b[ 	]*vreduceps \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 00 02 00 00 7b[ 	]*vreduceps \$0x7b,0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 80 7b[ 	]*vreduceps \$0x7b,-0x200\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 fc fd ff ff 7b[ 	]*vreduceps \$0x7b,-0x204\(%edx\)\{1to8\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 ab[ 	]*vextractf64x2 \$0xab,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ac f4 c0 1d fe ff 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 7f 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 80 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 \$0x7b,%ymm5,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 ab[ 	]*vextracti64x2 \$0xab,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ac f4 c0 1d fe ff 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 7f 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,0x7f0\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 80 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,-0x800\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 \$0x7b,%ymm5,-0x810\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a f5[ 	]*vcvttpd2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7a f5[ 	]*vcvttpd2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 31[ 	]*vcvttpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 30[ 	]*vcvttpd2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 7f[ 	]*vcvttpd2qq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 00 08 00 00[ 	]*vcvttpd2qq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 80[ 	]*vcvttpd2qq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 80[ 	]*vcvttpd2qq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a f5[ 	]*vcvttpd2qq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7a f5[ 	]*vcvttpd2qq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 31[ 	]*vcvttpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 30[ 	]*vcvttpd2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 7f[ 	]*vcvttpd2qq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 00 10 00 00[ 	]*vcvttpd2qq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 80[ 	]*vcvttpd2qq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 7f[ 	]*vcvttpd2qq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 00 04 00 00[ 	]*vcvttpd2qq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 80[ 	]*vcvttpd2qq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 f5[ 	]*vcvttpd2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 78 f5[ 	]*vcvttpd2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 31[ 	]*vcvttpd2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 30[ 	]*vcvttpd2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 7f[ 	]*vcvttpd2uqq 0x7f0\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 00 08 00 00[ 	]*vcvttpd2uqq 0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 80[ 	]*vcvttpd2uqq -0x800\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq -0x810\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 80[ 	]*vcvttpd2uqq -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 f5[ 	]*vcvttpd2uqq %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 78 f5[ 	]*vcvttpd2uqq %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 31[ 	]*vcvttpd2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 30[ 	]*vcvttpd2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 7f[ 	]*vcvttpd2uqq 0xfe0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 00 10 00 00[ 	]*vcvttpd2uqq 0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 80[ 	]*vcvttpd2uqq -0x1000\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq -0x1020\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 7f[ 	]*vcvttpd2uqq 0x3f8\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq 0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 80[ 	]*vcvttpd2uqq -0x400\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq -0x408\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a f5[ 	]*vcvttps2qq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7a f5[ 	]*vcvttps2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 31[ 	]*vcvttps2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 30[ 	]*vcvttps2qq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 7f[ 	]*vcvttps2qq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 00 04 00 00[ 	]*vcvttps2qq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 80[ 	]*vcvttps2qq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 f8 fb ff ff[ 	]*vcvttps2qq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 80[ 	]*vcvttps2qq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a f5[ 	]*vcvttps2qq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7a f5[ 	]*vcvttps2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 31[ 	]*vcvttps2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 30[ 	]*vcvttps2qq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 7f[ 	]*vcvttps2qq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 00 08 00 00[ 	]*vcvttps2qq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 80[ 	]*vcvttps2qq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 00 02 00 00[ 	]*vcvttps2qq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 80[ 	]*vcvttps2qq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 fc fd ff ff[ 	]*vcvttps2qq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 7f[ 	]*vcvttps2qq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 f5[ 	]*vcvttps2uqq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 78 f5[ 	]*vcvttps2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 31[ 	]*vcvttps2uqq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 30[ 	]*vcvttps2uqq \(%eax\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 7f[ 	]*vcvttps2uqq 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 00 04 00 00[ 	]*vcvttps2uqq 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 80[ 	]*vcvttps2uqq -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 80[ 	]*vcvttps2uqq -0x200\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to2\},%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 f5[ 	]*vcvttps2uqq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 78 f5[ 	]*vcvttps2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 31[ 	]*vcvttps2uqq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 30[ 	]*vcvttps2uqq \(%eax\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 7f[ 	]*vcvttps2uqq 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 00 08 00 00[ 	]*vcvttps2uqq 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 80[ 	]*vcvttps2uqq -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 00 02 00 00[ 	]*vcvttps2uqq 0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 80[ 	]*vcvttps2uqq -0x200\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq -0x204\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 7f[ 	]*vcvttps2uqq 0x1fc\(%edx\)\{1to4\},%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 39 ee[ 	]*vpmovd2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 39 ee[ 	]*vpmovd2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 39 ee[ 	]*vpmovq2m %xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 39 ee[ 	]*vpmovq2m %ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 38 f5[ 	]*vpmovm2d %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 38 f5[ 	]*vpmovm2d %k5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 38 f5[ 	]*vpmovm2q %k5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 38 f5[ 	]*vpmovm2q %k5,%ymm6
#pass
