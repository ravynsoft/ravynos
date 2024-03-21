#as:
#objdump: -dw
#name: i386 AVX512-FP16 insns
#source: avx512_fp16.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 f4[ 	]*vaddph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 f4[ 	]*vaddph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 f4[ 	]*vaddph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 58 b4 f4 00 00 00 10[ 	]*vaddph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 58 31[ 	]*vaddph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 71 7f[ 	]*vaddph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 58 72 80[ 	]*vaddph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 f4[ 	]*vaddsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 58 f4[ 	]*vaddsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 58 f4[ 	]*vaddsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 58 b4 f4 00 00 00 10[ 	]*vaddsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 31[ 	]*vaddsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 71 7f[ 	]*vaddsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 58 72 80[ 	]*vaddsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 ec 7b[ 	]*vcmpph \$0x7b,%zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm4,%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 4f c2 ac f4 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 58 c2 29 7b[ 	]*vcmpph \$0x7b,\(%ecx\)\{1to32\},%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0x1fc0\(%ecx\),%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 5f c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 ec 7b[ 	]*vcmpsh \$0x7b,%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 18 c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 00 00 00 10 7b[ 	]*vcmpsh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 29 7b[ 	]*vcmpsh \$0x7b,\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 69 7f 7b[ 	]*vcmpsh \$0x7b,0xfe\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 6a 80 7b[ 	]*vcmpsh \$0x7b,-0x100\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f f5[ 	]*vcomish %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2f f5[ 	]*vcomish \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f b4 f4 00 00 00 10[ 	]*vcomish 0x10000000\(%esp,%esi,8\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 31[ 	]*vcomish \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 71 7f[ 	]*vcomish 0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 72 80[ 	]*vcomish -0x100\(%edx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b f5[ 	]*vcvtdq2ph %zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5b b4 f4 00 00 00 10[ 	]*vcvtdq2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5b 31[ 	]*vcvtdq2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b 71 7f[ 	]*vcvtdq2ph 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5b 72 80[ 	]*vcvtdq2ph -0x200\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a f5[ 	]*vcvtpd2ph %zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 4f 5a b4 f4 00 00 00 10[ 	]*vcvtpd2phz 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 58 5a 31[ 	]*vcvtpd2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a 71 7f[ 	]*vcvtpd2phz 0x1fc0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd df 5a 72 80[ 	]*vcvtpd2ph -0x400\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b f5[ 	]*vcvtph2dq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 5b b4 f4 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 5b 31[ 	]*vcvtph2dq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b 71 7f[ 	]*vcvtph2dq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 5b 72 80[ 	]*vcvtph2dq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a f5[ 	]*vcvtph2pd %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a f5[ 	]*vcvtph2pd \{sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a f5[ 	]*vcvtph2pd \{sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5a b4 f4 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5a 31[ 	]*vcvtph2pd \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a 71 7f[ 	]*vcvtph2pd 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5a 72 80[ 	]*vcvtph2pd -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 f5[ 	]*vcvtph2psx %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 f5[ 	]*vcvtph2psx \{sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 f5[ 	]*vcvtph2psx \{sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 13 b4 f4 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 13 31[ 	]*vcvtph2psx \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 71 7f[ 	]*vcvtph2psx 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 13 72 80[ 	]*vcvtph2psx -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b f5[ 	]*vcvtph2qq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7b b4 f4 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7b 31[ 	]*vcvtph2qq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b 71 7f[ 	]*vcvtph2qq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7b 72 80[ 	]*vcvtph2qq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 f5[ 	]*vcvtph2udq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 79 b4 f4 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 79 31[ 	]*vcvtph2udq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 71 7f[ 	]*vcvtph2udq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 79 72 80[ 	]*vcvtph2udq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 f5[ 	]*vcvtph2uqq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 79 b4 f4 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 79 31[ 	]*vcvtph2uqq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 71 7f[ 	]*vcvtph2uqq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 79 72 80[ 	]*vcvtph2uqq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d f5[ 	]*vcvtph2uw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7d b4 f4 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7d 31[ 	]*vcvtph2uw \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d 71 7f[ 	]*vcvtph2uw 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7d 72 80[ 	]*vcvtph2uw -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d f5[ 	]*vcvtph2w %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7d b4 f4 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7d 31[ 	]*vcvtph2w \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d 71 7f[ 	]*vcvtph2w 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7d 72 80[ 	]*vcvtph2w -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d f5[ 	]*vcvtps2phx %zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 1d b4 f4 00 00 00 10[ 	]*vcvtps2phx 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 1d 31[ 	]*vcvtps2phx \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d 71 7f[ 	]*vcvtps2phx 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 1d 72 80[ 	]*vcvtps2phx -0x200\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b f5[ 	]*vcvtqq2ph %zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 4f 5b b4 f4 00 00 00 10[ 	]*vcvtqq2phz 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 58 5b 31[ 	]*vcvtqq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b 71 7f[ 	]*vcvtqq2phz 0x1fc0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc df 5b 72 80[ 	]*vcvtqq2ph -0x400\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a f4[ 	]*vcvtsd2sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 18 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 9f 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 0f 5a b4 f4 00 00 00 10[ 	]*vcvtsd2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 31[ 	]*vcvtsd2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 71 7f[ 	]*vcvtsd2sh 0x3f8\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 8f 5a 72 80[ 	]*vcvtsd2sh -0x400\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a f4[ 	]*vcvtsh2sd %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5a b4 f4 00 00 00 10[ 	]*vcvtsh2sd 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 31[ 	]*vcvtsh2sd \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 71 7f[ 	]*vcvtsh2sd 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5a 72 80[ 	]*vcvtsh2sd -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d d6[ 	]*vcvtsh2si %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2d d6[ 	]*vcvtsh2si \{rn-sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 94 f4 00 00 00 10[ 	]*vcvtsh2si 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 11[ 	]*vcvtsh2si \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 51 7f[ 	]*vcvtsh2si 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 52 80[ 	]*vcvtsh2si -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 f4[ 	]*vcvtsh2ss %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 18 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 9f 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 0f 13 b4 f4 00 00 00 10[ 	]*vcvtsh2ss 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 31[ 	]*vcvtsh2ss \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 71 7f[ 	]*vcvtsh2ss 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 8f 13 72 80[ 	]*vcvtsh2ss -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 d6[ 	]*vcvtsh2usi %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 79 d6[ 	]*vcvtsh2usi \{rn-sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 94 f4 00 00 00 10[ 	]*vcvtsh2usi 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 11[ 	]*vcvtsh2usi \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 51 7f[ 	]*vcvtsh2usi 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 52 80[ 	]*vcvtsh2usi -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a f2[ 	]*vcvtsi2sh %edx,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 2a f2[ 	]*vcvtsi2sh %edx,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a b4 f4 00 00 00 10[ 	]*vcvtsi2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 31[ 	]*vcvtsi2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 71 7f[ 	]*vcvtsi2sh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 72 80[ 	]*vcvtsi2sh -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d f4[ 	]*vcvtss2sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 1d b4 f4 00 00 00 10[ 	]*vcvtss2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 31[ 	]*vcvtss2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 71 7f[ 	]*vcvtss2sh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 1d 72 80[ 	]*vcvtss2sh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b f5[ 	]*vcvttph2dq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b f5[ 	]*vcvttph2dq \{sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b f5[ 	]*vcvttph2dq \{sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 5b b4 f4 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 5b 31[ 	]*vcvttph2dq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b 71 7f[ 	]*vcvttph2dq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 5b 72 80[ 	]*vcvttph2dq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a f5[ 	]*vcvttph2qq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a f5[ 	]*vcvttph2qq \{sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a f5[ 	]*vcvttph2qq \{sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7a b4 f4 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7a 31[ 	]*vcvttph2qq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a 71 7f[ 	]*vcvttph2qq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7a 72 80[ 	]*vcvttph2qq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 f5[ 	]*vcvttph2udq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 f5[ 	]*vcvttph2udq \{sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 f5[ 	]*vcvttph2udq \{sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 78 b4 f4 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 78 31[ 	]*vcvttph2udq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 71 7f[ 	]*vcvttph2udq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 78 72 80[ 	]*vcvttph2udq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 f5[ 	]*vcvttph2uqq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 78 b4 f4 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 78 31[ 	]*vcvttph2uqq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 71 7f[ 	]*vcvttph2uqq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 78 72 80[ 	]*vcvttph2uqq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c f5[ 	]*vcvttph2uw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c f5[ 	]*vcvttph2uw \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c f5[ 	]*vcvttph2uw \{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7c b4 f4 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7c 31[ 	]*vcvttph2uw \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c 71 7f[ 	]*vcvttph2uw 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7c 72 80[ 	]*vcvttph2uw -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c f5[ 	]*vcvttph2w %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c f5[ 	]*vcvttph2w \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c f5[ 	]*vcvttph2w \{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7c b4 f4 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7c 31[ 	]*vcvttph2w \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c 71 7f[ 	]*vcvttph2w 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7c 72 80[ 	]*vcvttph2w -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c d6[ 	]*vcvttsh2si %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2c d6[ 	]*vcvttsh2si \{sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 94 f4 00 00 00 10[ 	]*vcvttsh2si 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 11[ 	]*vcvttsh2si \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 51 7f[ 	]*vcvttsh2si 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 52 80[ 	]*vcvttsh2si -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 d6[ 	]*vcvttsh2usi %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 78 d6[ 	]*vcvttsh2usi \{sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 94 f4 00 00 00 10[ 	]*vcvttsh2usi 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 11[ 	]*vcvttsh2usi \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 51 7f[ 	]*vcvttsh2usi 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 52 80[ 	]*vcvttsh2usi -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a f5[ 	]*vcvtudq2ph %zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7a b4 f4 00 00 00 10[ 	]*vcvtudq2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7a 31[ 	]*vcvtudq2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a 71 7f[ 	]*vcvtudq2ph 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7a 72 80[ 	]*vcvtudq2ph -0x200\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a f5[ 	]*vcvtuqq2ph %zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 4f 7a b4 f4 00 00 00 10[ 	]*vcvtuqq2phz 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 58 7a 31[ 	]*vcvtuqq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a 71 7f[ 	]*vcvtuqq2phz 0x1fc0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff df 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b f2[ 	]*vcvtusi2sh %edx,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 7b f2[ 	]*vcvtusi2sh %edx,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b b4 f4 00 00 00 10[ 	]*vcvtusi2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 31[ 	]*vcvtusi2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 71 7f[ 	]*vcvtusi2sh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 72 80[ 	]*vcvtusi2sh -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d f5[ 	]*vcvtuw2ph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7d b4 f4 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7d 31[ 	]*vcvtuw2ph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d 71 7f[ 	]*vcvtuw2ph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7d 72 80[ 	]*vcvtuw2ph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d f5[ 	]*vcvtw2ph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 7d b4 f4 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 7d 31[ 	]*vcvtw2ph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d 71 7f[ 	]*vcvtw2ph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 7d 72 80[ 	]*vcvtw2ph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e f4[ 	]*vdivph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e f4[ 	]*vdivph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e f4[ 	]*vdivph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5e b4 f4 00 00 00 10[ 	]*vdivph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5e 31[ 	]*vdivph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e 71 7f[ 	]*vdivph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5e 72 80[ 	]*vdivph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e f4[ 	]*vdivsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5e f4[ 	]*vdivsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5e f4[ 	]*vdivsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5e b4 f4 00 00 00 10[ 	]*vdivsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 31[ 	]*vdivsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 71 7f[ 	]*vdivsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5e 72 80[ 	]*vdivsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 f4[ 	]*vfcmaddcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f 56 b4 f4 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 56 31[ 	]*vfcmaddcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 71 7f[ 	]*vfcmaddcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df 56 72 80[ 	]*vfcmaddcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 f4[ 	]*vfcmaddcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 57 b4 f4 00 00 00 10[ 	]*vfcmaddcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 31[ 	]*vfcmaddcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 71 7f[ 	]*vfcmaddcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 57 72 80[ 	]*vfcmaddcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 f4[ 	]*vfcmulcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f d6 b4 f4 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 d6 31[ 	]*vfcmulcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 71 7f[ 	]*vfcmulcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df d6 72 80[ 	]*vfcmulcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 f4[ 	]*vfcmulcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d7 b4 f4 00 00 00 10[ 	]*vfcmulcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 31[ 	]*vfcmulcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 71 7f[ 	]*vfcmulcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d7 72 80[ 	]*vfcmulcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 f4[ 	]*vfmadd132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 98 b4 f4 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 98 31[ 	]*vfmadd132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 71 7f[ 	]*vfmadd132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 98 72 80[ 	]*vfmadd132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 f4[ 	]*vfmadd132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 99 b4 f4 00 00 00 10[ 	]*vfmadd132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 31[ 	]*vfmadd132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 71 7f[ 	]*vfmadd132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 99 72 80[ 	]*vfmadd132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 f4[ 	]*vfmadd213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a8 b4 f4 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a8 31[ 	]*vfmadd213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 71 7f[ 	]*vfmadd213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a8 72 80[ 	]*vfmadd213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 f4[ 	]*vfmadd213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a9 b4 f4 00 00 00 10[ 	]*vfmadd213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 31[ 	]*vfmadd213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 71 7f[ 	]*vfmadd213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a9 72 80[ 	]*vfmadd213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 f4[ 	]*vfmadd231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b8 b4 f4 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b8 31[ 	]*vfmadd231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 71 7f[ 	]*vfmadd231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b8 72 80[ 	]*vfmadd231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 f4[ 	]*vfmadd231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b9 b4 f4 00 00 00 10[ 	]*vfmadd231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 31[ 	]*vfmadd231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 71 7f[ 	]*vfmadd231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b9 72 80[ 	]*vfmadd231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 f4[ 	]*vfmaddcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f 56 b4 f4 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 56 31[ 	]*vfmaddcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 71 7f[ 	]*vfmaddcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df 56 72 80[ 	]*vfmaddcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 f4[ 	]*vfmaddcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 57 b4 f4 00 00 00 10[ 	]*vfmaddcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 31[ 	]*vfmaddcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 71 7f[ 	]*vfmaddcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 57 72 80[ 	]*vfmaddcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 f4[ 	]*vfmaddsub132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 96 b4 f4 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 96 31[ 	]*vfmaddsub132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 71 7f[ 	]*vfmaddsub132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 96 72 80[ 	]*vfmaddsub132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 f4[ 	]*vfmaddsub213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a6 b4 f4 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a6 31[ 	]*vfmaddsub213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 71 7f[ 	]*vfmaddsub213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a6 72 80[ 	]*vfmaddsub213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 f4[ 	]*vfmaddsub231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b6 b4 f4 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b6 31[ 	]*vfmaddsub231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 71 7f[ 	]*vfmaddsub231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b6 72 80[ 	]*vfmaddsub231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a f4[ 	]*vfmsub132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9a b4 f4 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9a 31[ 	]*vfmsub132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a 71 7f[ 	]*vfmsub132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9a 72 80[ 	]*vfmsub132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b f4[ 	]*vfmsub132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9b b4 f4 00 00 00 10[ 	]*vfmsub132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 31[ 	]*vfmsub132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 71 7f[ 	]*vfmsub132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9b 72 80[ 	]*vfmsub132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa f4[ 	]*vfmsub213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f aa b4 f4 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 aa 31[ 	]*vfmsub213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa 71 7f[ 	]*vfmsub213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df aa 72 80[ 	]*vfmsub213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab f4[ 	]*vfmsub213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ab b4 f4 00 00 00 10[ 	]*vfmsub213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 31[ 	]*vfmsub213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 71 7f[ 	]*vfmsub213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ab 72 80[ 	]*vfmsub213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba f4[ 	]*vfmsub231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ba b4 f4 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ba 31[ 	]*vfmsub231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba 71 7f[ 	]*vfmsub231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ba 72 80[ 	]*vfmsub231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb f4[ 	]*vfmsub231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bb b4 f4 00 00 00 10[ 	]*vfmsub231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 31[ 	]*vfmsub231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 71 7f[ 	]*vfmsub231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bb 72 80[ 	]*vfmsub231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 f4[ 	]*vfmsubadd132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 97 b4 f4 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 97 31[ 	]*vfmsubadd132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 71 7f[ 	]*vfmsubadd132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 97 72 80[ 	]*vfmsubadd132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 f4[ 	]*vfmsubadd213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a7 b4 f4 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a7 31[ 	]*vfmsubadd213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 71 7f[ 	]*vfmsubadd213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a7 72 80[ 	]*vfmsubadd213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 f4[ 	]*vfmsubadd231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b7 b4 f4 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b7 31[ 	]*vfmsubadd231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 71 7f[ 	]*vfmsubadd231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b7 72 80[ 	]*vfmsubadd231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 f4[ 	]*vfmulcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f d6 b4 f4 00 00 00 10[ 	]*vfmulcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 d6 31[ 	]*vfmulcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 71 7f[ 	]*vfmulcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df d6 72 80[ 	]*vfmulcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 f4[ 	]*vfmulcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d7 b4 f4 00 00 00 10[ 	]*vfmulcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 31[ 	]*vfmulcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 71 7f[ 	]*vfmulcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d7 72 80[ 	]*vfmulcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c f4[ 	]*vfnmadd132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9c b4 f4 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9c 31[ 	]*vfnmadd132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c 71 7f[ 	]*vfnmadd132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9c 72 80[ 	]*vfnmadd132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d f4[ 	]*vfnmadd132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9d b4 f4 00 00 00 10[ 	]*vfnmadd132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 31[ 	]*vfnmadd132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 71 7f[ 	]*vfnmadd132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9d 72 80[ 	]*vfnmadd132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac f4[ 	]*vfnmadd213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ac b4 f4 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ac 31[ 	]*vfnmadd213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac 71 7f[ 	]*vfnmadd213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ac 72 80[ 	]*vfnmadd213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad f4[ 	]*vfnmadd213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ad b4 f4 00 00 00 10[ 	]*vfnmadd213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 31[ 	]*vfnmadd213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 71 7f[ 	]*vfnmadd213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ad 72 80[ 	]*vfnmadd213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc f4[ 	]*vfnmadd231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f bc b4 f4 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 bc 31[ 	]*vfnmadd231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc 71 7f[ 	]*vfnmadd231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df bc 72 80[ 	]*vfnmadd231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd f4[ 	]*vfnmadd231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bd b4 f4 00 00 00 10[ 	]*vfnmadd231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 31[ 	]*vfnmadd231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 71 7f[ 	]*vfnmadd231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bd 72 80[ 	]*vfnmadd231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e f4[ 	]*vfnmsub132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9e b4 f4 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9e 31[ 	]*vfnmsub132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e 71 7f[ 	]*vfnmsub132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9e 72 80[ 	]*vfnmsub132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f f4[ 	]*vfnmsub132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9f b4 f4 00 00 00 10[ 	]*vfnmsub132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 31[ 	]*vfnmsub132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 71 7f[ 	]*vfnmsub132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9f 72 80[ 	]*vfnmsub132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae f4[ 	]*vfnmsub213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ae b4 f4 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ae 31[ 	]*vfnmsub213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae 71 7f[ 	]*vfnmsub213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ae 72 80[ 	]*vfnmsub213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af f4[ 	]*vfnmsub213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f af b4 f4 00 00 00 10[ 	]*vfnmsub213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 31[ 	]*vfnmsub213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 71 7f[ 	]*vfnmsub213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f af 72 80[ 	]*vfnmsub213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be f4[ 	]*vfnmsub231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f be b4 f4 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 be 31[ 	]*vfnmsub231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be 71 7f[ 	]*vfnmsub231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df be 72 80[ 	]*vfnmsub231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf f4[ 	]*vfnmsub231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bf b4 f4 00 00 00 10[ 	]*vfnmsub231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 31[ 	]*vfnmsub231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 71 7f[ 	]*vfnmsub231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bf 72 80[ 	]*vfnmsub231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ac f4 00 00 00 10 7b[ 	]*vfpclassphz \$0x7b,0x10000000\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 66 29 7b[ 	]*vfpclassph \$0x7b,\(%ecx\)\{1to32\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 69 7f 7b[ 	]*vfpclassphz \$0x7b,0x1fc0\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 5f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%edx\)\{1to32\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 66 69 01 7b[ 	]*vfpclassph \$0x7b,0x2\(%ecx\)\{1to32\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ac f4 00 00 00 10 7b[ 	]*vfpclasssh \$0x7b,0x10000000\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 29 7b[ 	]*vfpclasssh \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 69 7f 7b[ 	]*vfpclasssh \$0x7b,0xfe\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 6a 80 7b[ 	]*vfpclasssh \$0x7b,-0x100\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 f5[ 	]*vgetexpph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 f5[ 	]*vgetexpph \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 f5[ 	]*vgetexpph \{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 42 b4 f4 00 00 00 10[ 	]*vgetexpph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 42 31[ 	]*vgetexpph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 71 7f[ 	]*vgetexpph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 42 72 80[ 	]*vgetexpph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 f4[ 	]*vgetexpsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 43 f4[ 	]*vgetexpsh \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 43 f4[ 	]*vgetexpsh \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 43 b4 f4 00 00 00 10[ 	]*vgetexpsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 31[ 	]*vgetexpsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 71 7f[ 	]*vgetexpsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 43 72 80[ 	]*vgetexpsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 f5 7b[ 	]*vgetmantph \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 26 b4 f4 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 26 31 7b[ 	]*vgetmantph \$0x7b,\(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 f4 7b[ 	]*vgetmantsh \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 27 b4 f4 00 00 00 10 7b[ 	]*vgetmantsh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 31 7b[ 	]*vgetmantsh \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 71 7f 7b[ 	]*vgetmantsh \$0x7b,0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 27 72 80 7b[ 	]*vgetmantsh \$0x7b,-0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f f4[ 	]*vmaxph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f f4[ 	]*vmaxph \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f f4[ 	]*vmaxph \{sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5f b4 f4 00 00 00 10[ 	]*vmaxph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5f 31[ 	]*vmaxph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f 71 7f[ 	]*vmaxph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5f 72 80[ 	]*vmaxph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f f4[ 	]*vmaxsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5f f4[ 	]*vmaxsh \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5f f4[ 	]*vmaxsh \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5f b4 f4 00 00 00 10[ 	]*vmaxsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 31[ 	]*vmaxsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 71 7f[ 	]*vmaxsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5f 72 80[ 	]*vmaxsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d f4[ 	]*vminph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d f4[ 	]*vminph \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d f4[ 	]*vminph \{sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5d b4 f4 00 00 00 10[ 	]*vminph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5d 31[ 	]*vminph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d 71 7f[ 	]*vminph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5d 72 80[ 	]*vminph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d f4[ 	]*vminsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5d f4[ 	]*vminsh \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5d f4[ 	]*vminsh \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5d b4 f4 00 00 00 10[ 	]*vminsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 31[ 	]*vminsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 71 7f[ 	]*vminsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5d 72 80[ 	]*vminsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 10 f4[ 	]*vmovsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 10 f4[ 	]*vmovsh %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 10 b4 f4 00 00 00 10[ 	]*vmovsh 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 31[ 	]*vmovsh \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 71 7f[ 	]*vmovsh 0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 10 72 80[ 	]*vmovsh -0x100\(%edx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 b4 f4 00 00 00 10[ 	]*vmovsh %xmm6,0x10000000\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 31[ 	]*vmovsh %xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 71 7f[ 	]*vmovsh %xmm6,0xfe\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 72 80[ 	]*vmovsh %xmm6,-0x100\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e f2[ 	]*vmovw  %edx,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e f2[ 	]*vmovw  %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e b4 f4 00 00 00 10[ 	]*vmovw  0x10000000\(%esp,%esi,8\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 31[ 	]*vmovw  \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 71 7f[ 	]*vmovw  0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 72 80[ 	]*vmovw  -0x100\(%edx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e b4 f4 00 00 00 10[ 	]*vmovw  %xmm6,0x10000000\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 31[ 	]*vmovw  %xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 71 7f[ 	]*vmovw  %xmm6,0xfe\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 72 80[ 	]*vmovw  %xmm6,-0x100\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 f4[ 	]*vmulph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 f4[ 	]*vmulph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 f4[ 	]*vmulph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 59 b4 f4 00 00 00 10[ 	]*vmulph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 59 31[ 	]*vmulph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 71 7f[ 	]*vmulph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 59 72 80[ 	]*vmulph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 f4[ 	]*vmulsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 59 f4[ 	]*vmulsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 59 f4[ 	]*vmulsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 59 b4 f4 00 00 00 10[ 	]*vmulsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 31[ 	]*vmulsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 71 7f[ 	]*vmulsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 59 72 80[ 	]*vmulsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c f5[ 	]*vrcpph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4c f5[ 	]*vrcpph %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4c b4 f4 00 00 00 10[ 	]*vrcpph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4c 31[ 	]*vrcpph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c 71 7f[ 	]*vrcpph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4c 72 80[ 	]*vrcpph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d f4[ 	]*vrcpsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d f4[ 	]*vrcpsh %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4d b4 f4 00 00 00 10[ 	]*vrcpsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 31[ 	]*vrcpsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 71 7f[ 	]*vrcpsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d 72 80[ 	]*vrcpsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 f5 7b[ 	]*vreduceph \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 56 b4 f4 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 56 31 7b[ 	]*vreduceph \$0x7b,\(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 71 7f 7b[ 	]*vreduceph \$0x7b,0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 f4 7b[ 	]*vreducesh \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 57 b4 f4 00 00 00 10 7b[ 	]*vreducesh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 31 7b[ 	]*vreducesh \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 71 7f 7b[ 	]*vreducesh \$0x7b,0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 57 72 80 7b[ 	]*vreducesh \$0x7b,-0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 f5 7b[ 	]*vrndscaleph \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 08 b4 f4 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a f4 7b[ 	]*vrndscalesh \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 0a b4 f4 00 00 00 10 7b[ 	]*vrndscalesh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 31 7b[ 	]*vrndscalesh \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 71 7f 7b[ 	]*vrndscalesh \$0x7b,0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 0a 72 80 7b[ 	]*vrndscalesh \$0x7b,-0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e f5[ 	]*vrsqrtph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4e f5[ 	]*vrsqrtph %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4e b4 f4 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4e 31[ 	]*vrsqrtph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e 71 7f[ 	]*vrsqrtph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4e 72 80[ 	]*vrsqrtph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f f4[ 	]*vrsqrtsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f f4[ 	]*vrsqrtsh %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4f b4 f4 00 00 00 10[ 	]*vrsqrtsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 31[ 	]*vrsqrtsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 71 7f[ 	]*vrsqrtsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f 72 80[ 	]*vrsqrtsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c f4[ 	]*vscalefph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c f4[ 	]*vscalefph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c f4[ 	]*vscalefph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 2c b4 f4 00 00 00 10[ 	]*vscalefph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 2c 31[ 	]*vscalefph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c 71 7f[ 	]*vscalefph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 2c 72 80[ 	]*vscalefph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d f4[ 	]*vscalefsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2d b4 f4 00 00 00 10[ 	]*vscalefsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 31[ 	]*vscalefsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 71 7f[ 	]*vscalefsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2d 72 80[ 	]*vscalefsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 f5[ 	]*vsqrtph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 51 b4 f4 00 00 00 10[ 	]*vsqrtph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 51 31[ 	]*vsqrtph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 71 7f[ 	]*vsqrtph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 51 72 80[ 	]*vsqrtph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 f4[ 	]*vsqrtsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 51 b4 f4 00 00 00 10[ 	]*vsqrtsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 31[ 	]*vsqrtsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 71 7f[ 	]*vsqrtsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 51 72 80[ 	]*vsqrtsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c f4[ 	]*vsubph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c f4[ 	]*vsubph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c f4[ 	]*vsubph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5c b4 f4 00 00 00 10[ 	]*vsubph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5c 31[ 	]*vsubph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c 71 7f[ 	]*vsubph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5c 72 80[ 	]*vsubph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c f4[ 	]*vsubsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5c f4[ 	]*vsubsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5c f4[ 	]*vsubsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5c b4 f4 00 00 00 10[ 	]*vsubsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 31[ 	]*vsubsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 71 7f[ 	]*vsubsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5c 72 80[ 	]*vsubsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e f5[ 	]*vucomish %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2e f5[ 	]*vucomish \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e b4 f4 00 00 00 10[ 	]*vucomish 0x10000000\(%esp,%esi,8\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 31[ 	]*vucomish \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 71 7f[ 	]*vucomish 0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 72 80[ 	]*vucomish -0x100\(%edx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 f4[ 	]*vaddph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 f4[ 	]*vaddph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 f4[ 	]*vaddph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 58 b4 f4 00 00 00 10[ 	]*vaddph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 58 31[ 	]*vaddph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 71 7f[ 	]*vaddph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 58 72 80[ 	]*vaddph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 f4[ 	]*vaddsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 58 f4[ 	]*vaddsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 58 f4[ 	]*vaddsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 58 b4 f4 00 00 00 10[ 	]*vaddsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 31[ 	]*vaddsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 71 7f[ 	]*vaddsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 58 72 80[ 	]*vaddsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 ec 7b[ 	]*vcmpph \$0x7b,%zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm4,%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 4f c2 ac f4 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 58 c2 29 7b[ 	]*vcmpph \$0x7b,\(%ecx\)\{1to32\},%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0x1fc0\(%ecx\),%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 5f c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 ec 7b[ 	]*vcmpsh \$0x7b,%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 18 c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm4,%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 00 00 00 10 7b[ 	]*vcmpsh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 29 7b[ 	]*vcmpsh \$0x7b,\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 69 7f 7b[ 	]*vcmpsh \$0x7b,0xfe\(%ecx\),%xmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 6a 80 7b[ 	]*vcmpsh \$0x7b,-0x100\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f f5[ 	]*vcomish %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2f f5[ 	]*vcomish \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f b4 f4 00 00 00 10[ 	]*vcomish 0x10000000\(%esp,%esi,8\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 31[ 	]*vcomish \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 71 7f[ 	]*vcomish 0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 72 80[ 	]*vcomish -0x100\(%edx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b f5[ 	]*vcvtdq2ph %zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5b b4 f4 00 00 00 10[ 	]*vcvtdq2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5b 31[ 	]*vcvtdq2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b 71 7f[ 	]*vcvtdq2ph 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5b 72 80[ 	]*vcvtdq2ph -0x200\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a f5[ 	]*vcvtpd2ph %zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 4f 5a b4 f4 00 00 00 10[ 	]*vcvtpd2phz 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 58 5a 31[ 	]*vcvtpd2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a 71 7f[ 	]*vcvtpd2phz 0x1fc0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd df 5a 72 80[ 	]*vcvtpd2ph -0x400\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b f5[ 	]*vcvtph2dq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 5b b4 f4 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 5b 31[ 	]*vcvtph2dq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b 71 7f[ 	]*vcvtph2dq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 5b 72 80[ 	]*vcvtph2dq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a f5[ 	]*vcvtph2pd %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a f5[ 	]*vcvtph2pd \{sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a f5[ 	]*vcvtph2pd \{sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5a b4 f4 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5a 31[ 	]*vcvtph2pd \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a 71 7f[ 	]*vcvtph2pd 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5a 72 80[ 	]*vcvtph2pd -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 f5[ 	]*vcvtph2psx %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 f5[ 	]*vcvtph2psx \{sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 f5[ 	]*vcvtph2psx \{sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 13 b4 f4 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 13 31[ 	]*vcvtph2psx \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 71 7f[ 	]*vcvtph2psx 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 13 72 80[ 	]*vcvtph2psx -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b f5[ 	]*vcvtph2qq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7b b4 f4 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7b 31[ 	]*vcvtph2qq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b 71 7f[ 	]*vcvtph2qq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7b 72 80[ 	]*vcvtph2qq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 f5[ 	]*vcvtph2udq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 79 b4 f4 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 79 31[ 	]*vcvtph2udq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 71 7f[ 	]*vcvtph2udq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 79 72 80[ 	]*vcvtph2udq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 f5[ 	]*vcvtph2uqq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 79 b4 f4 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 79 31[ 	]*vcvtph2uqq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 71 7f[ 	]*vcvtph2uqq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 79 72 80[ 	]*vcvtph2uqq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d f5[ 	]*vcvtph2uw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7d b4 f4 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7d 31[ 	]*vcvtph2uw \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d 71 7f[ 	]*vcvtph2uw 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7d 72 80[ 	]*vcvtph2uw -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d f5[ 	]*vcvtph2w %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7d b4 f4 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7d 31[ 	]*vcvtph2w \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d 71 7f[ 	]*vcvtph2w 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7d 72 80[ 	]*vcvtph2w -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d f5[ 	]*vcvtps2phx %zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 1d b4 f4 00 00 00 10[ 	]*vcvtps2phx 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 1d 31[ 	]*vcvtps2phx \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d 71 7f[ 	]*vcvtps2phx 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 1d 72 80[ 	]*vcvtps2phx -0x200\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b f5[ 	]*vcvtqq2ph %zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 4f 5b b4 f4 00 00 00 10[ 	]*vcvtqq2phz 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 58 5b 31[ 	]*vcvtqq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b 71 7f[ 	]*vcvtqq2phz 0x1fc0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc df 5b 72 80[ 	]*vcvtqq2ph -0x400\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a f4[ 	]*vcvtsd2sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 18 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 9f 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 0f 5a b4 f4 00 00 00 10[ 	]*vcvtsd2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 31[ 	]*vcvtsd2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 71 7f[ 	]*vcvtsd2sh 0x3f8\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 8f 5a 72 80[ 	]*vcvtsd2sh -0x400\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a f4[ 	]*vcvtsh2sd %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5a b4 f4 00 00 00 10[ 	]*vcvtsh2sd 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 31[ 	]*vcvtsh2sd \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 71 7f[ 	]*vcvtsh2sd 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5a 72 80[ 	]*vcvtsh2sd -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d d6[ 	]*vcvtsh2si %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2d d6[ 	]*vcvtsh2si \{rn-sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 94 f4 00 00 00 10[ 	]*vcvtsh2si 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 11[ 	]*vcvtsh2si \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 51 7f[ 	]*vcvtsh2si 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 52 80[ 	]*vcvtsh2si -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 f4[ 	]*vcvtsh2ss %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 18 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 9f 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 0f 13 b4 f4 00 00 00 10[ 	]*vcvtsh2ss 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 31[ 	]*vcvtsh2ss \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 71 7f[ 	]*vcvtsh2ss 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 8f 13 72 80[ 	]*vcvtsh2ss -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 d6[ 	]*vcvtsh2usi %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 79 d6[ 	]*vcvtsh2usi \{rn-sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 94 f4 00 00 00 10[ 	]*vcvtsh2usi 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 11[ 	]*vcvtsh2usi \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 51 7f[ 	]*vcvtsh2usi 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 52 80[ 	]*vcvtsh2usi -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a f2[ 	]*vcvtsi2sh %edx,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 2a f2[ 	]*vcvtsi2sh %edx,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a b4 f4 00 00 00 10[ 	]*vcvtsi2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 31[ 	]*vcvtsi2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 71 7f[ 	]*vcvtsi2sh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 72 80[ 	]*vcvtsi2sh -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d f4[ 	]*vcvtss2sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 1d b4 f4 00 00 00 10[ 	]*vcvtss2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 31[ 	]*vcvtss2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 71 7f[ 	]*vcvtss2sh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 1d 72 80[ 	]*vcvtss2sh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b f5[ 	]*vcvttph2dq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b f5[ 	]*vcvttph2dq \{sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b f5[ 	]*vcvttph2dq \{sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 5b b4 f4 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 5b 31[ 	]*vcvttph2dq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b 71 7f[ 	]*vcvttph2dq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 5b 72 80[ 	]*vcvttph2dq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a f5[ 	]*vcvttph2qq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a f5[ 	]*vcvttph2qq \{sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a f5[ 	]*vcvttph2qq \{sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7a b4 f4 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7a 31[ 	]*vcvttph2qq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a 71 7f[ 	]*vcvttph2qq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7a 72 80[ 	]*vcvttph2qq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 f5[ 	]*vcvttph2udq %ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 f5[ 	]*vcvttph2udq \{sae\},%ymm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 f5[ 	]*vcvttph2udq \{sae\},%ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 78 b4 f4 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 78 31[ 	]*vcvttph2udq \(%ecx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 71 7f[ 	]*vcvttph2udq 0xfe0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 78 72 80[ 	]*vcvttph2udq -0x100\(%edx\)\{1to16\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 f5[ 	]*vcvttph2uqq %xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 78 b4 f4 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 78 31[ 	]*vcvttph2uqq \(%ecx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 71 7f[ 	]*vcvttph2uqq 0x7f0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 78 72 80[ 	]*vcvttph2uqq -0x100\(%edx\)\{1to8\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c f5[ 	]*vcvttph2uw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c f5[ 	]*vcvttph2uw \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c f5[ 	]*vcvttph2uw \{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7c b4 f4 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7c 31[ 	]*vcvttph2uw \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c 71 7f[ 	]*vcvttph2uw 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7c 72 80[ 	]*vcvttph2uw -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c f5[ 	]*vcvttph2w %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c f5[ 	]*vcvttph2w \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c f5[ 	]*vcvttph2w \{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7c b4 f4 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7c 31[ 	]*vcvttph2w \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c 71 7f[ 	]*vcvttph2w 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7c 72 80[ 	]*vcvttph2w -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c d6[ 	]*vcvttsh2si %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2c d6[ 	]*vcvttsh2si \{sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 94 f4 00 00 00 10[ 	]*vcvttsh2si 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 11[ 	]*vcvttsh2si \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 51 7f[ 	]*vcvttsh2si 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 52 80[ 	]*vcvttsh2si -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 d6[ 	]*vcvttsh2usi %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 78 d6[ 	]*vcvttsh2usi \{sae\},%xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 94 f4 00 00 00 10[ 	]*vcvttsh2usi 0x10000000\(%esp,%esi,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 11[ 	]*vcvttsh2usi \(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 51 7f[ 	]*vcvttsh2usi 0xfe\(%ecx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 52 80[ 	]*vcvttsh2usi -0x100\(%edx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a f5[ 	]*vcvtudq2ph %zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7a b4 f4 00 00 00 10[ 	]*vcvtudq2ph 0x10000000\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7a 31[ 	]*vcvtudq2ph \(%ecx\)\{1to16\},%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a 71 7f[ 	]*vcvtudq2ph 0x1fc0\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7a 72 80[ 	]*vcvtudq2ph -0x200\(%edx\)\{1to16\},%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a f5[ 	]*vcvtuqq2ph %zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 4f 7a b4 f4 00 00 00 10[ 	]*vcvtuqq2phz 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 58 7a 31[ 	]*vcvtuqq2ph \(%ecx\)\{1to8\},%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a 71 7f[ 	]*vcvtuqq2phz 0x1fc0\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff df 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%edx\)\{1to8\},%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b f2[ 	]*vcvtusi2sh %edx,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 7b f2[ 	]*vcvtusi2sh %edx,\{rn-sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b b4 f4 00 00 00 10[ 	]*vcvtusi2sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 31[ 	]*vcvtusi2sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 71 7f[ 	]*vcvtusi2sh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 72 80[ 	]*vcvtusi2sh -0x200\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d f5[ 	]*vcvtuw2ph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7d b4 f4 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7d 31[ 	]*vcvtuw2ph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d 71 7f[ 	]*vcvtuw2ph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7d 72 80[ 	]*vcvtuw2ph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d f5[ 	]*vcvtw2ph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 7d b4 f4 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 7d 31[ 	]*vcvtw2ph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d 71 7f[ 	]*vcvtw2ph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 7d 72 80[ 	]*vcvtw2ph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e f4[ 	]*vdivph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e f4[ 	]*vdivph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e f4[ 	]*vdivph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5e b4 f4 00 00 00 10[ 	]*vdivph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5e 31[ 	]*vdivph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e 71 7f[ 	]*vdivph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5e 72 80[ 	]*vdivph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e f4[ 	]*vdivsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5e f4[ 	]*vdivsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5e f4[ 	]*vdivsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5e b4 f4 00 00 00 10[ 	]*vdivsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 31[ 	]*vdivsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 71 7f[ 	]*vdivsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5e 72 80[ 	]*vdivsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 f4[ 	]*vfcmaddcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f 56 b4 f4 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 56 31[ 	]*vfcmaddcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 71 7f[ 	]*vfcmaddcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df 56 72 80[ 	]*vfcmaddcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 f4[ 	]*vfcmaddcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 57 b4 f4 00 00 00 10[ 	]*vfcmaddcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 31[ 	]*vfcmaddcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 71 7f[ 	]*vfcmaddcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 57 72 80[ 	]*vfcmaddcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 f4[ 	]*vfcmulcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f d6 b4 f4 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 d6 31[ 	]*vfcmulcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 71 7f[ 	]*vfcmulcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df d6 72 80[ 	]*vfcmulcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 f4[ 	]*vfcmulcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d7 b4 f4 00 00 00 10[ 	]*vfcmulcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 31[ 	]*vfcmulcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 71 7f[ 	]*vfcmulcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d7 72 80[ 	]*vfcmulcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 f4[ 	]*vfmadd132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 98 b4 f4 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 98 31[ 	]*vfmadd132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 71 7f[ 	]*vfmadd132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 98 72 80[ 	]*vfmadd132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 f4[ 	]*vfmadd132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 99 b4 f4 00 00 00 10[ 	]*vfmadd132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 31[ 	]*vfmadd132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 71 7f[ 	]*vfmadd132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 99 72 80[ 	]*vfmadd132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 f4[ 	]*vfmadd213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a8 b4 f4 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a8 31[ 	]*vfmadd213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 71 7f[ 	]*vfmadd213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a8 72 80[ 	]*vfmadd213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 f4[ 	]*vfmadd213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a9 b4 f4 00 00 00 10[ 	]*vfmadd213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 31[ 	]*vfmadd213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 71 7f[ 	]*vfmadd213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a9 72 80[ 	]*vfmadd213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 f4[ 	]*vfmadd231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b8 b4 f4 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b8 31[ 	]*vfmadd231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 71 7f[ 	]*vfmadd231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b8 72 80[ 	]*vfmadd231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 f4[ 	]*vfmadd231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b9 b4 f4 00 00 00 10[ 	]*vfmadd231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 31[ 	]*vfmadd231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 71 7f[ 	]*vfmadd231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b9 72 80[ 	]*vfmadd231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 f4[ 	]*vfmaddcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f 56 b4 f4 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 56 31[ 	]*vfmaddcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 71 7f[ 	]*vfmaddcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df 56 72 80[ 	]*vfmaddcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 f4[ 	]*vfmaddcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 57 b4 f4 00 00 00 10[ 	]*vfmaddcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 31[ 	]*vfmaddcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 71 7f[ 	]*vfmaddcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 57 72 80[ 	]*vfmaddcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 f4[ 	]*vfmaddsub132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 96 b4 f4 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 96 31[ 	]*vfmaddsub132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 71 7f[ 	]*vfmaddsub132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 96 72 80[ 	]*vfmaddsub132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 f4[ 	]*vfmaddsub213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a6 b4 f4 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a6 31[ 	]*vfmaddsub213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 71 7f[ 	]*vfmaddsub213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a6 72 80[ 	]*vfmaddsub213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 f4[ 	]*vfmaddsub231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b6 b4 f4 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b6 31[ 	]*vfmaddsub231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 71 7f[ 	]*vfmaddsub231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b6 72 80[ 	]*vfmaddsub231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a f4[ 	]*vfmsub132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9a b4 f4 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9a 31[ 	]*vfmsub132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a 71 7f[ 	]*vfmsub132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9a 72 80[ 	]*vfmsub132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b f4[ 	]*vfmsub132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9b b4 f4 00 00 00 10[ 	]*vfmsub132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 31[ 	]*vfmsub132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 71 7f[ 	]*vfmsub132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9b 72 80[ 	]*vfmsub132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa f4[ 	]*vfmsub213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f aa b4 f4 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 aa 31[ 	]*vfmsub213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa 71 7f[ 	]*vfmsub213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df aa 72 80[ 	]*vfmsub213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab f4[ 	]*vfmsub213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ab b4 f4 00 00 00 10[ 	]*vfmsub213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 31[ 	]*vfmsub213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 71 7f[ 	]*vfmsub213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ab 72 80[ 	]*vfmsub213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba f4[ 	]*vfmsub231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ba b4 f4 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ba 31[ 	]*vfmsub231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba 71 7f[ 	]*vfmsub231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ba 72 80[ 	]*vfmsub231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb f4[ 	]*vfmsub231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bb b4 f4 00 00 00 10[ 	]*vfmsub231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 31[ 	]*vfmsub231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 71 7f[ 	]*vfmsub231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bb 72 80[ 	]*vfmsub231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 f4[ 	]*vfmsubadd132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 97 b4 f4 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 97 31[ 	]*vfmsubadd132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 71 7f[ 	]*vfmsubadd132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 97 72 80[ 	]*vfmsubadd132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 f4[ 	]*vfmsubadd213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a7 b4 f4 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a7 31[ 	]*vfmsubadd213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 71 7f[ 	]*vfmsubadd213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a7 72 80[ 	]*vfmsubadd213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 f4[ 	]*vfmsubadd231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b7 b4 f4 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b7 31[ 	]*vfmsubadd231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 71 7f[ 	]*vfmsubadd231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b7 72 80[ 	]*vfmsubadd231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 f4[ 	]*vfmulcph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f d6 b4 f4 00 00 00 10[ 	]*vfmulcph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 d6 31[ 	]*vfmulcph \(%ecx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 71 7f[ 	]*vfmulcph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df d6 72 80[ 	]*vfmulcph -0x200\(%edx\)\{1to16\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 f4[ 	]*vfmulcsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d7 b4 f4 00 00 00 10[ 	]*vfmulcsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 31[ 	]*vfmulcsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 71 7f[ 	]*vfmulcsh 0x1fc\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d7 72 80[ 	]*vfmulcsh -0x200\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c f4[ 	]*vfnmadd132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9c b4 f4 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9c 31[ 	]*vfnmadd132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c 71 7f[ 	]*vfnmadd132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9c 72 80[ 	]*vfnmadd132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d f4[ 	]*vfnmadd132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9d b4 f4 00 00 00 10[ 	]*vfnmadd132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 31[ 	]*vfnmadd132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 71 7f[ 	]*vfnmadd132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9d 72 80[ 	]*vfnmadd132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac f4[ 	]*vfnmadd213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ac b4 f4 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ac 31[ 	]*vfnmadd213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac 71 7f[ 	]*vfnmadd213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ac 72 80[ 	]*vfnmadd213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad f4[ 	]*vfnmadd213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ad b4 f4 00 00 00 10[ 	]*vfnmadd213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 31[ 	]*vfnmadd213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 71 7f[ 	]*vfnmadd213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ad 72 80[ 	]*vfnmadd213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc f4[ 	]*vfnmadd231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f bc b4 f4 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 bc 31[ 	]*vfnmadd231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc 71 7f[ 	]*vfnmadd231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df bc 72 80[ 	]*vfnmadd231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd f4[ 	]*vfnmadd231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bd b4 f4 00 00 00 10[ 	]*vfnmadd231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 31[ 	]*vfnmadd231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 71 7f[ 	]*vfnmadd231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bd 72 80[ 	]*vfnmadd231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e f4[ 	]*vfnmsub132ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9e b4 f4 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9e 31[ 	]*vfnmsub132ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e 71 7f[ 	]*vfnmsub132ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9e 72 80[ 	]*vfnmsub132ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f f4[ 	]*vfnmsub132sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9f b4 f4 00 00 00 10[ 	]*vfnmsub132sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 31[ 	]*vfnmsub132sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 71 7f[ 	]*vfnmsub132sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9f 72 80[ 	]*vfnmsub132sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae f4[ 	]*vfnmsub213ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ae b4 f4 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ae 31[ 	]*vfnmsub213ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae 71 7f[ 	]*vfnmsub213ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ae 72 80[ 	]*vfnmsub213ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af f4[ 	]*vfnmsub213sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f af b4 f4 00 00 00 10[ 	]*vfnmsub213sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 31[ 	]*vfnmsub213sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 71 7f[ 	]*vfnmsub213sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f af 72 80[ 	]*vfnmsub213sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be f4[ 	]*vfnmsub231ph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f be b4 f4 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 be 31[ 	]*vfnmsub231ph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be 71 7f[ 	]*vfnmsub231ph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df be 72 80[ 	]*vfnmsub231ph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf f4[ 	]*vfnmsub231sh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bf b4 f4 00 00 00 10[ 	]*vfnmsub231sh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 31[ 	]*vfnmsub231sh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 71 7f[ 	]*vfnmsub231sh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bf 72 80[ 	]*vfnmsub231sh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ac f4 00 00 00 10 7b[ 	]*vfpclassphz \$0x7b,0x10000000\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 66 29 7b[ 	]*vfpclassph \$0x7b,\(%ecx\)\{1to32\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 69 7f 7b[ 	]*vfpclassphz \$0x7b,0x1fc0\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 5f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%edx\)\{1to32\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ac f4 00 00 00 10 7b[ 	]*vfpclasssh \$0x7b,0x10000000\(%esp,%esi,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 29 7b[ 	]*vfpclasssh \$0x7b,\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 69 7f 7b[ 	]*vfpclasssh \$0x7b,0xfe\(%ecx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 6a 80 7b[ 	]*vfpclasssh \$0x7b,-0x100\(%edx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 f5[ 	]*vgetexpph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 f5[ 	]*vgetexpph \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 f5[ 	]*vgetexpph \{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 42 b4 f4 00 00 00 10[ 	]*vgetexpph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 42 31[ 	]*vgetexpph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 71 7f[ 	]*vgetexpph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 42 72 80[ 	]*vgetexpph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 f4[ 	]*vgetexpsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 43 f4[ 	]*vgetexpsh \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 43 f4[ 	]*vgetexpsh \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 43 b4 f4 00 00 00 10[ 	]*vgetexpsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 31[ 	]*vgetexpsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 71 7f[ 	]*vgetexpsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 43 72 80[ 	]*vgetexpsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 f5 7b[ 	]*vgetmantph \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 26 b4 f4 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 26 31 7b[ 	]*vgetmantph \$0x7b,\(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 f4 7b[ 	]*vgetmantsh \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 27 b4 f4 00 00 00 10 7b[ 	]*vgetmantsh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 31 7b[ 	]*vgetmantsh \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 71 7f 7b[ 	]*vgetmantsh \$0x7b,0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 27 72 80 7b[ 	]*vgetmantsh \$0x7b,-0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f f4[ 	]*vmaxph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f f4[ 	]*vmaxph \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f f4[ 	]*vmaxph \{sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5f b4 f4 00 00 00 10[ 	]*vmaxph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5f 31[ 	]*vmaxph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f 71 7f[ 	]*vmaxph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5f 72 80[ 	]*vmaxph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f f4[ 	]*vmaxsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5f f4[ 	]*vmaxsh \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5f f4[ 	]*vmaxsh \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5f b4 f4 00 00 00 10[ 	]*vmaxsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 31[ 	]*vmaxsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 71 7f[ 	]*vmaxsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5f 72 80[ 	]*vmaxsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d f4[ 	]*vminph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d f4[ 	]*vminph \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d f4[ 	]*vminph \{sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5d b4 f4 00 00 00 10[ 	]*vminph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5d 31[ 	]*vminph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d 71 7f[ 	]*vminph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5d 72 80[ 	]*vminph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d f4[ 	]*vminsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5d f4[ 	]*vminsh \{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5d f4[ 	]*vminsh \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5d b4 f4 00 00 00 10[ 	]*vminsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 31[ 	]*vminsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 71 7f[ 	]*vminsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5d 72 80[ 	]*vminsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 10 f4[ 	]*vmovsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 10 f4[ 	]*vmovsh %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 10 b4 f4 00 00 00 10[ 	]*vmovsh 0x10000000\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 31[ 	]*vmovsh \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 71 7f[ 	]*vmovsh 0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 10 72 80[ 	]*vmovsh -0x100\(%edx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 b4 f4 00 00 00 10[ 	]*vmovsh %xmm6,0x10000000\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 31[ 	]*vmovsh %xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 71 7f[ 	]*vmovsh %xmm6,0xfe\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 72 80[ 	]*vmovsh %xmm6,-0x100\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e f2[ 	]*vmovw  %edx,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e f2[ 	]*vmovw  %xmm6,%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e b4 f4 00 00 00 10[ 	]*vmovw  0x10000000\(%esp,%esi,8\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 31[ 	]*vmovw  \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 71 7f[ 	]*vmovw  0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 72 80[ 	]*vmovw  -0x100\(%edx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e b4 f4 00 00 00 10[ 	]*vmovw  %xmm6,0x10000000\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 31[ 	]*vmovw  %xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 71 7f[ 	]*vmovw  %xmm6,0xfe\(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 72 80[ 	]*vmovw  %xmm6,-0x100\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 f4[ 	]*vmulph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 f4[ 	]*vmulph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 f4[ 	]*vmulph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 59 b4 f4 00 00 00 10[ 	]*vmulph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 59 31[ 	]*vmulph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 71 7f[ 	]*vmulph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 59 72 80[ 	]*vmulph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 f4[ 	]*vmulsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 59 f4[ 	]*vmulsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 59 f4[ 	]*vmulsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 59 b4 f4 00 00 00 10[ 	]*vmulsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 31[ 	]*vmulsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 71 7f[ 	]*vmulsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 59 72 80[ 	]*vmulsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c f5[ 	]*vrcpph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4c f5[ 	]*vrcpph %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4c b4 f4 00 00 00 10[ 	]*vrcpph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4c 31[ 	]*vrcpph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c 71 7f[ 	]*vrcpph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4c 72 80[ 	]*vrcpph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d f4[ 	]*vrcpsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d f4[ 	]*vrcpsh %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4d b4 f4 00 00 00 10[ 	]*vrcpsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 31[ 	]*vrcpsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 71 7f[ 	]*vrcpsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d 72 80[ 	]*vrcpsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 f5 7b[ 	]*vreduceph \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 56 b4 f4 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 56 31 7b[ 	]*vreduceph \$0x7b,\(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 71 7f 7b[ 	]*vreduceph \$0x7b,0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 f4 7b[ 	]*vreducesh \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 57 b4 f4 00 00 00 10 7b[ 	]*vreducesh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 31 7b[ 	]*vreducesh \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 71 7f 7b[ 	]*vreducesh \$0x7b,0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 57 72 80 7b[ 	]*vreducesh \$0x7b,-0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 f5 7b[ 	]*vrndscaleph \$0x7b,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 08 b4 f4 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a f4 7b[ 	]*vrndscalesh \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 0a b4 f4 00 00 00 10 7b[ 	]*vrndscalesh \$0x7b,0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 31 7b[ 	]*vrndscalesh \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 71 7f 7b[ 	]*vrndscalesh \$0x7b,0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 0a 72 80 7b[ 	]*vrndscalesh \$0x7b,-0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e f5[ 	]*vrsqrtph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4e f5[ 	]*vrsqrtph %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4e b4 f4 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4e 31[ 	]*vrsqrtph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e 71 7f[ 	]*vrsqrtph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4e 72 80[ 	]*vrsqrtph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f f4[ 	]*vrsqrtsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f f4[ 	]*vrsqrtsh %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4f b4 f4 00 00 00 10[ 	]*vrsqrtsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 31[ 	]*vrsqrtsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 71 7f[ 	]*vrsqrtsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f 72 80[ 	]*vrsqrtsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c f4[ 	]*vscalefph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c f4[ 	]*vscalefph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c f4[ 	]*vscalefph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 2c b4 f4 00 00 00 10[ 	]*vscalefph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 2c 31[ 	]*vscalefph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c 71 7f[ 	]*vscalefph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 2c 72 80[ 	]*vscalefph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d f4[ 	]*vscalefsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2d b4 f4 00 00 00 10[ 	]*vscalefsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 31[ 	]*vscalefsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 71 7f[ 	]*vscalefsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2d 72 80[ 	]*vscalefsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 f5[ 	]*vsqrtph %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 51 b4 f4 00 00 00 10[ 	]*vsqrtph 0x10000000\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 51 31[ 	]*vsqrtph \(%ecx\)\{1to32\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 71 7f[ 	]*vsqrtph 0x1fc0\(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 51 72 80[ 	]*vsqrtph -0x100\(%edx\)\{1to32\},%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 f4[ 	]*vsqrtsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 51 b4 f4 00 00 00 10[ 	]*vsqrtsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 31[ 	]*vsqrtsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 71 7f[ 	]*vsqrtsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 51 72 80[ 	]*vsqrtsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c f4[ 	]*vsubph %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c f4[ 	]*vsubph \{rn-sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c f4[ 	]*vsubph \{rn-sae\},%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5c b4 f4 00 00 00 10[ 	]*vsubph 0x10000000\(%esp,%esi,8\),%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5c 31[ 	]*vsubph \(%ecx\)\{1to32\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c 71 7f[ 	]*vsubph 0x1fc0\(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5c 72 80[ 	]*vsubph -0x100\(%edx\)\{1to32\},%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c f4[ 	]*vsubsh %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5c f4[ 	]*vsubsh \{rn-sae\},%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5c f4[ 	]*vsubsh \{rn-sae\},%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5c b4 f4 00 00 00 10[ 	]*vsubsh 0x10000000\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 31[ 	]*vsubsh \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 71 7f[ 	]*vsubsh 0xfe\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5c 72 80[ 	]*vsubsh -0x100\(%edx\),%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e f5[ 	]*vucomish %xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2e f5[ 	]*vucomish \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e b4 f4 00 00 00 10[ 	]*vucomish 0x10000000\(%esp,%esi,8\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 31[ 	]*vucomish \(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 71 7f[ 	]*vucomish 0xfe\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 72 80[ 	]*vucomish -0x100\(%edx\),%xmm6
#pass
