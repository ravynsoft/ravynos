#as:
#objdump: -dw
#name: i386 AVX512VBMI2/VL insns
#source: avx512vbmi2_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 72 7e[ 	]*vpcompressb %xmm6,0x7e\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 72 7e[ 	]*vpcompressb %ymm6,0x7e\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 ee[ 	]*vpcompressb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 63 ee[ 	]*vpcompressb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 ee[ 	]*vpcompressb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 63 ee[ 	]*vpcompressb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 72 40[ 	]*vpcompressw %xmm6,0x80\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 72 40[ 	]*vpcompressw %ymm6,0x80\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 ee[ 	]*vpcompressw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 63 ee[ 	]*vpcompressw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 ee[ 	]*vpcompressw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 63 ee[ 	]*vpcompressw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 31[ 	]*vpexpandb \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 72 7e[ 	]*vpexpandb 0x7e\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 31[ 	]*vpexpandb \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 72 7e[ 	]*vpexpandb 0x7e\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 f5[ 	]*vpexpandb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 f5[ 	]*vpexpandb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 f5[ 	]*vpexpandb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 f5[ 	]*vpexpandb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 31[ 	]*vpexpandw \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 72 40[ 	]*vpexpandw 0x80\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 31[ 	]*vpexpandw \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 72 40[ 	]*vpexpandw 0x80\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 f5[ 	]*vpexpandw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 f5[ 	]*vpexpandw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 f5[ 	]*vpexpandw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 f5[ 	]*vpexpandw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 f4[ 	]*vpshldvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 70 f4[ 	]*vpshldvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 72 7f[ 	]*vpshldvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 f4[ 	]*vpshldvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 70 f4[ 	]*vpshldvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 72 7f[ 	]*vpshldvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 f4[ 	]*vpshldvd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 71 f4[ 	]*vpshldvd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 72 7f[ 	]*vpshldvd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 71 72 7f[ 	]*vpshldvd 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 f4[ 	]*vpshldvd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 71 f4[ 	]*vpshldvd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 72 7f[ 	]*vpshldvd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 71 72 7f[ 	]*vpshldvd 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 f4[ 	]*vpshldvq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 71 f4[ 	]*vpshldvq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 72 7f[ 	]*vpshldvq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 71 72 7f[ 	]*vpshldvq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 f4[ 	]*vpshldvq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 71 f4[ 	]*vpshldvq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 72 7f[ 	]*vpshldvq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 71 72 7f[ 	]*vpshldvq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 f4[ 	]*vpshrdvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 72 f4[ 	]*vpshrdvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 72 7f[ 	]*vpshrdvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 f4[ 	]*vpshrdvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 72 f4[ 	]*vpshrdvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 72 7f[ 	]*vpshrdvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 f4[ 	]*vpshrdvd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 73 f4[ 	]*vpshrdvd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 72 7f[ 	]*vpshrdvd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 73 72 7f[ 	]*vpshrdvd 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 f4[ 	]*vpshrdvd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 73 f4[ 	]*vpshrdvd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 72 7f[ 	]*vpshrdvd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 73 72 7f[ 	]*vpshrdvd 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 f4[ 	]*vpshrdvq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 73 f4[ 	]*vpshrdvq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 72 7f[ 	]*vpshrdvq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 73 72 7f[ 	]*vpshrdvq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 f4[ 	]*vpshrdvq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 73 f4[ 	]*vpshrdvq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 72 7f[ 	]*vpshrdvq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 73 72 7f[ 	]*vpshrdvq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 f4 ab[ 	]*vpshldw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 70 f4 ab[ 	]*vpshldw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 72 7f 7b[ 	]*vpshldw \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 f4 ab[ 	]*vpshldw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 70 f4 ab[ 	]*vpshldw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 72 7f 7b[ 	]*vpshldw \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 f4 ab[ 	]*vpshldd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 71 f4 ab[ 	]*vpshldd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 f4 ab[ 	]*vpshldd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 71 f4 ab[ 	]*vpshldd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 f4 ab[ 	]*vpshldq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 71 f4 ab[ 	]*vpshldq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 f4 ab[ 	]*vpshldq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 71 f4 ab[ 	]*vpshldq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 72 7e[ 	]*vpcompressb %xmm6,0x7e\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 72 7e[ 	]*vpcompressb %ymm6,0x7e\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 63 ee[ 	]*vpcompressb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 63 ee[ 	]*vpcompressb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 63 ee[ 	]*vpcompressb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 63 ee[ 	]*vpcompressb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw %xmm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 72 40[ 	]*vpcompressw %xmm6,0x80\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw %ymm6,-0x1e240\(%esp,%esi,8\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 72 40[ 	]*vpcompressw %ymm6,0x80\(%edx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 63 ee[ 	]*vpcompressw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 63 ee[ 	]*vpcompressw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 63 ee[ 	]*vpcompressw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 63 ee[ 	]*vpcompressw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 31[ 	]*vpexpandb \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 72 7e[ 	]*vpexpandb 0x7e\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 31[ 	]*vpexpandb \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 72 7e[ 	]*vpexpandb 0x7e\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 62 f5[ 	]*vpexpandb %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 62 f5[ 	]*vpexpandb %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 62 f5[ 	]*vpexpandb %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 62 f5[ 	]*vpexpandb %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 31[ 	]*vpexpandw \(%ecx\),%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 72 40[ 	]*vpexpandw 0x80\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 31[ 	]*vpexpandw \(%ecx\),%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 72 40[ 	]*vpexpandw 0x80\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 62 f5[ 	]*vpexpandw %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 62 f5[ 	]*vpexpandw %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 62 f5[ 	]*vpexpandw %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 62 f5[ 	]*vpexpandw %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 f4[ 	]*vpshldvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 70 f4[ 	]*vpshldvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 70 72 7f[ 	]*vpshldvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 f4[ 	]*vpshldvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 70 f4[ 	]*vpshldvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 70 72 7f[ 	]*vpshldvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 f4[ 	]*vpshldvd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 71 f4[ 	]*vpshldvd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 71 72 7f[ 	]*vpshldvd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 71 72 7f[ 	]*vpshldvd 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 f4[ 	]*vpshldvd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 71 f4[ 	]*vpshldvd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 71 72 7f[ 	]*vpshldvd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 71 72 7f[ 	]*vpshldvd 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 f4[ 	]*vpshldvq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 71 f4[ 	]*vpshldvq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 71 72 7f[ 	]*vpshldvq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 71 72 7f[ 	]*vpshldvq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 f4[ 	]*vpshldvq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 71 f4[ 	]*vpshldvq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 71 72 7f[ 	]*vpshldvq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 71 72 7f[ 	]*vpshldvq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 f4[ 	]*vpshrdvw %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 72 f4[ 	]*vpshrdvw %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 72 72 7f[ 	]*vpshrdvw 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 f4[ 	]*vpshrdvw %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 72 f4[ 	]*vpshrdvw %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 72 72 7f[ 	]*vpshrdvw 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 f4[ 	]*vpshrdvd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 73 f4[ 	]*vpshrdvd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 73 72 7f[ 	]*vpshrdvd 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 73 72 7f[ 	]*vpshrdvd 0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 f4[ 	]*vpshrdvd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 73 f4[ 	]*vpshrdvd %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 73 72 7f[ 	]*vpshrdvd 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 73 72 7f[ 	]*vpshrdvd 0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 f4[ 	]*vpshrdvq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 73 f4[ 	]*vpshrdvq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 73 72 7f[ 	]*vpshrdvq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 73 72 7f[ 	]*vpshrdvq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 f4[ 	]*vpshrdvq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 73 f4[ 	]*vpshrdvq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 73 72 7f[ 	]*vpshrdvq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 73 72 7f[ 	]*vpshrdvq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 f4 ab[ 	]*vpshldw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 70 f4 ab[ 	]*vpshldw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 70 72 7f 7b[ 	]*vpshldw \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 f4 ab[ 	]*vpshldw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 70 f4 ab[ 	]*vpshldw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 70 72 7f 7b[ 	]*vpshldw \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 f4 ab[ 	]*vpshldd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 71 f4 ab[ 	]*vpshldd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 f4 ab[ 	]*vpshldd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 71 f4 ab[ 	]*vpshldd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 f4 ab[ 	]*vpshldq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 71 f4 ab[ 	]*vpshldq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 f4 ab[ 	]*vpshldq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 71 f4 ab[ 	]*vpshldq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 72 f4 ab[ 	]*vpshrdw \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 72 f4 ab[ 	]*vpshrdw \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 72 72 7f 7b[ 	]*vpshrdw \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 73 f4 ab[ 	]*vpshrdd \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%edx\)\{1to4\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 73 f4 ab[ 	]*vpshrdd \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%edx\)\{1to8\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 73 f4 ab[ 	]*vpshrdq \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 73 f4 ab[ 	]*vpshrdq \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
#pass
