#objdump: -dw
#name: i386 XOP

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f e9 78 81 ff[ 	]+vfrczpd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 81 f0[ 	]+vfrczpd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 81 03[ 	]+vfrczpd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 81 3e[ 	]+vfrczpd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 81 c0[ 	]+vfrczpd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 81 38[ 	]+vfrczpd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 81 c7[ 	]+vfrczpd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 81 f1[ 	]+vfrczpd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 81 c1[ 	]+vfrczpd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 81 f8[ 	]+vfrczpd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 81 30[ 	]+vfrczpd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 81 f9[ 	]+vfrczpd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 81 06[ 	]+vfrczpd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 81 3b[ 	]+vfrczpd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 81 36[ 	]+vfrczpd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 81 00[ 	]+vfrczpd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 7c 81 ff[ 	]+vfrczpd %ymm7,%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 81 f0[ 	]+vfrczpd %ymm0,%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 81 03[ 	]+vfrczpd \(%ebx\),%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 81 3e[ 	]+vfrczpd \(%esi\),%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 81 c0[ 	]+vfrczpd %ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 81 38[ 	]+vfrczpd \(%eax\),%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 81 c7[ 	]+vfrczpd %ymm7,%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 81 f1[ 	]+vfrczpd %ymm1,%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 81 c1[ 	]+vfrczpd %ymm1,%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 81 f8[ 	]+vfrczpd %ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 81 30[ 	]+vfrczpd \(%eax\),%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 81 f9[ 	]+vfrczpd %ymm1,%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 81 06[ 	]+vfrczpd \(%esi\),%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 81 3b[ 	]+vfrczpd \(%ebx\),%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 81 36[ 	]+vfrczpd \(%esi\),%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 81 00[ 	]+vfrczpd \(%eax\),%ymm0
[ 	]*[a-f0-9]+:	8f e9 78 80 ff[ 	]+vfrczps %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 80 f0[ 	]+vfrczps %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 80 03[ 	]+vfrczps \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 80 3e[ 	]+vfrczps \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 80 c0[ 	]+vfrczps %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 80 38[ 	]+vfrczps \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 80 c7[ 	]+vfrczps %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 80 f1[ 	]+vfrczps %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 80 c1[ 	]+vfrczps %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 80 f8[ 	]+vfrczps %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 80 30[ 	]+vfrczps \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 80 f9[ 	]+vfrczps %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 80 06[ 	]+vfrczps \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 80 3b[ 	]+vfrczps \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 80 36[ 	]+vfrczps \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 80 00[ 	]+vfrczps \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 7c 80 ff[ 	]+vfrczps %ymm7,%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 80 f0[ 	]+vfrczps %ymm0,%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 80 03[ 	]+vfrczps \(%ebx\),%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 80 3e[ 	]+vfrczps \(%esi\),%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 80 c0[ 	]+vfrczps %ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 80 38[ 	]+vfrczps \(%eax\),%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 80 c7[ 	]+vfrczps %ymm7,%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 80 f1[ 	]+vfrczps %ymm1,%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 80 c1[ 	]+vfrczps %ymm1,%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 80 f8[ 	]+vfrczps %ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 80 30[ 	]+vfrczps \(%eax\),%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 80 f9[ 	]+vfrczps %ymm1,%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 80 06[ 	]+vfrczps \(%esi\),%ymm0
[ 	]*[a-f0-9]+:	8f e9 7c 80 3b[ 	]+vfrczps \(%ebx\),%ymm7
[ 	]*[a-f0-9]+:	8f e9 7c 80 36[ 	]+vfrczps \(%esi\),%ymm6
[ 	]*[a-f0-9]+:	8f e9 7c 80 00[ 	]+vfrczps \(%eax\),%ymm0
[ 	]*[a-f0-9]+:	8f e9 78 83 ff[ 	]+vfrczsd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 83 f0[ 	]+vfrczsd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 83 03[ 	]+vfrczsd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 83 3e[ 	]+vfrczsd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 83 c0[ 	]+vfrczsd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 83 38[ 	]+vfrczsd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 83 c7[ 	]+vfrczsd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 83 f1[ 	]+vfrczsd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 83 c1[ 	]+vfrczsd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 83 f8[ 	]+vfrczsd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 83 30[ 	]+vfrczsd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 83 f9[ 	]+vfrczsd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 83 06[ 	]+vfrczsd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 83 3b[ 	]+vfrczsd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 83 36[ 	]+vfrczsd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 83 00[ 	]+vfrczsd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 82 ff[ 	]+vfrczss %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 82 f0[ 	]+vfrczss %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 82 03[ 	]+vfrczss \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 82 3e[ 	]+vfrczss \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 82 c0[ 	]+vfrczss %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 82 38[ 	]+vfrczss \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 82 c7[ 	]+vfrczss %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 82 f1[ 	]+vfrczss %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 82 c1[ 	]+vfrczss %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 82 f8[ 	]+vfrczss %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 82 30[ 	]+vfrczss \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 82 f9[ 	]+vfrczss %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 82 06[ 	]+vfrczss \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 82 3b[ 	]+vfrczss \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 82 36[ 	]+vfrczss \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 82 00[ 	]+vfrczss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 a2 c7 00[ 	]+vpcmov %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a2 06 70[ 	]+vpcmov %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a2 06 10[ 	]+vpcmov %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a2 e8 10[ 	]+vpcmov %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a2 c6 10[ 	]+vpcmov %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a2 fe 10[ 	]+vpcmov %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 a2 3a 10[ 	]+vpcmov %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 a2 f8 70[ 	]+vpcmov %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a2 3e 70[ 	]+vpcmov %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a2 fe 70[ 	]+vpcmov %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a2 c7 70[ 	]+vpcmov %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a2 02 00[ 	]+vpcmov %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 a2 2a 10[ 	]+vpcmov %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a2 ef 10[ 	]+vpcmov %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a2 c7 10[ 	]+vpcmov %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a2 2e 70[ 	]+vpcmov %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 44 a2 c7 00[ 	]+vpcmov %ymm0,%ymm7,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	8f e8 7c a2 06 70[ 	]+vpcmov %ymm7,\(%esi\),%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e8 64 a2 06 10[ 	]+vpcmov %ymm1,\(%esi\),%ymm3,%ymm0
[ 	]*[a-f0-9]+:	8f e8 7c a2 e8 10[ 	]+vpcmov %ymm1,%ymm0,%ymm0,%ymm5
[ 	]*[a-f0-9]+:	8f e8 7c a2 c6 10[ 	]+vpcmov %ymm1,%ymm6,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e8 7c a2 fe 10[ 	]+vpcmov %ymm1,%ymm6,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e8 7c a2 3a 10[ 	]+vpcmov %ymm1,\(%edx\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e8 7c a2 f8 70[ 	]+vpcmov %ymm7,%ymm0,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e8 64 a2 3e 70[ 	]+vpcmov %ymm7,\(%esi\),%ymm3,%ymm7
[ 	]*[a-f0-9]+:	8f e8 64 a2 fe 70[ 	]+vpcmov %ymm7,%ymm6,%ymm3,%ymm7
[ 	]*[a-f0-9]+:	8f e8 64 a2 c7 70[ 	]+vpcmov %ymm7,%ymm7,%ymm3,%ymm0
[ 	]*[a-f0-9]+:	8f e8 64 a2 02 00[ 	]+vpcmov %ymm0,\(%edx\),%ymm3,%ymm0
[ 	]*[a-f0-9]+:	8f e8 44 a2 2a 10[ 	]+vpcmov %ymm1,\(%edx\),%ymm7,%ymm5
[ 	]*[a-f0-9]+:	8f e8 44 a2 ef 10[ 	]+vpcmov %ymm1,%ymm7,%ymm7,%ymm5
[ 	]*[a-f0-9]+:	8f e8 7c a2 c7 10[ 	]+vpcmov %ymm1,%ymm7,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e8 64 a2 2e 70[ 	]+vpcmov %ymm7,\(%esi\),%ymm3,%ymm5
[ 	]*[a-f0-9]+:	8f e8 40 a2 c6 00[ 	]+vpcmov %xmm0,%xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 f8 a2 06 70[ 	]+vpcmov \(%esi\),%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 e0 a2 00 70[ 	]+vpcmov \(%eax\),%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a2 e8 70[ 	]+vpcmov %xmm7,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a2 c0 70[ 	]+vpcmov %xmm7,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a2 f8 70[ 	]+vpcmov %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 f8 a2 38 60[ 	]+vpcmov \(%eax\),%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 f8 a2 3e 00[ 	]+vpcmov \(%esi\),%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 e0 a2 3b 70[ 	]+vpcmov \(%ebx\),%xmm7,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 e0 a2 3b 00[ 	]+vpcmov \(%ebx\),%xmm0,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 e0 a2 06 60[ 	]+vpcmov \(%esi\),%xmm6,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a2 c7 10[ 	]+vpcmov %xmm1,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 c0 a2 28 70[ 	]+vpcmov \(%eax\),%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a2 ee 70[ 	]+vpcmov %xmm7,%xmm6,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a2 c6 70[ 	]+vpcmov %xmm7,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 e0 a2 2b 70[ 	]+vpcmov \(%ebx\),%xmm7,%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 44 a2 c6 00[ 	]+vpcmov %ymm0,%ymm6,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	8f e8 fc a2 06 70[ 	]+vpcmov \(%esi\),%ymm7,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e8 e4 a2 00 70[ 	]+vpcmov \(%eax\),%ymm7,%ymm3,%ymm0
[ 	]*[a-f0-9]+:	8f e8 7c a2 e8 70[ 	]+vpcmov %ymm7,%ymm0,%ymm0,%ymm5
[ 	]*[a-f0-9]+:	8f e8 7c a2 c0 70[ 	]+vpcmov %ymm7,%ymm0,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e8 7c a2 f8 70[ 	]+vpcmov %ymm7,%ymm0,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e8 fc a2 38 60[ 	]+vpcmov \(%eax\),%ymm6,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e8 fc a2 3e 00[ 	]+vpcmov \(%esi\),%ymm0,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	8f e8 e4 a2 3b 70[ 	]+vpcmov \(%ebx\),%ymm7,%ymm3,%ymm7
[ 	]*[a-f0-9]+:	8f e8 e4 a2 3b 00[ 	]+vpcmov \(%ebx\),%ymm0,%ymm3,%ymm7
[ 	]*[a-f0-9]+:	8f e8 e4 a2 06 60[ 	]+vpcmov \(%esi\),%ymm6,%ymm3,%ymm0
[ 	]*[a-f0-9]+:	8f e8 64 a2 c7 10[ 	]+vpcmov %ymm1,%ymm7,%ymm3,%ymm0
[ 	]*[a-f0-9]+:	8f e8 c4 a2 28 70[ 	]+vpcmov \(%eax\),%ymm7,%ymm7,%ymm5
[ 	]*[a-f0-9]+:	8f e8 44 a2 ee 70[ 	]+vpcmov %ymm7,%ymm6,%ymm7,%ymm5
[ 	]*[a-f0-9]+:	8f e8 7c a2 c6 70[ 	]+vpcmov %ymm7,%ymm6,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f e8 e4 a2 2b 70[ 	]+vpcmov \(%ebx\),%ymm7,%ymm3,%ymm5
[ 	]*[a-f0-9]+:	8f e8 78 cc 38 03[ 	]+vpcomgeb \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc c8 ff[ 	]+vpcomb \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cc cd ff[ 	]+vpcomb \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cc cd 00[ 	]+vpcomltb %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cc cd 00[ 	]+vpcomltb %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 cc c8 00[ 	]+vpcomltb %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 cc f8 03[ 	]+vpcomgeb %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc fd 00[ 	]+vpcomltb %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc ff ff[ 	]+vpcomb \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc ff 00[ 	]+vpcomltb %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc c7 03[ 	]+vpcomgeb %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc cf ff[ 	]+vpcomb \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cc 08 ff[ 	]+vpcomb \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cc 08 03[ 	]+vpcomgeb \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cc f8 03[ 	]+vpcomgeb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 cc c7 ff[ 	]+vpcomb \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 38 03[ 	]+vpcomged \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce c8 ff[ 	]+vpcomd \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ce cd ff[ 	]+vpcomd \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ce cd 00[ 	]+vpcomltd %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ce cd 00[ 	]+vpcomltd %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ce c8 00[ 	]+vpcomltd %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ce f8 03[ 	]+vpcomged %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce fd 00[ 	]+vpcomltd %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce ff ff[ 	]+vpcomd \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce ff 00[ 	]+vpcomltd %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce c7 03[ 	]+vpcomged %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce cf ff[ 	]+vpcomd \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ce 08 ff[ 	]+vpcomd \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ce 08 03[ 	]+vpcomged \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ce f8 03[ 	]+vpcomged %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 ce c7 ff[ 	]+vpcomd \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 38 03[ 	]+vpcomgeq \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf c8 ff[ 	]+vpcomq \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cf cd ff[ 	]+vpcomq \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cf cd 00[ 	]+vpcomltq %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cf cd 00[ 	]+vpcomltq %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 cf c8 00[ 	]+vpcomltq %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 cf f8 03[ 	]+vpcomgeq %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf fd 00[ 	]+vpcomltq %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf ff ff[ 	]+vpcomq \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf ff 00[ 	]+vpcomltq %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf c7 03[ 	]+vpcomgeq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf cf ff[ 	]+vpcomq \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cf 08 ff[ 	]+vpcomq \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cf 08 03[ 	]+vpcomgeq \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cf f8 03[ 	]+vpcomgeq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 cf c7 ff[ 	]+vpcomq \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec 38 03[ 	]+vpcomgeub \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec c8 ff[ 	]+vpcomub \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ec cd ff[ 	]+vpcomub \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ec cd 00[ 	]+vpcomltub %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ec cd 00[ 	]+vpcomltub %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ec c8 00[ 	]+vpcomltub %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ec f8 03[ 	]+vpcomgeub %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec fd 00[ 	]+vpcomltub %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec ff ff[ 	]+vpcomub \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec ff 00[ 	]+vpcomltub %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec c7 03[ 	]+vpcomgeub %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec cf ff[ 	]+vpcomub \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ec 08 ff[ 	]+vpcomub \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ec 08 03[ 	]+vpcomgeub \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ec f8 03[ 	]+vpcomgeub %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 ec c7 ff[ 	]+vpcomub \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 38 03[ 	]+vpcomgeud \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee c8 ff[ 	]+vpcomud \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ee cd ff[ 	]+vpcomud \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ee cd 00[ 	]+vpcomltud %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ee cd 00[ 	]+vpcomltud %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ee c8 00[ 	]+vpcomltud %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ee f8 03[ 	]+vpcomgeud %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ee fd 00[ 	]+vpcomltud %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ee ff ff[ 	]+vpcomud \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ee ff 00[ 	]+vpcomltud %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee c7 03[ 	]+vpcomgeud %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee cf ff[ 	]+vpcomud \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ee 08 ff[ 	]+vpcomud \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ee 08 03[ 	]+vpcomgeud \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ee f8 03[ 	]+vpcomgeud %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 ee c7 ff[ 	]+vpcomud \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 38 03[ 	]+vpcomgeuq \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef c8 ff[ 	]+vpcomuq \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ef cd ff[ 	]+vpcomuq \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ef cd 00[ 	]+vpcomltuq %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ef cd 00[ 	]+vpcomltuq %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ef c8 00[ 	]+vpcomltuq %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ef f8 03[ 	]+vpcomgeuq %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef fd 00[ 	]+vpcomltuq %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef ff ff[ 	]+vpcomuq \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef ff 00[ 	]+vpcomltuq %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef c7 03[ 	]+vpcomgeuq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef cf ff[ 	]+vpcomuq \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ef 08 ff[ 	]+vpcomuq \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ef 08 03[ 	]+vpcomgeuq \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ef f8 03[ 	]+vpcomgeuq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 ef c7 ff[ 	]+vpcomuq \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 38 03[ 	]+vpcomgeuw \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed c8 ff[ 	]+vpcomuw \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ed cd ff[ 	]+vpcomuw \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ed cd 00[ 	]+vpcomltuw %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ed cd 00[ 	]+vpcomltuw %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ed c8 00[ 	]+vpcomltuw %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 ed f8 03[ 	]+vpcomgeuw %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed fd 00[ 	]+vpcomltuw %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed ff ff[ 	]+vpcomuw \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed ff 00[ 	]+vpcomltuw %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed c7 03[ 	]+vpcomgeuw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed cf ff[ 	]+vpcomuw \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ed 08 ff[ 	]+vpcomuw \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 ed 08 03[ 	]+vpcomgeuw \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 ed f8 03[ 	]+vpcomgeuw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 ed c7 ff[ 	]+vpcomuw \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cd 38 03[ 	]+vpcomgew \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd c8 ff[ 	]+vpcomw \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cd cd ff[ 	]+vpcomw \$0xff,%xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cd cd 00[ 	]+vpcomltw %xmm5,%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cd cd 00[ 	]+vpcomltw %xmm5,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 cd c8 00[ 	]+vpcomltw %xmm0,%xmm7,%xmm1
[ 	]*[a-f0-9]+:	8f e8 40 cd f8 03[ 	]+vpcomgew %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cd fd 00[ 	]+vpcomltw %xmm5,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cd ff ff[ 	]+vpcomw \$0xff,%xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cd ff 00[ 	]+vpcomltw %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd c7 03[ 	]+vpcomgew %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cd cf ff[ 	]+vpcomw \$0xff,%xmm7,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cd 08 ff[ 	]+vpcomw \$0xff,\(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 50 cd 08 03[ 	]+vpcomgew \(%eax\),%xmm5,%xmm1
[ 	]*[a-f0-9]+:	8f e8 78 cd f8 03[ 	]+vpcomgew %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 50 cd c7 ff[ 	]+vpcomw \$0xff,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 41 49 00 50[ 	]+vpermil2pd \$0x0,%xmm5,\(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 59 49 c2 11[ 	]+vpermil2pd \$0x1,%xmm1,%xmm2,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 41 49 10 42[ 	]+vpermil2pd \$0x2,%xmm4,\(%eax\),%xmm7,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 59 49 3c 83 33[ 	]+vpermil2pd \$0x3,%xmm3,\(%ebx,%eax,4\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 49 f7 30[ 	]+vpermil2pd \$0x0,%xmm3,%xmm7,%xmm0,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 49 14 16 71[ 	]+vpermil2pd \$0x1,%xmm7,\(%esi,%edx,1\),%xmm0,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 59 49 fd 32[ 	]+vpermil2pd \$0x2,%xmm3,%xmm5,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 71 49 d0 33[ 	]+vpermil2pd \$0x3,%xmm3,%xmm0,%xmm1,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 69 49 f1 72[ 	]+vpermil2pd \$0x2,%xmm7,%xmm1,%xmm2,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 e9 49 bc 19 88 07 00 00 01[ 	]+vpermil2pd \$0x1,0x788\(%ecx,%ebx,1\),%xmm0,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 49 f9 40[ 	]+vpermil2pd \$0x0,%xmm4,%xmm1,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 59 49 c7 33[ 	]+vpermil2pd \$0x3,%xmm3,%xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 f9 49 b4 59 88 07 00 00 73[ 	]+vpermil2pd \$0x3,0x788\(%ecx,%ebx,2\),%xmm7,%xmm0,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 51 49 c7 31[ 	]+vpermil2pd \$0x1,%xmm3,%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 59 49 f1 22[ 	]+vpermil2pd \$0x2,%xmm2,%xmm1,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 69 49 fb 03[ 	]+vpermil2pd \$0x3,%xmm0,%xmm3,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 75 49 d7 63[ 	]+vpermil2pd \$0x3,%ymm6,%ymm7,%ymm1,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 75 49 e7 61[ 	]+vpermil2pd \$0x1,%ymm6,%ymm7,%ymm1,%ymm4
[ 	]*[a-f0-9]+:	c4 e3 55 49 7c 87 05 02[ 	]+vpermil2pd \$0x2,%ymm0,0x5\(%edi,%eax,4\),%ymm5,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 7d 49 d6 50[ 	]+vpermil2pd \$0x0,%ymm5,%ymm6,%ymm0,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 65 49 c7 43[ 	]+vpermil2pd \$0x3,%ymm4,%ymm7,%ymm3,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 6d 49 c6 70[ 	]+vpermil2pd \$0x0,%ymm7,%ymm6,%ymm2,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 75 49 3e 42[ 	]+vpermil2pd \$0x2,%ymm4,\(%esi\),%ymm1,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 75 49 f8 61[ 	]+vpermil2pd \$0x1,%ymm6,%ymm0,%ymm1,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 c5 49 01 51[ 	]+vpermil2pd \$0x1,\(%ecx\),%ymm5,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 c5 49 04 46 43[ 	]+vpermil2pd \$0x3,\(%esi,%eax,2\),%ymm4,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 e5 49 39 00[ 	]+vpermil2pd \$0x0,\(%ecx\),%ymm0,%ymm3,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 cd 49 3c 06 22[ 	]+vpermil2pd \$0x2,\(%esi,%eax,1\),%ymm2,%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 cd 49 09 00[ 	]+vpermil2pd \$0x0,\(%ecx\),%ymm0,%ymm6,%ymm1
[ 	]*[a-f0-9]+:	c4 e3 45 49 c3 22[ 	]+vpermil2pd \$0x2,%ymm2,%ymm3,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 45 49 ca 03[ 	]+vpermil2pd \$0x3,%ymm0,%ymm2,%ymm7,%ymm1
[ 	]*[a-f0-9]+:	c4 e3 5d 49 f8 51[ 	]+vpermil2pd \$0x1,%ymm5,%ymm0,%ymm4,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 79 48 fc 33[ 	]+vpermil2ps \$0x3,%xmm3,%xmm4,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 59 48 38 01[ 	]+vpermil2ps \$0x1,%xmm0,\(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 41 48 38 32[ 	]+vpermil2ps \$0x2,%xmm3,\(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 41 48 14 c3 73[ 	]+vpermil2ps \$0x3,%xmm7,\(%ebx,%eax,8\),%xmm7,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 41 48 f8 72[ 	]+vpermil2ps \$0x2,%xmm7,%xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 48 3c 16 73[ 	]+vpermil2ps \$0x3,%xmm7,\(%esi,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 41 48 fc 71[ 	]+vpermil2ps \$0x1,%xmm7,%xmm4,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 41 48 10 30[ 	]+vpermil2ps \$0x0,%xmm3,\(%eax\),%xmm7,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 c1 48 33 72[ 	]+vpermil2ps \$0x2,\(%ebx\),%xmm7,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 48 04 1b 73[ 	]+vpermil2ps \$0x3,\(%ebx,%ebx,1\),%xmm7,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 c1 48 34 1b 10[ 	]+vpermil2ps \$0x0,\(%ebx,%ebx,1\),%xmm1,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 69 48 f9 02[ 	]+vpermil2ps \$0x2,%xmm0,%xmm1,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 48 34 1b 72[ 	]+vpermil2ps \$0x2,\(%ebx,%ebx,1\),%xmm7,%xmm2,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 c1 48 34 1b 13[ 	]+vpermil2ps \$0x3,\(%ebx,%ebx,1\),%xmm1,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 e9 48 3c 1b 70[ 	]+vpermil2ps \$0x0,\(%ebx,%ebx,1\),%xmm7,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 41 48 f9 71[ 	]+vpermil2ps \$0x1,%xmm7,%xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 75 48 d7 61[ 	]+vpermil2ps \$0x1,%ymm6,%ymm7,%ymm1,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 45 48 c6 73[ 	]+vpermil2ps \$0x3,%ymm7,%ymm6,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 45 48 d6 52[ 	]+vpermil2ps \$0x2,%ymm5,%ymm6,%ymm7,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 45 48 f8 20[ 	]+vpermil2ps \$0x0,%ymm2,%ymm0,%ymm7,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 45 48 04 cf 63[ 	]+vpermil2ps \$0x3,%ymm6,\(%edi,%ecx,8\),%ymm7,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 45 48 c7 62[ 	]+vpermil2ps \$0x2,%ymm6,%ymm7,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 75 48 d6 70[ 	]+vpermil2ps \$0x0,%ymm7,%ymm6,%ymm1,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 75 48 06 61[ 	]+vpermil2ps \$0x1,%ymm6,\(%esi\),%ymm1,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 fd 48 7c 43 0c 42[ 	]+vpermil2ps \$0x2,0xc\(%ebx,%eax,2\),%ymm4,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 48 c6 51[ 	]+vpermil2ps \$0x1,%ymm5,%ymm6,%ymm2,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 cd 48 3c 06 43[ 	]+vpermil2ps \$0x3,\(%esi,%eax,1\),%ymm4,%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 cd 48 04 de 31[ 	]+vpermil2ps \$0x1,\(%esi,%ebx,8\),%ymm3,%ymm6,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 fd 48 0c 48 70[ 	]+vpermil2ps \$0x0,\(%eax,%ecx,2\),%ymm7,%ymm0,%ymm1
[ 	]*[a-f0-9]+:	c4 e3 45 48 ff 62[ 	]+vpermil2ps \$0x2,%ymm6,%ymm7,%ymm7,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 48 c3 43[ 	]+vpermil2ps \$0x3,%ymm4,%ymm3,%ymm2,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 45 48 fe 00[ 	]+vpermil2ps \$0x0,%ymm0,%ymm6,%ymm7,%ymm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 ff[ 	]+vphaddbd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 f0[ 	]+vphaddbd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c2 03[ 	]+vphaddbd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c2 3e[ 	]+vphaddbd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 c0[ 	]+vphaddbd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c2 38[ 	]+vphaddbd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 c7[ 	]+vphaddbd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c2 f1[ 	]+vphaddbd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c2 c1[ 	]+vphaddbd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c2 f8[ 	]+vphaddbd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 30[ 	]+vphaddbd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c2 f9[ 	]+vphaddbd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 06[ 	]+vphaddbd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c2 3b[ 	]+vphaddbd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c2 36[ 	]+vphaddbd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c2 00[ 	]+vphaddbd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c3 ff[ 	]+vphaddbq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c3 f0[ 	]+vphaddbq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c3 03[ 	]+vphaddbq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c3 3e[ 	]+vphaddbq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c3 c0[ 	]+vphaddbq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c3 38[ 	]+vphaddbq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c3 c7[ 	]+vphaddbq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c3 f1[ 	]+vphaddbq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c3 c1[ 	]+vphaddbq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c3 f8[ 	]+vphaddbq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c3 30[ 	]+vphaddbq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c3 f9[ 	]+vphaddbq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c3 06[ 	]+vphaddbq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c3 3b[ 	]+vphaddbq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c3 36[ 	]+vphaddbq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c3 00[ 	]+vphaddbq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c1 ff[ 	]+vphaddbw %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c1 f0[ 	]+vphaddbw %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c1 03[ 	]+vphaddbw \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c1 3e[ 	]+vphaddbw \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c1 c0[ 	]+vphaddbw %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c1 38[ 	]+vphaddbw \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c1 c7[ 	]+vphaddbw %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c1 f1[ 	]+vphaddbw %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c1 c1[ 	]+vphaddbw %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c1 f8[ 	]+vphaddbw %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c1 30[ 	]+vphaddbw \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c1 f9[ 	]+vphaddbw %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c1 06[ 	]+vphaddbw \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c1 3b[ 	]+vphaddbw \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c1 36[ 	]+vphaddbw \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c1 00[ 	]+vphaddbw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 cb ff[ 	]+vphadddq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 cb f0[ 	]+vphadddq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 cb 03[ 	]+vphadddq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 cb 3e[ 	]+vphadddq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 cb c0[ 	]+vphadddq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 cb 38[ 	]+vphadddq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 cb c7[ 	]+vphadddq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 cb f1[ 	]+vphadddq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 cb c1[ 	]+vphadddq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 cb f8[ 	]+vphadddq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 cb 30[ 	]+vphadddq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 cb f9[ 	]+vphadddq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 cb 06[ 	]+vphadddq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 cb 3b[ 	]+vphadddq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 cb 36[ 	]+vphadddq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 cb 00[ 	]+vphadddq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d2 ff[ 	]+vphaddubd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d2 f0[ 	]+vphaddubd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d2 03[ 	]+vphaddubd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d2 3e[ 	]+vphaddubd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d2 c0[ 	]+vphaddubd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d2 38[ 	]+vphaddubd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d2 c7[ 	]+vphaddubd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d2 f1[ 	]+vphaddubd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d2 c1[ 	]+vphaddubd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d2 f8[ 	]+vphaddubd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d2 30[ 	]+vphaddubd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d2 f9[ 	]+vphaddubd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d2 06[ 	]+vphaddubd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d2 3b[ 	]+vphaddubd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d2 36[ 	]+vphaddubd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d2 00[ 	]+vphaddubd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d3 ff[ 	]+vphaddubq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d3 f0[ 	]+vphaddubq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d3 03[ 	]+vphaddubq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d3 3e[ 	]+vphaddubq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d3 c0[ 	]+vphaddubq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d3 38[ 	]+vphaddubq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d3 c7[ 	]+vphaddubq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d3 f1[ 	]+vphaddubq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d3 c1[ 	]+vphaddubq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d3 f8[ 	]+vphaddubq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d3 30[ 	]+vphaddubq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d3 f9[ 	]+vphaddubq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d3 06[ 	]+vphaddubq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d3 3b[ 	]+vphaddubq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d3 36[ 	]+vphaddubq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d3 00[ 	]+vphaddubq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d1 ff[ 	]+vphaddubw %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d1 f0[ 	]+vphaddubw %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d1 03[ 	]+vphaddubw \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d1 3e[ 	]+vphaddubw \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d1 c0[ 	]+vphaddubw %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d1 38[ 	]+vphaddubw \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d1 c7[ 	]+vphaddubw %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d1 f1[ 	]+vphaddubw %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d1 c1[ 	]+vphaddubw %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d1 f8[ 	]+vphaddubw %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d1 30[ 	]+vphaddubw \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d1 f9[ 	]+vphaddubw %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d1 06[ 	]+vphaddubw \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d1 3b[ 	]+vphaddubw \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d1 36[ 	]+vphaddubw \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d1 00[ 	]+vphaddubw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 db ff[ 	]+vphaddudq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 db f0[ 	]+vphaddudq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 db 03[ 	]+vphaddudq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 db 3e[ 	]+vphaddudq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 db c0[ 	]+vphaddudq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 db 38[ 	]+vphaddudq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 db c7[ 	]+vphaddudq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 db f1[ 	]+vphaddudq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 db c1[ 	]+vphaddudq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 db f8[ 	]+vphaddudq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 db 30[ 	]+vphaddudq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 db f9[ 	]+vphaddudq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 db 06[ 	]+vphaddudq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 db 3b[ 	]+vphaddudq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 db 36[ 	]+vphaddudq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 db 00[ 	]+vphaddudq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d6 ff[ 	]+vphadduwd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d6 f0[ 	]+vphadduwd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d6 03[ 	]+vphadduwd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d6 3e[ 	]+vphadduwd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d6 c0[ 	]+vphadduwd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d6 38[ 	]+vphadduwd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d6 c7[ 	]+vphadduwd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d6 f1[ 	]+vphadduwd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d6 c1[ 	]+vphadduwd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d6 f8[ 	]+vphadduwd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d6 30[ 	]+vphadduwd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d6 f9[ 	]+vphadduwd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d6 06[ 	]+vphadduwd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d6 3b[ 	]+vphadduwd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d6 36[ 	]+vphadduwd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d6 00[ 	]+vphadduwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d7 ff[ 	]+vphadduwq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d7 f0[ 	]+vphadduwq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d7 03[ 	]+vphadduwq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d7 3e[ 	]+vphadduwq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d7 c0[ 	]+vphadduwq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d7 38[ 	]+vphadduwq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d7 c7[ 	]+vphadduwq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d7 f1[ 	]+vphadduwq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d7 c1[ 	]+vphadduwq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d7 f8[ 	]+vphadduwq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d7 30[ 	]+vphadduwq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d7 f9[ 	]+vphadduwq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d7 06[ 	]+vphadduwq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 d7 3b[ 	]+vphadduwq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 d7 36[ 	]+vphadduwq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 d7 00[ 	]+vphadduwq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c6 ff[ 	]+vphaddwd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c6 f0[ 	]+vphaddwd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c6 03[ 	]+vphaddwd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c6 3e[ 	]+vphaddwd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c6 c0[ 	]+vphaddwd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c6 38[ 	]+vphaddwd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c6 c7[ 	]+vphaddwd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c6 f1[ 	]+vphaddwd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c6 c1[ 	]+vphaddwd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c6 f8[ 	]+vphaddwd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c6 30[ 	]+vphaddwd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c6 f9[ 	]+vphaddwd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c6 06[ 	]+vphaddwd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c6 3b[ 	]+vphaddwd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c6 36[ 	]+vphaddwd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c6 00[ 	]+vphaddwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c7 ff[ 	]+vphaddwq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c7 f0[ 	]+vphaddwq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c7 03[ 	]+vphaddwq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c7 3e[ 	]+vphaddwq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c7 c0[ 	]+vphaddwq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c7 38[ 	]+vphaddwq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c7 c7[ 	]+vphaddwq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c7 f1[ 	]+vphaddwq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c7 c1[ 	]+vphaddwq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c7 f8[ 	]+vphaddwq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c7 30[ 	]+vphaddwq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c7 f9[ 	]+vphaddwq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c7 06[ 	]+vphaddwq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 c7 3b[ 	]+vphaddwq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 c7 36[ 	]+vphaddwq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 c7 00[ 	]+vphaddwq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e1 ff[ 	]+vphsubbw %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e1 f0[ 	]+vphsubbw %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e1 03[ 	]+vphsubbw \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e1 3e[ 	]+vphsubbw \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e1 c0[ 	]+vphsubbw %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e1 38[ 	]+vphsubbw \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e1 c7[ 	]+vphsubbw %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e1 f1[ 	]+vphsubbw %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e1 c1[ 	]+vphsubbw %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e1 f8[ 	]+vphsubbw %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e1 30[ 	]+vphsubbw \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e1 f9[ 	]+vphsubbw %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e1 06[ 	]+vphsubbw \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e1 3b[ 	]+vphsubbw \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e1 36[ 	]+vphsubbw \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e1 00[ 	]+vphsubbw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e3 ff[ 	]+vphsubdq %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e3 f0[ 	]+vphsubdq %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e3 03[ 	]+vphsubdq \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e3 3e[ 	]+vphsubdq \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e3 c0[ 	]+vphsubdq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e3 38[ 	]+vphsubdq \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e3 c7[ 	]+vphsubdq %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e3 f1[ 	]+vphsubdq %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e3 c1[ 	]+vphsubdq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e3 f8[ 	]+vphsubdq %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e3 30[ 	]+vphsubdq \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e3 f9[ 	]+vphsubdq %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e3 06[ 	]+vphsubdq \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e3 3b[ 	]+vphsubdq \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e3 36[ 	]+vphsubdq \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e3 00[ 	]+vphsubdq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e2 ff[ 	]+vphsubwd %xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e2 f0[ 	]+vphsubwd %xmm0,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e2 03[ 	]+vphsubwd \(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e2 3e[ 	]+vphsubwd \(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e2 c0[ 	]+vphsubwd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e2 38[ 	]+vphsubwd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e2 c7[ 	]+vphsubwd %xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e2 f1[ 	]+vphsubwd %xmm1,%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e2 c1[ 	]+vphsubwd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e2 f8[ 	]+vphsubwd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e2 30[ 	]+vphsubwd \(%eax\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e2 f9[ 	]+vphsubwd %xmm1,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e2 06[ 	]+vphsubwd \(%esi\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 e2 3b[ 	]+vphsubwd \(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 e2 36[ 	]+vphsubwd \(%esi\),%xmm6
[ 	]*[a-f0-9]+:	8f e9 78 e2 00[ 	]+vphsubwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 9e c7 00[ 	]+vpmacsdd %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 9e 06 70[ 	]+vpmacsdd %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 9e 06 10[ 	]+vpmacsdd %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 9e e8 10[ 	]+vpmacsdd %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 9e c6 10[ 	]+vpmacsdd %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 9e fe 10[ 	]+vpmacsdd %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 9e 3a 10[ 	]+vpmacsdd %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 9e f8 70[ 	]+vpmacsdd %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 9e 3e 70[ 	]+vpmacsdd %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 9e fe 70[ 	]+vpmacsdd %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 9e c7 70[ 	]+vpmacsdd %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 9e 02 00[ 	]+vpmacsdd %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 9e 2a 10[ 	]+vpmacsdd %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 9e ef 10[ 	]+vpmacsdd %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 9e c7 10[ 	]+vpmacsdd %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 9e 2e 70[ 	]+vpmacsdd %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 9f c7 00[ 	]+vpmacsdqh %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 9f 06 70[ 	]+vpmacsdqh %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 9f 06 10[ 	]+vpmacsdqh %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 9f e8 10[ 	]+vpmacsdqh %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 9f c6 10[ 	]+vpmacsdqh %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 9f fe 10[ 	]+vpmacsdqh %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 9f 3a 10[ 	]+vpmacsdqh %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 9f f8 70[ 	]+vpmacsdqh %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 9f 3e 70[ 	]+vpmacsdqh %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 9f fe 70[ 	]+vpmacsdqh %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 9f c7 70[ 	]+vpmacsdqh %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 9f 02 00[ 	]+vpmacsdqh %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 9f 2a 10[ 	]+vpmacsdqh %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 9f ef 10[ 	]+vpmacsdqh %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 9f c7 10[ 	]+vpmacsdqh %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 9f 2e 70[ 	]+vpmacsdqh %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 97 c7 00[ 	]+vpmacsdql %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 97 06 70[ 	]+vpmacsdql %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 97 06 10[ 	]+vpmacsdql %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 97 e8 10[ 	]+vpmacsdql %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 97 c6 10[ 	]+vpmacsdql %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 97 fe 10[ 	]+vpmacsdql %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 97 3a 10[ 	]+vpmacsdql %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 97 f8 70[ 	]+vpmacsdql %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 97 3e 70[ 	]+vpmacsdql %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 97 fe 70[ 	]+vpmacsdql %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 97 c7 70[ 	]+vpmacsdql %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 97 02 00[ 	]+vpmacsdql %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 97 2a 10[ 	]+vpmacsdql %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 97 ef 10[ 	]+vpmacsdql %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 97 c7 10[ 	]+vpmacsdql %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 97 2e 70[ 	]+vpmacsdql %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 8e c7 00[ 	]+vpmacssdd %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 8e 06 70[ 	]+vpmacssdd %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 8e 06 10[ 	]+vpmacssdd %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 8e e8 10[ 	]+vpmacssdd %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 8e c6 10[ 	]+vpmacssdd %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 8e fe 10[ 	]+vpmacssdd %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 8e 3a 10[ 	]+vpmacssdd %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 8e f8 70[ 	]+vpmacssdd %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 8e 3e 70[ 	]+vpmacssdd %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 8e fe 70[ 	]+vpmacssdd %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 8e c7 70[ 	]+vpmacssdd %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 8e 02 00[ 	]+vpmacssdd %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 8e 2a 10[ 	]+vpmacssdd %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 8e ef 10[ 	]+vpmacssdd %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 8e c7 10[ 	]+vpmacssdd %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 8e 2e 70[ 	]+vpmacssdd %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 8f c7 00[ 	]+vpmacssdqh %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 8f 06 70[ 	]+vpmacssdqh %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 8f 06 10[ 	]+vpmacssdqh %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 8f e8 10[ 	]+vpmacssdqh %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 8f c6 10[ 	]+vpmacssdqh %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 8f fe 10[ 	]+vpmacssdqh %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 8f 3a 10[ 	]+vpmacssdqh %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 8f f8 70[ 	]+vpmacssdqh %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 8f 3e 70[ 	]+vpmacssdqh %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 8f fe 70[ 	]+vpmacssdqh %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 8f c7 70[ 	]+vpmacssdqh %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 8f 02 00[ 	]+vpmacssdqh %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 8f 2a 10[ 	]+vpmacssdqh %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 8f ef 10[ 	]+vpmacssdqh %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 8f c7 10[ 	]+vpmacssdqh %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 8f 2e 70[ 	]+vpmacssdqh %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 87 c7 00[ 	]+vpmacssdql %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 87 06 70[ 	]+vpmacssdql %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 87 06 10[ 	]+vpmacssdql %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 87 e8 10[ 	]+vpmacssdql %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 87 c6 10[ 	]+vpmacssdql %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 87 fe 10[ 	]+vpmacssdql %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 87 3a 10[ 	]+vpmacssdql %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 87 f8 70[ 	]+vpmacssdql %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 87 3e 70[ 	]+vpmacssdql %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 87 fe 70[ 	]+vpmacssdql %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 87 c7 70[ 	]+vpmacssdql %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 87 02 00[ 	]+vpmacssdql %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 87 2a 10[ 	]+vpmacssdql %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 87 ef 10[ 	]+vpmacssdql %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 87 c7 10[ 	]+vpmacssdql %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 87 2e 70[ 	]+vpmacssdql %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 86 c7 00[ 	]+vpmacsswd %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 86 06 70[ 	]+vpmacsswd %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 86 06 10[ 	]+vpmacsswd %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 86 e8 10[ 	]+vpmacsswd %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 86 c6 10[ 	]+vpmacsswd %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 86 fe 10[ 	]+vpmacsswd %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 86 3a 10[ 	]+vpmacsswd %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 86 f8 70[ 	]+vpmacsswd %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 86 3e 70[ 	]+vpmacsswd %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 86 fe 70[ 	]+vpmacsswd %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 86 c7 70[ 	]+vpmacsswd %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 86 02 00[ 	]+vpmacsswd %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 86 2a 10[ 	]+vpmacsswd %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 86 ef 10[ 	]+vpmacsswd %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 86 c7 10[ 	]+vpmacsswd %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 86 2e 70[ 	]+vpmacsswd %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 85 c7 00[ 	]+vpmacssww %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 85 06 70[ 	]+vpmacssww %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 85 06 10[ 	]+vpmacssww %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 85 e8 10[ 	]+vpmacssww %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 85 c6 10[ 	]+vpmacssww %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 85 fe 10[ 	]+vpmacssww %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 85 3a 10[ 	]+vpmacssww %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 85 f8 70[ 	]+vpmacssww %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 85 3e 70[ 	]+vpmacssww %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 85 fe 70[ 	]+vpmacssww %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 85 c7 70[ 	]+vpmacssww %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 85 02 00[ 	]+vpmacssww %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 85 2a 10[ 	]+vpmacssww %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 85 ef 10[ 	]+vpmacssww %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 85 c7 10[ 	]+vpmacssww %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 85 2e 70[ 	]+vpmacssww %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 96 c7 00[ 	]+vpmacswd %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 96 06 70[ 	]+vpmacswd %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 96 06 10[ 	]+vpmacswd %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 96 e8 10[ 	]+vpmacswd %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 96 c6 10[ 	]+vpmacswd %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 96 fe 10[ 	]+vpmacswd %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 96 3a 10[ 	]+vpmacswd %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 96 f8 70[ 	]+vpmacswd %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 96 3e 70[ 	]+vpmacswd %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 96 fe 70[ 	]+vpmacswd %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 96 c7 70[ 	]+vpmacswd %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 96 02 00[ 	]+vpmacswd %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 96 2a 10[ 	]+vpmacswd %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 96 ef 10[ 	]+vpmacswd %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 96 c7 10[ 	]+vpmacswd %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 96 2e 70[ 	]+vpmacswd %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 95 c7 00[ 	]+vpmacsww %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 95 06 70[ 	]+vpmacsww %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 95 06 10[ 	]+vpmacsww %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 95 e8 10[ 	]+vpmacsww %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 95 c6 10[ 	]+vpmacsww %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 95 fe 10[ 	]+vpmacsww %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 95 3a 10[ 	]+vpmacsww %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 95 f8 70[ 	]+vpmacsww %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 95 3e 70[ 	]+vpmacsww %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 95 fe 70[ 	]+vpmacsww %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 95 c7 70[ 	]+vpmacsww %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 95 02 00[ 	]+vpmacsww %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 95 2a 10[ 	]+vpmacsww %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 95 ef 10[ 	]+vpmacsww %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 95 c7 10[ 	]+vpmacsww %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 95 2e 70[ 	]+vpmacsww %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a6 c7 00[ 	]+vpmadcsswd %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a6 06 70[ 	]+vpmadcsswd %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a6 06 10[ 	]+vpmadcsswd %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a6 e8 10[ 	]+vpmadcsswd %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a6 c6 10[ 	]+vpmadcsswd %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a6 fe 10[ 	]+vpmadcsswd %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 a6 3a 10[ 	]+vpmadcsswd %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 a6 f8 70[ 	]+vpmadcsswd %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a6 3e 70[ 	]+vpmadcsswd %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a6 fe 70[ 	]+vpmadcsswd %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a6 c7 70[ 	]+vpmadcsswd %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a6 02 00[ 	]+vpmadcsswd %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 a6 2a 10[ 	]+vpmadcsswd %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a6 ef 10[ 	]+vpmadcsswd %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a6 c7 10[ 	]+vpmadcsswd %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a6 2e 70[ 	]+vpmadcsswd %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 b6 c7 00[ 	]+vpmadcswd %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 b6 06 70[ 	]+vpmadcswd %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 b6 06 10[ 	]+vpmadcswd %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 b6 e8 10[ 	]+vpmadcswd %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 b6 c6 10[ 	]+vpmadcswd %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 b6 fe 10[ 	]+vpmadcswd %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 b6 3a 10[ 	]+vpmadcswd %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 b6 f8 70[ 	]+vpmadcswd %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 b6 3e 70[ 	]+vpmadcswd %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 b6 fe 70[ 	]+vpmadcswd %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 b6 c7 70[ 	]+vpmadcswd %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 b6 02 00[ 	]+vpmadcswd %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 b6 2a 10[ 	]+vpmadcswd %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 b6 ef 10[ 	]+vpmadcswd %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 b6 c7 10[ 	]+vpmadcswd %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 b6 2e 70[ 	]+vpmadcswd %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a3 c6 00[ 	]+vpperm %xmm0,%xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 f8 a3 06 70[ 	]+vpperm \(%esi\),%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 e0 a3 00 70[ 	]+vpperm \(%eax\),%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a3 e8 70[ 	]+vpperm %xmm7,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a3 c0 70[ 	]+vpperm %xmm7,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a3 f8 70[ 	]+vpperm %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 f8 a3 38 60[ 	]+vpperm \(%eax\),%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 f8 a3 3e 00[ 	]+vpperm \(%esi\),%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 e0 a3 3b 70[ 	]+vpperm \(%ebx\),%xmm7,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 e0 a3 3b 00[ 	]+vpperm \(%ebx\),%xmm0,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 e0 a3 06 60[ 	]+vpperm \(%esi\),%xmm6,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a3 c7 10[ 	]+vpperm %xmm1,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 c0 a3 28 70[ 	]+vpperm \(%eax\),%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a3 ee 70[ 	]+vpperm %xmm7,%xmm6,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a3 c6 70[ 	]+vpperm %xmm7,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 e0 a3 2b 70[ 	]+vpperm \(%ebx\),%xmm7,%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a3 c7 00[ 	]+vpperm %xmm0,%xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a3 06 70[ 	]+vpperm %xmm7,\(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a3 06 10[ 	]+vpperm %xmm1,\(%esi\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a3 e8 10[ 	]+vpperm %xmm1,%xmm0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a3 c6 10[ 	]+vpperm %xmm1,%xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 a3 fe 10[ 	]+vpperm %xmm1,%xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 a3 3a 10[ 	]+vpperm %xmm1,\(%edx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 a3 f8 70[ 	]+vpperm %xmm7,%xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a3 3e 70[ 	]+vpperm %xmm7,\(%esi\),%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a3 fe 70[ 	]+vpperm %xmm7,%xmm6,%xmm3,%xmm7
[ 	]*[a-f0-9]+:	8f e8 60 a3 c7 70[ 	]+vpperm %xmm7,%xmm7,%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a3 02 00[ 	]+vpperm %xmm0,\(%edx\),%xmm3,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 a3 2a 10[ 	]+vpperm %xmm1,\(%edx\),%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 40 a3 ef 10[ 	]+vpperm %xmm1,%xmm7,%xmm7,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 a3 c7 10[ 	]+vpperm %xmm1,%xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 60 a3 2e 70[ 	]+vpperm %xmm7,\(%esi\),%xmm3,%xmm5
[ 	]*[a-f0-9]+:	8f e9 40 90 d8[ 	]+vprotb %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 90 fe[ 	]+vprotb %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 90 c0[ 	]+vprotb %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 90 1e[ 	]+vprotb %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 90 c7[ 	]+vprotb %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 90 df[ 	]+vprotb %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 90 c6[ 	]+vprotb %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 90 c6[ 	]+vprotb %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 90 df[ 	]+vprotb %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 90 3e[ 	]+vprotb %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 90 fe[ 	]+vprotb %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 90 1e[ 	]+vprotb %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 90 02[ 	]+vprotb %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 90 3e[ 	]+vprotb %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 90 c7[ 	]+vprotb %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 90 1a[ 	]+vprotb %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 90 1b[ 	]+vprotb \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 90 3b[ 	]+vprotb \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 90 06[ 	]+vprotb \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 90 18[ 	]+vprotb \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 90 c6[ 	]+vprotb %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 90 de[ 	]+vprotb %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 90 c0[ 	]+vprotb %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 90 c0[ 	]+vprotb %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 90 1e[ 	]+vprotb \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 90 ff[ 	]+vprotb %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 90 f8[ 	]+vprotb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 90 1b[ 	]+vprotb \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 90 03[ 	]+vprotb \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 90 38[ 	]+vprotb \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 90 00[ 	]+vprotb \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 90 df[ 	]+vprotb %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 c0 d5 03[ 	]+vprotb \$0x3,%xmm5,%xmm2
[ 	]*[a-f0-9]+:	8f e8 78 c0 c0 ff[ 	]+vprotb \$0xff,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 fd ff[ 	]+vprotb \$0xff,%xmm5,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c0 fd 00[ 	]+vprotb \$0x0,%xmm5,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c0 ff 00[ 	]+vprotb \$0x0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c0 d0 00[ 	]+vprotb \$0x0,%xmm0,%xmm2
[ 	]*[a-f0-9]+:	8f e8 78 c0 c5 ff[ 	]+vprotb \$0xff,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 c0 03[ 	]+vprotb \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 c5 03[ 	]+vprotb \$0x3,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 f8 00[ 	]+vprotb \$0x0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c0 c7 ff[ 	]+vprotb \$0xff,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 d0 ff[ 	]+vprotb \$0xff,%xmm0,%xmm2
[ 	]*[a-f0-9]+:	8f e8 78 c0 d7 ff[ 	]+vprotb \$0xff,%xmm7,%xmm2
[ 	]*[a-f0-9]+:	8f e8 78 c0 ff 03[ 	]+vprotb \$0x3,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c0 d5 ff[ 	]+vprotb \$0xff,%xmm5,%xmm2
[ 	]*[a-f0-9]+:	8f e8 78 c0 d0 03[ 	]+vprotb \$0x3,%xmm0,%xmm2
[ 	]*[a-f0-9]+:	8f e9 40 92 d8[ 	]+vprotd %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 92 fe[ 	]+vprotd %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 92 c0[ 	]+vprotd %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 92 1e[ 	]+vprotd %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 92 c7[ 	]+vprotd %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 92 df[ 	]+vprotd %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 92 c6[ 	]+vprotd %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 92 c6[ 	]+vprotd %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 92 df[ 	]+vprotd %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 92 3e[ 	]+vprotd %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 92 fe[ 	]+vprotd %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 92 1e[ 	]+vprotd %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 92 02[ 	]+vprotd %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 92 3e[ 	]+vprotd %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 92 c7[ 	]+vprotd %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 92 1a[ 	]+vprotd %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 92 1b[ 	]+vprotd \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 92 3b[ 	]+vprotd \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 92 06[ 	]+vprotd \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 92 18[ 	]+vprotd \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 92 c6[ 	]+vprotd %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 92 de[ 	]+vprotd %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 92 c0[ 	]+vprotd %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 92 c0[ 	]+vprotd %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 92 1e[ 	]+vprotd \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 92 ff[ 	]+vprotd %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 92 f8[ 	]+vprotd %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 92 1b[ 	]+vprotd \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 92 03[ 	]+vprotd \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 92 38[ 	]+vprotd \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 92 00[ 	]+vprotd \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 92 df[ 	]+vprotd %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 c2 ff 00[ 	]+vprotd \$0x0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c2 3b 00[ 	]+vprotd \$0x0,\(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c2 e8 00[ 	]+vprotd \$0x0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c2 c5 ff[ 	]+vprotd \$0xff,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 c0 03[ 	]+vprotd \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 c7 03[ 	]+vprotd \$0x3,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 ed 00[ 	]+vprotd \$0x0,%xmm5,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c2 f8 00[ 	]+vprotd \$0x0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c2 00 03[ 	]+vprotd \$0x3,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 03 ff[ 	]+vprotd \$0xff,\(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 38 00[ 	]+vprotd \$0x0,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c2 ff ff[ 	]+vprotd \$0xff,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c2 ed ff[ 	]+vprotd \$0xff,%xmm5,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c2 2b ff[ 	]+vprotd \$0xff,\(%ebx\),%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c2 c7 ff[ 	]+vprotd \$0xff,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 38 03[ 	]+vprotd \$0x3,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 93 d8[ 	]+vprotq %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 93 fe[ 	]+vprotq %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 93 c0[ 	]+vprotq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 93 1e[ 	]+vprotq %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 93 c7[ 	]+vprotq %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 93 df[ 	]+vprotq %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 93 c6[ 	]+vprotq %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 93 c6[ 	]+vprotq %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 93 df[ 	]+vprotq %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 93 3e[ 	]+vprotq %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 93 fe[ 	]+vprotq %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 93 1e[ 	]+vprotq %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 93 02[ 	]+vprotq %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 93 3e[ 	]+vprotq %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 93 c7[ 	]+vprotq %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 93 1a[ 	]+vprotq %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 93 1b[ 	]+vprotq \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 93 3b[ 	]+vprotq \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 93 06[ 	]+vprotq \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 93 18[ 	]+vprotq \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 93 c6[ 	]+vprotq %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 93 de[ 	]+vprotq %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 93 c0[ 	]+vprotq %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 93 c0[ 	]+vprotq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 93 1e[ 	]+vprotq \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 93 ff[ 	]+vprotq %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 93 f8[ 	]+vprotq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 93 1b[ 	]+vprotq \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 93 03[ 	]+vprotq \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 93 38[ 	]+vprotq \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 93 00[ 	]+vprotq \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 93 df[ 	]+vprotq %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 c3 ff 00[ 	]+vprotq \$0x0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c3 3b 00[ 	]+vprotq \$0x0,\(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c3 e8 00[ 	]+vprotq \$0x0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c3 c5 ff[ 	]+vprotq \$0xff,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 c0 03[ 	]+vprotq \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 c7 03[ 	]+vprotq \$0x3,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 ed 00[ 	]+vprotq \$0x0,%xmm5,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c3 f8 00[ 	]+vprotq \$0x0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c3 00 03[ 	]+vprotq \$0x3,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 03 ff[ 	]+vprotq \$0xff,\(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 38 00[ 	]+vprotq \$0x0,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c3 ff ff[ 	]+vprotq \$0xff,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c3 ed ff[ 	]+vprotq \$0xff,%xmm5,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c3 2b ff[ 	]+vprotq \$0xff,\(%ebx\),%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c3 c7 ff[ 	]+vprotq \$0xff,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 38 03[ 	]+vprotq \$0x3,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 91 d8[ 	]+vprotw %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 91 fe[ 	]+vprotw %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 91 c0[ 	]+vprotw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 91 1e[ 	]+vprotw %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 91 c7[ 	]+vprotw %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 91 df[ 	]+vprotw %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 91 c6[ 	]+vprotw %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 91 c6[ 	]+vprotw %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 91 df[ 	]+vprotw %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 91 3e[ 	]+vprotw %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 91 fe[ 	]+vprotw %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 91 1e[ 	]+vprotw %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 91 02[ 	]+vprotw %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 91 3e[ 	]+vprotw %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 91 c7[ 	]+vprotw %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 91 1a[ 	]+vprotw %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 91 1b[ 	]+vprotw \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 91 3b[ 	]+vprotw \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 91 06[ 	]+vprotw \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 91 18[ 	]+vprotw \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 91 c6[ 	]+vprotw %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 91 de[ 	]+vprotw %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 91 c0[ 	]+vprotw %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 91 c0[ 	]+vprotw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 91 1e[ 	]+vprotw \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 91 ff[ 	]+vprotw %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 91 f8[ 	]+vprotw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 91 1b[ 	]+vprotw \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 91 03[ 	]+vprotw \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 91 38[ 	]+vprotw \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 91 00[ 	]+vprotw \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 91 df[ 	]+vprotw %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 c1 ff 00[ 	]+vprotw \$0x0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c1 3b 00[ 	]+vprotw \$0x0,\(%ebx\),%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c1 e8 00[ 	]+vprotw \$0x0,%xmm0,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c1 c5 ff[ 	]+vprotw \$0xff,%xmm5,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 c0 03[ 	]+vprotw \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 c7 03[ 	]+vprotw \$0x3,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 ed 00[ 	]+vprotw \$0x0,%xmm5,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c1 f8 00[ 	]+vprotw \$0x0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c1 00 03[ 	]+vprotw \$0x3,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 03 ff[ 	]+vprotw \$0xff,\(%ebx\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 38 00[ 	]+vprotw \$0x0,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c1 ff ff[ 	]+vprotw \$0xff,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 c1 ed ff[ 	]+vprotw \$0xff,%xmm5,%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c1 2b ff[ 	]+vprotw \$0xff,\(%ebx\),%xmm5
[ 	]*[a-f0-9]+:	8f e8 78 c1 c7 ff[ 	]+vprotw \$0xff,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 38 03[ 	]+vprotw \$0x3,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 98 d8[ 	]+vpshab %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 98 fe[ 	]+vpshab %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 98 c0[ 	]+vpshab %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 98 1e[ 	]+vpshab %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 98 c7[ 	]+vpshab %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 98 df[ 	]+vpshab %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 98 c6[ 	]+vpshab %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 98 c6[ 	]+vpshab %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 98 df[ 	]+vpshab %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 98 3e[ 	]+vpshab %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 98 fe[ 	]+vpshab %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 98 1e[ 	]+vpshab %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 98 02[ 	]+vpshab %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 98 3e[ 	]+vpshab %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 98 c7[ 	]+vpshab %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 98 1a[ 	]+vpshab %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 98 1b[ 	]+vpshab \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 98 3b[ 	]+vpshab \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 98 06[ 	]+vpshab \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 98 18[ 	]+vpshab \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 98 c6[ 	]+vpshab %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 98 de[ 	]+vpshab %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 98 c0[ 	]+vpshab %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 98 c0[ 	]+vpshab %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 98 1e[ 	]+vpshab \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 98 ff[ 	]+vpshab %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 98 f8[ 	]+vpshab %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 98 1b[ 	]+vpshab \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 98 03[ 	]+vpshab \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 98 38[ 	]+vpshab \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 98 00[ 	]+vpshab \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 98 df[ 	]+vpshab %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 9a d8[ 	]+vpshad %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 9a fe[ 	]+vpshad %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 9a c0[ 	]+vpshad %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9a 1e[ 	]+vpshad %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 9a c7[ 	]+vpshad %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 9a df[ 	]+vpshad %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 9a c6[ 	]+vpshad %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9a c6[ 	]+vpshad %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 9a df[ 	]+vpshad %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 9a 3e[ 	]+vpshad %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 9a fe[ 	]+vpshad %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 9a 1e[ 	]+vpshad %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 9a 02[ 	]+vpshad %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9a 3e[ 	]+vpshad %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 9a c7[ 	]+vpshad %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 9a 1a[ 	]+vpshad %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 9a 1b[ 	]+vpshad \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 9a 3b[ 	]+vpshad \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 9a 06[ 	]+vpshad \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 9a 18[ 	]+vpshad \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 9a c6[ 	]+vpshad %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9a de[ 	]+vpshad %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 9a c0[ 	]+vpshad %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 9a c0[ 	]+vpshad %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 9a 1e[ 	]+vpshad \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 9a ff[ 	]+vpshad %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 9a f8[ 	]+vpshad %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 9a 1b[ 	]+vpshad \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 9a 03[ 	]+vpshad \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 9a 38[ 	]+vpshad \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 9a 00[ 	]+vpshad \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9a df[ 	]+vpshad %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 9b d8[ 	]+vpshaq %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 9b fe[ 	]+vpshaq %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 9b c0[ 	]+vpshaq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9b 1e[ 	]+vpshaq %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 9b c7[ 	]+vpshaq %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 9b df[ 	]+vpshaq %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 9b c6[ 	]+vpshaq %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9b c6[ 	]+vpshaq %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 9b df[ 	]+vpshaq %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 9b 3e[ 	]+vpshaq %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 9b fe[ 	]+vpshaq %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 9b 1e[ 	]+vpshaq %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 9b 02[ 	]+vpshaq %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9b 3e[ 	]+vpshaq %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 9b c7[ 	]+vpshaq %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 9b 1a[ 	]+vpshaq %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 9b 1b[ 	]+vpshaq \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 9b 3b[ 	]+vpshaq \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 9b 06[ 	]+vpshaq \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 9b 18[ 	]+vpshaq \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 9b c6[ 	]+vpshaq %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9b de[ 	]+vpshaq %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 9b c0[ 	]+vpshaq %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 9b c0[ 	]+vpshaq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 9b 1e[ 	]+vpshaq \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 9b ff[ 	]+vpshaq %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 9b f8[ 	]+vpshaq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 9b 1b[ 	]+vpshaq \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 9b 03[ 	]+vpshaq \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 9b 38[ 	]+vpshaq \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 9b 00[ 	]+vpshaq \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 9b df[ 	]+vpshaq %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 99 d8[ 	]+vpshaw %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 99 fe[ 	]+vpshaw %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 99 c0[ 	]+vpshaw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 99 1e[ 	]+vpshaw %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 99 c7[ 	]+vpshaw %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 99 df[ 	]+vpshaw %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 99 c6[ 	]+vpshaw %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 99 c6[ 	]+vpshaw %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 99 df[ 	]+vpshaw %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 99 3e[ 	]+vpshaw %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 99 fe[ 	]+vpshaw %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 99 1e[ 	]+vpshaw %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 99 02[ 	]+vpshaw %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 99 3e[ 	]+vpshaw %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 99 c7[ 	]+vpshaw %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 99 1a[ 	]+vpshaw %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 99 1b[ 	]+vpshaw \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 99 3b[ 	]+vpshaw \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 99 06[ 	]+vpshaw \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 99 18[ 	]+vpshaw \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 99 c6[ 	]+vpshaw %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 99 de[ 	]+vpshaw %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 99 c0[ 	]+vpshaw %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 99 c0[ 	]+vpshaw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 99 1e[ 	]+vpshaw \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 99 ff[ 	]+vpshaw %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 99 f8[ 	]+vpshaw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 99 1b[ 	]+vpshaw \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 99 03[ 	]+vpshaw \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 99 38[ 	]+vpshaw \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 99 00[ 	]+vpshaw \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 99 df[ 	]+vpshaw %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 94 d8[ 	]+vpshlb %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 94 fe[ 	]+vpshlb %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 94 c0[ 	]+vpshlb %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 94 1e[ 	]+vpshlb %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 94 c7[ 	]+vpshlb %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 94 df[ 	]+vpshlb %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 94 c6[ 	]+vpshlb %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 94 c6[ 	]+vpshlb %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 94 df[ 	]+vpshlb %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 94 3e[ 	]+vpshlb %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 94 fe[ 	]+vpshlb %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 94 1e[ 	]+vpshlb %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 94 02[ 	]+vpshlb %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 94 3e[ 	]+vpshlb %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 94 c7[ 	]+vpshlb %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 94 1a[ 	]+vpshlb %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 94 1b[ 	]+vpshlb \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 94 3b[ 	]+vpshlb \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 94 06[ 	]+vpshlb \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 94 18[ 	]+vpshlb \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 94 c6[ 	]+vpshlb %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 94 de[ 	]+vpshlb %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 94 c0[ 	]+vpshlb %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 94 c0[ 	]+vpshlb %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 94 1e[ 	]+vpshlb \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 94 ff[ 	]+vpshlb %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 94 f8[ 	]+vpshlb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 94 1b[ 	]+vpshlb \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 94 03[ 	]+vpshlb \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 94 38[ 	]+vpshlb \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 94 00[ 	]+vpshlb \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 94 df[ 	]+vpshlb %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 96 d8[ 	]+vpshld %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 96 fe[ 	]+vpshld %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 96 c0[ 	]+vpshld %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 96 1e[ 	]+vpshld %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 96 c7[ 	]+vpshld %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 96 df[ 	]+vpshld %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 96 c6[ 	]+vpshld %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 96 c6[ 	]+vpshld %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 96 df[ 	]+vpshld %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 96 3e[ 	]+vpshld %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 96 fe[ 	]+vpshld %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 96 1e[ 	]+vpshld %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 96 02[ 	]+vpshld %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 96 3e[ 	]+vpshld %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 96 c7[ 	]+vpshld %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 96 1a[ 	]+vpshld %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 96 1b[ 	]+vpshld \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 96 3b[ 	]+vpshld \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 96 06[ 	]+vpshld \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 96 18[ 	]+vpshld \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 96 c6[ 	]+vpshld %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 96 de[ 	]+vpshld %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 96 c0[ 	]+vpshld %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 96 c0[ 	]+vpshld %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 96 1e[ 	]+vpshld \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 96 ff[ 	]+vpshld %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 96 f8[ 	]+vpshld %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 96 1b[ 	]+vpshld \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 96 03[ 	]+vpshld \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 96 38[ 	]+vpshld \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 96 00[ 	]+vpshld \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 96 df[ 	]+vpshld %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 97 d8[ 	]+vpshlq %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 97 fe[ 	]+vpshlq %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 97 c0[ 	]+vpshlq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 97 1e[ 	]+vpshlq %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 97 c7[ 	]+vpshlq %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 97 df[ 	]+vpshlq %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 97 c6[ 	]+vpshlq %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 97 c6[ 	]+vpshlq %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 97 df[ 	]+vpshlq %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 97 3e[ 	]+vpshlq %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 97 fe[ 	]+vpshlq %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 97 1e[ 	]+vpshlq %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 97 02[ 	]+vpshlq %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 97 3e[ 	]+vpshlq %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 97 c7[ 	]+vpshlq %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 97 1a[ 	]+vpshlq %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 97 1b[ 	]+vpshlq \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 97 3b[ 	]+vpshlq \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 97 06[ 	]+vpshlq \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 97 18[ 	]+vpshlq \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 97 c6[ 	]+vpshlq %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 97 de[ 	]+vpshlq %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 97 c0[ 	]+vpshlq %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 97 c0[ 	]+vpshlq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 97 1e[ 	]+vpshlq \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 97 ff[ 	]+vpshlq %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 97 f8[ 	]+vpshlq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 97 1b[ 	]+vpshlq \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 97 03[ 	]+vpshlq \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 97 38[ 	]+vpshlq \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 97 00[ 	]+vpshlq \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 97 df[ 	]+vpshlq %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 95 d8[ 	]+vpshlw %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 95 fe[ 	]+vpshlw %xmm7,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 95 c0[ 	]+vpshlw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 95 1e[ 	]+vpshlw %xmm1,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 95 c7[ 	]+vpshlw %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 95 df[ 	]+vpshlw %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 95 c6[ 	]+vpshlw %xmm0,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 95 c6[ 	]+vpshlw %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 95 df[ 	]+vpshlw %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 78 95 3e[ 	]+vpshlw %xmm0,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 95 fe[ 	]+vpshlw %xmm0,%xmm6,%xmm7
[ 	]*[a-f0-9]+:	8f e9 40 95 1e[ 	]+vpshlw %xmm7,\(%esi\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 40 95 02[ 	]+vpshlw %xmm7,\(%edx\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 95 3e[ 	]+vpshlw %xmm1,\(%esi\),%xmm7
[ 	]*[a-f0-9]+:	8f e9 70 95 c7[ 	]+vpshlw %xmm1,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 95 1a[ 	]+vpshlw %xmm0,\(%edx\),%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 95 1b[ 	]+vpshlw \(%ebx\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e9 f8 95 3b[ 	]+vpshlw \(%ebx\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 95 06[ 	]+vpshlw \(%esi\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 95 18[ 	]+vpshlw \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 95 c6[ 	]+vpshlw %xmm1,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 95 de[ 	]+vpshlw %xmm1,%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 95 c0[ 	]+vpshlw %xmm1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 40 95 c0[ 	]+vpshlw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c8 95 1e[ 	]+vpshlw \(%esi\),%xmm6,%xmm3
[ 	]*[a-f0-9]+:	8f e9 70 95 ff[ 	]+vpshlw %xmm1,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 95 f8[ 	]+vpshlw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c0 95 1b[ 	]+vpshlw \(%ebx\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e9 c0 95 03[ 	]+vpshlw \(%ebx\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e9 c0 95 38[ 	]+vpshlw \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e9 c8 95 00[ 	]+vpshlw \(%eax\),%xmm6,%xmm0
[ 	]*[a-f0-9]+:	8f e9 70 95 df[ 	]+vpshlw %xmm1,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc c6 00[ 	]+vpcomltb %xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc fe 00[ 	]+vpcomltb %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 04 47 00[ 	]+vpcomltb \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc c6 00[ 	]+vpcomltb %xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc ff 00[ 	]+vpcomltb %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc 38 00[ 	]+vpcomltb \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc c6 00[ 	]+vpcomltb %xmm6,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc 3c 0a 00[ 	]+vpcomltb \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce 00 00[ 	]+vpcomltd \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ce 3c 47 00[ 	]+vpcomltd \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 00 00[ 	]+vpcomltd \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ce 1c 47 00[ 	]+vpcomltd \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ce 04 47 00[ 	]+vpcomltd \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 7c 10 01 00[ 	]+vpcomltd 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 44 10 01 00[ 	]+vpcomltd 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 1c 47 00[ 	]+vpcomltd \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf 3c 47 00[ 	]+vpcomltq \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf 44 10 01 00[ 	]+vpcomltq 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 3c 0a 00[ 	]+vpcomltq \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf d8 00[ 	]+vpcomltq %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf 5c 10 01 00[ 	]+vpcomltq 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cf 1c 47 00[ 	]+vpcomltq \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf 38 00[ 	]+vpcomltq \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf fe 00[ 	]+vpcomltq %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ec 1c 0a 00[ 	]+vpcomltub \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec 00 00[ 	]+vpcomltub \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec df 00[ 	]+vpcomltub %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec d8 00[ 	]+vpcomltub %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec c7 00[ 	]+vpcomltub %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec fe 00[ 	]+vpcomltub %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec df 00[ 	]+vpcomltub %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec 04 0a 00[ 	]+vpcomltub \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee 04 0a 00[ 	]+vpcomltud \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 3c 0a 00[ 	]+vpcomltud \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 3c 0a 00[ 	]+vpcomltud \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ee 1c 0a 00[ 	]+vpcomltud \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee c7 00[ 	]+vpcomltud %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee d8 00[ 	]+vpcomltud %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee 5c 10 01 00[ 	]+vpcomltud 0x1\(%eax,%edx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee ff 00[ 	]+vpcomltud %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef 04 47 00[ 	]+vpcomltuq \(%edi,%eax,2\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ef 38 00[ 	]+vpcomltuq \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef de 00[ 	]+vpcomltuq %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef c7 00[ 	]+vpcomltuq %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 5c 10 01 00[ 	]+vpcomltuq 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 1c 47 00[ 	]+vpcomltuq \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 04 0a 00[ 	]+vpcomltuq \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef f8 00[ 	]+vpcomltuq %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed ff 00[ 	]+vpcomltuw %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed 44 10 01 00[ 	]+vpcomltuw 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 04 47 00[ 	]+vpcomltuw \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed 04 0a 00[ 	]+vpcomltuw \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 3c 47 00[ 	]+vpcomltuw \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed 3c 47 00[ 	]+vpcomltuw \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed 04 0a 00[ 	]+vpcomltuw \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed df 00[ 	]+vpcomltuw %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd fe 00[ 	]+vpcomltw %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd de 00[ 	]+vpcomltw %xmm6,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 18 00[ 	]+vpcomltw \(%eax\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 1c 47 00[ 	]+vpcomltw \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd ff 00[ 	]+vpcomltw %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cd 5c 10 01 00[ 	]+vpcomltw 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 7c 10 01 00[ 	]+vpcomltw 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd 44 10 01 00[ 	]+vpcomltw 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 04 0a 01[ 	]+vpcomleb \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cc c6 01[ 	]+vpcomleb %xmm6,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cc 04 0a 01[ 	]+vpcomleb \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 7c 10 01 01[ 	]+vpcomleb 0x1\(%eax,%edx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc 44 10 01 01[ 	]+vpcomleb 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 38 01[ 	]+vpcomleb \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc 04 47 01[ 	]+vpcomleb \(%edi,%eax,2\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cc d8 01[ 	]+vpcomleb %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce 00 01[ 	]+vpcomled \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ce 3c 0a 01[ 	]+vpcomled \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 1c 47 01[ 	]+vpcomled \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce 04 0a 01[ 	]+vpcomled \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ce df 01[ 	]+vpcomled %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce d8 01[ 	]+vpcomled %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce 7c 10 01 01[ 	]+vpcomled 0x1\(%eax,%edx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 1c 47 01[ 	]+vpcomled \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf 7c 10 01 01[ 	]+vpcomleq 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf 5c 10 01 01[ 	]+vpcomleq 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cf ff 01[ 	]+vpcomleq %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf 3c 47 01[ 	]+vpcomleq \(%edi,%eax,2\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf 1c 0a 01[ 	]+vpcomleq \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cf 44 10 01 01[ 	]+vpcomleq 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf d8 01[ 	]+vpcomleq %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf de 01[ 	]+vpcomleq %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec c0 01[ 	]+vpcomleub %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ec f8 01[ 	]+vpcomleub %xmm0,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec f8 01[ 	]+vpcomleub %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 38 01[ 	]+vpcomleub \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec d8 01[ 	]+vpcomleub %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec ff 01[ 	]+vpcomleub %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec c7 01[ 	]+vpcomleub %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec df 01[ 	]+vpcomleub %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ee 38 01[ 	]+vpcomleud \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 5c 10 01 01[ 	]+vpcomleud 0x1\(%eax,%edx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee 04 47 01[ 	]+vpcomleud \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee 1c 0a 01[ 	]+vpcomleud \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee 1c 47 01[ 	]+vpcomleud \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ee de 01[ 	]+vpcomleud %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee df 01[ 	]+vpcomleud %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee d8 01[ 	]+vpcomleud %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef fe 01[ 	]+vpcomleuq %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ef de 01[ 	]+vpcomleuq %xmm6,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ef 7c 10 01 01[ 	]+vpcomleuq 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef 04 47 01[ 	]+vpcomleuq \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef de 01[ 	]+vpcomleuq %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef 04 0a 01[ 	]+vpcomleuq \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ef c0 01[ 	]+vpcomleuq %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 3c 0a 01[ 	]+vpcomleuq \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed 3c 0a 01[ 	]+vpcomleuw \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed 1c 47 01[ 	]+vpcomleuw \(%edi,%eax,2\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ed c6 01[ 	]+vpcomleuw %xmm6,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed c7 01[ 	]+vpcomleuw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 44 10 01 01[ 	]+vpcomleuw 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 5c 10 01 01[ 	]+vpcomleuw 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed fe 01[ 	]+vpcomleuw %xmm6,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed d8 01[ 	]+vpcomleuw %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd 44 10 01 01[ 	]+vpcomlew 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cd df 01[ 	]+vpcomlew %xmm7,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd ff 01[ 	]+vpcomlew %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd 44 10 01 01[ 	]+vpcomlew 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd 00 01[ 	]+vpcomlew \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd 1c 47 01[ 	]+vpcomlew \(%edi,%eax,2\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 3c 0a 01[ 	]+vpcomlew \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd 3c 0a 01[ 	]+vpcomlew \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 00 02[ 	]+vpcomgtb \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 18 02[ 	]+vpcomgtb \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc 38 02[ 	]+vpcomgtb \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc 04 47 02[ 	]+vpcomgtb \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc df 02[ 	]+vpcomgtb %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cc f8 02[ 	]+vpcomgtb %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc 3c 0a 02[ 	]+vpcomgtb \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc 3c 0a 02[ 	]+vpcomgtb \(%edx,%ecx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 04 47 02[ 	]+vpcomgtd \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 1c 0a 02[ 	]+vpcomgtd \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ce 00 02[ 	]+vpcomgtd \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 5c 10 01 02[ 	]+vpcomgtd 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce f8 02[ 	]+vpcomgtd %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce 1c 47 02[ 	]+vpcomgtd \(%edi,%eax,2\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce c0 02[ 	]+vpcomgtd %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce fe 02[ 	]+vpcomgtd %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf 3c 47 02[ 	]+vpcomgtq \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf 04 0a 02[ 	]+vpcomgtq \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf d8 02[ 	]+vpcomgtq %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf f8 02[ 	]+vpcomgtq %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf df 02[ 	]+vpcomgtq %xmm7,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf 3c 0a 02[ 	]+vpcomgtq \(%edx,%ecx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf fe 02[ 	]+vpcomgtq %xmm6,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf de 02[ 	]+vpcomgtq %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec 00 02[ 	]+vpcomgtub \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec 04 0a 02[ 	]+vpcomgtub \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec 3c 0a 02[ 	]+vpcomgtub \(%edx,%ecx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec c7 02[ 	]+vpcomgtub %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ec fe 02[ 	]+vpcomgtub %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 47 02[ 	]+vpcomgtub \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ec 3c 0a 02[ 	]+vpcomgtub \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec 04 0a 02[ 	]+vpcomgtub \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee c7 02[ 	]+vpcomgtud %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 1c 47 02[ 	]+vpcomgtud \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee c6 02[ 	]+vpcomgtud %xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 47 02[ 	]+vpcomgtud \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee fe 02[ 	]+vpcomgtud %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 44 10 01 02[ 	]+vpcomgtud 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee d8 02[ 	]+vpcomgtud %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee 1c 0a 02[ 	]+vpcomgtud \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ef 00 02[ 	]+vpcomgtuq \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 18 02[ 	]+vpcomgtuq \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 1c 0a 02[ 	]+vpcomgtuq \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef df 02[ 	]+vpcomgtuq %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 7c 10 01 02[ 	]+vpcomgtuq 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef 44 10 01 02[ 	]+vpcomgtuq 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef 5c 10 01 02[ 	]+vpcomgtuq 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef c7 02[ 	]+vpcomgtuq %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 04 0a 02[ 	]+vpcomgtuw \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed de 02[ 	]+vpcomgtuw %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ed f8 02[ 	]+vpcomgtuw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed fe 02[ 	]+vpcomgtuw %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed 38 02[ 	]+vpcomgtuw \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed d8 02[ 	]+vpcomgtuw %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed 44 10 01 02[ 	]+vpcomgtuw 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 1c 0a 02[ 	]+vpcomgtuw \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 5c 10 01 02[ 	]+vpcomgtw 0x1\(%eax,%edx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 1c 0a 02[ 	]+vpcomgtw \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd 3c 0a 02[ 	]+vpcomgtw \(%edx,%ecx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd 38 02[ 	]+vpcomgtw \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd c7 02[ 	]+vpcomgtw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd df 02[ 	]+vpcomgtw %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd c6 02[ 	]+vpcomgtw %xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cd fe 02[ 	]+vpcomgtw %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc c6 03[ 	]+vpcomgeb %xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 5c 10 01 03[ 	]+vpcomgeb 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc 18 03[ 	]+vpcomgeb \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc 04 0a 03[ 	]+vpcomgeb \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc f8 03[ 	]+vpcomgeb %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc 38 03[ 	]+vpcomgeb \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 3c 47 03[ 	]+vpcomgeb \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc f8 03[ 	]+vpcomgeb %xmm0,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 18 03[ 	]+vpcomged \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce 3c 0a 03[ 	]+vpcomged \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 3c 47 03[ 	]+vpcomged \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce de 03[ 	]+vpcomged %xmm6,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce d8 03[ 	]+vpcomged %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce fe 03[ 	]+vpcomged %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce f8 03[ 	]+vpcomged %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce 00 03[ 	]+vpcomged \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf fe 03[ 	]+vpcomgeq %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf c7 03[ 	]+vpcomgeq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf 5c 10 01 03[ 	]+vpcomgeq 0x1\(%eax,%edx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf 1c 0a 03[ 	]+vpcomgeq \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf d8 03[ 	]+vpcomgeq %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cf 7c 10 01 03[ 	]+vpcomgeq 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf 04 47 03[ 	]+vpcomgeq \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf 38 03[ 	]+vpcomgeq \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 04 47 03[ 	]+vpcomgeub \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec 18 03[ 	]+vpcomgeub \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec 44 10 01 03[ 	]+vpcomgeub 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ec 5c 10 01 03[ 	]+vpcomgeub 0x1\(%eax,%edx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec 18 03[ 	]+vpcomgeub \(%eax\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec de 03[ 	]+vpcomgeub %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec c6 03[ 	]+vpcomgeub %xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec 5c 10 01 03[ 	]+vpcomgeub 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee 44 10 01 03[ 	]+vpcomgeud 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee 5c 10 01 03[ 	]+vpcomgeud 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 0a 03[ 	]+vpcomgeud \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee df 03[ 	]+vpcomgeud %xmm7,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee d8 03[ 	]+vpcomgeud %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ee 04 0a 03[ 	]+vpcomgeud \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee ff 03[ 	]+vpcomgeud %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 1c 47 03[ 	]+vpcomgeud \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 44 10 01 03[ 	]+vpcomgeuq 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef 3c 47 03[ 	]+vpcomgeuq \(%edi,%eax,2\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ef ff 03[ 	]+vpcomgeuq %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ef 1c 47 03[ 	]+vpcomgeuq \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 1c 0a 03[ 	]+vpcomgeuq \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef d8 03[ 	]+vpcomgeuq %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef ff 03[ 	]+vpcomgeuq %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef 1c 0a 03[ 	]+vpcomgeuq \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed c7 03[ 	]+vpcomgeuw %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 1c 47 03[ 	]+vpcomgeuw \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed 1c 0a 03[ 	]+vpcomgeuw \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ed 44 10 01 03[ 	]+vpcomgeuw 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 44 10 01 03[ 	]+vpcomgeuw 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 00 03[ 	]+vpcomgeuw \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed c0 03[ 	]+vpcomgeuw %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed ff 03[ 	]+vpcomgeuw %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd c0 03[ 	]+vpcomgew %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd 38 03[ 	]+vpcomgew \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd c6 03[ 	]+vpcomgew %xmm6,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cd 00 03[ 	]+vpcomgew \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cd d8 03[ 	]+vpcomgew %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 44 10 01 03[ 	]+vpcomgew 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd 3c 47 03[ 	]+vpcomgew \(%edi,%eax,2\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd de 03[ 	]+vpcomgew %xmm6,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc 38 04[ 	]+vpcomeqb \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc 00 04[ 	]+vpcomeqb \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc 1c 0a 04[ 	]+vpcomeqb \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cc ff 04[ 	]+vpcomeqb %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc 04 47 04[ 	]+vpcomeqb \(%edi,%eax,2\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc 3c 0a 04[ 	]+vpcomeqb \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc fe 04[ 	]+vpcomeqb %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc f8 04[ 	]+vpcomeqb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 3c 47 04[ 	]+vpcomeqd \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 3c 0a 04[ 	]+vpcomeqd \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce 44 10 01 04[ 	]+vpcomeqd 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce ff 04[ 	]+vpcomeqd %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 04 0a 04[ 	]+vpcomeqd \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ce f8 04[ 	]+vpcomeqd %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce d8 04[ 	]+vpcomeqd %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce 1c 0a 04[ 	]+vpcomeqd \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf 00 04[ 	]+vpcomeqq \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf c6 04[ 	]+vpcomeqq %xmm6,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf 04 47 04[ 	]+vpcomeqq \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf c6 04[ 	]+vpcomeqq %xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf 04 0a 04[ 	]+vpcomeqq \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 3c 47 04[ 	]+vpcomeqq \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf c0 04[ 	]+vpcomeqq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf 44 10 01 04[ 	]+vpcomeqq 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ec c7 04[ 	]+vpcomequb %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec 18 04[ 	]+vpcomequb \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec 3c 0a 04[ 	]+vpcomequb \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec 1c 0a 04[ 	]+vpcomequb \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec 1c 0a 04[ 	]+vpcomequb \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec de 04[ 	]+vpcomequb %xmm6,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec 5c 10 01 04[ 	]+vpcomequb 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec c6 04[ 	]+vpcomequb %xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee 00 04[ 	]+vpcomequd \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee 3c 47 04[ 	]+vpcomequd \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 38 04[ 	]+vpcomequd \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee de 04[ 	]+vpcomequd %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee 1c 47 04[ 	]+vpcomequd \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ee 7c 10 01 04[ 	]+vpcomequd 0x1\(%eax,%edx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ee ff 04[ 	]+vpcomequd %xmm7,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee fe 04[ 	]+vpcomequd %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef 5c 10 01 04[ 	]+vpcomequq 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef fe 04[ 	]+vpcomequq %xmm6,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef 44 10 01 04[ 	]+vpcomequq 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef ff 04[ 	]+vpcomequq %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ef c7 04[ 	]+vpcomequq %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef de 04[ 	]+vpcomequq %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ef de 04[ 	]+vpcomequq %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ef 44 10 01 04[ 	]+vpcomequq 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 04 47 04[ 	]+vpcomequw \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed 00 04[ 	]+vpcomequw \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 18 04[ 	]+vpcomequw \(%eax\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ed 44 10 01 04[ 	]+vpcomequw 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed ff 04[ 	]+vpcomequw %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed 38 04[ 	]+vpcomequw \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed 18 04[ 	]+vpcomequw \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ed d8 04[ 	]+vpcomequw %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 1c 0a 04[ 	]+vpcomeqw \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 04 0a 04[ 	]+vpcomeqw \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cd 38 04[ 	]+vpcomeqw \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cd 38 04[ 	]+vpcomeqw \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd 1c 47 04[ 	]+vpcomeqw \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd d8 04[ 	]+vpcomeqw %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd d8 04[ 	]+vpcomeqw %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd c7 04[ 	]+vpcomeqw %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc 44 10 01 05[ 	]+vpcomneqb 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc 18 05[ 	]+vpcomneqb \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cc 00 05[ 	]+vpcomneqb \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 7c 10 01 05[ 	]+vpcomneqb 0x1\(%eax,%edx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc fe 05[ 	]+vpcomneqb %xmm6,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc f8 05[ 	]+vpcomneqb %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 1c 47 05[ 	]+vpcomneqb \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc de 05[ 	]+vpcomneqb %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce d8 05[ 	]+vpcomneqd %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ce 04 0a 05[ 	]+vpcomneqd \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ce 3c 47 05[ 	]+vpcomneqd \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 38 05[ 	]+vpcomneqd \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce f8 05[ 	]+vpcomneqd %xmm0,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce 1c 0a 05[ 	]+vpcomneqd \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce df 05[ 	]+vpcomneqd %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce 18 05[ 	]+vpcomneqd \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf de 05[ 	]+vpcomneqq %xmm6,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf c0 05[ 	]+vpcomneqq %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 5c 10 01 05[ 	]+vpcomneqq 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf c7 05[ 	]+vpcomneqq %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf 00 05[ 	]+vpcomneqq \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf ff 05[ 	]+vpcomneqq %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf 3c 0a 05[ 	]+vpcomneqq \(%edx,%ecx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cf 3c 47 05[ 	]+vpcomneqq \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec 5c 10 01 05[ 	]+vpcomnequb 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec 04 0a 05[ 	]+vpcomnequb \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec 1c 0a 05[ 	]+vpcomnequb \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec de 05[ 	]+vpcomnequb %xmm6,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec f8 05[ 	]+vpcomnequb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec df 05[ 	]+vpcomnequb %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec c7 05[ 	]+vpcomnequb %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec d8 05[ 	]+vpcomnequb %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee c0 05[ 	]+vpcomnequd %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee 1c 47 05[ 	]+vpcomnequd \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee 3c 0a 05[ 	]+vpcomnequd \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee 1c 0a 05[ 	]+vpcomnequd \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee 00 05[ 	]+vpcomnequd \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee 38 05[ 	]+vpcomnequd \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee de 05[ 	]+vpcomnequd %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee 38 05[ 	]+vpcomnequd \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef df 05[ 	]+vpcomnequq %xmm7,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ef 04 0a 05[ 	]+vpcomnequq \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ef c7 05[ 	]+vpcomnequq %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ef d8 05[ 	]+vpcomnequq %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ef 3c 0a 05[ 	]+vpcomnequq \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ef 04 47 05[ 	]+vpcomnequq \(%edi,%eax,2\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef 18 05[ 	]+vpcomnequq \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef 5c 10 01 05[ 	]+vpcomnequq 0x1\(%eax,%edx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ed 00 05[ 	]+vpcomnequw \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 7c 10 01 05[ 	]+vpcomnequw 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed c0 05[ 	]+vpcomnequw %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed fe 05[ 	]+vpcomnequw %xmm6,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed 04 0a 05[ 	]+vpcomnequw \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed c7 05[ 	]+vpcomnequw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed 44 10 01 05[ 	]+vpcomnequw 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed de 05[ 	]+vpcomnequw %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd fe 05[ 	]+vpcomneqw %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd 38 05[ 	]+vpcomneqw \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd ff 05[ 	]+vpcomneqw %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd d8 05[ 	]+vpcomneqw %xmm0,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd 00 05[ 	]+vpcomneqw \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd df 05[ 	]+vpcomneqw %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 18 05[ 	]+vpcomneqw \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 3c 47 05[ 	]+vpcomneqw \(%edi,%eax,2\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cc 1c 0a 06[ 	]+vpcomfalseb \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cc 5c 10 01 06[ 	]+vpcomfalseb 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc c7 06[ 	]+vpcomfalseb %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc 38 06[ 	]+vpcomfalseb \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 38 06[ 	]+vpcomfalseb \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 1c 47 06[ 	]+vpcomfalseb \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cc 7c 10 01 06[ 	]+vpcomfalseb 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 00 06[ 	]+vpcomfalseb \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce de 06[ 	]+vpcomfalsed %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ce 7c 10 01 06[ 	]+vpcomfalsed 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ce 7c 10 01 06[ 	]+vpcomfalsed 0x1\(%eax,%edx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce df 06[ 	]+vpcomfalsed %xmm7,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ce c0 06[ 	]+vpcomfalsed %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 18 06[ 	]+vpcomfalsed \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ce 04 0a 06[ 	]+vpcomfalsed \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 04 0a 06[ 	]+vpcomfalsed \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf c6 06[ 	]+vpcomfalseq %xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 7c 10 01 06[ 	]+vpcomfalseq 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf c0 06[ 	]+vpcomfalseq %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf 04 0a 06[ 	]+vpcomfalseq \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf 00 06[ 	]+vpcomfalseq \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cf 18 06[ 	]+vpcomfalseq \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf ff 06[ 	]+vpcomfalseq %xmm7,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cf 1c 0a 06[ 	]+vpcomfalseq \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ec fe 06[ 	]+vpcomfalseub %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ec 38 06[ 	]+vpcomfalseub \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 47 06[ 	]+vpcomfalseub \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ec c0 06[ 	]+vpcomfalseub %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec c7 06[ 	]+vpcomfalseub %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ec 1c 0a 06[ 	]+vpcomfalseub \(%edx,%ecx,1\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec d8 06[ 	]+vpcomfalseub %xmm0,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec 7c 10 01 06[ 	]+vpcomfalseub 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 3c 0a 06[ 	]+vpcomfalseud \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee 38 06[ 	]+vpcomfalseud \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee 00 06[ 	]+vpcomfalseud \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee c7 06[ 	]+vpcomfalseud %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee 00 06[ 	]+vpcomfalseud \(%eax\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee 5c 10 01 06[ 	]+vpcomfalseud 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ee 04 47 06[ 	]+vpcomfalseud \(%edi,%eax,2\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 0a 06[ 	]+vpcomfalseud \(%edx,%ecx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ef c0 06[ 	]+vpcomfalseuq %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 44 10 01 06[ 	]+vpcomfalseuq 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 38 06[ 	]+vpcomfalseuq \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef c0 06[ 	]+vpcomfalseuq %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef c7 06[ 	]+vpcomfalseuq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ef 7c 10 01 06[ 	]+vpcomfalseuq 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef 18 06[ 	]+vpcomfalseuq \(%eax\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ef c7 06[ 	]+vpcomfalseuq %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed 18 06[ 	]+vpcomfalseuw \(%eax\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ed 04 0a 06[ 	]+vpcomfalseuw \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ed fe 06[ 	]+vpcomfalseuw %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed df 06[ 	]+vpcomfalseuw %xmm7,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed f8 06[ 	]+vpcomfalseuw %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ed c7 06[ 	]+vpcomfalseuw %xmm7,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed 44 10 01 06[ 	]+vpcomfalseuw 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 38 06[ 	]+vpcomfalseuw \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd 7c 10 01 06[ 	]+vpcomfalsew 0x1\(%eax,%edx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd 18 06[ 	]+vpcomfalsew \(%eax\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cd 3c 47 06[ 	]+vpcomfalsew \(%edi,%eax,2\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd 1c 47 06[ 	]+vpcomfalsew \(%edi,%eax,2\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 3c 0a 06[ 	]+vpcomfalsew \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd 7c 10 01 06[ 	]+vpcomfalsew 0x1\(%eax,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd fe 06[ 	]+vpcomfalsew %xmm6,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd ff 06[ 	]+vpcomfalsew %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 3c 47 07[ 	]+vpcomtrueb \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cc 1c 47 07[ 	]+vpcomtrueb \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cc 18 07[ 	]+vpcomtrueb \(%eax\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cc 44 10 01 07[ 	]+vpcomtrueb 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cc 5c 10 01 07[ 	]+vpcomtrueb 0x1\(%eax,%edx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cc c7 07[ 	]+vpcomtrueb %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cc df 07[ 	]+vpcomtrueb %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cc c0 07[ 	]+vpcomtrueb %xmm0,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ce 38 07[ 	]+vpcomtrued \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce c6 07[ 	]+vpcomtrued %xmm6,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ce f8 07[ 	]+vpcomtrued %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ce 04 0a 07[ 	]+vpcomtrued \(%edx,%ecx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ce 44 10 01 07[ 	]+vpcomtrued 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce ff 07[ 	]+vpcomtrued %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 38 07[ 	]+vpcomtrued \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 1c 0a 07[ 	]+vpcomtrued \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf 1c 47 07[ 	]+vpcomtrueq \(%edi,%eax,2\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 cf df 07[ 	]+vpcomtrueq %xmm7,%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cf fe 07[ 	]+vpcomtrueq %xmm6,%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cf 04 47 07[ 	]+vpcomtrueq \(%edi,%eax,2\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 cf c0 07[ 	]+vpcomtrueq %xmm0,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf c7 07[ 	]+vpcomtrueq %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 04 47 07[ 	]+vpcomtrueq \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 38 07[ 	]+vpcomtrueq \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 0a 07[ 	]+vpcomtrueub \(%edx,%ecx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ec 1c 47 07[ 	]+vpcomtrueub \(%edi,%eax,2\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ec 44 10 01 07[ 	]+vpcomtrueub 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec 5c 10 01 07[ 	]+vpcomtrueub 0x1\(%eax,%edx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ec 44 10 01 07[ 	]+vpcomtrueub 0x1\(%eax,%edx,1\),%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ec 38 07[ 	]+vpcomtrueub \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 04 47 07[ 	]+vpcomtrueub \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec de 07[ 	]+vpcomtrueub %xmm6,%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee 3c 47 07[ 	]+vpcomtrueud \(%edi,%eax,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ee df 07[ 	]+vpcomtrueud %xmm7,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 ee c7 07[ 	]+vpcomtrueud %xmm7,%xmm4,%xmm0
[ 	]*[a-f0-9]+:	8f e8 58 ee 1c 47 07[ 	]+vpcomtrueud \(%edi,%eax,2\),%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 ee 38 07[ 	]+vpcomtrueud \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ee c6 07[ 	]+vpcomtrueud %xmm6,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ee 44 10 01 07[ 	]+vpcomtrueud 0x1\(%eax,%edx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 47 07[ 	]+vpcomtrueud \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef 3c 0a 07[ 	]+vpcomtrueuq \(%edx,%ecx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef 38 07[ 	]+vpcomtrueuq \(%eax\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef 7c 10 01 07[ 	]+vpcomtrueuq 0x1\(%eax,%edx,1\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef ff 07[ 	]+vpcomtrueuq %xmm7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 ef 00 07[ 	]+vpcomtrueuq \(%eax\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 04 47 07[ 	]+vpcomtrueuq \(%edi,%eax,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ef 04 0a 07[ 	]+vpcomtrueuq \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef c0 07[ 	]+vpcomtrueuq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed 1c 0a 07[ 	]+vpcomtrueuw \(%edx,%ecx,1\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed 3c 47 07[ 	]+vpcomtrueuw \(%edi,%eax,2\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed 44 10 01 07[ 	]+vpcomtrueuw 0x1\(%eax,%edx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ed c7 07[ 	]+vpcomtrueuw %xmm7,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 ed f8 07[ 	]+vpcomtrueuw %xmm0,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 ed d8 07[ 	]+vpcomtrueuw %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 40 ed 38 07[ 	]+vpcomtrueuw \(%eax\),%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed f8 07[ 	]+vpcomtrueuw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd c6 07[ 	]+vpcomtruew %xmm6,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd 04 0a 07[ 	]+vpcomtruew \(%edx,%ecx,1\),%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 40 cd 1c 0a 07[ 	]+vpcomtruew \(%edx,%ecx,1\),%xmm7,%xmm3
[ 	]*[a-f0-9]+:	8f e8 58 cd 3c 0a 07[ 	]+vpcomtruew \(%edx,%ecx,1\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd 38 07[ 	]+vpcomtruew \(%eax\),%xmm4,%xmm7
[ 	]*[a-f0-9]+:	8f e8 40 cd fe 07[ 	]+vpcomtruew %xmm6,%xmm7,%xmm7
[ 	]*[a-f0-9]+:	8f e8 58 cd d8 07[ 	]+vpcomtruew %xmm0,%xmm4,%xmm3
[ 	]*[a-f0-9]+:	8f e8 78 cd 1c 0a 07[ 	]+vpcomtruew \(%edx,%ecx,1\),%xmm0,%xmm3
#pass
