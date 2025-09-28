#as:
#objdump: -dw
#name: x86_64 AVX512-FP16 insns
#source: x86-64-avx512_fp16.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 58 f4[ 	]*vaddph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 58 f4[ 	]*vaddph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 58 f4[ 	]*vaddph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 58 b4 f5 00 00 00 10[ 	]*vaddph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 58 31[ 	]*vaddph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 58 71 7f[ 	]*vaddph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 58 72 80[ 	]*vaddph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 58 f4[ 	]*vaddsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 58 f4[ 	]*vaddsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 58 f4[ 	]*vaddsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 58 b4 f5 00 00 00 10[ 	]*vaddsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 58 31[ 	]*vaddsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 58 71 7f[ 	]*vaddsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 58 72 80[ 	]*vaddsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 93 14 40 c2 ec 7b[ 	]*vcmpph \$0x7b,%zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 14 10 c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 14 17 c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm28,%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 14 47 c2 ac f5 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 14 50 c2 29 7b[ 	]*vcmpph \$0x7b,\(%r9\)\{1to32\},%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 40 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0x1fc0\(%rcx\),%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 57 c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 00 c2 ec 7b[ 	]*vcmpsh \$0x7b,%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 16 10 c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f5 00 00 00 10 7b[ 	]*vcmpsh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 16 00 c2 29 7b[ 	]*vcmpsh \$0x7b,\(%r9\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 00 c2 69 7f 7b[ 	]*vcmpsh \$0x7b,0xfe\(%rcx\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 6a 80 7b[ 	]*vcmpsh \$0x7b,-0x100\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 2f f5[ 	]*vcomish %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 2f f5[ 	]*vcomish \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 08 2f b4 f5 00 00 00 10[ 	]*vcomish 0x10000000\(%rbp,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 08 2f 31[ 	]*vcomish \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2f 71 7f[ 	]*vcomish 0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2f 72 80[ 	]*vcomish -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 5b f5[ 	]*vcvtdq2ph %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 5b b4 f5 00 00 00 10[ 	]*vcvtdq2ph 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 5b 31[ 	]*vcvtdq2ph \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 5b 71 7f[ 	]*vcvtdq2ph 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 5b 72 80[ 	]*vcvtdq2ph -0x200\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 48 5a f5[ 	]*vcvtpd2ph %zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 18 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 9f 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 fd 4f 5a b4 f5 00 00 00 10[ 	]*vcvtpd2phz 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fd 58 5a 31[ 	]*vcvtpd2ph \(%r9\)\{1to8\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 48 5a 71 7f[ 	]*vcvtpd2phz 0x1fc0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fd df 5a 72 80[ 	]*vcvtpd2ph -0x400\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 5b f5[ 	]*vcvtph2dq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 5b b4 f5 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 5b 31[ 	]*vcvtph2dq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 5b 71 7f[ 	]*vcvtph2dq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 5b 72 80[ 	]*vcvtph2dq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 5a f5[ 	]*vcvtph2pd %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 5a f5[ 	]*vcvtph2pd \{sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 5a f5[ 	]*vcvtph2pd \{sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 5a b4 f5 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 5a 31[ 	]*vcvtph2pd \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 5a 71 7f[ 	]*vcvtph2pd 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 5a 72 80[ 	]*vcvtph2pd -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 13 f5[ 	]*vcvtph2psx %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 18 13 f5[ 	]*vcvtph2psx \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 9f 13 f5[ 	]*vcvtph2psx \{sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 13 b4 f5 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 13 31[ 	]*vcvtph2psx \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 13 71 7f[ 	]*vcvtph2psx 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 13 72 80[ 	]*vcvtph2psx -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7b f5[ 	]*vcvtph2qq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7b b4 f5 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7b 31[ 	]*vcvtph2qq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7b 71 7f[ 	]*vcvtph2qq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7b 72 80[ 	]*vcvtph2qq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 79 f5[ 	]*vcvtph2udq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 79 b4 f5 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 79 31[ 	]*vcvtph2udq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 79 71 7f[ 	]*vcvtph2udq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 79 72 80[ 	]*vcvtph2udq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 79 f5[ 	]*vcvtph2uqq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 79 b4 f5 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 79 31[ 	]*vcvtph2uqq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 79 71 7f[ 	]*vcvtph2uqq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 79 72 80[ 	]*vcvtph2uqq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 7d f5[ 	]*vcvtph2uw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 7d b4 f5 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 7d 31[ 	]*vcvtph2uw \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 7d 71 7f[ 	]*vcvtph2uw 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 7d 72 80[ 	]*vcvtph2uw -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7d f5[ 	]*vcvtph2w %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7d b4 f5 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7d 31[ 	]*vcvtph2w \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7d 71 7f[ 	]*vcvtph2w 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7d 72 80[ 	]*vcvtph2w -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 1d f5[ 	]*vcvtps2phx %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 1d b4 f5 00 00 00 10[ 	]*vcvtps2phx 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 1d 31[ 	]*vcvtps2phx \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 1d 71 7f[ 	]*vcvtps2phx 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 1d 72 80[ 	]*vcvtps2phx -0x200\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 48 5b f5[ 	]*vcvtqq2ph %zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 18 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 9f 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 fc 4f 5b b4 f5 00 00 00 10[ 	]*vcvtqq2phz 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fc 58 5b 31[ 	]*vcvtqq2ph \(%r9\)\{1to8\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 48 5b 71 7f[ 	]*vcvtqq2phz 0x1fc0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fc df 5b 72 80[ 	]*vcvtqq2ph -0x400\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 97 00 5a f4[ 	]*vcvtsd2sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 97 10 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 97 97 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 97 07 5a b4 f5 00 00 00 10[ 	]*vcvtsd2sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 97 00 5a 31[ 	]*vcvtsd2sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 97 00 5a 71 7f[ 	]*vcvtsd2sh 0x3f8\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 97 87 5a 72 80[ 	]*vcvtsd2sh -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5a f4[ 	]*vcvtsh2sd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5a b4 f5 00 00 00 10[ 	]*vcvtsh2sd 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5a 31[ 	]*vcvtsh2sd \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5a 71 7f[ 	]*vcvtsh2sd 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5a 72 80[ 	]*vcvtsh2sd -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 2d d6[ 	]*vcvtsh2si %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 2d d6[ 	]*vcvtsh2si \{rn-sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 2d e6[ 	]*vcvtsh2si %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 2d e6[ 	]*vcvtsh2si \{rn-sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 2d 94 f5 00 00 00 10[ 	]*vcvtsh2si 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 2d 11[ 	]*vcvtsh2si \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 51 7f[ 	]*vcvtsh2si 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 52 80[ 	]*vcvtsh2si -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 2d a4 f5 00 00 00 10[ 	]*vcvtsh2si 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 2d 21[ 	]*vcvtsh2si \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2d 61 7f[ 	]*vcvtsh2si 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2d 62 80[ 	]*vcvtsh2si -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 06 14 00 13 f4[ 	]*vcvtsh2ss %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 14 10 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 14 97 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 14 07 13 b4 f5 00 00 00 10[ 	]*vcvtsh2ss 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 14 00 13 31[ 	]*vcvtsh2ss \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 14 00 13 71 7f[ 	]*vcvtsh2ss 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 14 87 13 72 80[ 	]*vcvtsh2ss -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 79 d6[ 	]*vcvtsh2usi %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 79 d6[ 	]*vcvtsh2usi \{rn-sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 79 e6[ 	]*vcvtsh2usi %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 79 e6[ 	]*vcvtsh2usi \{rn-sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 79 94 f5 00 00 00 10[ 	]*vcvtsh2usi 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 79 11[ 	]*vcvtsh2usi \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 51 7f[ 	]*vcvtsh2usi 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 52 80[ 	]*vcvtsh2usi -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 79 a4 f5 00 00 00 10[ 	]*vcvtsh2usi 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 79 21[ 	]*vcvtsh2usi \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 79 61 7f[ 	]*vcvtsh2usi 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 79 62 80[ 	]*vcvtsh2usi -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 45 96 00 2a f4[ 	]*vcvtsi2sh %r12,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 96 10 2a f4[ 	]*vcvtsi2sh %r12,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 2a f2[ 	]*vcvtsi2sh %edx,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 10 2a f2[ 	]*vcvtsi2sh %edx,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 16 00 2a b4 f5 00 00 00 10[ 	]*vcvtsi2shl 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 2a 31[ 	]*vcvtsi2shl \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 2a 71 7f[ 	]*vcvtsi2shl 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 2a 72 80[ 	]*vcvtsi2shl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 2a 71 7f[ 	]*vcvtsi2shq 0x3f8\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 2a 72 80[ 	]*vcvtsi2shq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 1d f4[ 	]*vcvtss2sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 1d b4 f5 00 00 00 10[ 	]*vcvtss2sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 00 1d 31[ 	]*vcvtss2sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 1d 71 7f[ 	]*vcvtss2sh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 87 1d 72 80[ 	]*vcvtss2sh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 48 5b f5[ 	]*vcvttph2dq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 18 5b f5[ 	]*vcvttph2dq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 9f 5b f5[ 	]*vcvttph2dq \{sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 4f 5b b4 f5 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 58 5b 31[ 	]*vcvttph2dq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 48 5b 71 7f[ 	]*vcvttph2dq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e df 5b 72 80[ 	]*vcvttph2dq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7a f5[ 	]*vcvttph2qq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7a f5[ 	]*vcvttph2qq \{sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7a f5[ 	]*vcvttph2qq \{sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7a b4 f5 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7a 31[ 	]*vcvttph2qq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7a 71 7f[ 	]*vcvttph2qq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7a 72 80[ 	]*vcvttph2qq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 78 f5[ 	]*vcvttph2udq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 78 f5[ 	]*vcvttph2udq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 78 f5[ 	]*vcvttph2udq \{sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 78 b4 f5 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 78 31[ 	]*vcvttph2udq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 78 71 7f[ 	]*vcvttph2udq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 78 72 80[ 	]*vcvttph2udq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 78 f5[ 	]*vcvttph2uqq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 78 b4 f5 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 78 31[ 	]*vcvttph2uqq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 78 71 7f[ 	]*vcvttph2uqq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 78 72 80[ 	]*vcvttph2uqq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 7c f5[ 	]*vcvttph2uw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 7c f5[ 	]*vcvttph2uw \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 7c f5[ 	]*vcvttph2uw \{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 7c b4 f5 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 7c 31[ 	]*vcvttph2uw \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 7c 71 7f[ 	]*vcvttph2uw 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 7c 72 80[ 	]*vcvttph2uw -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7c f5[ 	]*vcvttph2w %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7c f5[ 	]*vcvttph2w \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7c f5[ 	]*vcvttph2w \{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7c b4 f5 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7c 31[ 	]*vcvttph2w \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7c 71 7f[ 	]*vcvttph2w 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7c 72 80[ 	]*vcvttph2w -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 2c d6[ 	]*vcvttsh2si %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 2c d6[ 	]*vcvttsh2si \{sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 2c e6[ 	]*vcvttsh2si %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 2c e6[ 	]*vcvttsh2si \{sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 2c 94 f5 00 00 00 10[ 	]*vcvttsh2si 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 2c 11[ 	]*vcvttsh2si \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 51 7f[ 	]*vcvttsh2si 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 52 80[ 	]*vcvttsh2si -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 2c a4 f5 00 00 00 10[ 	]*vcvttsh2si 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 2c 21[ 	]*vcvttsh2si \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2c 61 7f[ 	]*vcvttsh2si 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2c 62 80[ 	]*vcvttsh2si -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 78 d6[ 	]*vcvttsh2usi %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 78 d6[ 	]*vcvttsh2usi \{sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 78 e6[ 	]*vcvttsh2usi %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 78 e6[ 	]*vcvttsh2usi \{sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 78 94 f5 00 00 00 10[ 	]*vcvttsh2usi 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 78 11[ 	]*vcvttsh2usi \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 51 7f[ 	]*vcvttsh2usi 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 52 80[ 	]*vcvttsh2usi -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 78 a4 f5 00 00 00 10[ 	]*vcvttsh2usi 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 78 21[ 	]*vcvttsh2usi \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 78 61 7f[ 	]*vcvttsh2usi 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 78 62 80[ 	]*vcvttsh2usi -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 48 7a f5[ 	]*vcvtudq2ph %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 18 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 9f 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 4f 7a b4 f5 00 00 00 10[ 	]*vcvtudq2ph 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 58 7a 31[ 	]*vcvtudq2ph \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 48 7a 71 7f[ 	]*vcvtudq2ph 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f df 7a 72 80[ 	]*vcvtudq2ph -0x200\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 48 7a f5[ 	]*vcvtuqq2ph %zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 18 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 9f 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 ff 4f 7a b4 f5 00 00 00 10[ 	]*vcvtuqq2phz 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 ff 58 7a 31[ 	]*vcvtuqq2ph \(%r9\)\{1to8\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 48 7a 71 7f[ 	]*vcvtuqq2phz 0x1fc0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 ff df 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 45 96 00 7b f4[ 	]*vcvtusi2sh %r12,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 96 10 7b f4[ 	]*vcvtusi2sh %r12,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 7b f2[ 	]*vcvtusi2sh %edx,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 10 7b f2[ 	]*vcvtusi2sh %edx,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 16 00 7b b4 f5 00 00 00 10[ 	]*vcvtusi2shl 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 7b 31[ 	]*vcvtusi2shl \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 7b 71 7f[ 	]*vcvtusi2shl 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 7b 72 80[ 	]*vcvtusi2shl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 7b 71 7f[ 	]*vcvtusi2shq 0x3f8\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 7b 72 80[ 	]*vcvtusi2shq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 48 7d f5[ 	]*vcvtuw2ph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 18 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 9f 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 4f 7d b4 f5 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 58 7d 31[ 	]*vcvtuw2ph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 48 7d 71 7f[ 	]*vcvtuw2ph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f df 7d 72 80[ 	]*vcvtuw2ph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 48 7d f5[ 	]*vcvtw2ph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 18 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 9f 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 4f 7d b4 f5 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 58 7d 31[ 	]*vcvtw2ph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 48 7d 71 7f[ 	]*vcvtw2ph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e df 7d 72 80[ 	]*vcvtw2ph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5e f4[ 	]*vdivph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5e f4[ 	]*vdivph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5e f4[ 	]*vdivph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5e b4 f5 00 00 00 10[ 	]*vdivph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5e 31[ 	]*vdivph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5e 71 7f[ 	]*vdivph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5e 72 80[ 	]*vdivph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5e f4[ 	]*vdivsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5e f4[ 	]*vdivsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5e f4[ 	]*vdivsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5e b4 f5 00 00 00 10[ 	]*vdivsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5e 31[ 	]*vdivsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5e 71 7f[ 	]*vdivsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5e 72 80[ 	]*vdivsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 40 56 f4[ 	]*vfcmaddcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 47 56 b4 f5 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 50 56 31[ 	]*vfcmaddcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 40 56 71 7f[ 	]*vfcmaddcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 d7 56 72 80[ 	]*vfcmaddcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 57 f4[ 	]*vfcmaddcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 57 b4 f5 00 00 00 10[ 	]*vfcmaddcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 00 57 31[ 	]*vfcmaddcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 57 71 7f[ 	]*vfcmaddcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 87 57 72 80[ 	]*vfcmaddcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 40 d6 f4[ 	]*vfcmulcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 47 d6 b4 f5 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 50 d6 31[ 	]*vfcmulcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 40 d6 71 7f[ 	]*vfcmulcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 d7 d6 72 80[ 	]*vfcmulcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 d7 f4[ 	]*vfcmulcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 d7 b4 f5 00 00 00 10[ 	]*vfcmulcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 00 d7 31[ 	]*vfcmulcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 d7 71 7f[ 	]*vfcmulcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 87 d7 72 80[ 	]*vfcmulcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 98 f4[ 	]*vfmadd132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 98 b4 f5 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 98 31[ 	]*vfmadd132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 98 71 7f[ 	]*vfmadd132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 98 72 80[ 	]*vfmadd132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 99 f4[ 	]*vfmadd132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 99 b4 f5 00 00 00 10[ 	]*vfmadd132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 99 31[ 	]*vfmadd132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 99 71 7f[ 	]*vfmadd132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 99 72 80[ 	]*vfmadd132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 a8 f4[ 	]*vfmadd213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 a8 b4 f5 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 a8 31[ 	]*vfmadd213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 a8 71 7f[ 	]*vfmadd213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 a8 72 80[ 	]*vfmadd213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a9 f4[ 	]*vfmadd213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a9 b4 f5 00 00 00 10[ 	]*vfmadd213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 a9 31[ 	]*vfmadd213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a9 71 7f[ 	]*vfmadd213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 a9 72 80[ 	]*vfmadd213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 b8 f4[ 	]*vfmadd231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 b8 b4 f5 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 b8 31[ 	]*vfmadd231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 b8 71 7f[ 	]*vfmadd231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 b8 72 80[ 	]*vfmadd231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b9 f4[ 	]*vfmadd231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b9 b4 f5 00 00 00 10[ 	]*vfmadd231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 b9 31[ 	]*vfmadd231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b9 71 7f[ 	]*vfmadd231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 b9 72 80[ 	]*vfmadd231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 40 56 f4[ 	]*vfmaddcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 47 56 b4 f5 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 50 56 31[ 	]*vfmaddcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 40 56 71 7f[ 	]*vfmaddcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 d7 56 72 80[ 	]*vfmaddcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 57 f4[ 	]*vfmaddcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 57 b4 f5 00 00 00 10[ 	]*vfmaddcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 00 57 31[ 	]*vfmaddcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 57 71 7f[ 	]*vfmaddcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 87 57 72 80[ 	]*vfmaddcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 96 f4[ 	]*vfmaddsub132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 96 b4 f5 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 96 31[ 	]*vfmaddsub132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 96 71 7f[ 	]*vfmaddsub132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 96 72 80[ 	]*vfmaddsub132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 a6 f4[ 	]*vfmaddsub213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 a6 b4 f5 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 a6 31[ 	]*vfmaddsub213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 a6 71 7f[ 	]*vfmaddsub213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 a6 72 80[ 	]*vfmaddsub213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 b6 f4[ 	]*vfmaddsub231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 b6 b4 f5 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 b6 31[ 	]*vfmaddsub231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 b6 71 7f[ 	]*vfmaddsub231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 b6 72 80[ 	]*vfmaddsub231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 9a f4[ 	]*vfmsub132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 9a b4 f5 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 9a 31[ 	]*vfmsub132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 9a 71 7f[ 	]*vfmsub132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 9a 72 80[ 	]*vfmsub132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9b f4[ 	]*vfmsub132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9b b4 f5 00 00 00 10[ 	]*vfmsub132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 9b 31[ 	]*vfmsub132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9b 71 7f[ 	]*vfmsub132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 9b 72 80[ 	]*vfmsub132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 aa f4[ 	]*vfmsub213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 aa b4 f5 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 aa 31[ 	]*vfmsub213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 aa 71 7f[ 	]*vfmsub213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 aa 72 80[ 	]*vfmsub213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ab f4[ 	]*vfmsub213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ab b4 f5 00 00 00 10[ 	]*vfmsub213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 ab 31[ 	]*vfmsub213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ab 71 7f[ 	]*vfmsub213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 ab 72 80[ 	]*vfmsub213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 ba f4[ 	]*vfmsub231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 ba b4 f5 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 ba 31[ 	]*vfmsub231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 ba 71 7f[ 	]*vfmsub231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 ba 72 80[ 	]*vfmsub231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bb f4[ 	]*vfmsub231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bb b4 f5 00 00 00 10[ 	]*vfmsub231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 bb 31[ 	]*vfmsub231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bb 71 7f[ 	]*vfmsub231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 bb 72 80[ 	]*vfmsub231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 97 f4[ 	]*vfmsubadd132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 97 b4 f5 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 97 31[ 	]*vfmsubadd132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 97 71 7f[ 	]*vfmsubadd132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 97 72 80[ 	]*vfmsubadd132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 a7 f4[ 	]*vfmsubadd213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 a7 b4 f5 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 a7 31[ 	]*vfmsubadd213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 a7 71 7f[ 	]*vfmsubadd213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 a7 72 80[ 	]*vfmsubadd213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 b7 f4[ 	]*vfmsubadd231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 b7 b4 f5 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 b7 31[ 	]*vfmsubadd231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 b7 71 7f[ 	]*vfmsubadd231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 b7 72 80[ 	]*vfmsubadd231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 40 d6 f4[ 	]*vfmulcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 47 d6 b4 f5 00 00 00 10[ 	]*vfmulcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 50 d6 31[ 	]*vfmulcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 40 d6 71 7f[ 	]*vfmulcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 d7 d6 72 80[ 	]*vfmulcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 d7 f4[ 	]*vfmulcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 d7 b4 f5 00 00 00 10[ 	]*vfmulcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 00 d7 31[ 	]*vfmulcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 d7 71 7f[ 	]*vfmulcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 87 d7 72 80[ 	]*vfmulcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 9c f4[ 	]*vfnmadd132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 9c b4 f5 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 9c 31[ 	]*vfnmadd132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 9c 71 7f[ 	]*vfnmadd132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 9c 72 80[ 	]*vfnmadd132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9d f4[ 	]*vfnmadd132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9d b4 f5 00 00 00 10[ 	]*vfnmadd132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 9d 31[ 	]*vfnmadd132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9d 71 7f[ 	]*vfnmadd132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 9d 72 80[ 	]*vfnmadd132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 ac f4[ 	]*vfnmadd213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 ac b4 f5 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 ac 31[ 	]*vfnmadd213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 ac 71 7f[ 	]*vfnmadd213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 ac 72 80[ 	]*vfnmadd213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ad f4[ 	]*vfnmadd213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ad b4 f5 00 00 00 10[ 	]*vfnmadd213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 ad 31[ 	]*vfnmadd213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ad 71 7f[ 	]*vfnmadd213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 ad 72 80[ 	]*vfnmadd213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 bc f4[ 	]*vfnmadd231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 bc b4 f5 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 bc 31[ 	]*vfnmadd231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 bc 71 7f[ 	]*vfnmadd231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 bc 72 80[ 	]*vfnmadd231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bd f4[ 	]*vfnmadd231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bd b4 f5 00 00 00 10[ 	]*vfnmadd231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 bd 31[ 	]*vfnmadd231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bd 71 7f[ 	]*vfnmadd231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 bd 72 80[ 	]*vfnmadd231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 9e f4[ 	]*vfnmsub132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 9e b4 f5 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 9e 31[ 	]*vfnmsub132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 9e 71 7f[ 	]*vfnmsub132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 9e 72 80[ 	]*vfnmsub132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9f f4[ 	]*vfnmsub132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9f b4 f5 00 00 00 10[ 	]*vfnmsub132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 9f 31[ 	]*vfnmsub132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9f 71 7f[ 	]*vfnmsub132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 9f 72 80[ 	]*vfnmsub132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 ae f4[ 	]*vfnmsub213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 ae b4 f5 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 ae 31[ 	]*vfnmsub213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 ae 71 7f[ 	]*vfnmsub213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 ae 72 80[ 	]*vfnmsub213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 af f4[ 	]*vfnmsub213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 af b4 f5 00 00 00 10[ 	]*vfnmsub213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 af 31[ 	]*vfnmsub213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 af 71 7f[ 	]*vfnmsub213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 af 72 80[ 	]*vfnmsub213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 be f4[ 	]*vfnmsub231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 be b4 f5 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 be 31[ 	]*vfnmsub231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 be 71 7f[ 	]*vfnmsub231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 be 72 80[ 	]*vfnmsub231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bf f4[ 	]*vfnmsub231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bf b4 f5 00 00 00 10[ 	]*vfnmsub231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 bf 31[ 	]*vfnmsub231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bf 71 7f[ 	]*vfnmsub231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 bf 72 80[ 	]*vfnmsub231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 48 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 4f 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 7c 4f 66 ac f5 00 00 00 10 7b[ 	]*vfpclassphz \$0x7b,0x10000000\(%rbp,%r14,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 58 66 29 7b[ 	]*vfpclassph \$0x7b,\(%r9\)\{1to32\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 69 7f 7b[ 	]*vfpclassphz \$0x7b,0x1fc0\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 5f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%rdx\)\{1to32\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 08 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 0f 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 7c 0f 67 ac f5 00 00 00 10 7b[ 	]*vfpclasssh \$0x7b,0x10000000\(%rbp,%r14,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 08 67 29 7b[ 	]*vfpclasssh \$0x7b,\(%r9\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 69 7f 7b[ 	]*vfpclasssh \$0x7b,0xfe\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 6a 80 7b[ 	]*vfpclasssh \$0x7b,-0x100\(%rdx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 42 f5[ 	]*vgetexpph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 18 42 f5[ 	]*vgetexpph \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 9f 42 f5[ 	]*vgetexpph \{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 42 b4 f5 00 00 00 10[ 	]*vgetexpph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 42 31[ 	]*vgetexpph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 42 71 7f[ 	]*vgetexpph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 42 72 80[ 	]*vgetexpph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 43 f4[ 	]*vgetexpsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 43 f4[ 	]*vgetexpsh \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 43 f4[ 	]*vgetexpsh \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 43 b4 f5 00 00 00 10[ 	]*vgetexpsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 43 31[ 	]*vgetexpsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 43 71 7f[ 	]*vgetexpsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 43 72 80[ 	]*vgetexpsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 48 26 f5 7b[ 	]*vgetmantph \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 18 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 9f 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 4f 26 b4 f5 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 58 26 31 7b[ 	]*vgetmantph \$0x7b,\(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 48 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c df 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 14 00 27 f4 7b[ 	]*vgetmantsh \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 10 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 97 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 14 07 27 b4 f5 00 00 00 10 7b[ 	]*vgetmantsh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 14 00 27 31 7b[ 	]*vgetmantsh \$0x7b,\(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 00 27 71 7f 7b[ 	]*vgetmantsh \$0x7b,0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 87 27 72 80 7b[ 	]*vgetmantsh \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5f f4[ 	]*vmaxph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5f f4[ 	]*vmaxph \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5f f4[ 	]*vmaxph \{sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5f b4 f5 00 00 00 10[ 	]*vmaxph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5f 31[ 	]*vmaxph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5f 71 7f[ 	]*vmaxph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5f 72 80[ 	]*vmaxph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5f f4[ 	]*vmaxsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5f f4[ 	]*vmaxsh \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5f f4[ 	]*vmaxsh \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5f b4 f5 00 00 00 10[ 	]*vmaxsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5f 31[ 	]*vmaxsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5f 71 7f[ 	]*vmaxsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5f 72 80[ 	]*vmaxsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5d f4[ 	]*vminph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5d f4[ 	]*vminph \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5d f4[ 	]*vminph \{sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5d b4 f5 00 00 00 10[ 	]*vminph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5d 31[ 	]*vminph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5d 71 7f[ 	]*vminph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5d 72 80[ 	]*vminph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5d f4[ 	]*vminsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5d f4[ 	]*vminsh \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5d f4[ 	]*vminsh \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5d b4 f5 00 00 00 10[ 	]*vminsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5d 31[ 	]*vminsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5d 71 7f[ 	]*vminsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5d 72 80[ 	]*vminsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 10 f4[ 	]*vmovsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 87 10 f4[ 	]*vmovsh %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 10 b4 f5 00 00 00 10[ 	]*vmovsh 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 08 10 31[ 	]*vmovsh \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 10 71 7f[ 	]*vmovsh 0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 8f 10 72 80[ 	]*vmovsh -0x100\(%rdx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 11 b4 f5 00 00 00 10[ 	]*vmovsh %xmm30,0x10000000\(%rbp,%r14,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 08 11 31[ 	]*vmovsh %xmm30,\(%r9\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 11 71 7f[ 	]*vmovsh %xmm30,0xfe\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 0f 11 72 80[ 	]*vmovsh %xmm30,-0x100\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 08 6e b4 f5 00 00 00 10[ 	]*vmovw  0x10000000\(%rbp,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 08 6e 31[ 	]*vmovw  \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 6e 71 7f[ 	]*vmovw  0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 6e 72 80[ 	]*vmovw  -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 08 7e b4 f5 00 00 00 10[ 	]*vmovw  %xmm30,0x10000000\(%rbp,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 08 7e 31[ 	]*vmovw  %xmm30,\(%r9\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7e 71 7f[ 	]*vmovw  %xmm30,0xfe\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7e 72 80[ 	]*vmovw  %xmm30,-0x100\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 59 f4[ 	]*vmulph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 59 f4[ 	]*vmulph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 59 f4[ 	]*vmulph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 59 b4 f5 00 00 00 10[ 	]*vmulph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 59 31[ 	]*vmulph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 59 71 7f[ 	]*vmulph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 59 72 80[ 	]*vmulph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 59 f4[ 	]*vmulsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 59 f4[ 	]*vmulsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 59 f4[ 	]*vmulsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 59 b4 f5 00 00 00 10[ 	]*vmulsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 59 31[ 	]*vmulsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 59 71 7f[ 	]*vmulsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 59 72 80[ 	]*vmulsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 4c f5[ 	]*vrcpph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d cf 4c f5[ 	]*vrcpph %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 4c b4 f5 00 00 00 10[ 	]*vrcpph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 4c 31[ 	]*vrcpph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 4c 71 7f[ 	]*vrcpph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 4c 72 80[ 	]*vrcpph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 4d f4[ 	]*vrcpsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 4d f4[ 	]*vrcpsh %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 4d b4 f5 00 00 00 10[ 	]*vrcpsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 4d 31[ 	]*vrcpsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 4d 71 7f[ 	]*vrcpsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 4d 72 80[ 	]*vrcpsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 48 56 f5 7b[ 	]*vreduceph \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 18 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 9f 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 4f 56 b4 f5 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 58 56 31 7b[ 	]*vreduceph \$0x7b,\(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 48 56 71 7f 7b[ 	]*vreduceph \$0x7b,0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c df 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 14 00 57 f4 7b[ 	]*vreducesh \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 10 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 97 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 14 07 57 b4 f5 00 00 00 10 7b[ 	]*vreducesh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 14 00 57 31 7b[ 	]*vreducesh \$0x7b,\(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 00 57 71 7f 7b[ 	]*vreducesh \$0x7b,0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 87 57 72 80 7b[ 	]*vreducesh \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 48 08 f5 7b[ 	]*vrndscaleph \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 18 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 9f 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 4f 08 b4 f5 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 58 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 48 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c df 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 14 00 0a f4 7b[ 	]*vrndscalesh \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 10 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 97 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 14 07 0a b4 f5 00 00 00 10 7b[ 	]*vrndscalesh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 14 00 0a 31 7b[ 	]*vrndscalesh \$0x7b,\(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 00 0a 71 7f 7b[ 	]*vrndscalesh \$0x7b,0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 87 0a 72 80 7b[ 	]*vrndscalesh \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 4e f5[ 	]*vrsqrtph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d cf 4e f5[ 	]*vrsqrtph %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 4e b4 f5 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 4e 31[ 	]*vrsqrtph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 4e 71 7f[ 	]*vrsqrtph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 4e 72 80[ 	]*vrsqrtph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 4f f4[ 	]*vrsqrtsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 4f f4[ 	]*vrsqrtsh %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 4f b4 f5 00 00 00 10[ 	]*vrsqrtsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 4f 31[ 	]*vrsqrtsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 4f 71 7f[ 	]*vrsqrtsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 4f 72 80[ 	]*vrsqrtsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 2c f4[ 	]*vscalefph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 2c f4[ 	]*vscalefph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 2c f4[ 	]*vscalefph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 2c b4 f5 00 00 00 10[ 	]*vscalefph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 2c 31[ 	]*vscalefph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 2c 71 7f[ 	]*vscalefph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 2c 72 80[ 	]*vscalefph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 2d f4[ 	]*vscalefsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 2d b4 f5 00 00 00 10[ 	]*vscalefsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 2d 31[ 	]*vscalefsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 2d 71 7f[ 	]*vscalefsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 2d 72 80[ 	]*vscalefsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 51 f5[ 	]*vsqrtph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 51 b4 f5 00 00 00 10[ 	]*vsqrtph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 51 31[ 	]*vsqrtph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 51 71 7f[ 	]*vsqrtph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 51 72 80[ 	]*vsqrtph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 51 f4[ 	]*vsqrtsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 51 b4 f5 00 00 00 10[ 	]*vsqrtsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 51 31[ 	]*vsqrtsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 51 71 7f[ 	]*vsqrtsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 51 72 80[ 	]*vsqrtsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5c f4[ 	]*vsubph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5c f4[ 	]*vsubph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5c f4[ 	]*vsubph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5c b4 f5 00 00 00 10[ 	]*vsubph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5c 31[ 	]*vsubph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5c 71 7f[ 	]*vsubph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5c 72 80[ 	]*vsubph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5c f4[ 	]*vsubsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5c f4[ 	]*vsubsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5c f4[ 	]*vsubsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5c b4 f5 00 00 00 10[ 	]*vsubsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5c 31[ 	]*vsubsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5c 71 7f[ 	]*vsubsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5c 72 80[ 	]*vsubsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 2e f5[ 	]*vucomish %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 2e f5[ 	]*vucomish \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 08 2e b4 f5 00 00 00 10[ 	]*vucomish 0x10000000\(%rbp,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 08 2e 31[ 	]*vucomish \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2e 71 7f[ 	]*vucomish 0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2e 72 80[ 	]*vucomish -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 58 f4[ 	]*vaddph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 58 f4[ 	]*vaddph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 58 f4[ 	]*vaddph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 58 b4 f5 00 00 00 10[ 	]*vaddph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 58 31[ 	]*vaddph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 58 71 7f[ 	]*vaddph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 58 72 80[ 	]*vaddph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 58 f4[ 	]*vaddsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 58 f4[ 	]*vaddsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 58 f4[ 	]*vaddsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 58 b4 f5 00 00 00 10[ 	]*vaddsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 58 31[ 	]*vaddsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 58 71 7f[ 	]*vaddsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 58 72 80[ 	]*vaddsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 93 14 40 c2 ec 7b[ 	]*vcmpph \$0x7b,%zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 14 10 c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 14 17 c2 ec 7b[ 	]*vcmpph \$0x7b,\{sae\},%zmm28,%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 14 47 c2 ac f5 00 00 00 10 7b[ 	]*vcmpph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 14 50 c2 29 7b[ 	]*vcmpph \$0x7b,\(%r9\)\{1to32\},%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 40 c2 69 7f 7b[ 	]*vcmpph \$0x7b,0x1fc0\(%rcx\),%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 57 c2 6a 80 7b[ 	]*vcmpph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 00 c2 ec 7b[ 	]*vcmpsh \$0x7b,%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 16 10 c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm28,%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 7b[ 	]*vcmpsh \$0x7b,\{sae\},%xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f5 00 00 00 10 7b[ 	]*vcmpsh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 16 00 c2 29 7b[ 	]*vcmpsh \$0x7b,\(%r9\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 00 c2 69 7f 7b[ 	]*vcmpsh \$0x7b,0xfe\(%rcx\),%xmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 6a 80 7b[ 	]*vcmpsh \$0x7b,-0x100\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 2f f5[ 	]*vcomish %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 2f f5[ 	]*vcomish \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 08 2f b4 f5 00 00 00 10[ 	]*vcomish 0x10000000\(%rbp,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 08 2f 31[ 	]*vcomish \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2f 71 7f[ 	]*vcomish 0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2f 72 80[ 	]*vcomish -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 5b f5[ 	]*vcvtdq2ph %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 5b f5[ 	]*vcvtdq2ph \{rn-sae\},%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 5b b4 f5 00 00 00 10[ 	]*vcvtdq2ph 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 5b 31[ 	]*vcvtdq2ph \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 5b 71 7f[ 	]*vcvtdq2ph 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 5b 72 80[ 	]*vcvtdq2ph -0x200\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 48 5a f5[ 	]*vcvtpd2ph %zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 18 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 9f 5a f5[ 	]*vcvtpd2ph \{rn-sae\},%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 fd 4f 5a b4 f5 00 00 00 10[ 	]*vcvtpd2phz 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fd 58 5a 31[ 	]*vcvtpd2ph \(%r9\)\{1to8\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 48 5a 71 7f[ 	]*vcvtpd2phz 0x1fc0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fd df 5a 72 80[ 	]*vcvtpd2ph -0x400\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 5b f5[ 	]*vcvtph2dq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 5b f5[ 	]*vcvtph2dq \{rn-sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 5b b4 f5 00 00 00 10[ 	]*vcvtph2dq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 5b 31[ 	]*vcvtph2dq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 5b 71 7f[ 	]*vcvtph2dq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 5b 72 80[ 	]*vcvtph2dq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 5a f5[ 	]*vcvtph2pd %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 5a f5[ 	]*vcvtph2pd \{sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 5a f5[ 	]*vcvtph2pd \{sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 5a b4 f5 00 00 00 10[ 	]*vcvtph2pd 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 5a 31[ 	]*vcvtph2pd \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 5a 71 7f[ 	]*vcvtph2pd 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 5a 72 80[ 	]*vcvtph2pd -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 13 f5[ 	]*vcvtph2psx %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 18 13 f5[ 	]*vcvtph2psx \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 9f 13 f5[ 	]*vcvtph2psx \{sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 13 b4 f5 00 00 00 10[ 	]*vcvtph2psx 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 13 31[ 	]*vcvtph2psx \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 13 71 7f[ 	]*vcvtph2psx 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 13 72 80[ 	]*vcvtph2psx -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7b f5[ 	]*vcvtph2qq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7b f5[ 	]*vcvtph2qq \{rn-sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7b b4 f5 00 00 00 10[ 	]*vcvtph2qq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7b 31[ 	]*vcvtph2qq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7b 71 7f[ 	]*vcvtph2qq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7b 72 80[ 	]*vcvtph2qq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 79 f5[ 	]*vcvtph2udq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 79 f5[ 	]*vcvtph2udq \{rn-sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 79 b4 f5 00 00 00 10[ 	]*vcvtph2udq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 79 31[ 	]*vcvtph2udq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 79 71 7f[ 	]*vcvtph2udq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 79 72 80[ 	]*vcvtph2udq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 79 f5[ 	]*vcvtph2uqq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 79 f5[ 	]*vcvtph2uqq \{rn-sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 79 b4 f5 00 00 00 10[ 	]*vcvtph2uqq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 79 31[ 	]*vcvtph2uqq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 79 71 7f[ 	]*vcvtph2uqq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 79 72 80[ 	]*vcvtph2uqq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 7d f5[ 	]*vcvtph2uw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 7d f5[ 	]*vcvtph2uw \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 7d b4 f5 00 00 00 10[ 	]*vcvtph2uw 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 7d 31[ 	]*vcvtph2uw \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 7d 71 7f[ 	]*vcvtph2uw 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 7d 72 80[ 	]*vcvtph2uw -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7d f5[ 	]*vcvtph2w %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7d f5[ 	]*vcvtph2w \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7d b4 f5 00 00 00 10[ 	]*vcvtph2w 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7d 31[ 	]*vcvtph2w \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7d 71 7f[ 	]*vcvtph2w 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7d 72 80[ 	]*vcvtph2w -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 1d f5[ 	]*vcvtps2phx %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 1d f5[ 	]*vcvtps2phx \{rn-sae\},%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 1d b4 f5 00 00 00 10[ 	]*vcvtps2phx 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 1d 31[ 	]*vcvtps2phx \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 1d 71 7f[ 	]*vcvtps2phx 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 1d 72 80[ 	]*vcvtps2phx -0x200\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 48 5b f5[ 	]*vcvtqq2ph %zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 18 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 9f 5b f5[ 	]*vcvtqq2ph \{rn-sae\},%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 fc 4f 5b b4 f5 00 00 00 10[ 	]*vcvtqq2phz 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fc 58 5b 31[ 	]*vcvtqq2ph \(%r9\)\{1to8\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 48 5b 71 7f[ 	]*vcvtqq2phz 0x1fc0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 fc df 5b 72 80[ 	]*vcvtqq2ph -0x400\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 97 00 5a f4[ 	]*vcvtsd2sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 97 10 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 97 97 5a f4[ 	]*vcvtsd2sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 97 07 5a b4 f5 00 00 00 10[ 	]*vcvtsd2sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 97 00 5a 31[ 	]*vcvtsd2sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 97 00 5a 71 7f[ 	]*vcvtsd2sh 0x3f8\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 97 87 5a 72 80[ 	]*vcvtsd2sh -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5a f4[ 	]*vcvtsh2sd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5a f4[ 	]*vcvtsh2sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5a b4 f5 00 00 00 10[ 	]*vcvtsh2sd 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5a 31[ 	]*vcvtsh2sd \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5a 71 7f[ 	]*vcvtsh2sd 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5a 72 80[ 	]*vcvtsh2sd -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 2d d6[ 	]*vcvtsh2si %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 2d d6[ 	]*vcvtsh2si \{rn-sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 2d e6[ 	]*vcvtsh2si %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 2d e6[ 	]*vcvtsh2si \{rn-sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 2d 94 f5 00 00 00 10[ 	]*vcvtsh2si 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 2d 11[ 	]*vcvtsh2si \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 51 7f[ 	]*vcvtsh2si 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 52 80[ 	]*vcvtsh2si -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 2d a4 f5 00 00 00 10[ 	]*vcvtsh2si 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 2d 21[ 	]*vcvtsh2si \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2d 61 7f[ 	]*vcvtsh2si 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2d 62 80[ 	]*vcvtsh2si -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 06 14 00 13 f4[ 	]*vcvtsh2ss %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 14 10 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 14 97 13 f4[ 	]*vcvtsh2ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 14 07 13 b4 f5 00 00 00 10[ 	]*vcvtsh2ss 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 14 00 13 31[ 	]*vcvtsh2ss \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 14 00 13 71 7f[ 	]*vcvtsh2ss 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 14 87 13 72 80[ 	]*vcvtsh2ss -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 79 d6[ 	]*vcvtsh2usi %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 79 d6[ 	]*vcvtsh2usi \{rn-sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 79 e6[ 	]*vcvtsh2usi %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 79 e6[ 	]*vcvtsh2usi \{rn-sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 79 94 f5 00 00 00 10[ 	]*vcvtsh2usi 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 79 11[ 	]*vcvtsh2usi \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 51 7f[ 	]*vcvtsh2usi 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 52 80[ 	]*vcvtsh2usi -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 79 a4 f5 00 00 00 10[ 	]*vcvtsh2usi 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 79 21[ 	]*vcvtsh2usi \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 79 61 7f[ 	]*vcvtsh2usi 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 79 62 80[ 	]*vcvtsh2usi -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 45 96 00 2a f4[ 	]*vcvtsi2sh %r12,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 96 10 2a f4[ 	]*vcvtsi2sh %r12,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 2a f2[ 	]*vcvtsi2sh %edx,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 10 2a f2[ 	]*vcvtsi2sh %edx,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 16 00 2a b4 f5 00 00 00 10[ 	]*vcvtsi2shl 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 2a 31[ 	]*vcvtsi2shl \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 2a 71 7f[ 	]*vcvtsi2shl 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 2a 72 80[ 	]*vcvtsi2shl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 2a 71 7f[ 	]*vcvtsi2shq 0x3f8\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 2a 72 80[ 	]*vcvtsi2shq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 1d f4[ 	]*vcvtss2sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 1d f4[ 	]*vcvtss2sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 1d b4 f5 00 00 00 10[ 	]*vcvtss2sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 00 1d 31[ 	]*vcvtss2sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 1d 71 7f[ 	]*vcvtss2sh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 87 1d 72 80[ 	]*vcvtss2sh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 48 5b f5[ 	]*vcvttph2dq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 18 5b f5[ 	]*vcvttph2dq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 9f 5b f5[ 	]*vcvttph2dq \{sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 4f 5b b4 f5 00 00 00 10[ 	]*vcvttph2dq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 58 5b 31[ 	]*vcvttph2dq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 48 5b 71 7f[ 	]*vcvttph2dq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e df 5b 72 80[ 	]*vcvttph2dq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7a f5[ 	]*vcvttph2qq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7a f5[ 	]*vcvttph2qq \{sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7a f5[ 	]*vcvttph2qq \{sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7a b4 f5 00 00 00 10[ 	]*vcvttph2qq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7a 31[ 	]*vcvttph2qq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7a 71 7f[ 	]*vcvttph2qq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7a 72 80[ 	]*vcvttph2qq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 78 f5[ 	]*vcvttph2udq %ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 78 f5[ 	]*vcvttph2udq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 78 f5[ 	]*vcvttph2udq \{sae\},%ymm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 78 b4 f5 00 00 00 10[ 	]*vcvttph2udq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 78 31[ 	]*vcvttph2udq \(%r9\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 78 71 7f[ 	]*vcvttph2udq 0xfe0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 78 72 80[ 	]*vcvttph2udq -0x100\(%rdx\)\{1to16\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 78 f5[ 	]*vcvttph2uqq %xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 78 f5[ 	]*vcvttph2uqq \{sae\},%xmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 78 b4 f5 00 00 00 10[ 	]*vcvttph2uqq 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 78 31[ 	]*vcvttph2uqq \(%r9\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 78 71 7f[ 	]*vcvttph2uqq 0x7f0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 78 72 80[ 	]*vcvttph2uqq -0x100\(%rdx\)\{1to8\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 7c f5[ 	]*vcvttph2uw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 7c f5[ 	]*vcvttph2uw \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 7c f5[ 	]*vcvttph2uw \{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 7c b4 f5 00 00 00 10[ 	]*vcvttph2uw 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 7c 31[ 	]*vcvttph2uw \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 7c 71 7f[ 	]*vcvttph2uw 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 7c 72 80[ 	]*vcvttph2uw -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 48 7c f5[ 	]*vcvttph2w %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 18 7c f5[ 	]*vcvttph2w \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 9f 7c f5[ 	]*vcvttph2w \{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 4f 7c b4 f5 00 00 00 10[ 	]*vcvttph2w 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 58 7c 31[ 	]*vcvttph2w \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 48 7c 71 7f[ 	]*vcvttph2w 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d df 7c 72 80[ 	]*vcvttph2w -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 2c d6[ 	]*vcvttsh2si %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 2c d6[ 	]*vcvttsh2si \{sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 2c e6[ 	]*vcvttsh2si %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 2c e6[ 	]*vcvttsh2si \{sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 2c 94 f5 00 00 00 10[ 	]*vcvttsh2si 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 2c 11[ 	]*vcvttsh2si \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 51 7f[ 	]*vcvttsh2si 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 52 80[ 	]*vcvttsh2si -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 2c a4 f5 00 00 00 10[ 	]*vcvttsh2si 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 2c 21[ 	]*vcvttsh2si \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2c 61 7f[ 	]*vcvttsh2si 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 2c 62 80[ 	]*vcvttsh2si -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 08 78 d6[ 	]*vcvttsh2usi %xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 95 7e 18 78 d6[ 	]*vcvttsh2usi \{sae\},%xmm30,%edx
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 08 78 e6[ 	]*vcvttsh2usi %xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 15 fe 18 78 e6[ 	]*vcvttsh2usi \{sae\},%xmm30,%r12
[ 	]*[a-f0-9]+:[ 	]*62 b5 7e 08 78 94 f5 00 00 00 10[ 	]*vcvttsh2usi 0x10000000\(%rbp,%r14,8\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 d5 7e 08 78 11[ 	]*vcvttsh2usi \(%r9\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 51 7f[ 	]*vcvttsh2usi 0xfe\(%rcx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 52 80[ 	]*vcvttsh2usi -0x100\(%rdx\),%edx
[ 	]*[a-f0-9]+:[ 	]*62 35 fe 08 78 a4 f5 00 00 00 10[ 	]*vcvttsh2usi 0x10000000\(%rbp,%r14,8\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 55 fe 08 78 21[ 	]*vcvttsh2usi \(%r9\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 78 61 7f[ 	]*vcvttsh2usi 0xfe\(%rcx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 75 fe 08 78 62 80[ 	]*vcvttsh2usi -0x100\(%rdx\),%r12
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 48 7a f5[ 	]*vcvtudq2ph %zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 18 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 9f 7a f5[ 	]*vcvtudq2ph \{rn-sae\},%zmm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 4f 7a b4 f5 00 00 00 10[ 	]*vcvtudq2ph 0x10000000\(%rbp,%r14,8\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 58 7a 31[ 	]*vcvtudq2ph \(%r9\)\{1to16\},%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 48 7a 71 7f[ 	]*vcvtudq2ph 0x1fc0\(%rcx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f df 7a 72 80[ 	]*vcvtudq2ph -0x200\(%rdx\)\{1to16\},%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 48 7a f5[ 	]*vcvtuqq2ph %zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 18 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 9f 7a f5[ 	]*vcvtuqq2ph \{rn-sae\},%zmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 ff 4f 7a b4 f5 00 00 00 10[ 	]*vcvtuqq2phz 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 ff 58 7a 31[ 	]*vcvtuqq2ph \(%r9\)\{1to8\},%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 48 7a 71 7f[ 	]*vcvtuqq2phz 0x1fc0\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 ff df 7a 72 80[ 	]*vcvtuqq2ph -0x400\(%rdx\)\{1to8\},%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 45 96 00 7b f4[ 	]*vcvtusi2sh %r12,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 96 10 7b f4[ 	]*vcvtusi2sh %r12,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 7b f2[ 	]*vcvtusi2sh %edx,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 10 7b f2[ 	]*vcvtusi2sh %edx,\{rn-sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 16 00 7b b4 f5 00 00 00 10[ 	]*vcvtusi2shl 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 7b 31[ 	]*vcvtusi2shl \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 7b 71 7f[ 	]*vcvtusi2shl 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 7b 72 80[ 	]*vcvtusi2shl -0x200\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 7b 71 7f[ 	]*vcvtusi2shq 0x3f8\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 96 00 7b 72 80[ 	]*vcvtusi2shq -0x400\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 48 7d f5[ 	]*vcvtuw2ph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 18 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 9f 7d f5[ 	]*vcvtuw2ph \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 4f 7d b4 f5 00 00 00 10[ 	]*vcvtuw2ph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 58 7d 31[ 	]*vcvtuw2ph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 48 7d 71 7f[ 	]*vcvtuw2ph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7f df 7d 72 80[ 	]*vcvtuw2ph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 48 7d f5[ 	]*vcvtw2ph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 18 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 9f 7d f5[ 	]*vcvtw2ph \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 4f 7d b4 f5 00 00 00 10[ 	]*vcvtw2ph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 58 7d 31[ 	]*vcvtw2ph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 48 7d 71 7f[ 	]*vcvtw2ph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e df 7d 72 80[ 	]*vcvtw2ph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5e f4[ 	]*vdivph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5e f4[ 	]*vdivph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5e f4[ 	]*vdivph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5e b4 f5 00 00 00 10[ 	]*vdivph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5e 31[ 	]*vdivph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5e 71 7f[ 	]*vdivph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5e 72 80[ 	]*vdivph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5e f4[ 	]*vdivsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5e f4[ 	]*vdivsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5e f4[ 	]*vdivsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5e b4 f5 00 00 00 10[ 	]*vdivsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5e 31[ 	]*vdivsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5e 71 7f[ 	]*vdivsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5e 72 80[ 	]*vdivsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 40 56 f4[ 	]*vfcmaddcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 56 f4[ 	]*vfcmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 47 56 b4 f5 00 00 00 10[ 	]*vfcmaddcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 50 56 31[ 	]*vfcmaddcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 40 56 71 7f[ 	]*vfcmaddcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 d7 56 72 80[ 	]*vfcmaddcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 57 f4[ 	]*vfcmaddcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 57 f4[ 	]*vfcmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 57 b4 f5 00 00 00 10[ 	]*vfcmaddcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 00 57 31[ 	]*vfcmaddcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 57 71 7f[ 	]*vfcmaddcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 87 57 72 80[ 	]*vfcmaddcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 40 d6 f4[ 	]*vfcmulcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 d6 f4[ 	]*vfcmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 47 d6 b4 f5 00 00 00 10[ 	]*vfcmulcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 50 d6 31[ 	]*vfcmulcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 40 d6 71 7f[ 	]*vfcmulcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 d7 d6 72 80[ 	]*vfcmulcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 d7 f4[ 	]*vfcmulcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 10 d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 17 97 d7 f4[ 	]*vfcmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 d7 b4 f5 00 00 00 10[ 	]*vfcmulcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 17 00 d7 31[ 	]*vfcmulcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 d7 71 7f[ 	]*vfcmulcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 17 87 d7 72 80[ 	]*vfcmulcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 98 f4[ 	]*vfmadd132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 98 f4[ 	]*vfmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 98 b4 f5 00 00 00 10[ 	]*vfmadd132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 98 31[ 	]*vfmadd132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 98 71 7f[ 	]*vfmadd132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 98 72 80[ 	]*vfmadd132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 99 f4[ 	]*vfmadd132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 99 f4[ 	]*vfmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 99 b4 f5 00 00 00 10[ 	]*vfmadd132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 99 31[ 	]*vfmadd132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 99 71 7f[ 	]*vfmadd132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 99 72 80[ 	]*vfmadd132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 a8 f4[ 	]*vfmadd213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a8 f4[ 	]*vfmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 a8 b4 f5 00 00 00 10[ 	]*vfmadd213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 a8 31[ 	]*vfmadd213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 a8 71 7f[ 	]*vfmadd213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 a8 72 80[ 	]*vfmadd213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a9 f4[ 	]*vfmadd213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a9 f4[ 	]*vfmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a9 b4 f5 00 00 00 10[ 	]*vfmadd213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 a9 31[ 	]*vfmadd213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a9 71 7f[ 	]*vfmadd213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 a9 72 80[ 	]*vfmadd213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 b8 f4[ 	]*vfmadd231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b8 f4[ 	]*vfmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 b8 b4 f5 00 00 00 10[ 	]*vfmadd231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 b8 31[ 	]*vfmadd231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 b8 71 7f[ 	]*vfmadd231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 b8 72 80[ 	]*vfmadd231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b9 f4[ 	]*vfmadd231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b9 f4[ 	]*vfmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b9 b4 f5 00 00 00 10[ 	]*vfmadd231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 b9 31[ 	]*vfmadd231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b9 71 7f[ 	]*vfmadd231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 b9 72 80[ 	]*vfmadd231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 40 56 f4[ 	]*vfmaddcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 56 f4[ 	]*vfmaddcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 47 56 b4 f5 00 00 00 10[ 	]*vfmaddcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 50 56 31[ 	]*vfmaddcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 40 56 71 7f[ 	]*vfmaddcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 d7 56 72 80[ 	]*vfmaddcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 57 f4[ 	]*vfmaddcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 57 f4[ 	]*vfmaddcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 57 b4 f5 00 00 00 10[ 	]*vfmaddcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 00 57 31[ 	]*vfmaddcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 57 71 7f[ 	]*vfmaddcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 87 57 72 80[ 	]*vfmaddcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 96 f4[ 	]*vfmaddsub132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 96 f4[ 	]*vfmaddsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 96 b4 f5 00 00 00 10[ 	]*vfmaddsub132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 96 31[ 	]*vfmaddsub132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 96 71 7f[ 	]*vfmaddsub132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 96 72 80[ 	]*vfmaddsub132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 a6 f4[ 	]*vfmaddsub213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a6 f4[ 	]*vfmaddsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 a6 b4 f5 00 00 00 10[ 	]*vfmaddsub213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 a6 31[ 	]*vfmaddsub213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 a6 71 7f[ 	]*vfmaddsub213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 a6 72 80[ 	]*vfmaddsub213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 b6 f4[ 	]*vfmaddsub231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b6 f4[ 	]*vfmaddsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 b6 b4 f5 00 00 00 10[ 	]*vfmaddsub231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 b6 31[ 	]*vfmaddsub231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 b6 71 7f[ 	]*vfmaddsub231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 b6 72 80[ 	]*vfmaddsub231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 9a f4[ 	]*vfmsub132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9a f4[ 	]*vfmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 9a b4 f5 00 00 00 10[ 	]*vfmsub132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 9a 31[ 	]*vfmsub132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 9a 71 7f[ 	]*vfmsub132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 9a 72 80[ 	]*vfmsub132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9b f4[ 	]*vfmsub132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9b f4[ 	]*vfmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9b b4 f5 00 00 00 10[ 	]*vfmsub132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 9b 31[ 	]*vfmsub132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9b 71 7f[ 	]*vfmsub132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 9b 72 80[ 	]*vfmsub132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 aa f4[ 	]*vfmsub213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 aa f4[ 	]*vfmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 aa b4 f5 00 00 00 10[ 	]*vfmsub213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 aa 31[ 	]*vfmsub213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 aa 71 7f[ 	]*vfmsub213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 aa 72 80[ 	]*vfmsub213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ab f4[ 	]*vfmsub213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ab f4[ 	]*vfmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ab b4 f5 00 00 00 10[ 	]*vfmsub213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 ab 31[ 	]*vfmsub213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ab 71 7f[ 	]*vfmsub213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 ab 72 80[ 	]*vfmsub213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 ba f4[ 	]*vfmsub231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ba f4[ 	]*vfmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 ba b4 f5 00 00 00 10[ 	]*vfmsub231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 ba 31[ 	]*vfmsub231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 ba 71 7f[ 	]*vfmsub231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 ba 72 80[ 	]*vfmsub231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bb f4[ 	]*vfmsub231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bb f4[ 	]*vfmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bb b4 f5 00 00 00 10[ 	]*vfmsub231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 bb 31[ 	]*vfmsub231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bb 71 7f[ 	]*vfmsub231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 bb 72 80[ 	]*vfmsub231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 97 f4[ 	]*vfmsubadd132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 97 f4[ 	]*vfmsubadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 97 b4 f5 00 00 00 10[ 	]*vfmsubadd132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 97 31[ 	]*vfmsubadd132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 97 71 7f[ 	]*vfmsubadd132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 97 72 80[ 	]*vfmsubadd132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 a7 f4[ 	]*vfmsubadd213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 a7 f4[ 	]*vfmsubadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 a7 b4 f5 00 00 00 10[ 	]*vfmsubadd213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 a7 31[ 	]*vfmsubadd213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 a7 71 7f[ 	]*vfmsubadd213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 a7 72 80[ 	]*vfmsubadd213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 b7 f4[ 	]*vfmsubadd231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 b7 f4[ 	]*vfmsubadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 b7 b4 f5 00 00 00 10[ 	]*vfmsubadd231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 b7 31[ 	]*vfmsubadd231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 b7 71 7f[ 	]*vfmsubadd231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 b7 72 80[ 	]*vfmsubadd231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 40 d6 f4[ 	]*vfmulcph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 d6 f4[ 	]*vfmulcph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 47 d6 b4 f5 00 00 00 10[ 	]*vfmulcph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 50 d6 31[ 	]*vfmulcph \(%r9\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 40 d6 71 7f[ 	]*vfmulcph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 d7 d6 72 80[ 	]*vfmulcph -0x200\(%rdx\)\{1to16\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 d7 f4[ 	]*vfmulcsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 10 d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 16 97 d7 f4[ 	]*vfmulcsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 d7 b4 f5 00 00 00 10[ 	]*vfmulcsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 16 00 d7 31[ 	]*vfmulcsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 d7 71 7f[ 	]*vfmulcsh 0x1fc\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 16 87 d7 72 80[ 	]*vfmulcsh -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 9c f4[ 	]*vfnmadd132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9c f4[ 	]*vfnmadd132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 9c b4 f5 00 00 00 10[ 	]*vfnmadd132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 9c 31[ 	]*vfnmadd132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 9c 71 7f[ 	]*vfnmadd132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 9c 72 80[ 	]*vfnmadd132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9d f4[ 	]*vfnmadd132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9d f4[ 	]*vfnmadd132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9d b4 f5 00 00 00 10[ 	]*vfnmadd132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 9d 31[ 	]*vfnmadd132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9d 71 7f[ 	]*vfnmadd132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 9d 72 80[ 	]*vfnmadd132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 ac f4[ 	]*vfnmadd213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ac f4[ 	]*vfnmadd213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 ac b4 f5 00 00 00 10[ 	]*vfnmadd213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 ac 31[ 	]*vfnmadd213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 ac 71 7f[ 	]*vfnmadd213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 ac 72 80[ 	]*vfnmadd213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ad f4[ 	]*vfnmadd213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ad f4[ 	]*vfnmadd213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ad b4 f5 00 00 00 10[ 	]*vfnmadd213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 ad 31[ 	]*vfnmadd213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ad 71 7f[ 	]*vfnmadd213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 ad 72 80[ 	]*vfnmadd213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 bc f4[ 	]*vfnmadd231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bc f4[ 	]*vfnmadd231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 bc b4 f5 00 00 00 10[ 	]*vfnmadd231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 bc 31[ 	]*vfnmadd231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 bc 71 7f[ 	]*vfnmadd231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 bc 72 80[ 	]*vfnmadd231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bd f4[ 	]*vfnmadd231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bd f4[ 	]*vfnmadd231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bd b4 f5 00 00 00 10[ 	]*vfnmadd231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 bd 31[ 	]*vfnmadd231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bd 71 7f[ 	]*vfnmadd231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 bd 72 80[ 	]*vfnmadd231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 9e f4[ 	]*vfnmsub132ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9e f4[ 	]*vfnmsub132ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 9e b4 f5 00 00 00 10[ 	]*vfnmsub132ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 9e 31[ 	]*vfnmsub132ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 9e 71 7f[ 	]*vfnmsub132ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 9e 72 80[ 	]*vfnmsub132ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9f f4[ 	]*vfnmsub132sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 9f f4[ 	]*vfnmsub132sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9f b4 f5 00 00 00 10[ 	]*vfnmsub132sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 9f 31[ 	]*vfnmsub132sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9f 71 7f[ 	]*vfnmsub132sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 9f 72 80[ 	]*vfnmsub132sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 ae f4[ 	]*vfnmsub213ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 ae f4[ 	]*vfnmsub213ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 ae b4 f5 00 00 00 10[ 	]*vfnmsub213ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 ae 31[ 	]*vfnmsub213ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 ae 71 7f[ 	]*vfnmsub213ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 ae 72 80[ 	]*vfnmsub213ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 af f4[ 	]*vfnmsub213sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 af f4[ 	]*vfnmsub213sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 af b4 f5 00 00 00 10[ 	]*vfnmsub213sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 af 31[ 	]*vfnmsub213sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 af 71 7f[ 	]*vfnmsub213sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 af 72 80[ 	]*vfnmsub213sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 be f4[ 	]*vfnmsub231ph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 be f4[ 	]*vfnmsub231ph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 be b4 f5 00 00 00 10[ 	]*vfnmsub231ph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 be 31[ 	]*vfnmsub231ph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 be 71 7f[ 	]*vfnmsub231ph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 be 72 80[ 	]*vfnmsub231ph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bf f4[ 	]*vfnmsub231sh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 bf f4[ 	]*vfnmsub231sh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bf b4 f5 00 00 00 10[ 	]*vfnmsub231sh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 bf 31[ 	]*vfnmsub231sh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bf 71 7f[ 	]*vfnmsub231sh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 bf 72 80[ 	]*vfnmsub231sh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 48 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 4f 66 ee 7b[ 	]*vfpclassph \$0x7b,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 7c 4f 66 ac f5 00 00 00 10 7b[ 	]*vfpclassphz \$0x7b,0x10000000\(%rbp,%r14,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 58 66 29 7b[ 	]*vfpclassph \$0x7b,\(%r9\)\{1to32\},%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 69 7f 7b[ 	]*vfpclassphz \$0x7b,0x1fc0\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 5f 66 6a 80 7b[ 	]*vfpclassph \$0x7b,-0x100\(%rdx\)\{1to32\},%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 08 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 0f 67 ee 7b[ 	]*vfpclasssh \$0x7b,%xmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 7c 0f 67 ac f5 00 00 00 10 7b[ 	]*vfpclasssh \$0x7b,0x10000000\(%rbp,%r14,8\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 08 67 29 7b[ 	]*vfpclasssh \$0x7b,\(%r9\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 69 7f 7b[ 	]*vfpclasssh \$0x7b,0xfe\(%rcx\),%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 6a 80 7b[ 	]*vfpclasssh \$0x7b,-0x100\(%rdx\),%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 42 f5[ 	]*vgetexpph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 18 42 f5[ 	]*vgetexpph \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 9f 42 f5[ 	]*vgetexpph \{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 42 b4 f5 00 00 00 10[ 	]*vgetexpph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 42 31[ 	]*vgetexpph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 42 71 7f[ 	]*vgetexpph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 42 72 80[ 	]*vgetexpph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 43 f4[ 	]*vgetexpsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 43 f4[ 	]*vgetexpsh \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 43 f4[ 	]*vgetexpsh \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 43 b4 f5 00 00 00 10[ 	]*vgetexpsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 43 31[ 	]*vgetexpsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 43 71 7f[ 	]*vgetexpsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 43 72 80[ 	]*vgetexpsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 48 26 f5 7b[ 	]*vgetmantph \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 18 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 9f 26 f5 7b[ 	]*vgetmantph \$0x7b,\{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 4f 26 b4 f5 00 00 00 10 7b[ 	]*vgetmantph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 58 26 31 7b[ 	]*vgetmantph \$0x7b,\(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 48 26 71 7f 7b[ 	]*vgetmantph \$0x7b,0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c df 26 72 80 7b[ 	]*vgetmantph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 14 00 27 f4 7b[ 	]*vgetmantsh \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 10 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 97 27 f4 7b[ 	]*vgetmantsh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 14 07 27 b4 f5 00 00 00 10 7b[ 	]*vgetmantsh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 14 00 27 31 7b[ 	]*vgetmantsh \$0x7b,\(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 00 27 71 7f 7b[ 	]*vgetmantsh \$0x7b,0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 87 27 72 80 7b[ 	]*vgetmantsh \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5f f4[ 	]*vmaxph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5f f4[ 	]*vmaxph \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5f f4[ 	]*vmaxph \{sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5f b4 f5 00 00 00 10[ 	]*vmaxph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5f 31[ 	]*vmaxph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5f 71 7f[ 	]*vmaxph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5f 72 80[ 	]*vmaxph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5f f4[ 	]*vmaxsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5f f4[ 	]*vmaxsh \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5f f4[ 	]*vmaxsh \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5f b4 f5 00 00 00 10[ 	]*vmaxsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5f 31[ 	]*vmaxsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5f 71 7f[ 	]*vmaxsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5f 72 80[ 	]*vmaxsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5d f4[ 	]*vminph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5d f4[ 	]*vminph \{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5d f4[ 	]*vminph \{sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5d b4 f5 00 00 00 10[ 	]*vminph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5d 31[ 	]*vminph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5d 71 7f[ 	]*vminph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5d 72 80[ 	]*vminph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5d f4[ 	]*vminsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5d f4[ 	]*vminsh \{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5d f4[ 	]*vminsh \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5d b4 f5 00 00 00 10[ 	]*vminsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5d 31[ 	]*vminsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5d 71 7f[ 	]*vminsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5d 72 80[ 	]*vminsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 10 f4[ 	]*vmovsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 87 10 f4[ 	]*vmovsh %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 10 b4 f5 00 00 00 10[ 	]*vmovsh 0x10000000\(%rbp,%r14,8\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 08 10 31[ 	]*vmovsh \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 10 71 7f[ 	]*vmovsh 0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 8f 10 72 80[ 	]*vmovsh -0x100\(%rdx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 11 b4 f5 00 00 00 10[ 	]*vmovsh %xmm30,0x10000000\(%rbp,%r14,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 08 11 31[ 	]*vmovsh %xmm30,\(%r9\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 11 71 7f[ 	]*vmovsh %xmm30,0xfe\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 0f 11 72 80[ 	]*vmovsh %xmm30,-0x100\(%rdx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 08 6e b4 f5 00 00 00 10[ 	]*vmovw  0x10000000\(%rbp,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 08 6e 31[ 	]*vmovw  \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 6e 71 7f[ 	]*vmovw  0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 6e 72 80[ 	]*vmovw  -0x100\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 08 7e b4 f5 00 00 00 10[ 	]*vmovw  %xmm30,0x10000000\(%rbp,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 08 7e 31[ 	]*vmovw  %xmm30,\(%r9\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7e 71 7f[ 	]*vmovw  %xmm30,0xfe\(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7e 72 80[ 	]*vmovw  %xmm30,-0x100\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 59 f4[ 	]*vmulph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 59 f4[ 	]*vmulph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 59 f4[ 	]*vmulph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 59 b4 f5 00 00 00 10[ 	]*vmulph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 59 31[ 	]*vmulph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 59 71 7f[ 	]*vmulph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 59 72 80[ 	]*vmulph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 59 f4[ 	]*vmulsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 59 f4[ 	]*vmulsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 59 f4[ 	]*vmulsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 59 b4 f5 00 00 00 10[ 	]*vmulsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 59 31[ 	]*vmulsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 59 71 7f[ 	]*vmulsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 59 72 80[ 	]*vmulsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 4c f5[ 	]*vrcpph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d cf 4c f5[ 	]*vrcpph %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 4c b4 f5 00 00 00 10[ 	]*vrcpph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 4c 31[ 	]*vrcpph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 4c 71 7f[ 	]*vrcpph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 4c 72 80[ 	]*vrcpph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 4d f4[ 	]*vrcpsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 4d f4[ 	]*vrcpsh %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 4d b4 f5 00 00 00 10[ 	]*vrcpsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 4d 31[ 	]*vrcpsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 4d 71 7f[ 	]*vrcpsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 4d 72 80[ 	]*vrcpsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 48 56 f5 7b[ 	]*vreduceph \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 18 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 9f 56 f5 7b[ 	]*vreduceph \$0x7b,\{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 4f 56 b4 f5 00 00 00 10 7b[ 	]*vreduceph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 58 56 31 7b[ 	]*vreduceph \$0x7b,\(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 48 56 71 7f 7b[ 	]*vreduceph \$0x7b,0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c df 56 72 80 7b[ 	]*vreduceph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 14 00 57 f4 7b[ 	]*vreducesh \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 10 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 97 57 f4 7b[ 	]*vreducesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 14 07 57 b4 f5 00 00 00 10 7b[ 	]*vreducesh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 14 00 57 31 7b[ 	]*vreducesh \$0x7b,\(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 00 57 71 7f 7b[ 	]*vreducesh \$0x7b,0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 87 57 72 80 7b[ 	]*vreducesh \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 48 08 f5 7b[ 	]*vrndscaleph \$0x7b,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 18 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 9f 08 f5 7b[ 	]*vrndscaleph \$0x7b,\{sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 4f 08 b4 f5 00 00 00 10 7b[ 	]*vrndscaleph \$0x7b,0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 58 08 31 7b[ 	]*vrndscaleph \$0x7b,\(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 48 08 71 7f 7b[ 	]*vrndscaleph \$0x7b,0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 7c df 08 72 80 7b[ 	]*vrndscaleph \$0x7b,-0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 03 14 00 0a f4 7b[ 	]*vrndscalesh \$0x7b,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 10 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 14 97 0a f4 7b[ 	]*vrndscalesh \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 14 07 0a b4 f5 00 00 00 10 7b[ 	]*vrndscalesh \$0x7b,0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 43 14 00 0a 31 7b[ 	]*vrndscalesh \$0x7b,\(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 00 0a 71 7f 7b[ 	]*vrndscalesh \$0x7b,0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 14 87 0a 72 80 7b[ 	]*vrndscalesh \$0x7b,-0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 48 4e f5[ 	]*vrsqrtph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 7d cf 4e f5[ 	]*vrsqrtph %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 4f 4e b4 f5 00 00 00 10[ 	]*vrsqrtph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 58 4e 31[ 	]*vrsqrtph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 48 4e 71 7f[ 	]*vrsqrtph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 7d df 4e 72 80[ 	]*vrsqrtph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 4f f4[ 	]*vrsqrtsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 4f f4[ 	]*vrsqrtsh %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 4f b4 f5 00 00 00 10[ 	]*vrsqrtsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 4f 31[ 	]*vrsqrtsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 4f 71 7f[ 	]*vrsqrtsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 4f 72 80[ 	]*vrsqrtsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 40 2c f4[ 	]*vscalefph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 2c f4[ 	]*vscalefph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 2c f4[ 	]*vscalefph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 47 2c b4 f5 00 00 00 10[ 	]*vscalefph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 50 2c 31[ 	]*vscalefph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 40 2c 71 7f[ 	]*vscalefph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 d7 2c 72 80[ 	]*vscalefph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 2d f4[ 	]*vscalefsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 10 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 06 15 97 2d f4[ 	]*vscalefsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 2d b4 f5 00 00 00 10[ 	]*vscalefsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 46 15 00 2d 31[ 	]*vscalefsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 2d 71 7f[ 	]*vscalefsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 66 15 87 2d 72 80[ 	]*vscalefsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 48 51 f5[ 	]*vsqrtph %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 9f 51 f5[ 	]*vsqrtph \{rn-sae\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 4f 51 b4 f5 00 00 00 10[ 	]*vsqrtph 0x10000000\(%rbp,%r14,8\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 58 51 31[ 	]*vsqrtph \(%r9\)\{1to32\},%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 48 51 71 7f[ 	]*vsqrtph 0x1fc0\(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c df 51 72 80[ 	]*vsqrtph -0x100\(%rdx\)\{1to32\},%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 51 f4[ 	]*vsqrtsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 51 f4[ 	]*vsqrtsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 51 b4 f5 00 00 00 10[ 	]*vsqrtsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 51 31[ 	]*vsqrtsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 51 71 7f[ 	]*vsqrtsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 51 72 80[ 	]*vsqrtsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 14 40 5c f4[ 	]*vsubph %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 10 5c f4[ 	]*vsubph \{rn-sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 14 97 5c f4[ 	]*vsubph \{rn-sae\},%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 14 47 5c b4 f5 00 00 00 10[ 	]*vsubph 0x10000000\(%rbp,%r14,8\),%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 14 50 5c 31[ 	]*vsubph \(%r9\)\{1to32\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 40 5c 71 7f[ 	]*vsubph 0x1fc0\(%rcx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 14 d7 5c 72 80[ 	]*vsubph -0x100\(%rdx\)\{1to32\},%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 16 00 5c f4[ 	]*vsubsh %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 10 5c f4[ 	]*vsubsh \{rn-sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 16 97 5c f4[ 	]*vsubsh \{rn-sae\},%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 25 16 07 5c b4 f5 00 00 00 10[ 	]*vsubsh 0x10000000\(%rbp,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 45 16 00 5c 31[ 	]*vsubsh \(%r9\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 00 5c 71 7f[ 	]*vsubsh 0xfe\(%rcx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 16 87 5c 72 80[ 	]*vsubsh -0x100\(%rdx\),%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 2e f5[ 	]*vucomish %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 18 2e f5[ 	]*vucomish \{sae\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 08 2e b4 f5 00 00 00 10[ 	]*vucomish 0x10000000\(%rbp,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 08 2e 31[ 	]*vucomish \(%r9\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2e 71 7f[ 	]*vucomish 0xfe\(%rcx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 2e 72 80[ 	]*vucomish -0x100\(%rdx\),%xmm30
#pass
