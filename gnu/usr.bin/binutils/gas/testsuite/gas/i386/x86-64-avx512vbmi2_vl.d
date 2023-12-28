#as:
#objdump: -dw
#name: x86_64 AVX512VBMI2/VL insns
#source: x86-64-avx512vbmi2_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 63 31[ 	]*vpcompressb %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 63 b4 f0 23 01 00 00[ 	]*vpcompressb %xmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 63 72 7f[ 	]*vpcompressb %xmm30,0x7f\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 63 31[ 	]*vpcompressb %ymm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 63 b4 f0 23 01 00 00[ 	]*vpcompressb %ymm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 63 72 7f[ 	]*vpcompressb %ymm30,0x7f\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 63 ee[ 	]*vpcompressb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 63 ee[ 	]*vpcompressb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 63 ee[ 	]*vpcompressb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 63 ee[ 	]*vpcompressb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 63 ee[ 	]*vpcompressb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 63 ee[ 	]*vpcompressb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 63 31[ 	]*vpcompressw %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 63 b4 f0 23 01 00 00[ 	]*vpcompressw %xmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 63 72 7f[ 	]*vpcompressw %xmm30,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 63 31[ 	]*vpcompressw %ymm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 63 b4 f0 23 01 00 00[ 	]*vpcompressw %ymm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 63 72 7f[ 	]*vpcompressw %ymm30,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 63 ee[ 	]*vpcompressw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 63 ee[ 	]*vpcompressw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 63 ee[ 	]*vpcompressw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 63 ee[ 	]*vpcompressw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 63 ee[ 	]*vpcompressw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 63 ee[ 	]*vpcompressw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 62 31[ 	]*vpexpandb \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 62 31[ 	]*vpexpandb \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 62 b4 f0 23 01 00 00[ 	]*vpexpandb 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 62 72 7f[ 	]*vpexpandb 0x7f\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 62 31[ 	]*vpexpandb \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 62 31[ 	]*vpexpandb \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 62 b4 f0 23 01 00 00[ 	]*vpexpandb 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 62 72 7f[ 	]*vpexpandb 0x7f\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 62 f5[ 	]*vpexpandb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 62 f5[ 	]*vpexpandb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 62 f5[ 	]*vpexpandb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 62 f5[ 	]*vpexpandb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 62 f5[ 	]*vpexpandb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 62 f5[ 	]*vpexpandb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 62 31[ 	]*vpexpandw \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 8f 62 31[ 	]*vpexpandw \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 62 b4 f0 23 01 00 00[ 	]*vpexpandw 0x123\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 62 72 7f[ 	]*vpexpandw 0xfe\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 62 31[ 	]*vpexpandw \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 62 31[ 	]*vpexpandw \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 62 b4 f0 23 01 00 00[ 	]*vpexpandw 0x123\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 62 72 7f[ 	]*vpexpandw 0xfe\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 62 f5[ 	]*vpexpandw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 62 f5[ 	]*vpexpandw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 62 f5[ 	]*vpexpandw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 62 f5[ 	]*vpexpandw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 62 f5[ 	]*vpexpandw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 62 f5[ 	]*vpexpandw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 70 f4[ 	]*vpshldvw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 70 f4[ 	]*vpshldvw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 70 f4[ 	]*vpshldvw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 70 b4 f0 23 01 00 00[ 	]*vpshldvw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 70 72 7f[ 	]*vpshldvw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 70 f4[ 	]*vpshldvw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 70 f4[ 	]*vpshldvw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 70 f4[ 	]*vpshldvw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 70 b4 f0 23 01 00 00[ 	]*vpshldvw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 70 72 7f[ 	]*vpshldvw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 71 f4[ 	]*vpshldvd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 71 f4[ 	]*vpshldvd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 71 f4[ 	]*vpshldvd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 71 b4 f0 23 01 00 00[ 	]*vpshldvd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 71 72 7f[ 	]*vpshldvd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 71 72 7f[ 	]*vpshldvd 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 71 f4[ 	]*vpshldvd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 71 f4[ 	]*vpshldvd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 71 f4[ 	]*vpshldvd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 71 b4 f0 23 01 00 00[ 	]*vpshldvd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 71 72 7f[ 	]*vpshldvd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 71 72 7f[ 	]*vpshldvd 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 71 f4[ 	]*vpshldvq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 71 f4[ 	]*vpshldvq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 71 f4[ 	]*vpshldvq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 71 b4 f0 23 01 00 00[ 	]*vpshldvq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 71 72 7f[ 	]*vpshldvq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 71 72 7f[ 	]*vpshldvq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 71 f4[ 	]*vpshldvq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 71 f4[ 	]*vpshldvq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 71 f4[ 	]*vpshldvq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 71 b4 f0 23 01 00 00[ 	]*vpshldvq 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 71 72 7f[ 	]*vpshldvq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 71 72 7f[ 	]*vpshldvq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 72 f4[ 	]*vpshrdvw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 72 f4[ 	]*vpshrdvw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 72 f4[ 	]*vpshrdvw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 72 b4 f0 23 01 00 00[ 	]*vpshrdvw 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 72 72 7f[ 	]*vpshrdvw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 72 f4[ 	]*vpshrdvw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 72 f4[ 	]*vpshrdvw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 72 f4[ 	]*vpshrdvw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 72 b4 f0 23 01 00 00[ 	]*vpshrdvw 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 72 72 7f[ 	]*vpshrdvw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 73 f4[ 	]*vpshrdvd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 73 f4[ 	]*vpshrdvd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 73 f4[ 	]*vpshrdvd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 73 b4 f0 23 01 00 00[ 	]*vpshrdvd 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 73 72 7f[ 	]*vpshrdvd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 73 72 7f[ 	]*vpshrdvd 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 73 f4[ 	]*vpshrdvd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 73 f4[ 	]*vpshrdvd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 73 f4[ 	]*vpshrdvd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 73 b4 f0 23 01 00 00[ 	]*vpshrdvd 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 73 72 7f[ 	]*vpshrdvd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 73 72 7f[ 	]*vpshrdvd 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 73 f4[ 	]*vpshrdvq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 73 f4[ 	]*vpshrdvq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 73 f4[ 	]*vpshrdvq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 73 b4 f0 23 01 00 00[ 	]*vpshrdvq 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 73 72 7f[ 	]*vpshrdvq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 73 72 7f[ 	]*vpshrdvq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 73 f4[ 	]*vpshrdvq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 73 f4[ 	]*vpshrdvq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 73 f4[ 	]*vpshrdvq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 73 b4 f0 23 01 00 00[ 	]*vpshrdvq 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 73 72 7f[ 	]*vpshrdvq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 73 72 7f[ 	]*vpshrdvq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 70 f4 ab[ 	]*vpshldw \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 70 f4 ab[ 	]*vpshldw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 70 f4 ab[ 	]*vpshldw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 70 b4 f0 23 01 00 00 7b[ 	]*vpshldw \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 70 72 7f 7b[ 	]*vpshldw \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 70 f4 ab[ 	]*vpshldw \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 70 f4 ab[ 	]*vpshldw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 70 f4 ab[ 	]*vpshldw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 70 b4 f0 23 01 00 00 7b[ 	]*vpshldw \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 70 72 7f 7b[ 	]*vpshldw \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 71 f4 ab[ 	]*vpshldd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 71 f4 ab[ 	]*vpshldd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 71 f4 ab[ 	]*vpshldd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 71 b4 f0 23 01 00 00 7b[ 	]*vpshldd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 71 f4 ab[ 	]*vpshldd \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 71 f4 ab[ 	]*vpshldd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 71 f4 ab[ 	]*vpshldd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 71 b4 f0 23 01 00 00 7b[ 	]*vpshldd \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 71 72 7f 7b[ 	]*vpshldd \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 71 f4 ab[ 	]*vpshldq \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 71 f4 ab[ 	]*vpshldq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 71 f4 ab[ 	]*vpshldq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 71 b4 f0 23 01 00 00 7b[ 	]*vpshldq \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 71 f4 ab[ 	]*vpshldq \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 71 f4 ab[ 	]*vpshldq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 71 f4 ab[ 	]*vpshldq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 71 b4 f0 23 01 00 00 7b[ 	]*vpshldq \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 71 72 7f 7b[ 	]*vpshldq \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 72 b4 f0 23 01 00 00 7b[ 	]*vpshrdw \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 72 b4 f0 23 01 00 00 7b[ 	]*vpshrdw \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdd \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdd \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdq \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdq \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 63 31[ 	]*vpcompressb %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 63 b4 f0 34 12 00 00[ 	]*vpcompressb %xmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 63 72 7f[ 	]*vpcompressb %xmm30,0x7f\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 63 31[ 	]*vpcompressb %ymm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 63 b4 f0 34 12 00 00[ 	]*vpcompressb %ymm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 63 72 7f[ 	]*vpcompressb %ymm30,0x7f\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 63 ee[ 	]*vpcompressb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 63 ee[ 	]*vpcompressb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 63 ee[ 	]*vpcompressb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 63 ee[ 	]*vpcompressb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 63 ee[ 	]*vpcompressb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 63 ee[ 	]*vpcompressb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 63 31[ 	]*vpcompressw %xmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 63 b4 f0 34 12 00 00[ 	]*vpcompressw %xmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 63 72 7f[ 	]*vpcompressw %xmm30,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 63 31[ 	]*vpcompressw %ymm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 63 b4 f0 34 12 00 00[ 	]*vpcompressw %ymm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 63 72 7f[ 	]*vpcompressw %ymm30,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 63 ee[ 	]*vpcompressw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 63 ee[ 	]*vpcompressw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 63 ee[ 	]*vpcompressw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 63 ee[ 	]*vpcompressw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 63 ee[ 	]*vpcompressw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 63 ee[ 	]*vpcompressw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 62 31[ 	]*vpexpandb \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 62 31[ 	]*vpexpandb \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 62 b4 f0 34 12 00 00[ 	]*vpexpandb 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 62 72 7f[ 	]*vpexpandb 0x7f\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 62 31[ 	]*vpexpandb \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 62 31[ 	]*vpexpandb \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 62 b4 f0 34 12 00 00[ 	]*vpexpandb 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 62 72 7f[ 	]*vpexpandb 0x7f\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 62 f5[ 	]*vpexpandb %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 62 f5[ 	]*vpexpandb %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 62 f5[ 	]*vpexpandb %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 62 f5[ 	]*vpexpandb %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 62 f5[ 	]*vpexpandb %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 62 f5[ 	]*vpexpandb %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 0f 62 31[ 	]*vpexpandw \(%rcx\),%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 8f 62 31[ 	]*vpexpandw \(%rcx\),%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 62 b4 f0 34 12 00 00[ 	]*vpexpandw 0x1234\(%rax,%r14,8\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 62 72 7f[ 	]*vpexpandw 0xfe\(%rdx\),%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 62 31[ 	]*vpexpandw \(%rcx\),%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 62 31[ 	]*vpexpandw \(%rcx\),%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 62 b4 f0 34 12 00 00[ 	]*vpexpandw 0x1234\(%rax,%r14,8\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 62 72 7f[ 	]*vpexpandw 0xfe\(%rdx\),%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 62 f5[ 	]*vpexpandw %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 62 f5[ 	]*vpexpandw %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 62 f5[ 	]*vpexpandw %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 62 f5[ 	]*vpexpandw %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 62 f5[ 	]*vpexpandw %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 62 f5[ 	]*vpexpandw %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 70 f4[ 	]*vpshldvw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 70 f4[ 	]*vpshldvw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 70 f4[ 	]*vpshldvw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 70 b4 f0 34 12 00 00[ 	]*vpshldvw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 70 72 7f[ 	]*vpshldvw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 70 f4[ 	]*vpshldvw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 70 f4[ 	]*vpshldvw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 70 f4[ 	]*vpshldvw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 70 b4 f0 34 12 00 00[ 	]*vpshldvw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 70 72 7f[ 	]*vpshldvw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 71 f4[ 	]*vpshldvd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 71 f4[ 	]*vpshldvd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 71 f4[ 	]*vpshldvd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 71 b4 f0 34 12 00 00[ 	]*vpshldvd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 71 72 7f[ 	]*vpshldvd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 71 72 7f[ 	]*vpshldvd 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 71 f4[ 	]*vpshldvd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 71 f4[ 	]*vpshldvd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 71 f4[ 	]*vpshldvd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 71 b4 f0 34 12 00 00[ 	]*vpshldvd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 71 72 7f[ 	]*vpshldvd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 71 72 7f[ 	]*vpshldvd 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 71 f4[ 	]*vpshldvq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 71 f4[ 	]*vpshldvq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 71 f4[ 	]*vpshldvq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 71 b4 f0 34 12 00 00[ 	]*vpshldvq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 71 72 7f[ 	]*vpshldvq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 71 72 7f[ 	]*vpshldvq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 71 f4[ 	]*vpshldvq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 71 f4[ 	]*vpshldvq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 71 f4[ 	]*vpshldvq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 71 b4 f0 34 12 00 00[ 	]*vpshldvq 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 71 72 7f[ 	]*vpshldvq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 71 72 7f[ 	]*vpshldvq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 72 f4[ 	]*vpshrdvw %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 72 f4[ 	]*vpshrdvw %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 72 f4[ 	]*vpshrdvw %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 72 b4 f0 34 12 00 00[ 	]*vpshrdvw 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 72 72 7f[ 	]*vpshrdvw 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 72 f4[ 	]*vpshrdvw %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 72 f4[ 	]*vpshrdvw %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 72 f4[ 	]*vpshrdvw %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 72 b4 f0 34 12 00 00[ 	]*vpshrdvw 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 72 72 7f[ 	]*vpshrdvw 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 73 f4[ 	]*vpshrdvd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 73 f4[ 	]*vpshrdvd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 73 f4[ 	]*vpshrdvd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 73 b4 f0 34 12 00 00[ 	]*vpshrdvd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 73 72 7f[ 	]*vpshrdvd 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 73 72 7f[ 	]*vpshrdvd 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 73 f4[ 	]*vpshrdvd %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 73 f4[ 	]*vpshrdvd %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 73 f4[ 	]*vpshrdvd %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 73 b4 f0 34 12 00 00[ 	]*vpshrdvd 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 73 72 7f[ 	]*vpshrdvd 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 73 72 7f[ 	]*vpshrdvd 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 73 f4[ 	]*vpshrdvq %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 73 f4[ 	]*vpshrdvq %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 73 f4[ 	]*vpshrdvq %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 73 b4 f0 34 12 00 00[ 	]*vpshrdvq 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 73 72 7f[ 	]*vpshrdvq 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 73 72 7f[ 	]*vpshrdvq 0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 73 f4[ 	]*vpshrdvq %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 73 f4[ 	]*vpshrdvq %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 73 f4[ 	]*vpshrdvq %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 73 b4 f0 34 12 00 00[ 	]*vpshrdvq 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 73 72 7f[ 	]*vpshrdvq 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 73 72 7f[ 	]*vpshrdvq 0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 70 f4 ab[ 	]*vpshldw \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 70 f4 ab[ 	]*vpshldw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 70 f4 ab[ 	]*vpshldw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 70 b4 f0 34 12 00 00 7b[ 	]*vpshldw \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 70 72 7f 7b[ 	]*vpshldw \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 70 f4 ab[ 	]*vpshldw \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 70 f4 ab[ 	]*vpshldw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 70 f4 ab[ 	]*vpshldw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 70 b4 f0 34 12 00 00 7b[ 	]*vpshldw \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 70 72 7f 7b[ 	]*vpshldw \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 71 f4 ab[ 	]*vpshldd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 71 f4 ab[ 	]*vpshldd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 71 f4 ab[ 	]*vpshldd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 71 b4 f0 34 12 00 00 7b[ 	]*vpshldd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 71 f4 ab[ 	]*vpshldd \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 71 f4 ab[ 	]*vpshldd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 71 f4 ab[ 	]*vpshldd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 71 b4 f0 34 12 00 00 7b[ 	]*vpshldd \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 71 72 7f 7b[ 	]*vpshldd \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 71 f4 ab[ 	]*vpshldq \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 71 f4 ab[ 	]*vpshldq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 71 f4 ab[ 	]*vpshldq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 71 b4 f0 34 12 00 00 7b[ 	]*vpshldq \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 71 f4 ab[ 	]*vpshldq \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 71 f4 ab[ 	]*vpshldq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 71 f4 ab[ 	]*vpshldq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 71 b4 f0 34 12 00 00 7b[ 	]*vpshldq \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 71 72 7f 7b[ 	]*vpshldq \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 72 b4 f0 34 12 00 00 7b[ 	]*vpshrdw \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 72 b4 f0 34 12 00 00 7b[ 	]*vpshrdw \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdd \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdd \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdq \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdq \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
#pass
