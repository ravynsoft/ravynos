#as:
#objdump: -dw
#name: i386 AVX512-FP16,AVX512VL insns
#source: avx512_fp16_vl.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 f4[ 	]*vaddph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 58 f4[ 	]*vaddph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 f4[ 	]*vaddph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 58 f4[ 	]*vaddph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 58 b4 f4 00 00 00 10[ 	]*vaddph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 58 31[ 	]*vaddph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 71 7f[ 	]*vaddph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 58 72 80[ 	]*vaddph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 58 b4 f4 00 00 00 10[ 	]*vaddph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 31[ 	]*vaddph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 71 7f[ 	]*vaddph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 72 80[ 	]*vaddph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 ec 7b[ 	]*vcmpph \$0x7b,%ymm4,%ymm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ec 7b[ 	]*vcmpph \$0x7b,%ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 ec 7b[ 	]*vcmpph \$0x7b,%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ec 7b[ 	]*vcmpph \$0x7b,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ac f4 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 29 7b[ 	]*vcmpph \$0x7b,\(%ecx\)\{1to8\},%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0x7f0\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ac f4 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 38 c2 29 7b[ 	]*vcmpph \$0x7b,\(%ecx\)\{1to16\},%ymm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0xfe0\(%ecx\),%ymm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 3f c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b f5[ 	]*vcvtdq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5b f5[ 	]*vcvtdq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b f5[ 	]*vcvtdq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5b f5[ 	]*vcvtdq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5b b4 f4 00 00 00 10[ 	]*vcvtdq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b 31[ 	]*vcvtdq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b 71 7f[ 	]*vcvtdq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b 72 80[ 	]*vcvtdq2ph -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5b 31[ 	]*vcvtdq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b 71 7f[ 	]*vcvtdq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5b 72 80[ 	]*vcvtdq2ph -0x200\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a f5[ 	]*vcvtpd2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 8f 5a f5[ 	]*vcvtpd2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a f5[ 	]*vcvtpd2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd af 5a f5[ 	]*vcvtpd2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 0f 5a b4 f4 00 00 00 10[ 	]*vcvtpd2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a 31[ 	]*vcvtpd2ph \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a 71 7f[ 	]*vcvtpd2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a 72 80[ 	]*vcvtpd2ph -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 38 5a 31[ 	]*vcvtpd2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a 71 7f[ 	]*vcvtpd2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd bf 5a 72 80[ 	]*vcvtpd2ph -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b f5[ 	]*vcvtph2dq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 5b f5[ 	]*vcvtph2dq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b f5[ 	]*vcvtph2dq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 5b f5[ 	]*vcvtph2dq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 5b b4 f4 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b 31[ 	]*vcvtph2dq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b 71 7f[ 	]*vcvtph2dq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b 72 80[ 	]*vcvtph2dq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 5b b4 f4 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 5b 31[ 	]*vcvtph2dq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b 71 7f[ 	]*vcvtph2dq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 5b 72 80[ 	]*vcvtph2dq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a f5[ 	]*vcvtph2pd %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5a f5[ 	]*vcvtph2pd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a f5[ 	]*vcvtph2pd %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5a f5[ 	]*vcvtph2pd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5a b4 f4 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a 31[ 	]*vcvtph2pd \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a 71 7f[ 	]*vcvtph2pd 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a 72 80[ 	]*vcvtph2pd -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 5a b4 f4 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5a 31[ 	]*vcvtph2pd \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a 71 7f[ 	]*vcvtph2pd 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5a 72 80[ 	]*vcvtph2pd -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 f5[ 	]*vcvtph2psx %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 13 f5[ 	]*vcvtph2psx %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 f5[ 	]*vcvtph2psx %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 13 f5[ 	]*vcvtph2psx %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 13 b4 f4 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 31[ 	]*vcvtph2psx \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 71 7f[ 	]*vcvtph2psx 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 72 80[ 	]*vcvtph2psx -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 13 b4 f4 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 13 31[ 	]*vcvtph2psx \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 71 7f[ 	]*vcvtph2psx 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 13 72 80[ 	]*vcvtph2psx -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b f5[ 	]*vcvtph2qq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7b f5[ 	]*vcvtph2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b f5[ 	]*vcvtph2qq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7b f5[ 	]*vcvtph2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7b b4 f4 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b 31[ 	]*vcvtph2qq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b 71 7f[ 	]*vcvtph2qq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b 72 80[ 	]*vcvtph2qq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7b b4 f4 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7b 31[ 	]*vcvtph2qq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b 71 7f[ 	]*vcvtph2qq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7b 72 80[ 	]*vcvtph2qq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 f5[ 	]*vcvtph2udq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 79 f5[ 	]*vcvtph2udq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 f5[ 	]*vcvtph2udq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 79 f5[ 	]*vcvtph2udq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 79 b4 f4 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 31[ 	]*vcvtph2udq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 71 7f[ 	]*vcvtph2udq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 72 80[ 	]*vcvtph2udq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 79 b4 f4 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 79 31[ 	]*vcvtph2udq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 71 7f[ 	]*vcvtph2udq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 79 72 80[ 	]*vcvtph2udq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 f5[ 	]*vcvtph2uqq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 79 f5[ 	]*vcvtph2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 f5[ 	]*vcvtph2uqq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 79 f5[ 	]*vcvtph2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 79 b4 f4 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 31[ 	]*vcvtph2uqq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 71 7f[ 	]*vcvtph2uqq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 72 80[ 	]*vcvtph2uqq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 79 b4 f4 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 79 31[ 	]*vcvtph2uqq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 71 7f[ 	]*vcvtph2uqq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 79 72 80[ 	]*vcvtph2uqq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d f5[ 	]*vcvtph2uw %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7d f5[ 	]*vcvtph2uw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d f5[ 	]*vcvtph2uw %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7d f5[ 	]*vcvtph2uw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7d b4 f4 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d 31[ 	]*vcvtph2uw \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d 71 7f[ 	]*vcvtph2uw 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d 72 80[ 	]*vcvtph2uw -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7d b4 f4 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7d 31[ 	]*vcvtph2uw \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d 71 7f[ 	]*vcvtph2uw 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7d 72 80[ 	]*vcvtph2uw -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d f5[ 	]*vcvtph2w %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7d f5[ 	]*vcvtph2w %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d f5[ 	]*vcvtph2w %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7d f5[ 	]*vcvtph2w %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7d b4 f4 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d 31[ 	]*vcvtph2w \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d 71 7f[ 	]*vcvtph2w 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d 72 80[ 	]*vcvtph2w -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7d b4 f4 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7d 31[ 	]*vcvtph2w \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d 71 7f[ 	]*vcvtph2w 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7d 72 80[ 	]*vcvtph2w -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d f5[ 	]*vcvtps2phx %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 1d f5[ 	]*vcvtps2phx %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d f5[ 	]*vcvtps2phx %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 1d f5[ 	]*vcvtps2phx %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 1d b4 f4 00 00 00 10[ 	]*vcvtps2phxx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d 31[ 	]*vcvtps2phx \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d 71 7f[ 	]*vcvtps2phxx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d 72 80[ 	]*vcvtps2phx -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 1d 31[ 	]*vcvtps2phx \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d 71 7f[ 	]*vcvtps2phxy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 1d 72 80[ 	]*vcvtps2phx -0x200\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b f5[ 	]*vcvtqq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 8f 5b f5[ 	]*vcvtqq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b f5[ 	]*vcvtqq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc af 5b f5[ 	]*vcvtqq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 0f 5b b4 f4 00 00 00 10[ 	]*vcvtqq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b 31[ 	]*vcvtqq2ph \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b 71 7f[ 	]*vcvtqq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b 72 80[ 	]*vcvtqq2ph -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 38 5b 31[ 	]*vcvtqq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b 71 7f[ 	]*vcvtqq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc bf 5b 72 80[ 	]*vcvtqq2ph -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b f5[ 	]*vcvttph2dq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 5b f5[ 	]*vcvttph2dq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b f5[ 	]*vcvttph2dq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 5b f5[ 	]*vcvttph2dq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 5b b4 f4 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b 31[ 	]*vcvttph2dq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b 71 7f[ 	]*vcvttph2dq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b 72 80[ 	]*vcvttph2dq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 5b b4 f4 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 5b 31[ 	]*vcvttph2dq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b 71 7f[ 	]*vcvttph2dq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 5b 72 80[ 	]*vcvttph2dq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a f5[ 	]*vcvttph2qq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7a f5[ 	]*vcvttph2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a f5[ 	]*vcvttph2qq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7a f5[ 	]*vcvttph2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7a b4 f4 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a 31[ 	]*vcvttph2qq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a 71 7f[ 	]*vcvttph2qq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a 72 80[ 	]*vcvttph2qq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7a b4 f4 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7a 31[ 	]*vcvttph2qq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a 71 7f[ 	]*vcvttph2qq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7a 72 80[ 	]*vcvttph2qq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 f5[ 	]*vcvttph2udq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 78 f5[ 	]*vcvttph2udq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 f5[ 	]*vcvttph2udq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 78 f5[ 	]*vcvttph2udq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 78 b4 f4 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 31[ 	]*vcvttph2udq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 71 7f[ 	]*vcvttph2udq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 72 80[ 	]*vcvttph2udq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 78 b4 f4 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 78 31[ 	]*vcvttph2udq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 71 7f[ 	]*vcvttph2udq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 78 72 80[ 	]*vcvttph2udq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 f5[ 	]*vcvttph2uqq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 78 f5[ 	]*vcvttph2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 f5[ 	]*vcvttph2uqq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 78 f5[ 	]*vcvttph2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 78 b4 f4 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 31[ 	]*vcvttph2uqq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 71 7f[ 	]*vcvttph2uqq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 72 80[ 	]*vcvttph2uqq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 78 b4 f4 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 78 31[ 	]*vcvttph2uqq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 71 7f[ 	]*vcvttph2uqq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 78 72 80[ 	]*vcvttph2uqq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c f5[ 	]*vcvttph2uw %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7c f5[ 	]*vcvttph2uw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c f5[ 	]*vcvttph2uw %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7c f5[ 	]*vcvttph2uw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7c b4 f4 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c 31[ 	]*vcvttph2uw \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c 71 7f[ 	]*vcvttph2uw 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c 72 80[ 	]*vcvttph2uw -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7c b4 f4 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7c 31[ 	]*vcvttph2uw \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c 71 7f[ 	]*vcvttph2uw 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7c 72 80[ 	]*vcvttph2uw -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c f5[ 	]*vcvttph2w %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7c f5[ 	]*vcvttph2w %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c f5[ 	]*vcvttph2w %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7c f5[ 	]*vcvttph2w %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7c b4 f4 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c 31[ 	]*vcvttph2w \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c 71 7f[ 	]*vcvttph2w 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c 72 80[ 	]*vcvttph2w -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7c b4 f4 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7c 31[ 	]*vcvttph2w \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c 71 7f[ 	]*vcvttph2w 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7c 72 80[ 	]*vcvttph2w -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a f5[ 	]*vcvtudq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7a f5[ 	]*vcvtudq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a f5[ 	]*vcvtudq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7a f5[ 	]*vcvtudq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7a b4 f4 00 00 00 10[ 	]*vcvtudq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a 31[ 	]*vcvtudq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a 71 7f[ 	]*vcvtudq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a 72 80[ 	]*vcvtudq2ph -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7a 31[ 	]*vcvtudq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a 71 7f[ 	]*vcvtudq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7a 72 80[ 	]*vcvtudq2ph -0x200\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a f5[ 	]*vcvtuqq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 8f 7a f5[ 	]*vcvtuqq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a f5[ 	]*vcvtuqq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff af 7a f5[ 	]*vcvtuqq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 0f 7a b4 f4 00 00 00 10[ 	]*vcvtuqq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a 31[ 	]*vcvtuqq2ph \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a 71 7f[ 	]*vcvtuqq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 38 7a 31[ 	]*vcvtuqq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a 71 7f[ 	]*vcvtuqq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff bf 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d f5[ 	]*vcvtuw2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7d f5[ 	]*vcvtuw2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d f5[ 	]*vcvtuw2ph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7d f5[ 	]*vcvtuw2ph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7d b4 f4 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d 31[ 	]*vcvtuw2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d 71 7f[ 	]*vcvtuw2ph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d 72 80[ 	]*vcvtuw2ph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 2f 7d b4 f4 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7d 31[ 	]*vcvtuw2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d 71 7f[ 	]*vcvtuw2ph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7d 72 80[ 	]*vcvtuw2ph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d f5[ 	]*vcvtw2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 7d f5[ 	]*vcvtw2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d f5[ 	]*vcvtw2ph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 7d f5[ 	]*vcvtw2ph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 7d b4 f4 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d 31[ 	]*vcvtw2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d 71 7f[ 	]*vcvtw2ph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d 72 80[ 	]*vcvtw2ph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 7d b4 f4 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 7d 31[ 	]*vcvtw2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d 71 7f[ 	]*vcvtw2ph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 7d 72 80[ 	]*vcvtw2ph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e f4[ 	]*vdivph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5e f4[ 	]*vdivph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e f4[ 	]*vdivph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5e f4[ 	]*vdivph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5e b4 f4 00 00 00 10[ 	]*vdivph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5e 31[ 	]*vdivph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e 71 7f[ 	]*vdivph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5e 72 80[ 	]*vdivph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5e b4 f4 00 00 00 10[ 	]*vdivph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e 31[ 	]*vdivph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e 71 7f[ 	]*vdivph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e 72 80[ 	]*vdivph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 f4[ 	]*vfcmaddcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af 56 f4[ 	]*vfcmaddcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 f4[ 	]*vfcmaddcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 56 f4[ 	]*vfcmaddcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f 56 b4 f4 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 56 31[ 	]*vfcmaddcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 71 7f[ 	]*vfcmaddcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf 56 72 80[ 	]*vfcmaddcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 56 b4 f4 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 31[ 	]*vfcmaddcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 71 7f[ 	]*vfcmaddcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 72 80[ 	]*vfcmaddcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 f4[ 	]*vfcmulcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af d6 f4[ 	]*vfcmulcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 f4[ 	]*vfcmulcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d6 f4[ 	]*vfcmulcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f d6 b4 f4 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 d6 31[ 	]*vfcmulcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 71 7f[ 	]*vfcmulcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf d6 72 80[ 	]*vfcmulcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d6 b4 f4 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 31[ 	]*vfcmulcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 71 7f[ 	]*vfcmulcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 72 80[ 	]*vfcmulcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 f4[ 	]*vfmadd132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 98 f4[ 	]*vfmadd132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 f4[ 	]*vfmadd132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 98 f4[ 	]*vfmadd132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 98 b4 f4 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 98 31[ 	]*vfmadd132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 71 7f[ 	]*vfmadd132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 98 72 80[ 	]*vfmadd132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 98 b4 f4 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 31[ 	]*vfmadd132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 71 7f[ 	]*vfmadd132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 72 80[ 	]*vfmadd132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 f4[ 	]*vfmadd213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a8 f4[ 	]*vfmadd213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 f4[ 	]*vfmadd213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a8 f4[ 	]*vfmadd213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a8 b4 f4 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a8 31[ 	]*vfmadd213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 71 7f[ 	]*vfmadd213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a8 72 80[ 	]*vfmadd213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a8 b4 f4 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 31[ 	]*vfmadd213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 71 7f[ 	]*vfmadd213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 72 80[ 	]*vfmadd213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 f4[ 	]*vfmadd231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b8 f4[ 	]*vfmadd231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 f4[ 	]*vfmadd231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b8 f4[ 	]*vfmadd231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b8 b4 f4 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b8 31[ 	]*vfmadd231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 71 7f[ 	]*vfmadd231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b8 72 80[ 	]*vfmadd231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b8 b4 f4 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 31[ 	]*vfmadd231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 71 7f[ 	]*vfmadd231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 72 80[ 	]*vfmadd231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 f4[ 	]*vfmaddcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af 56 f4[ 	]*vfmaddcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 f4[ 	]*vfmaddcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 56 f4[ 	]*vfmaddcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f 56 b4 f4 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 56 31[ 	]*vfmaddcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 71 7f[ 	]*vfmaddcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf 56 72 80[ 	]*vfmaddcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 56 b4 f4 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 31[ 	]*vfmaddcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 71 7f[ 	]*vfmaddcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 72 80[ 	]*vfmaddcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 f4[ 	]*vfmaddsub132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 96 f4[ 	]*vfmaddsub132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 f4[ 	]*vfmaddsub132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 96 f4[ 	]*vfmaddsub132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 96 b4 f4 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 96 31[ 	]*vfmaddsub132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 71 7f[ 	]*vfmaddsub132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 96 72 80[ 	]*vfmaddsub132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 96 b4 f4 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 31[ 	]*vfmaddsub132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 71 7f[ 	]*vfmaddsub132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 72 80[ 	]*vfmaddsub132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 f4[ 	]*vfmaddsub213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a6 f4[ 	]*vfmaddsub213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 f4[ 	]*vfmaddsub213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a6 f4[ 	]*vfmaddsub213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a6 b4 f4 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a6 31[ 	]*vfmaddsub213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 71 7f[ 	]*vfmaddsub213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a6 72 80[ 	]*vfmaddsub213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a6 b4 f4 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 31[ 	]*vfmaddsub213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 71 7f[ 	]*vfmaddsub213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 72 80[ 	]*vfmaddsub213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 f4[ 	]*vfmaddsub231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b6 f4[ 	]*vfmaddsub231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 f4[ 	]*vfmaddsub231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b6 f4[ 	]*vfmaddsub231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b6 b4 f4 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b6 31[ 	]*vfmaddsub231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 71 7f[ 	]*vfmaddsub231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b6 72 80[ 	]*vfmaddsub231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b6 b4 f4 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 31[ 	]*vfmaddsub231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 71 7f[ 	]*vfmaddsub231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 72 80[ 	]*vfmaddsub231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a f4[ 	]*vfmsub132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9a f4[ 	]*vfmsub132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a f4[ 	]*vfmsub132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9a f4[ 	]*vfmsub132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9a b4 f4 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9a 31[ 	]*vfmsub132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a 71 7f[ 	]*vfmsub132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9a 72 80[ 	]*vfmsub132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9a b4 f4 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a 31[ 	]*vfmsub132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a 71 7f[ 	]*vfmsub132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a 72 80[ 	]*vfmsub132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa f4[ 	]*vfmsub213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af aa f4[ 	]*vfmsub213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa f4[ 	]*vfmsub213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f aa f4[ 	]*vfmsub213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f aa b4 f4 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 aa 31[ 	]*vfmsub213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa 71 7f[ 	]*vfmsub213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf aa 72 80[ 	]*vfmsub213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f aa b4 f4 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa 31[ 	]*vfmsub213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa 71 7f[ 	]*vfmsub213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa 72 80[ 	]*vfmsub213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba f4[ 	]*vfmsub231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ba f4[ 	]*vfmsub231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba f4[ 	]*vfmsub231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ba f4[ 	]*vfmsub231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ba b4 f4 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ba 31[ 	]*vfmsub231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba 71 7f[ 	]*vfmsub231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ba 72 80[ 	]*vfmsub231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ba b4 f4 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba 31[ 	]*vfmsub231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba 71 7f[ 	]*vfmsub231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba 72 80[ 	]*vfmsub231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 f4[ 	]*vfmsubadd132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 97 f4[ 	]*vfmsubadd132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 f4[ 	]*vfmsubadd132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 97 f4[ 	]*vfmsubadd132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 97 b4 f4 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 97 31[ 	]*vfmsubadd132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 71 7f[ 	]*vfmsubadd132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 97 72 80[ 	]*vfmsubadd132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 97 b4 f4 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 31[ 	]*vfmsubadd132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 71 7f[ 	]*vfmsubadd132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 72 80[ 	]*vfmsubadd132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 f4[ 	]*vfmsubadd213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a7 f4[ 	]*vfmsubadd213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 f4[ 	]*vfmsubadd213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a7 f4[ 	]*vfmsubadd213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a7 b4 f4 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a7 31[ 	]*vfmsubadd213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 71 7f[ 	]*vfmsubadd213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a7 72 80[ 	]*vfmsubadd213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a7 b4 f4 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 31[ 	]*vfmsubadd213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 71 7f[ 	]*vfmsubadd213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 72 80[ 	]*vfmsubadd213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 f4[ 	]*vfmsubadd231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b7 f4[ 	]*vfmsubadd231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 f4[ 	]*vfmsubadd231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b7 f4[ 	]*vfmsubadd231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b7 b4 f4 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b7 31[ 	]*vfmsubadd231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 71 7f[ 	]*vfmsubadd231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b7 72 80[ 	]*vfmsubadd231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b7 b4 f4 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 31[ 	]*vfmsubadd231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 71 7f[ 	]*vfmsubadd231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 72 80[ 	]*vfmsubadd231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 f4[ 	]*vfmulcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af d6 f4[ 	]*vfmulcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 f4[ 	]*vfmulcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d6 f4[ 	]*vfmulcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f d6 b4 f4 00 00 00 10[ 	]*vfmulcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 d6 31[ 	]*vfmulcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 71 7f[ 	]*vfmulcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf d6 72 80[ 	]*vfmulcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d6 b4 f4 00 00 00 10[ 	]*vfmulcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 31[ 	]*vfmulcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 71 7f[ 	]*vfmulcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 72 80[ 	]*vfmulcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c f4[ 	]*vfnmadd132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9c f4[ 	]*vfnmadd132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c f4[ 	]*vfnmadd132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9c f4[ 	]*vfnmadd132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9c b4 f4 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9c 31[ 	]*vfnmadd132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c 71 7f[ 	]*vfnmadd132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9c 72 80[ 	]*vfnmadd132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9c b4 f4 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c 31[ 	]*vfnmadd132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c 71 7f[ 	]*vfnmadd132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c 72 80[ 	]*vfnmadd132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac f4[ 	]*vfnmadd213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ac f4[ 	]*vfnmadd213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac f4[ 	]*vfnmadd213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ac f4[ 	]*vfnmadd213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ac b4 f4 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ac 31[ 	]*vfnmadd213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac 71 7f[ 	]*vfnmadd213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ac 72 80[ 	]*vfnmadd213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ac b4 f4 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac 31[ 	]*vfnmadd213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac 71 7f[ 	]*vfnmadd213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac 72 80[ 	]*vfnmadd213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc f4[ 	]*vfnmadd231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af bc f4[ 	]*vfnmadd231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc f4[ 	]*vfnmadd231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bc f4[ 	]*vfnmadd231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f bc b4 f4 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 bc 31[ 	]*vfnmadd231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc 71 7f[ 	]*vfnmadd231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf bc 72 80[ 	]*vfnmadd231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bc b4 f4 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc 31[ 	]*vfnmadd231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc 71 7f[ 	]*vfnmadd231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc 72 80[ 	]*vfnmadd231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e f4[ 	]*vfnmsub132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9e f4[ 	]*vfnmsub132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e f4[ 	]*vfnmsub132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9e f4[ 	]*vfnmsub132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9e b4 f4 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9e 31[ 	]*vfnmsub132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e 71 7f[ 	]*vfnmsub132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9e 72 80[ 	]*vfnmsub132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9e b4 f4 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e 31[ 	]*vfnmsub132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e 71 7f[ 	]*vfnmsub132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e 72 80[ 	]*vfnmsub132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae f4[ 	]*vfnmsub213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ae f4[ 	]*vfnmsub213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae f4[ 	]*vfnmsub213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ae f4[ 	]*vfnmsub213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ae b4 f4 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ae 31[ 	]*vfnmsub213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae 71 7f[ 	]*vfnmsub213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ae 72 80[ 	]*vfnmsub213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ae b4 f4 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae 31[ 	]*vfnmsub213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae 71 7f[ 	]*vfnmsub213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae 72 80[ 	]*vfnmsub213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be f4[ 	]*vfnmsub231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af be f4[ 	]*vfnmsub231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be f4[ 	]*vfnmsub231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f be f4[ 	]*vfnmsub231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f be b4 f4 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 be 31[ 	]*vfnmsub231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be 71 7f[ 	]*vfnmsub231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf be 72 80[ 	]*vfnmsub231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f be b4 f4 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be 31[ 	]*vfnmsub231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be 71 7f[ 	]*vfnmsub231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be 72 80[ 	]*vfnmsub231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 ee 7b[ 	]*vfpclassph \$0x7b,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ee 7b[ 	]*vfpclassph \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 ee 7b[ 	]*vfpclassph \$0x7b,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 66 ee 7b[ 	]*vfpclassph \$0x7b,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ac f4 00 00 00 10 7b[ 	]*vfpclassphx \$0x7b,0x10000000\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 66 29 7b[ 	]*vfpclassph \$0x7b,\(%ecx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 69 7f 7b[ 	]*vfpclassphx \$0x7b,0x7f0\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 1f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 66 69 01 7b[ 	]*vfpclassph \$0x7b,0x2\(%ecx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 66 29 7b[ 	]*vfpclassph \$0x7b,\(%ecx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 69 7f 7b[ 	]*vfpclassphy \$0x7b,0xfe0\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 3f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%edx\)\{1to16\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 66 69 01 7b[ 	]*vfpclassph \$0x7b,0x2\(%ecx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 f5[ 	]*vgetexpph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 42 f5[ 	]*vgetexpph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 f5[ 	]*vgetexpph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 42 f5[ 	]*vgetexpph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 42 b4 f4 00 00 00 10[ 	]*vgetexpph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 31[ 	]*vgetexpph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 71 7f[ 	]*vgetexpph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 72 80[ 	]*vgetexpph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 42 b4 f4 00 00 00 10[ 	]*vgetexpph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 42 31[ 	]*vgetexpph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 71 7f[ 	]*vgetexpph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 42 72 80[ 	]*vgetexpph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 f5 7b[ 	]*vgetmantph \$0x7b,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 26 f5 7b[ 	]*vgetmantph \$0x7b,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 f5 7b[ 	]*vgetmantph \$0x7b,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 26 f5 7b[ 	]*vgetmantph \$0x7b,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 26 b4 f4 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 31 7b[ 	]*vgetmantph \$0x7b,\(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 26 b4 f4 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 26 31 7b[ 	]*vgetmantph \$0x7b,\(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f f4[ 	]*vmaxph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5f f4[ 	]*vmaxph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f f4[ 	]*vmaxph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5f f4[ 	]*vmaxph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5f b4 f4 00 00 00 10[ 	]*vmaxph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5f 31[ 	]*vmaxph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f 71 7f[ 	]*vmaxph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5f 72 80[ 	]*vmaxph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5f b4 f4 00 00 00 10[ 	]*vmaxph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f 31[ 	]*vmaxph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f 71 7f[ 	]*vmaxph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f 72 80[ 	]*vmaxph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d f4[ 	]*vminph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5d f4[ 	]*vminph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d f4[ 	]*vminph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5d f4[ 	]*vminph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5d b4 f4 00 00 00 10[ 	]*vminph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5d 31[ 	]*vminph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d 71 7f[ 	]*vminph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5d 72 80[ 	]*vminph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5d b4 f4 00 00 00 10[ 	]*vminph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d 31[ 	]*vminph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d 71 7f[ 	]*vminph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d 72 80[ 	]*vminph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 f4[ 	]*vmulph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 59 f4[ 	]*vmulph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 f4[ 	]*vmulph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 59 f4[ 	]*vmulph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 59 b4 f4 00 00 00 10[ 	]*vmulph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 59 31[ 	]*vmulph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 71 7f[ 	]*vmulph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 59 72 80[ 	]*vmulph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 59 b4 f4 00 00 00 10[ 	]*vmulph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 31[ 	]*vmulph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 71 7f[ 	]*vmulph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 72 80[ 	]*vmulph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c f5[ 	]*vrcpph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4c f5[ 	]*vrcpph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c f5[ 	]*vrcpph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4c f5[ 	]*vrcpph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4c b4 f4 00 00 00 10[ 	]*vrcpph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4c 31[ 	]*vrcpph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c 71 7f[ 	]*vrcpph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4c 72 80[ 	]*vrcpph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4c b4 f4 00 00 00 10[ 	]*vrcpph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4c 31[ 	]*vrcpph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c 71 7f[ 	]*vrcpph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4c 72 80[ 	]*vrcpph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 f5 7b[ 	]*vreduceph \$0x7b,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 56 f5 7b[ 	]*vreduceph \$0x7b,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 f5 7b[ 	]*vreduceph \$0x7b,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 56 f5 7b[ 	]*vreduceph \$0x7b,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 56 b4 f4 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 31 7b[ 	]*vreduceph \$0x7b,\(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 71 7f 7b[ 	]*vreduceph \$0x7b,0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 56 b4 f4 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 56 31 7b[ 	]*vreduceph \$0x7b,\(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 71 7f 7b[ 	]*vreduceph \$0x7b,0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 f5 7b[ 	]*vrndscaleph \$0x7b,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 08 f5 7b[ 	]*vrndscaleph \$0x7b,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 f5 7b[ 	]*vrndscaleph \$0x7b,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 08 f5 7b[ 	]*vrndscaleph \$0x7b,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 08 b4 f4 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 08 b4 f4 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e f5[ 	]*vrsqrtph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4e f5[ 	]*vrsqrtph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e f5[ 	]*vrsqrtph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4e f5[ 	]*vrsqrtph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4e b4 f4 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4e 31[ 	]*vrsqrtph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e 71 7f[ 	]*vrsqrtph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4e 72 80[ 	]*vrsqrtph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4e b4 f4 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4e 31[ 	]*vrsqrtph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e 71 7f[ 	]*vrsqrtph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4e 72 80[ 	]*vrsqrtph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c f4[ 	]*vscalefph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 2c f4[ 	]*vscalefph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c f4[ 	]*vscalefph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2c f4[ 	]*vscalefph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 2c b4 f4 00 00 00 10[ 	]*vscalefph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 2c 31[ 	]*vscalefph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c 71 7f[ 	]*vscalefph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 2c 72 80[ 	]*vscalefph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2c b4 f4 00 00 00 10[ 	]*vscalefph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c 31[ 	]*vscalefph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c 71 7f[ 	]*vscalefph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c 72 80[ 	]*vscalefph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 f5[ 	]*vsqrtph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 51 f5[ 	]*vsqrtph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 f5[ 	]*vsqrtph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 51 f5[ 	]*vsqrtph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 51 b4 f4 00 00 00 10[ 	]*vsqrtph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 31[ 	]*vsqrtph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 71 7f[ 	]*vsqrtph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 72 80[ 	]*vsqrtph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 51 b4 f4 00 00 00 10[ 	]*vsqrtph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 51 31[ 	]*vsqrtph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 71 7f[ 	]*vsqrtph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 51 72 80[ 	]*vsqrtph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c f4[ 	]*vsubph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5c f4[ 	]*vsubph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c f4[ 	]*vsubph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5c f4[ 	]*vsubph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5c b4 f4 00 00 00 10[ 	]*vsubph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5c 31[ 	]*vsubph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c 71 7f[ 	]*vsubph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5c 72 80[ 	]*vsubph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5c b4 f4 00 00 00 10[ 	]*vsubph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c 31[ 	]*vsubph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c 71 7f[ 	]*vsubph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c 72 80[ 	]*vsubph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 f4[ 	]*vaddph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 58 f4[ 	]*vaddph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 f4[ 	]*vaddph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 58 f4[ 	]*vaddph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 58 b4 f4 00 00 00 10[ 	]*vaddph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 58 31[ 	]*vaddph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 71 7f[ 	]*vaddph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 58 72 80[ 	]*vaddph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 58 b4 f4 00 00 00 10[ 	]*vaddph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 31[ 	]*vaddph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 71 7f[ 	]*vaddph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 72 80[ 	]*vaddph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 ec 7b[ 	]*vcmpph \$0x7b,%ymm4,%ymm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ec 7b[ 	]*vcmpph \$0x7b,%ymm4,%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 ec 7b[ 	]*vcmpph \$0x7b,%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ec 7b[ 	]*vcmpph \$0x7b,%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ac f4 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 29 7b[ 	]*vcmpph \$0x7b,\(%ecx\)\{1to8\},%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0x7f0\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ac f4 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 38 c2 29 7b[ 	]*vcmpph \$0x7b,\(%ecx\)\{1to16\},%ymm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0xfe0\(%ecx\),%ymm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 3f c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b f5[ 	]*vcvtdq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5b f5[ 	]*vcvtdq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b f5[ 	]*vcvtdq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5b f5[ 	]*vcvtdq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5b b4 f4 00 00 00 10[ 	]*vcvtdq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b 31[ 	]*vcvtdq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b 71 7f[ 	]*vcvtdq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b 72 80[ 	]*vcvtdq2ph -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5b 31[ 	]*vcvtdq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b 71 7f[ 	]*vcvtdq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5b 72 80[ 	]*vcvtdq2ph -0x200\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a f5[ 	]*vcvtpd2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 8f 5a f5[ 	]*vcvtpd2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a f5[ 	]*vcvtpd2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd af 5a f5[ 	]*vcvtpd2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 0f 5a b4 f4 00 00 00 10[ 	]*vcvtpd2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a 31[ 	]*vcvtpd2ph \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a 71 7f[ 	]*vcvtpd2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a 72 80[ 	]*vcvtpd2ph -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 38 5a 31[ 	]*vcvtpd2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a 71 7f[ 	]*vcvtpd2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd bf 5a 72 80[ 	]*vcvtpd2ph -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b f5[ 	]*vcvtph2dq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 5b f5[ 	]*vcvtph2dq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b f5[ 	]*vcvtph2dq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 5b f5[ 	]*vcvtph2dq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 5b b4 f4 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b 31[ 	]*vcvtph2dq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b 71 7f[ 	]*vcvtph2dq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b 72 80[ 	]*vcvtph2dq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 5b b4 f4 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 5b 31[ 	]*vcvtph2dq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b 71 7f[ 	]*vcvtph2dq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 5b 72 80[ 	]*vcvtph2dq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a f5[ 	]*vcvtph2pd %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5a f5[ 	]*vcvtph2pd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a f5[ 	]*vcvtph2pd %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5a f5[ 	]*vcvtph2pd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5a b4 f4 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a 31[ 	]*vcvtph2pd \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a 71 7f[ 	]*vcvtph2pd 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a 72 80[ 	]*vcvtph2pd -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 5a b4 f4 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5a 31[ 	]*vcvtph2pd \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a 71 7f[ 	]*vcvtph2pd 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5a 72 80[ 	]*vcvtph2pd -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 f5[ 	]*vcvtph2psx %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 13 f5[ 	]*vcvtph2psx %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 f5[ 	]*vcvtph2psx %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 13 f5[ 	]*vcvtph2psx %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 13 b4 f4 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 31[ 	]*vcvtph2psx \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 71 7f[ 	]*vcvtph2psx 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 72 80[ 	]*vcvtph2psx -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 13 b4 f4 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 13 31[ 	]*vcvtph2psx \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 71 7f[ 	]*vcvtph2psx 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 13 72 80[ 	]*vcvtph2psx -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b f5[ 	]*vcvtph2qq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7b f5[ 	]*vcvtph2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b f5[ 	]*vcvtph2qq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7b f5[ 	]*vcvtph2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7b b4 f4 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b 31[ 	]*vcvtph2qq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b 71 7f[ 	]*vcvtph2qq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b 72 80[ 	]*vcvtph2qq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7b b4 f4 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7b 31[ 	]*vcvtph2qq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b 71 7f[ 	]*vcvtph2qq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7b 72 80[ 	]*vcvtph2qq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 f5[ 	]*vcvtph2udq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 79 f5[ 	]*vcvtph2udq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 f5[ 	]*vcvtph2udq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 79 f5[ 	]*vcvtph2udq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 79 b4 f4 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 31[ 	]*vcvtph2udq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 71 7f[ 	]*vcvtph2udq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 72 80[ 	]*vcvtph2udq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 79 b4 f4 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 79 31[ 	]*vcvtph2udq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 71 7f[ 	]*vcvtph2udq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 79 72 80[ 	]*vcvtph2udq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 f5[ 	]*vcvtph2uqq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 79 f5[ 	]*vcvtph2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 f5[ 	]*vcvtph2uqq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 79 f5[ 	]*vcvtph2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 79 b4 f4 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 31[ 	]*vcvtph2uqq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 71 7f[ 	]*vcvtph2uqq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 72 80[ 	]*vcvtph2uqq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 79 b4 f4 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 79 31[ 	]*vcvtph2uqq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 71 7f[ 	]*vcvtph2uqq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 79 72 80[ 	]*vcvtph2uqq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d f5[ 	]*vcvtph2uw %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7d f5[ 	]*vcvtph2uw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d f5[ 	]*vcvtph2uw %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7d f5[ 	]*vcvtph2uw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7d b4 f4 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d 31[ 	]*vcvtph2uw \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d 71 7f[ 	]*vcvtph2uw 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d 72 80[ 	]*vcvtph2uw -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7d b4 f4 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7d 31[ 	]*vcvtph2uw \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d 71 7f[ 	]*vcvtph2uw 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7d 72 80[ 	]*vcvtph2uw -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d f5[ 	]*vcvtph2w %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7d f5[ 	]*vcvtph2w %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d f5[ 	]*vcvtph2w %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7d f5[ 	]*vcvtph2w %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7d b4 f4 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d 31[ 	]*vcvtph2w \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d 71 7f[ 	]*vcvtph2w 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d 72 80[ 	]*vcvtph2w -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7d b4 f4 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7d 31[ 	]*vcvtph2w \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d 71 7f[ 	]*vcvtph2w 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7d 72 80[ 	]*vcvtph2w -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d f5[ 	]*vcvtps2phx %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 1d f5[ 	]*vcvtps2phx %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d f5[ 	]*vcvtps2phx %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 1d f5[ 	]*vcvtps2phx %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 1d b4 f4 00 00 00 10[ 	]*vcvtps2phxx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d 31[ 	]*vcvtps2phx \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d 71 7f[ 	]*vcvtps2phxx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d 72 80[ 	]*vcvtps2phx -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 1d 31[ 	]*vcvtps2phx \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d 71 7f[ 	]*vcvtps2phxy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 1d 72 80[ 	]*vcvtps2phx -0x200\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b f5[ 	]*vcvtqq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 8f 5b f5[ 	]*vcvtqq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b f5[ 	]*vcvtqq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc af 5b f5[ 	]*vcvtqq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 0f 5b b4 f4 00 00 00 10[ 	]*vcvtqq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b 31[ 	]*vcvtqq2ph \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b 71 7f[ 	]*vcvtqq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b 72 80[ 	]*vcvtqq2ph -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 38 5b 31[ 	]*vcvtqq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b 71 7f[ 	]*vcvtqq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc bf 5b 72 80[ 	]*vcvtqq2ph -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b f5[ 	]*vcvttph2dq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 5b f5[ 	]*vcvttph2dq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b f5[ 	]*vcvttph2dq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 5b f5[ 	]*vcvttph2dq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 5b b4 f4 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b 31[ 	]*vcvttph2dq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b 71 7f[ 	]*vcvttph2dq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b 72 80[ 	]*vcvttph2dq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 5b b4 f4 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 5b 31[ 	]*vcvttph2dq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b 71 7f[ 	]*vcvttph2dq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 5b 72 80[ 	]*vcvttph2dq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a f5[ 	]*vcvttph2qq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7a f5[ 	]*vcvttph2qq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a f5[ 	]*vcvttph2qq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7a f5[ 	]*vcvttph2qq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7a b4 f4 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a 31[ 	]*vcvttph2qq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a 71 7f[ 	]*vcvttph2qq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a 72 80[ 	]*vcvttph2qq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7a b4 f4 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7a 31[ 	]*vcvttph2qq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a 71 7f[ 	]*vcvttph2qq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7a 72 80[ 	]*vcvttph2qq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 f5[ 	]*vcvttph2udq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 78 f5[ 	]*vcvttph2udq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 f5[ 	]*vcvttph2udq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 78 f5[ 	]*vcvttph2udq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 78 b4 f4 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 31[ 	]*vcvttph2udq \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 71 7f[ 	]*vcvttph2udq 0x3f8\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 72 80[ 	]*vcvttph2udq -0x100\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 78 b4 f4 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 78 31[ 	]*vcvttph2udq \(%ecx\)\{1to8\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 71 7f[ 	]*vcvttph2udq 0x7f0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 78 72 80[ 	]*vcvttph2udq -0x100\(%edx\)\{1to8\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 f5[ 	]*vcvttph2uqq %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 78 f5[ 	]*vcvttph2uqq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 f5[ 	]*vcvttph2uqq %xmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 78 f5[ 	]*vcvttph2uqq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 78 b4 f4 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 31[ 	]*vcvttph2uqq \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 71 7f[ 	]*vcvttph2uqq 0x1fc\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 72 80[ 	]*vcvttph2uqq -0x100\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 78 b4 f4 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 78 31[ 	]*vcvttph2uqq \(%ecx\)\{1to4\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 71 7f[ 	]*vcvttph2uqq 0x3f8\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 78 72 80[ 	]*vcvttph2uqq -0x100\(%edx\)\{1to4\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c f5[ 	]*vcvttph2uw %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7c f5[ 	]*vcvttph2uw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c f5[ 	]*vcvttph2uw %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7c f5[ 	]*vcvttph2uw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7c b4 f4 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c 31[ 	]*vcvttph2uw \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c 71 7f[ 	]*vcvttph2uw 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c 72 80[ 	]*vcvttph2uw -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7c b4 f4 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7c 31[ 	]*vcvttph2uw \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c 71 7f[ 	]*vcvttph2uw 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7c 72 80[ 	]*vcvttph2uw -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c f5[ 	]*vcvttph2w %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7c f5[ 	]*vcvttph2w %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c f5[ 	]*vcvttph2w %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7c f5[ 	]*vcvttph2w %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7c b4 f4 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c 31[ 	]*vcvttph2w \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c 71 7f[ 	]*vcvttph2w 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c 72 80[ 	]*vcvttph2w -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7c b4 f4 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7c 31[ 	]*vcvttph2w \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c 71 7f[ 	]*vcvttph2w 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7c 72 80[ 	]*vcvttph2w -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a f5[ 	]*vcvtudq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7a f5[ 	]*vcvtudq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a f5[ 	]*vcvtudq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7a f5[ 	]*vcvtudq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7a b4 f4 00 00 00 10[ 	]*vcvtudq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a 31[ 	]*vcvtudq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a 71 7f[ 	]*vcvtudq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a 72 80[ 	]*vcvtudq2ph -0x200\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7a 31[ 	]*vcvtudq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a 71 7f[ 	]*vcvtudq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7a 72 80[ 	]*vcvtudq2ph -0x200\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a f5[ 	]*vcvtuqq2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 8f 7a f5[ 	]*vcvtuqq2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a f5[ 	]*vcvtuqq2ph %ymm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff af 7a f5[ 	]*vcvtuqq2ph %ymm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 0f 7a b4 f4 00 00 00 10[ 	]*vcvtuqq2phx 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a 31[ 	]*vcvtuqq2ph \(%ecx\)\{1to2\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a 71 7f[ 	]*vcvtuqq2phx 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%edx\)\{1to2\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 38 7a 31[ 	]*vcvtuqq2ph \(%ecx\)\{1to4\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a 71 7f[ 	]*vcvtuqq2phy 0xfe0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff bf 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%edx\)\{1to4\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d f5[ 	]*vcvtuw2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7d f5[ 	]*vcvtuw2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d f5[ 	]*vcvtuw2ph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7d f5[ 	]*vcvtuw2ph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7d b4 f4 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d 31[ 	]*vcvtuw2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d 71 7f[ 	]*vcvtuw2ph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d 72 80[ 	]*vcvtuw2ph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 2f 7d b4 f4 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7d 31[ 	]*vcvtuw2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d 71 7f[ 	]*vcvtuw2ph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7d 72 80[ 	]*vcvtuw2ph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d f5[ 	]*vcvtw2ph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 7d f5[ 	]*vcvtw2ph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d f5[ 	]*vcvtw2ph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 7d f5[ 	]*vcvtw2ph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 7d b4 f4 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d 31[ 	]*vcvtw2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d 71 7f[ 	]*vcvtw2ph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d 72 80[ 	]*vcvtw2ph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 7d b4 f4 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 7d 31[ 	]*vcvtw2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d 71 7f[ 	]*vcvtw2ph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 7d 72 80[ 	]*vcvtw2ph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e f4[ 	]*vdivph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5e f4[ 	]*vdivph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e f4[ 	]*vdivph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5e f4[ 	]*vdivph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5e b4 f4 00 00 00 10[ 	]*vdivph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5e 31[ 	]*vdivph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e 71 7f[ 	]*vdivph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5e 72 80[ 	]*vdivph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5e b4 f4 00 00 00 10[ 	]*vdivph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e 31[ 	]*vdivph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e 71 7f[ 	]*vdivph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e 72 80[ 	]*vdivph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 f4[ 	]*vfcmaddcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af 56 f4[ 	]*vfcmaddcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 f4[ 	]*vfcmaddcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 56 f4[ 	]*vfcmaddcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f 56 b4 f4 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 56 31[ 	]*vfcmaddcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 71 7f[ 	]*vfcmaddcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf 56 72 80[ 	]*vfcmaddcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 56 b4 f4 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 31[ 	]*vfcmaddcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 71 7f[ 	]*vfcmaddcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 72 80[ 	]*vfcmaddcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 f4[ 	]*vfcmulcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af d6 f4[ 	]*vfcmulcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 f4[ 	]*vfcmulcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d6 f4[ 	]*vfcmulcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f d6 b4 f4 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 d6 31[ 	]*vfcmulcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 71 7f[ 	]*vfcmulcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf d6 72 80[ 	]*vfcmulcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d6 b4 f4 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 31[ 	]*vfcmulcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 71 7f[ 	]*vfcmulcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 72 80[ 	]*vfcmulcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 f4[ 	]*vfmadd132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 98 f4[ 	]*vfmadd132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 f4[ 	]*vfmadd132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 98 f4[ 	]*vfmadd132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 98 b4 f4 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 98 31[ 	]*vfmadd132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 71 7f[ 	]*vfmadd132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 98 72 80[ 	]*vfmadd132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 98 b4 f4 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 31[ 	]*vfmadd132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 71 7f[ 	]*vfmadd132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 72 80[ 	]*vfmadd132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 f4[ 	]*vfmadd213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a8 f4[ 	]*vfmadd213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 f4[ 	]*vfmadd213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a8 f4[ 	]*vfmadd213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a8 b4 f4 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a8 31[ 	]*vfmadd213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 71 7f[ 	]*vfmadd213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a8 72 80[ 	]*vfmadd213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a8 b4 f4 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 31[ 	]*vfmadd213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 71 7f[ 	]*vfmadd213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 72 80[ 	]*vfmadd213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 f4[ 	]*vfmadd231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b8 f4[ 	]*vfmadd231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 f4[ 	]*vfmadd231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b8 f4[ 	]*vfmadd231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b8 b4 f4 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b8 31[ 	]*vfmadd231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 71 7f[ 	]*vfmadd231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b8 72 80[ 	]*vfmadd231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b8 b4 f4 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 31[ 	]*vfmadd231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 71 7f[ 	]*vfmadd231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 72 80[ 	]*vfmadd231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 f4[ 	]*vfmaddcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af 56 f4[ 	]*vfmaddcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 f4[ 	]*vfmaddcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 56 f4[ 	]*vfmaddcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f 56 b4 f4 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 56 31[ 	]*vfmaddcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 71 7f[ 	]*vfmaddcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf 56 72 80[ 	]*vfmaddcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 56 b4 f4 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 31[ 	]*vfmaddcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 71 7f[ 	]*vfmaddcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 72 80[ 	]*vfmaddcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 f4[ 	]*vfmaddsub132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 96 f4[ 	]*vfmaddsub132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 f4[ 	]*vfmaddsub132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 96 f4[ 	]*vfmaddsub132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 96 b4 f4 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 96 31[ 	]*vfmaddsub132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 71 7f[ 	]*vfmaddsub132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 96 72 80[ 	]*vfmaddsub132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 96 b4 f4 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 31[ 	]*vfmaddsub132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 71 7f[ 	]*vfmaddsub132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 72 80[ 	]*vfmaddsub132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 f4[ 	]*vfmaddsub213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a6 f4[ 	]*vfmaddsub213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 f4[ 	]*vfmaddsub213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a6 f4[ 	]*vfmaddsub213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a6 b4 f4 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a6 31[ 	]*vfmaddsub213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 71 7f[ 	]*vfmaddsub213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a6 72 80[ 	]*vfmaddsub213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a6 b4 f4 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 31[ 	]*vfmaddsub213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 71 7f[ 	]*vfmaddsub213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 72 80[ 	]*vfmaddsub213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 f4[ 	]*vfmaddsub231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b6 f4[ 	]*vfmaddsub231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 f4[ 	]*vfmaddsub231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b6 f4[ 	]*vfmaddsub231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b6 b4 f4 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b6 31[ 	]*vfmaddsub231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 71 7f[ 	]*vfmaddsub231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b6 72 80[ 	]*vfmaddsub231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b6 b4 f4 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 31[ 	]*vfmaddsub231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 71 7f[ 	]*vfmaddsub231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 72 80[ 	]*vfmaddsub231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a f4[ 	]*vfmsub132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9a f4[ 	]*vfmsub132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a f4[ 	]*vfmsub132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9a f4[ 	]*vfmsub132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9a b4 f4 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9a 31[ 	]*vfmsub132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a 71 7f[ 	]*vfmsub132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9a 72 80[ 	]*vfmsub132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9a b4 f4 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a 31[ 	]*vfmsub132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a 71 7f[ 	]*vfmsub132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a 72 80[ 	]*vfmsub132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa f4[ 	]*vfmsub213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af aa f4[ 	]*vfmsub213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa f4[ 	]*vfmsub213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f aa f4[ 	]*vfmsub213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f aa b4 f4 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 aa 31[ 	]*vfmsub213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa 71 7f[ 	]*vfmsub213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf aa 72 80[ 	]*vfmsub213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f aa b4 f4 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa 31[ 	]*vfmsub213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa 71 7f[ 	]*vfmsub213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa 72 80[ 	]*vfmsub213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba f4[ 	]*vfmsub231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ba f4[ 	]*vfmsub231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba f4[ 	]*vfmsub231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ba f4[ 	]*vfmsub231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ba b4 f4 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ba 31[ 	]*vfmsub231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba 71 7f[ 	]*vfmsub231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ba 72 80[ 	]*vfmsub231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ba b4 f4 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba 31[ 	]*vfmsub231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba 71 7f[ 	]*vfmsub231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba 72 80[ 	]*vfmsub231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 f4[ 	]*vfmsubadd132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 97 f4[ 	]*vfmsubadd132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 f4[ 	]*vfmsubadd132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 97 f4[ 	]*vfmsubadd132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 97 b4 f4 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 97 31[ 	]*vfmsubadd132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 71 7f[ 	]*vfmsubadd132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 97 72 80[ 	]*vfmsubadd132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 97 b4 f4 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 31[ 	]*vfmsubadd132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 71 7f[ 	]*vfmsubadd132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 72 80[ 	]*vfmsubadd132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 f4[ 	]*vfmsubadd213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a7 f4[ 	]*vfmsubadd213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 f4[ 	]*vfmsubadd213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a7 f4[ 	]*vfmsubadd213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a7 b4 f4 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a7 31[ 	]*vfmsubadd213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 71 7f[ 	]*vfmsubadd213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a7 72 80[ 	]*vfmsubadd213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a7 b4 f4 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 31[ 	]*vfmsubadd213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 71 7f[ 	]*vfmsubadd213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 72 80[ 	]*vfmsubadd213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 f4[ 	]*vfmsubadd231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b7 f4[ 	]*vfmsubadd231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 f4[ 	]*vfmsubadd231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b7 f4[ 	]*vfmsubadd231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b7 b4 f4 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b7 31[ 	]*vfmsubadd231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 71 7f[ 	]*vfmsubadd231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b7 72 80[ 	]*vfmsubadd231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b7 b4 f4 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 31[ 	]*vfmsubadd231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 71 7f[ 	]*vfmsubadd231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 72 80[ 	]*vfmsubadd231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 f4[ 	]*vfmulcph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af d6 f4[ 	]*vfmulcph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 f4[ 	]*vfmulcph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d6 f4[ 	]*vfmulcph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f d6 b4 f4 00 00 00 10[ 	]*vfmulcph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 d6 31[ 	]*vfmulcph \(%ecx\)\{1to8\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 71 7f[ 	]*vfmulcph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf d6 72 80[ 	]*vfmulcph -0x200\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d6 b4 f4 00 00 00 10[ 	]*vfmulcph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 31[ 	]*vfmulcph \(%ecx\)\{1to4\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 71 7f[ 	]*vfmulcph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 72 80[ 	]*vfmulcph -0x200\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c f4[ 	]*vfnmadd132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9c f4[ 	]*vfnmadd132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c f4[ 	]*vfnmadd132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9c f4[ 	]*vfnmadd132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9c b4 f4 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9c 31[ 	]*vfnmadd132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c 71 7f[ 	]*vfnmadd132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9c 72 80[ 	]*vfnmadd132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9c b4 f4 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c 31[ 	]*vfnmadd132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c 71 7f[ 	]*vfnmadd132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c 72 80[ 	]*vfnmadd132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac f4[ 	]*vfnmadd213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ac f4[ 	]*vfnmadd213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac f4[ 	]*vfnmadd213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ac f4[ 	]*vfnmadd213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ac b4 f4 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ac 31[ 	]*vfnmadd213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac 71 7f[ 	]*vfnmadd213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ac 72 80[ 	]*vfnmadd213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ac b4 f4 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac 31[ 	]*vfnmadd213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac 71 7f[ 	]*vfnmadd213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac 72 80[ 	]*vfnmadd213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc f4[ 	]*vfnmadd231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af bc f4[ 	]*vfnmadd231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc f4[ 	]*vfnmadd231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bc f4[ 	]*vfnmadd231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f bc b4 f4 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 bc 31[ 	]*vfnmadd231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc 71 7f[ 	]*vfnmadd231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf bc 72 80[ 	]*vfnmadd231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bc b4 f4 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc 31[ 	]*vfnmadd231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc 71 7f[ 	]*vfnmadd231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc 72 80[ 	]*vfnmadd231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e f4[ 	]*vfnmsub132ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9e f4[ 	]*vfnmsub132ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e f4[ 	]*vfnmsub132ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9e f4[ 	]*vfnmsub132ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9e b4 f4 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9e 31[ 	]*vfnmsub132ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e 71 7f[ 	]*vfnmsub132ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9e 72 80[ 	]*vfnmsub132ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9e b4 f4 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e 31[ 	]*vfnmsub132ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e 71 7f[ 	]*vfnmsub132ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e 72 80[ 	]*vfnmsub132ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae f4[ 	]*vfnmsub213ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ae f4[ 	]*vfnmsub213ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae f4[ 	]*vfnmsub213ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ae f4[ 	]*vfnmsub213ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ae b4 f4 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ae 31[ 	]*vfnmsub213ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae 71 7f[ 	]*vfnmsub213ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ae 72 80[ 	]*vfnmsub213ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ae b4 f4 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae 31[ 	]*vfnmsub213ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae 71 7f[ 	]*vfnmsub213ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae 72 80[ 	]*vfnmsub213ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be f4[ 	]*vfnmsub231ph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af be f4[ 	]*vfnmsub231ph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be f4[ 	]*vfnmsub231ph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f be f4[ 	]*vfnmsub231ph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f be b4 f4 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 be 31[ 	]*vfnmsub231ph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be 71 7f[ 	]*vfnmsub231ph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf be 72 80[ 	]*vfnmsub231ph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f be b4 f4 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be 31[ 	]*vfnmsub231ph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be 71 7f[ 	]*vfnmsub231ph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be 72 80[ 	]*vfnmsub231ph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 ee 7b[ 	]*vfpclassph \$0x7b,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ee 7b[ 	]*vfpclassph \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 ee 7b[ 	]*vfpclassph \$0x7b,%ymm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 66 ee 7b[ 	]*vfpclassph \$0x7b,%ymm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ac f4 00 00 00 10 7b[ 	]*vfpclassphx \$0x7b,0x10000000\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 66 29 7b[ 	]*vfpclassph \$0x7b,\(%ecx\)\{1to8\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 69 7f 7b[ 	]*vfpclassphx \$0x7b,0x7f0\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 1f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%edx\)\{1to8\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 66 29 7b[ 	]*vfpclassph \$0x7b,\(%ecx\)\{1to16\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 69 7f 7b[ 	]*vfpclassphy \$0x7b,0xfe0\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 3f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%edx\)\{1to16\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 f5[ 	]*vgetexpph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 42 f5[ 	]*vgetexpph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 f5[ 	]*vgetexpph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 42 f5[ 	]*vgetexpph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 42 b4 f4 00 00 00 10[ 	]*vgetexpph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 31[ 	]*vgetexpph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 71 7f[ 	]*vgetexpph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 72 80[ 	]*vgetexpph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 42 b4 f4 00 00 00 10[ 	]*vgetexpph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 42 31[ 	]*vgetexpph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 71 7f[ 	]*vgetexpph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 42 72 80[ 	]*vgetexpph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 f5 7b[ 	]*vgetmantph \$0x7b,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 26 f5 7b[ 	]*vgetmantph \$0x7b,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 f5 7b[ 	]*vgetmantph \$0x7b,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 26 f5 7b[ 	]*vgetmantph \$0x7b,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 26 b4 f4 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 31 7b[ 	]*vgetmantph \$0x7b,\(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 26 b4 f4 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 26 31 7b[ 	]*vgetmantph \$0x7b,\(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f f4[ 	]*vmaxph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5f f4[ 	]*vmaxph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f f4[ 	]*vmaxph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5f f4[ 	]*vmaxph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5f b4 f4 00 00 00 10[ 	]*vmaxph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5f 31[ 	]*vmaxph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f 71 7f[ 	]*vmaxph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5f 72 80[ 	]*vmaxph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5f b4 f4 00 00 00 10[ 	]*vmaxph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f 31[ 	]*vmaxph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f 71 7f[ 	]*vmaxph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f 72 80[ 	]*vmaxph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d f4[ 	]*vminph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5d f4[ 	]*vminph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d f4[ 	]*vminph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5d f4[ 	]*vminph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5d b4 f4 00 00 00 10[ 	]*vminph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5d 31[ 	]*vminph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d 71 7f[ 	]*vminph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5d 72 80[ 	]*vminph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5d b4 f4 00 00 00 10[ 	]*vminph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d 31[ 	]*vminph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d 71 7f[ 	]*vminph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d 72 80[ 	]*vminph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 f4[ 	]*vmulph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 59 f4[ 	]*vmulph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 f4[ 	]*vmulph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 59 f4[ 	]*vmulph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 59 b4 f4 00 00 00 10[ 	]*vmulph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 59 31[ 	]*vmulph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 71 7f[ 	]*vmulph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 59 72 80[ 	]*vmulph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 59 b4 f4 00 00 00 10[ 	]*vmulph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 31[ 	]*vmulph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 71 7f[ 	]*vmulph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 72 80[ 	]*vmulph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c f5[ 	]*vrcpph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4c f5[ 	]*vrcpph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c f5[ 	]*vrcpph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4c f5[ 	]*vrcpph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4c b4 f4 00 00 00 10[ 	]*vrcpph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4c 31[ 	]*vrcpph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c 71 7f[ 	]*vrcpph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4c 72 80[ 	]*vrcpph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4c b4 f4 00 00 00 10[ 	]*vrcpph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4c 31[ 	]*vrcpph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c 71 7f[ 	]*vrcpph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4c 72 80[ 	]*vrcpph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 f5 7b[ 	]*vreduceph \$0x7b,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 56 f5 7b[ 	]*vreduceph \$0x7b,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 f5 7b[ 	]*vreduceph \$0x7b,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 56 f5 7b[ 	]*vreduceph \$0x7b,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 56 b4 f4 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 31 7b[ 	]*vreduceph \$0x7b,\(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 71 7f 7b[ 	]*vreduceph \$0x7b,0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 56 b4 f4 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 56 31 7b[ 	]*vreduceph \$0x7b,\(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 71 7f 7b[ 	]*vreduceph \$0x7b,0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 f5 7b[ 	]*vrndscaleph \$0x7b,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 08 f5 7b[ 	]*vrndscaleph \$0x7b,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 f5 7b[ 	]*vrndscaleph \$0x7b,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 08 f5 7b[ 	]*vrndscaleph \$0x7b,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 08 b4 f4 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 08 b4 f4 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e f5[ 	]*vrsqrtph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4e f5[ 	]*vrsqrtph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e f5[ 	]*vrsqrtph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4e f5[ 	]*vrsqrtph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4e b4 f4 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4e 31[ 	]*vrsqrtph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e 71 7f[ 	]*vrsqrtph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4e 72 80[ 	]*vrsqrtph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4e b4 f4 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4e 31[ 	]*vrsqrtph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e 71 7f[ 	]*vrsqrtph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4e 72 80[ 	]*vrsqrtph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c f4[ 	]*vscalefph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 2c f4[ 	]*vscalefph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c f4[ 	]*vscalefph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2c f4[ 	]*vscalefph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 2c b4 f4 00 00 00 10[ 	]*vscalefph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 2c 31[ 	]*vscalefph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c 71 7f[ 	]*vscalefph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 2c 72 80[ 	]*vscalefph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2c b4 f4 00 00 00 10[ 	]*vscalefph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c 31[ 	]*vscalefph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c 71 7f[ 	]*vscalefph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c 72 80[ 	]*vscalefph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 f5[ 	]*vsqrtph %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 51 f5[ 	]*vsqrtph %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 f5[ 	]*vsqrtph %ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 51 f5[ 	]*vsqrtph %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 51 b4 f4 00 00 00 10[ 	]*vsqrtph 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 31[ 	]*vsqrtph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 71 7f[ 	]*vsqrtph 0x7f0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 72 80[ 	]*vsqrtph -0x100\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 51 b4 f4 00 00 00 10[ 	]*vsqrtph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 51 31[ 	]*vsqrtph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 71 7f[ 	]*vsqrtph 0xfe0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 51 72 80[ 	]*vsqrtph -0x100\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c f4[ 	]*vsubph %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5c f4[ 	]*vsubph %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c f4[ 	]*vsubph %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5c f4[ 	]*vsubph %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5c b4 f4 00 00 00 10[ 	]*vsubph 0x10000000\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5c 31[ 	]*vsubph \(%ecx\)\{1to16\},%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c 71 7f[ 	]*vsubph 0xfe0\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5c 72 80[ 	]*vsubph -0x100\(%edx\)\{1to16\},%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5c b4 f4 00 00 00 10[ 	]*vsubph 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c 31[ 	]*vsubph \(%ecx\)\{1to8\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c 71 7f[ 	]*vsubph 0x7f0\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c 72 80[ 	]*vsubph -0x100\(%edx\)\{1to8\},%xmm5,%xmm6\{%k7\}\{z\}
#pass
