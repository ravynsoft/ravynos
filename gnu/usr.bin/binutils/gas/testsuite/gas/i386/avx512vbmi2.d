#as:
#objdump: -dw
#name: i386 AVX512VBMI2 insns
#source: avx512vbmi2.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 31[ 	]*vpcompressb %zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb %zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 72 7e[ 	]*vpcompressb %zmm6,0x7e\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 ee[ 	]*vpcompressb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 ee[ 	]*vpcompressb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 63 ee[ 	]*vpcompressb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 31[ 	]*vpcompressw %zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw %zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 72 40[ 	]*vpcompressw %zmm6,0x80\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 ee[ 	]*vpcompressw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 ee[ 	]*vpcompressw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 63 ee[ 	]*vpcompressw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 31[ 	]*vpexpandb \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 31[ 	]*vpexpandb \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 72 7e[ 	]*vpexpandb 0x7e\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 f5[ 	]*vpexpandb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 f5[ 	]*vpexpandb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 f5[ 	]*vpexpandb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 31[ 	]*vpexpandw \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 31[ 	]*vpexpandw \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 72 40[ 	]*vpexpandw 0x80\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 f5[ 	]*vpexpandw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 f5[ 	]*vpexpandw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 f5[ 	]*vpexpandw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 f4[ 	]*vpshldvw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 70 f4[ 	]*vpshldvw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 70 f4[ 	]*vpshldvw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 72 02[ 	]*vpshldvw 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 f4[ 	]*vpshldvd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 71 f4[ 	]*vpshldvd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 71 f4[ 	]*vpshldvd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 72 02[ 	]*vpshldvd 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 71 72 7f[ 	]*vpshldvd 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 f4[ 	]*vpshldvq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 71 f4[ 	]*vpshldvq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 71 f4[ 	]*vpshldvq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 72 02[ 	]*vpshldvq 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 71 72 7f[ 	]*vpshldvq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 f4[ 	]*vpshrdvw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 72 f4[ 	]*vpshrdvw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 72 f4[ 	]*vpshrdvw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 72 02[ 	]*vpshrdvw 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 f4[ 	]*vpshrdvd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 73 f4[ 	]*vpshrdvd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 73 f4[ 	]*vpshrdvd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 72 02[ 	]*vpshrdvd 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 73 72 7f[ 	]*vpshrdvd 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 f4[ 	]*vpshrdvq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 73 f4[ 	]*vpshrdvq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 73 f4[ 	]*vpshrdvq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 72 02[ 	]*vpshrdvq 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 73 72 7f[ 	]*vpshrdvq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 70 f4 ab[ 	]*vpshldw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 70 f4 ab[ 	]*vpshldw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 f4 7b[ 	]*vpshldw \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 72 02 7b[ 	]*vpshldw \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 71 f4 ab[ 	]*vpshldd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 71 f4 ab[ 	]*vpshldd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 f4 7b[ 	]*vpshldd \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 72 02 7b[ 	]*vpshldd \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 71 f4 ab[ 	]*vpshldq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 71 f4 ab[ 	]*vpshldq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 72 02 7b[ 	]*vpshldq \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 72 02 7b[ 	]*vpshrdw \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 72 02 7b[ 	]*vpshrdd \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 f4 7b[ 	]*vpshrdq \$0x7b,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 72 02 7b[ 	]*vpshrdq \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 31[ 	]*vpcompressb %zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb %zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 72 7e[ 	]*vpcompressb %zmm6,0x7e\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 ee[ 	]*vpcompressb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 ee[ 	]*vpcompressb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 63 ee[ 	]*vpcompressb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 31[ 	]*vpcompressw %zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw %zmm6,-0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 72 40[ 	]*vpcompressw %zmm6,0x80\(%edx\)
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 ee[ 	]*vpcompressw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 ee[ 	]*vpcompressw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 63 ee[ 	]*vpcompressw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 31[ 	]*vpexpandb \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 31[ 	]*vpexpandb \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 72 7e[ 	]*vpexpandb 0x7e\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 f5[ 	]*vpexpandb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 f5[ 	]*vpexpandb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 f5[ 	]*vpexpandb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 31[ 	]*vpexpandw \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 31[ 	]*vpexpandw \(%ecx\),%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 72 40[ 	]*vpexpandw 0x80\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 f5[ 	]*vpexpandw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 f5[ 	]*vpexpandw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 f5[ 	]*vpexpandw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 f4[ 	]*vpshldvw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 70 f4[ 	]*vpshldvw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 70 f4[ 	]*vpshldvw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 72 02[ 	]*vpshldvw 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 f4[ 	]*vpshldvd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 71 f4[ 	]*vpshldvd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 71 f4[ 	]*vpshldvd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 72 02[ 	]*vpshldvd 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 71 72 7f[ 	]*vpshldvd 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 f4[ 	]*vpshldvq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 71 f4[ 	]*vpshldvq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 71 f4[ 	]*vpshldvq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 72 02[ 	]*vpshldvq 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 71 72 7f[ 	]*vpshldvq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 f4[ 	]*vpshrdvw %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 72 f4[ 	]*vpshrdvw %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 72 f4[ 	]*vpshrdvw %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 72 02[ 	]*vpshrdvw 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 f4[ 	]*vpshrdvd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 73 f4[ 	]*vpshrdvd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 73 f4[ 	]*vpshrdvd %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 72 02[ 	]*vpshrdvd 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 73 72 7f[ 	]*vpshrdvd 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 f4[ 	]*vpshrdvq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 73 f4[ 	]*vpshrdvq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 73 f4[ 	]*vpshrdvq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 72 02[ 	]*vpshrdvq 0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 73 72 7f[ 	]*vpshrdvq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 f4 ab[ 	]*vpshldw \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 70 f4 ab[ 	]*vpshldw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 70 f4 ab[ 	]*vpshldw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 72 02 7b[ 	]*vpshldw \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 f4 ab[ 	]*vpshldd \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 71 f4 ab[ 	]*vpshldd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 71 f4 ab[ 	]*vpshldd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 72 02 7b[ 	]*vpshldd \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 71 72 7f 7b[ 	]*vpshldd \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 f4 ab[ 	]*vpshldq \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 71 f4 ab[ 	]*vpshldq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 71 f4 ab[ 	]*vpshldq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 72 02 7b[ 	]*vpshldq \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 71 72 7f 7b[ 	]*vpshldq \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 72 f4 ab[ 	]*vpshrdw \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 72 02 7b[ 	]*vpshrdw \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 73 f4 ab[ 	]*vpshrdd \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 72 02 7b[ 	]*vpshrdd \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 73 72 7f 7b[ 	]*vpshrdd \$0x7b,0x1fc\(%edx\)\{1to16\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 73 f4 ab[ 	]*vpshrdq \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 72 02 7b[ 	]*vpshrdq \$0x7b,0x80\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 73 72 7f 7b[ 	]*vpshrdq \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
#pass
