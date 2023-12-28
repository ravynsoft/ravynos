#as:
#objdump: -dw
#name: x86_64 AVX512VBMI2 insns
#source: x86-64-avx512vbmi2.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 63 31[ 	]*vpcompressb %zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 63 b4 f0 23 01 00 00[ 	]*vpcompressb %zmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 63 72 7e[ 	]*vpcompressb %zmm30,0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 63 ee[ 	]*vpcompressb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 63 ee[ 	]*vpcompressb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 63 ee[ 	]*vpcompressb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 63 31[ 	]*vpcompressw %zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 63 b4 f0 23 01 00 00[ 	]*vpcompressw %zmm30,0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 63 72 7f[ 	]*vpcompressw %zmm30,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 63 ee[ 	]*vpcompressw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 63 ee[ 	]*vpcompressw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 63 ee[ 	]*vpcompressw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 62 31[ 	]*vpexpandb \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 62 31[ 	]*vpexpandb \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 62 b4 f0 23 01 00 00[ 	]*vpexpandb 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 62 72 7e[ 	]*vpexpandb 0x7e\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 62 f5[ 	]*vpexpandb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 62 f5[ 	]*vpexpandb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 62 f5[ 	]*vpexpandb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 62 31[ 	]*vpexpandw \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 62 31[ 	]*vpexpandw \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 62 b4 f0 23 01 00 00[ 	]*vpexpandw 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 62 72 7f[ 	]*vpexpandw 0xfe\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 62 f5[ 	]*vpexpandw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 62 f5[ 	]*vpexpandw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 62 f5[ 	]*vpexpandw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 70 f4[ 	]*vpshldvw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 70 f4[ 	]*vpshldvw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 70 f4[ 	]*vpshldvw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 70 b4 f0 23 01 00 00[ 	]*vpshldvw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 70 72 7f[ 	]*vpshldvw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 71 f4[ 	]*vpshldvd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 71 f4[ 	]*vpshldvd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 71 f4[ 	]*vpshldvd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 71 b4 f0 23 01 00 00[ 	]*vpshldvd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 71 72 7f[ 	]*vpshldvd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 71 72 7f[ 	]*vpshldvd 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 71 f4[ 	]*vpshldvq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 71 f4[ 	]*vpshldvq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 71 f4[ 	]*vpshldvq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 71 b4 f0 23 01 00 00[ 	]*vpshldvq 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 71 72 7f[ 	]*vpshldvq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 71 72 7f[ 	]*vpshldvq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 72 f4[ 	]*vpshrdvw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 72 f4[ 	]*vpshrdvw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 72 f4[ 	]*vpshrdvw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 72 b4 f0 23 01 00 00[ 	]*vpshrdvw 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 72 72 7f[ 	]*vpshrdvw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 73 f4[ 	]*vpshrdvd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 73 f4[ 	]*vpshrdvd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 73 f4[ 	]*vpshrdvd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 73 b4 f0 23 01 00 00[ 	]*vpshrdvd 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 73 72 7f[ 	]*vpshrdvd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 73 f4[ 	]*vpshrdvq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 73 f4[ 	]*vpshrdvq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 73 f4[ 	]*vpshrdvq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 73 b4 f0 23 01 00 00[ 	]*vpshrdvq 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 73 72 7f[ 	]*vpshrdvq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 70 f4 ab[ 	]*vpshldw \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 70 f4 ab[ 	]*vpshldw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 70 f4 ab[ 	]*vpshldw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 70 b4 f0 23 01 00 00 7b[ 	]*vpshldw \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 70 72 7f 7b[ 	]*vpshldw \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 71 f4 ab[ 	]*vpshldd \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 71 f4 ab[ 	]*vpshldd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 71 f4 ab[ 	]*vpshldd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 71 b4 f0 23 01 00 00 7b[ 	]*vpshldd \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 71 f4 ab[ 	]*vpshldq \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 71 f4 ab[ 	]*vpshldq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 71 f4 ab[ 	]*vpshldq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 71 b4 f0 23 01 00 00 7b[ 	]*vpshldq \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 72 b4 f0 23 01 00 00 7b[ 	]*vpshrdw \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdd \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 73 31 7b[ 	]*vpshrdd \$0x7b,\(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 73 b4 f0 23 01 00 00 7b[ 	]*vpshrdq \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 73 31 7b[ 	]*vpshrdq \$0x7b,\(%rcx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 63 31[ 	]*vpcompressb %zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 63 b4 f0 34 12 00 00[ 	]*vpcompressb %zmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 63 72 7e[ 	]*vpcompressb %zmm30,0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 63 ee[ 	]*vpcompressb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 63 ee[ 	]*vpcompressb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 63 ee[ 	]*vpcompressb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 63 31[ 	]*vpcompressw %zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 63 b4 f0 34 12 00 00[ 	]*vpcompressw %zmm30,0x1234\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 63 72 7f[ 	]*vpcompressw %zmm30,0xfe\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 63 ee[ 	]*vpcompressw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 63 ee[ 	]*vpcompressw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 63 ee[ 	]*vpcompressw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 62 31[ 	]*vpexpandb \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 62 31[ 	]*vpexpandb \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 62 b4 f0 34 12 00 00[ 	]*vpexpandb 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 62 72 7e[ 	]*vpexpandb 0x7e\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 62 f5[ 	]*vpexpandb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 62 f5[ 	]*vpexpandb %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 62 f5[ 	]*vpexpandb %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 62 31[ 	]*vpexpandw \(%rcx\),%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 62 31[ 	]*vpexpandw \(%rcx\),%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 62 b4 f0 34 12 00 00[ 	]*vpexpandw 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 62 72 7f[ 	]*vpexpandw 0xfe\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 62 f5[ 	]*vpexpandw %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 62 f5[ 	]*vpexpandw %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 62 f5[ 	]*vpexpandw %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 70 f4[ 	]*vpshldvw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 70 f4[ 	]*vpshldvw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 70 f4[ 	]*vpshldvw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 70 b4 f0 34 12 00 00[ 	]*vpshldvw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 70 72 7f[ 	]*vpshldvw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 71 f4[ 	]*vpshldvd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 71 f4[ 	]*vpshldvd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 71 f4[ 	]*vpshldvd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 71 b4 f0 34 12 00 00[ 	]*vpshldvd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 71 31[ 	]*vpshldvd \(%rcx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 71 72 7f[ 	]*vpshldvd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 71 72 7f[ 	]*vpshldvd 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 71 f4[ 	]*vpshldvq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 71 f4[ 	]*vpshldvq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 71 f4[ 	]*vpshldvq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 71 b4 f0 34 12 00 00[ 	]*vpshldvq 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 71 72 7f[ 	]*vpshldvq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 71 72 7f[ 	]*vpshldvq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 72 f4[ 	]*vpshrdvw %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 72 f4[ 	]*vpshrdvw %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 72 f4[ 	]*vpshrdvw %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 72 b4 f0 34 12 00 00[ 	]*vpshrdvw 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 72 72 7f[ 	]*vpshrdvw 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 73 f4[ 	]*vpshrdvd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 73 f4[ 	]*vpshrdvd %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 73 f4[ 	]*vpshrdvd %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 73 b4 f0 34 12 00 00[ 	]*vpshrdvd 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 73 72 7f[ 	]*vpshrdvd 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 50 73 72 7f[ 	]*vpshrdvd 0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 73 f4[ 	]*vpshrdvq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 73 f4[ 	]*vpshrdvq %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 73 f4[ 	]*vpshrdvq %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 73 b4 f0 34 12 00 00[ 	]*vpshrdvq 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 73 72 7f[ 	]*vpshrdvq 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 73 72 7f[ 	]*vpshrdvq 0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 70 f4 ab[ 	]*vpshldw \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 70 f4 ab[ 	]*vpshldw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 70 f4 ab[ 	]*vpshldw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 70 b4 f0 34 12 00 00 7b[ 	]*vpshldw \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 70 72 7f 7b[ 	]*vpshldw \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 71 f4 ab[ 	]*vpshldd \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 71 f4 ab[ 	]*vpshldd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 71 f4 ab[ 	]*vpshldd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 71 b4 f0 34 12 00 00 7b[ 	]*vpshldd \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 71 f4 ab[ 	]*vpshldq \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 71 f4 ab[ 	]*vpshldq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 71 f4 ab[ 	]*vpshldq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 71 b4 f0 34 12 00 00 7b[ 	]*vpshldq \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 72 b4 f0 34 12 00 00 7b[ 	]*vpshrdw \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdd \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%rdx\)\{1to16\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 73 b4 f0 34 12 00 00 7b[ 	]*vpshrdq \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
#pass
