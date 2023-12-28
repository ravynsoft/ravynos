#objdump: -dw
#name: x86-64 XOP

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f 69 78 81 fa[ 	]+vfrczpd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 81 e0[ 	]+vfrczpd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 81 04 24[ 	]+vfrczpd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 81 38[ 	]+vfrczpd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 81 c0[ 	]+vfrczpd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 81 3a[ 	]+vfrczpd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 81 c2[ 	]+vfrczpd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 81 e7[ 	]+vfrczpd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 81 c7[ 	]+vfrczpd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 81 f8[ 	]+vfrczpd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 81 22[ 	]+vfrczpd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 81 ff[ 	]+vfrczpd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 81 00[ 	]+vfrczpd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 81 3c 24[ 	]+vfrczpd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 81 20[ 	]+vfrczpd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 81 02[ 	]+vfrczpd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 7c 81 fa[ 	]+vfrczpd %ymm2,%ymm15
[ 	]*[a-f0-9]+:	8f 69 7c 81 e0[ 	]+vfrczpd %ymm0,%ymm12
[ 	]*[a-f0-9]+:	8f c9 7c 81 04 24[ 	]+vfrczpd \(%r12\),%ymm0
[ 	]*[a-f0-9]+:	8f 69 7c 81 38[ 	]+vfrczpd \(%rax\),%ymm15
[ 	]*[a-f0-9]+:	8f e9 7c 81 c0[ 	]+vfrczpd %ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f 49 7c 81 3a[ 	]+vfrczpd \(%r10\),%ymm15
[ 	]*[a-f0-9]+:	8f e9 7c 81 c2[ 	]+vfrczpd %ymm2,%ymm0
[ 	]*[a-f0-9]+:	8f 49 7c 81 e7[ 	]+vfrczpd %ymm15,%ymm12
[ 	]*[a-f0-9]+:	8f c9 7c 81 c7[ 	]+vfrczpd %ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f 69 7c 81 f8[ 	]+vfrczpd %ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 49 7c 81 22[ 	]+vfrczpd \(%r10\),%ymm12
[ 	]*[a-f0-9]+:	8f 49 7c 81 ff[ 	]+vfrczpd %ymm15,%ymm15
[ 	]*[a-f0-9]+:	8f e9 7c 81 00[ 	]+vfrczpd \(%rax\),%ymm0
[ 	]*[a-f0-9]+:	8f 49 7c 81 3c 24[ 	]+vfrczpd \(%r12\),%ymm15
[ 	]*[a-f0-9]+:	8f 69 7c 81 20[ 	]+vfrczpd \(%rax\),%ymm12
[ 	]*[a-f0-9]+:	8f c9 7c 81 02[ 	]+vfrczpd \(%r10\),%ymm0
[ 	]*[a-f0-9]+:	8f 69 78 80 fa[ 	]+vfrczps %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 80 e0[ 	]+vfrczps %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 80 04 24[ 	]+vfrczps \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 80 38[ 	]+vfrczps \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 80 c0[ 	]+vfrczps %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 80 3a[ 	]+vfrczps \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 80 c2[ 	]+vfrczps %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 80 e7[ 	]+vfrczps %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 80 c7[ 	]+vfrczps %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 80 f8[ 	]+vfrczps %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 80 22[ 	]+vfrczps \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 80 ff[ 	]+vfrczps %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 80 00[ 	]+vfrczps \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 80 3c 24[ 	]+vfrczps \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 80 20[ 	]+vfrczps \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 80 02[ 	]+vfrczps \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 7c 80 fa[ 	]+vfrczps %ymm2,%ymm15
[ 	]*[a-f0-9]+:	8f 69 7c 80 e0[ 	]+vfrczps %ymm0,%ymm12
[ 	]*[a-f0-9]+:	8f c9 7c 80 04 24[ 	]+vfrczps \(%r12\),%ymm0
[ 	]*[a-f0-9]+:	8f 69 7c 80 38[ 	]+vfrczps \(%rax\),%ymm15
[ 	]*[a-f0-9]+:	8f e9 7c 80 c0[ 	]+vfrczps %ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f 49 7c 80 3a[ 	]+vfrczps \(%r10\),%ymm15
[ 	]*[a-f0-9]+:	8f e9 7c 80 c2[ 	]+vfrczps %ymm2,%ymm0
[ 	]*[a-f0-9]+:	8f 49 7c 80 e7[ 	]+vfrczps %ymm15,%ymm12
[ 	]*[a-f0-9]+:	8f c9 7c 80 c7[ 	]+vfrczps %ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f 69 7c 80 f8[ 	]+vfrczps %ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 49 7c 80 22[ 	]+vfrczps \(%r10\),%ymm12
[ 	]*[a-f0-9]+:	8f 49 7c 80 ff[ 	]+vfrczps %ymm15,%ymm15
[ 	]*[a-f0-9]+:	8f e9 7c 80 00[ 	]+vfrczps \(%rax\),%ymm0
[ 	]*[a-f0-9]+:	8f 49 7c 80 3c 24[ 	]+vfrczps \(%r12\),%ymm15
[ 	]*[a-f0-9]+:	8f 69 7c 80 20[ 	]+vfrczps \(%rax\),%ymm12
[ 	]*[a-f0-9]+:	8f c9 7c 80 02[ 	]+vfrczps \(%r10\),%ymm0
[ 	]*[a-f0-9]+:	8f 69 78 83 fa[ 	]+vfrczsd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 83 e0[ 	]+vfrczsd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 83 04 24[ 	]+vfrczsd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 83 38[ 	]+vfrczsd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 83 c0[ 	]+vfrczsd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 83 3a[ 	]+vfrczsd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 83 c2[ 	]+vfrczsd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 83 e7[ 	]+vfrczsd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 83 c7[ 	]+vfrczsd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 83 f8[ 	]+vfrczsd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 83 22[ 	]+vfrczsd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 83 ff[ 	]+vfrczsd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 83 00[ 	]+vfrczsd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 83 3c 24[ 	]+vfrczsd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 83 20[ 	]+vfrczsd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 83 02[ 	]+vfrczsd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 82 fa[ 	]+vfrczss %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 82 e0[ 	]+vfrczss %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 82 04 24[ 	]+vfrczss \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 82 38[ 	]+vfrczss \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 82 c0[ 	]+vfrczss %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 82 3a[ 	]+vfrczss \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 82 c2[ 	]+vfrczss %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 82 e7[ 	]+vfrczss %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 82 c7[ 	]+vfrczss %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 82 f8[ 	]+vfrczss %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 82 22[ 	]+vfrczss \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 82 ff[ 	]+vfrczss %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 82 00[ 	]+vfrczss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 82 3c 24[ 	]+vfrczss \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 82 20[ 	]+vfrczss \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 82 02[ 	]+vfrczss \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f c8 40 a2 c7 00[ 	]+vpcmov %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 a2 01 20[ 	]+vpcmov %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a2 01 f0[ 	]+vpcmov %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a2 d8 f0[ 	]+vpcmov %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a2 c4 f0[ 	]+vpcmov %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 a2 fc f0[ 	]+vpcmov %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 a2 3c 24 f0[ 	]+vpcmov %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 a2 f8 20[ 	]+vpcmov %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 a2 39 20[ 	]+vpcmov %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 a2 fc 20[ 	]+vpcmov %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 a2 04 24 20[ 	]+vpcmov %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a2 45 00 00[ 	]+vpcmov %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 a2 5d 00 f0[ 	]+vpcmov %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 a2 1c 24 f0[ 	]+vpcmov %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a2 c7 f0[ 	]+vpcmov %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 a2 19 20[ 	]+vpcmov %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 44 a2 c7 00[ 	]+vpcmov %ymm0,%ymm15,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	8f c8 7c a2 01 20[ 	]+vpcmov %ymm2,\(%r9\),%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f c8 04 a2 01 f0[ 	]+vpcmov %ymm15,\(%r9\),%ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f 68 7c a2 d8 f0[ 	]+vpcmov %ymm15,%ymm0,%ymm0,%ymm11
[ 	]*[a-f0-9]+:	8f c8 7c a2 c4 f0[ 	]+vpcmov %ymm15,%ymm12,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f 48 7c a2 fc f0[ 	]+vpcmov %ymm15,%ymm12,%ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 48 7c a2 3c 24 f0[ 	]+vpcmov %ymm15,\(%r12\),%ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 68 7c a2 f8 20[ 	]+vpcmov %ymm2,%ymm0,%ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 48 04 a2 39 20[ 	]+vpcmov %ymm2,\(%r9\),%ymm15,%ymm15
[ 	]*[a-f0-9]+:	8f 48 04 a2 fc 20[ 	]+vpcmov %ymm2,%ymm12,%ymm15,%ymm15
[ 	]*[a-f0-9]+:	8f c8 04 a2 04 24 20[ 	]+vpcmov %ymm2,\(%r12\),%ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f c8 04 a2 45 00 00[ 	]+vpcmov %ymm0,0x0\(%r13\),%ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f 48 44 a2 5d 00 f0[ 	]+vpcmov %ymm15,0x0\(%r13\),%ymm7,%ymm11
[ 	]*[a-f0-9]+:	8f 48 44 a2 1c 24 f0[ 	]+vpcmov %ymm15,\(%r12\),%ymm7,%ymm11
[ 	]*[a-f0-9]+:	8f c8 7c a2 c7 f0[ 	]+vpcmov %ymm15,%ymm15,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f 48 04 a2 19 20[ 	]+vpcmov %ymm2,\(%r9\),%ymm15,%ymm11
[ 	]*[a-f0-9]+:	8f c8 40 a2 c4 00[ 	]+vpcmov %xmm0,%xmm12,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 f8 a2 00 f0[ 	]+vpcmov \(%rax\),%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 80 a2 02 f0[ 	]+vpcmov \(%r10\),%xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a2 d8 20[ 	]+vpcmov %xmm2,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f e8 78 a2 c0 20[ 	]+vpcmov %xmm2,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a2 f8 20[ 	]+vpcmov %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 f8 a2 3a c0[ 	]+vpcmov \(%r10\),%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 f8 a2 38 00[ 	]+vpcmov \(%rax\),%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 80 a2 3c 24 f0[ 	]+vpcmov \(%r12\),%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 80 a2 3c 24 00[ 	]+vpcmov \(%r12\),%xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 80 a2 00 c0[ 	]+vpcmov \(%rax\),%xmm12,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a2 c7 f0[ 	]+vpcmov %xmm15,%xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 c0 a2 1a f0[ 	]+vpcmov \(%r10\),%xmm15,%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 a2 dc 20[ 	]+vpcmov %xmm2,%xmm12,%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a2 c4 20[ 	]+vpcmov %xmm2,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 80 a2 1c 24 f0[ 	]+vpcmov \(%r12\),%xmm15,%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 44 a2 c4 00[ 	]+vpcmov %ymm0,%ymm12,%ymm7,%ymm0
[ 	]*[a-f0-9]+:	8f e8 fc a2 00 f0[ 	]+vpcmov \(%rax\),%ymm15,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f c8 84 a2 02 f0[ 	]+vpcmov \(%r10\),%ymm15,%ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f 68 7c a2 d8 20[ 	]+vpcmov %ymm2,%ymm0,%ymm0,%ymm11
[ 	]*[a-f0-9]+:	8f e8 7c a2 c0 20[ 	]+vpcmov %ymm2,%ymm0,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f 68 7c a2 f8 20[ 	]+vpcmov %ymm2,%ymm0,%ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 48 fc a2 3a c0[ 	]+vpcmov \(%r10\),%ymm12,%ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 68 fc a2 38 00[ 	]+vpcmov \(%rax\),%ymm0,%ymm0,%ymm15
[ 	]*[a-f0-9]+:	8f 48 84 a2 3c 24 f0[ 	]+vpcmov \(%r12\),%ymm15,%ymm15,%ymm15
[ 	]*[a-f0-9]+:	8f 48 84 a2 3c 24 00[ 	]+vpcmov \(%r12\),%ymm0,%ymm15,%ymm15
[ 	]*[a-f0-9]+:	8f e8 84 a2 00 c0[ 	]+vpcmov \(%rax\),%ymm12,%ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f c8 04 a2 c7 f0[ 	]+vpcmov %ymm15,%ymm15,%ymm15,%ymm0
[ 	]*[a-f0-9]+:	8f 48 c4 a2 1a f0[ 	]+vpcmov \(%r10\),%ymm15,%ymm7,%ymm11
[ 	]*[a-f0-9]+:	8f 48 44 a2 dc 20[ 	]+vpcmov %ymm2,%ymm12,%ymm7,%ymm11
[ 	]*[a-f0-9]+:	8f c8 7c a2 c4 20[ 	]+vpcmov %ymm2,%ymm12,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	8f 48 84 a2 1c 24 f0[ 	]+vpcmov \(%r12\),%ymm15,%ymm15,%ymm11
[ 	]*[a-f0-9]+:	8f 68 78 cc 3f 03[ 	]+vpcomgeb \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cc c8 ff[ 	]+vpcomb \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 cc cf ff[ 	]+vpcomb \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 cc cb 00[ 	]+vpcomltb %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 cc cb 00[ 	]+vpcomltb %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 cc c8 00[ 	]+vpcomltb %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 cc fb 03[ 	]+vpcomgeb %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cc fb 00[ 	]+vpcomltb %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cc ff ff[ 	]+vpcomb \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 cc 39 00[ 	]+vpcomltb \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cc 01 03[ 	]+vpcomgeb \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cc 0f ff[ 	]+vpcomb \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 cc 0f 03[ 	]+vpcomgeb \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 cc f8 03[ 	]+vpcomgeb %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 cc 01 ff[ 	]+vpcomb \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cc 0e 03[ 	]+vpcomgeb \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ce 3f 03[ 	]+vpcomged \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ce c8 ff[ 	]+vpcomd \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ce cf ff[ 	]+vpcomd \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 ce cb 00[ 	]+vpcomltd %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ce cb 00[ 	]+vpcomltd %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 ce c8 00[ 	]+vpcomltd %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 ce fb 03[ 	]+vpcomged %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ce fb 00[ 	]+vpcomltd %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ce ff ff[ 	]+vpcomd \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ce 39 00[ 	]+vpcomltd \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ce 01 03[ 	]+vpcomged \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ce 0f ff[ 	]+vpcomd \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 ce 0f 03[ 	]+vpcomged \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ce f8 03[ 	]+vpcomged %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ce 01 ff[ 	]+vpcomd \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ce 0e 03[ 	]+vpcomged \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 cf 3f 03[ 	]+vpcomgeq \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cf c8 ff[ 	]+vpcomq \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 cf cf ff[ 	]+vpcomq \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 cf cb 00[ 	]+vpcomltq %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 cf cb 00[ 	]+vpcomltq %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 cf c8 00[ 	]+vpcomltq %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 cf fb 03[ 	]+vpcomgeq %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cf fb 00[ 	]+vpcomltq %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cf ff ff[ 	]+vpcomq \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 cf 39 00[ 	]+vpcomltq \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cf 01 03[ 	]+vpcomgeq \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cf 0f ff[ 	]+vpcomq \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 cf 0f 03[ 	]+vpcomgeq \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 cf f8 03[ 	]+vpcomgeq %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 cf 01 ff[ 	]+vpcomq \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cf 0e 03[ 	]+vpcomgeq \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ec 3f 03[ 	]+vpcomgeub \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ec c8 ff[ 	]+vpcomub \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ec cf ff[ 	]+vpcomub \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 ec cb 00[ 	]+vpcomltub %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ec cb 00[ 	]+vpcomltub %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 ec c8 00[ 	]+vpcomltub %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 ec fb 03[ 	]+vpcomgeub %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ec fb 00[ 	]+vpcomltub %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ec ff ff[ 	]+vpcomub \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ec 39 00[ 	]+vpcomltub \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ec 01 03[ 	]+vpcomgeub \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ec 0f ff[ 	]+vpcomub \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 ec 0f 03[ 	]+vpcomgeub \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ec f8 03[ 	]+vpcomgeub %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ec 01 ff[ 	]+vpcomub \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ec 0e 03[ 	]+vpcomgeub \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ee 3f 03[ 	]+vpcomgeud \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ee c8 ff[ 	]+vpcomud \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ee cf ff[ 	]+vpcomud \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 ee cb 00[ 	]+vpcomltud %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ee cb 00[ 	]+vpcomltud %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 ee c8 00[ 	]+vpcomltud %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 ee fb 03[ 	]+vpcomgeud %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ee fb 00[ 	]+vpcomltud %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ee ff ff[ 	]+vpcomud \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ee 39 00[ 	]+vpcomltud \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ee 01 03[ 	]+vpcomgeud \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ee 0f ff[ 	]+vpcomud \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 ee 0f 03[ 	]+vpcomgeud \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ee f8 03[ 	]+vpcomgeud %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ee 01 ff[ 	]+vpcomud \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ee 0e 03[ 	]+vpcomgeud \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ef 3f 03[ 	]+vpcomgeuq \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef c8 ff[ 	]+vpcomuq \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ef cf ff[ 	]+vpcomuq \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 ef cb 00[ 	]+vpcomltuq %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ef cb 00[ 	]+vpcomltuq %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 ef c8 00[ 	]+vpcomltuq %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 ef fb 03[ 	]+vpcomgeuq %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ef fb 00[ 	]+vpcomltuq %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ef ff ff[ 	]+vpcomuq \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ef 39 00[ 	]+vpcomltuq \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef 01 03[ 	]+vpcomgeuq \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ef 0f ff[ 	]+vpcomuq \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 ef 0f 03[ 	]+vpcomgeuq \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ef f8 03[ 	]+vpcomgeuq %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ef 01 ff[ 	]+vpcomuq \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ef 0e 03[ 	]+vpcomgeuq \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ed 3f 03[ 	]+vpcomgeuw \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ed c8 ff[ 	]+vpcomuw \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ed cf ff[ 	]+vpcomuw \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 ed cb 00[ 	]+vpcomltuw %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 ed cb 00[ 	]+vpcomltuw %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 ed c8 00[ 	]+vpcomltuw %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 ed fb 03[ 	]+vpcomgeuw %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ed fb 00[ 	]+vpcomltuw %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ed ff ff[ 	]+vpcomuw \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ed 39 00[ 	]+vpcomltuw \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ed 01 03[ 	]+vpcomgeuw \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ed 0f ff[ 	]+vpcomuw \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 ed 0f 03[ 	]+vpcomgeuw \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 ed f8 03[ 	]+vpcomgeuw %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ed 01 ff[ 	]+vpcomuw \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ed 0e 03[ 	]+vpcomgeuw \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 cd 3f 03[ 	]+vpcomgew \(%rdi\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cd c8 ff[ 	]+vpcomw \$0xff,%xmm0,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 cd cf ff[ 	]+vpcomw \$0xff,%xmm15,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f c8 20 cd cb 00[ 	]+vpcomltw %xmm11,%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f c8 78 cd cb 00[ 	]+vpcomltw %xmm11,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	8f e8 00 cd c8 00[ 	]+vpcomltw %xmm0,%xmm15,%xmm1
[ 	]*[a-f0-9]+:	8f 48 00 cd fb 03[ 	]+vpcomgew %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cd fb 00[ 	]+vpcomltw %xmm11,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cd ff ff[ 	]+vpcomw \$0xff,%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 cd 39 00[ 	]+vpcomltw \(%rcx\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cd 01 03[ 	]+vpcomgew \(%rcx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cd 0f ff[ 	]+vpcomw \$0xff,\(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f e8 20 cd 0f 03[ 	]+vpcomgew \(%rdi\),%xmm11,%xmm1
[ 	]*[a-f0-9]+:	8f 68 78 cd f8 03[ 	]+vpcomgew %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 cd 01 ff[ 	]+vpcomw \$0xff,\(%rcx\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cd 0e 03[ 	]+vpcomgew \(%rsi\),%xmm15,%xmm1
[ 	]*[a-f0-9]+:	c4 43 21 49 d5 e3[ 	]+vpermil2pd \$0x3,%xmm14,%xmm13,%xmm11,%xmm10
[ 	]*[a-f0-9]+:	c4 a3 71 49 04 07 f2[ 	]+vpermil2pd \$0x2,%xmm15,\(%rdi,%r8,1\),%xmm1,%xmm0
[ 	]*[a-f0-9]+:	c4 83 79 49 54 e4 23 01[ 	]+vpermil2pd \$0x1,%xmm0,0x23\(%r12,%r12,8\),%xmm0,%xmm2
[ 	]*[a-f0-9]+:	c4 c3 11 49 d7 30[ 	]+vpermil2pd \$0x0,%xmm3,%xmm15,%xmm13,%xmm2
[ 	]*[a-f0-9]+:	c4 c3 21 49 c6 32[ 	]+vpermil2pd \$0x2,%xmm3,%xmm14,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 71 49 02 01[ 	]+vpermil2pd \$0x1,%xmm0,\(%rdx\),%xmm1,%xmm0
[ 	]*[a-f0-9]+:	c4 63 79 49 c8 33[ 	]+vpermil2pd \$0x3,%xmm3,%xmm0,%xmm0,%xmm9
[ 	]*[a-f0-9]+:	c4 83 79 49 5c e4 23 20[ 	]+vpermil2pd \$0x0,%xmm2,0x23\(%r12,%r12,8\),%xmm0,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 21 49 c7 00[ 	]+vpermil2pd \$0x0,%xmm0,%xmm7,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 01 49 c5 41[ 	]+vpermil2pd \$0x1,%xmm4,%xmm5,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	c4 43 f9 49 54 9c 04 83[ 	]+vpermil2pd \$0x3,0x4\(%r12,%rbx,4\),%xmm8,%xmm0,%xmm10
[ 	]*[a-f0-9]+:	c4 e3 41 49 f0 12[ 	]+vpermil2pd \$0x2,%xmm1,%xmm0,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	c4 43 c9 49 54 1d 00 c1[ 	]+vpermil2pd \$0x1,0x0\(%r13,%rbx,1\),%xmm12,%xmm6,%xmm10
[ 	]*[a-f0-9]+:	c4 63 79 49 ce 42[ 	]+vpermil2pd \$0x2,%xmm4,%xmm6,%xmm0,%xmm9
[ 	]*[a-f0-9]+:	c4 63 c9 49 1c db 80[ 	]+vpermil2pd \$0x0,\(%rbx,%rbx,8\),%xmm8,%xmm6,%xmm11
[ 	]*[a-f0-9]+:	c4 c3 49 49 c5 53[ 	]+vpermil2pd \$0x3,%xmm5,%xmm13,%xmm6,%xmm0
[ 	]*[a-f0-9]+:	c4 63 7d 49 ed 71[ 	]+vpermil2pd \$0x1,%ymm7,%ymm5,%ymm0,%ymm13
[ 	]*[a-f0-9]+:	c4 23 5d 49 24 49 70[ 	]+vpermil2pd \$0x0,%ymm7,\(%rcx,%r9,2\),%ymm4,%ymm12
[ 	]*[a-f0-9]+:	c4 03 7d 49 04 1e 33[ 	]+vpermil2pd \$0x3,%ymm3,\(%r14,%r11,1\),%ymm0,%ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 49 8c 81 07 01 00 00 72[ 	]+vpermil2pd \$0x2,%ymm7,0x107\(%r9,%rax,4\),%ymm0,%ymm9
[ 	]*[a-f0-9]+:	c4 03 7d 49 04 1e 72[ 	]+vpermil2pd \$0x2,%ymm7,\(%r14,%r11,1\),%ymm0,%ymm8
[ 	]*[a-f0-9]+:	c4 a3 5d 49 04 49 03[ 	]+vpermil2pd \$0x3,%ymm0,\(%rcx,%r9,2\),%ymm4,%ymm0
[ 	]*[a-f0-9]+:	c4 83 25 49 2c 1e 81[ 	]+vpermil2pd \$0x1,%ymm8,\(%r14,%r11,1\),%ymm11,%ymm5
[ 	]*[a-f0-9]+:	c4 63 7d 49 2e 20[ 	]+vpermil2pd \$0x0,%ymm2,\(%rsi\),%ymm0,%ymm13
[ 	]*[a-f0-9]+:	c4 63 ad 49 3c 31 01[ 	]+vpermil2pd \$0x1,\(%rcx,%rsi,1\),%ymm0,%ymm10,%ymm15
[ 	]*[a-f0-9]+:	c4 c3 fd 49 01 c2[ 	]+vpermil2pd \$0x2,\(%r9\),%ymm12,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	c4 a3 a5 49 a4 31 d9 d8 15 00 80[ 	]+vpermil2pd \$0x0,0x15d8d9\(%rcx,%r14,1\),%ymm8,%ymm11,%ymm4
[ 	]*[a-f0-9]+:	c4 c3 7d 49 c4 93[ 	]+vpermil2pd \$0x3,%ymm9,%ymm12,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	c4 03 8d 49 44 1d 00 13[ 	]+vpermil2pd \$0x3,0x0\(%r13,%r11,1\),%ymm1,%ymm14,%ymm8
[ 	]*[a-f0-9]+:	c4 23 fd 49 9c 31 d9 d8 15 00 00[ 	]+vpermil2pd \$0x0,0x15d8d9\(%rcx,%r14,1\),%ymm0,%ymm0,%ymm11
[ 	]*[a-f0-9]+:	c4 83 85 49 44 1d 00 11[ 	]+vpermil2pd \$0x1,0x0\(%r13,%r11,1\),%ymm1,%ymm15,%ymm0
[ 	]*[a-f0-9]+:	c4 c3 25 49 ed 92[ 	]+vpermil2pd \$0x2,%ymm9,%ymm13,%ymm11,%ymm5
[ 	]*[a-f0-9]+:	c4 03 79 48 7c e4 23 02[ 	]+vpermil2ps \$0x2,%xmm0,0x23\(%r12,%r12,8\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	c4 03 61 48 4c e4 23 00[ 	]+vpermil2ps \$0x0,%xmm0,0x23\(%r12,%r12,8\),%xmm3,%xmm9
[ 	]*[a-f0-9]+:	c4 43 41 48 dc 03[ 	]+vpermil2ps \$0x3,%xmm0,%xmm12,%xmm7,%xmm11
[ 	]*[a-f0-9]+:	c4 e3 79 48 02 31[ 	]+vpermil2ps \$0x1,%xmm3,\(%rdx\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 83 79 48 04 0e 32[ 	]+vpermil2ps \$0x2,%xmm3,\(%r14,%r9,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 71 48 02 31[ 	]+vpermil2ps \$0x1,%xmm3,\(%rdx\),%xmm1,%xmm0
[ 	]*[a-f0-9]+:	c4 43 79 48 e0 30[ 	]+vpermil2ps \$0x0,%xmm3,%xmm8,%xmm0,%xmm12
[ 	]*[a-f0-9]+:	c4 83 71 48 14 0e 33[ 	]+vpermil2ps \$0x3,%xmm3,\(%r14,%r9,1\),%xmm1,%xmm2
[ 	]*[a-f0-9]+:	c4 43 f9 48 5c 05 00 01[ 	]+vpermil2ps \$0x1,0x0\(%r13,%rax,1\),%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	c4 63 c9 48 17 f3[ 	]+vpermil2ps \$0x3,\(%rdi\),%xmm15,%xmm6,%xmm10
[ 	]*[a-f0-9]+:	c4 c3 79 48 c5 02[ 	]+vpermil2ps \$0x2,%xmm0,%xmm13,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 63 09 48 d0 40[ 	]+vpermil2ps \$0x0,%xmm4,%xmm0,%xmm14,%xmm10
[ 	]*[a-f0-9]+:	c4 63 61 48 d8 01[ 	]+vpermil2ps \$0x1,%xmm0,%xmm0,%xmm3,%xmm11
[ 	]*[a-f0-9]+:	c4 63 c9 48 14 db b2[ 	]+vpermil2ps \$0x2,\(%rbx,%rbx,8\),%xmm11,%xmm6,%xmm10
[ 	]*[a-f0-9]+:	c4 63 49 48 fd 43[ 	]+vpermil2ps \$0x3,%xmm4,%xmm5,%xmm6,%xmm15
[ 	]*[a-f0-9]+:	c4 43 f9 48 54 9c 04 00[ 	]+vpermil2ps \$0x0,0x4\(%r12,%rbx,4\),%xmm0,%xmm0,%xmm10
[ 	]*[a-f0-9]+:	c4 e3 7d 48 06 01[ 	]+vpermil2ps \$0x1,%ymm0,\(%rsi\),%ymm0,%ymm0
[ 	]*[a-f0-9]+:	c4 c3 25 48 84 81 07 01 00 00 f2[ 	]+vpermil2ps \$0x2,%ymm15,0x107\(%r9,%rax,4\),%ymm11,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 5d 48 c0 70[ 	]+vpermil2ps \$0x0,%ymm7,%ymm0,%ymm4,%ymm0
[ 	]*[a-f0-9]+:	c4 23 5d 48 3c 49 73[ 	]+vpermil2ps \$0x3,%ymm7,\(%rcx,%r9,2\),%ymm4,%ymm15
[ 	]*[a-f0-9]+:	c4 63 5d 48 f8 02[ 	]+vpermil2ps \$0x2,%ymm0,%ymm0,%ymm4,%ymm15
[ 	]*[a-f0-9]+:	c4 43 05 48 ac 81 07 01 00 00 03[ 	]+vpermil2ps \$0x3,%ymm0,0x107\(%r9,%rax,4\),%ymm15,%ymm13
[ 	]*[a-f0-9]+:	c4 43 0d 48 e8 70[ 	]+vpermil2ps \$0x0,%ymm7,%ymm8,%ymm14,%ymm13
[ 	]*[a-f0-9]+:	c4 43 5d 48 ef 71[ 	]+vpermil2ps \$0x1,%ymm7,%ymm15,%ymm4,%ymm13
[ 	]*[a-f0-9]+:	c4 23 fd 48 bc 31 d9 d8 15 00 00[ 	]+vpermil2ps \$0x0,0x15d8d9\(%rcx,%r14,1\),%ymm0,%ymm0,%ymm15
[ 	]*[a-f0-9]+:	c4 83 85 48 64 1d 00 c3[ 	]+vpermil2ps \$0x3,0x0\(%r13,%r11,1\),%ymm12,%ymm15,%ymm4
[ 	]*[a-f0-9]+:	c4 a3 fd 48 84 31 d9 d8 15 00 02[ 	]+vpermil2ps \$0x2,0x15d8d9\(%rcx,%r14,1\),%ymm0,%ymm0,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 65 48 e2 01[ 	]+vpermil2ps \$0x1,%ymm0,%ymm2,%ymm3,%ymm4
[ 	]*[a-f0-9]+:	c4 e3 fd 48 24 31 c3[ 	]+vpermil2ps \$0x3,\(%rcx,%rsi,1\),%ymm12,%ymm0,%ymm4
[ 	]*[a-f0-9]+:	c4 e3 fd 48 24 31 12[ 	]+vpermil2ps \$0x2,\(%rcx,%rsi,1\),%ymm1,%ymm0,%ymm4
[ 	]*[a-f0-9]+:	c4 83 fd 48 64 1d 00 00[ 	]+vpermil2ps \$0x0,0x0\(%r13,%r11,1\),%ymm0,%ymm0,%ymm4
[ 	]*[a-f0-9]+:	c4 e3 5d 48 c7 81[ 	]+vpermil2ps \$0x1,%ymm8,%ymm7,%ymm4,%ymm0
[ 	]*[a-f0-9]+:	8f 69 78 c2 fa[ 	]+vphaddbd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c2 e0[ 	]+vphaddbd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c2 04 24[ 	]+vphaddbd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c2 38[ 	]+vphaddbd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c2 c0[ 	]+vphaddbd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c2 3a[ 	]+vphaddbd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c2 c2[ 	]+vphaddbd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c2 e7[ 	]+vphaddbd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c2 c7[ 	]+vphaddbd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c2 f8[ 	]+vphaddbd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 c2 22[ 	]+vphaddbd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 c2 ff[ 	]+vphaddbd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c2 00[ 	]+vphaddbd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c2 3c 24[ 	]+vphaddbd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c2 20[ 	]+vphaddbd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c2 02[ 	]+vphaddbd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c3 fa[ 	]+vphaddbq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c3 e0[ 	]+vphaddbq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c3 04 24[ 	]+vphaddbq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c3 38[ 	]+vphaddbq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c3 c0[ 	]+vphaddbq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c3 3a[ 	]+vphaddbq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c3 c2[ 	]+vphaddbq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c3 e7[ 	]+vphaddbq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c3 c7[ 	]+vphaddbq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c3 f8[ 	]+vphaddbq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 c3 22[ 	]+vphaddbq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 c3 ff[ 	]+vphaddbq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c3 00[ 	]+vphaddbq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c3 3c 24[ 	]+vphaddbq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c3 20[ 	]+vphaddbq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c3 02[ 	]+vphaddbq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c1 fa[ 	]+vphaddbw %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c1 e0[ 	]+vphaddbw %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c1 04 24[ 	]+vphaddbw \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c1 38[ 	]+vphaddbw \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c1 c0[ 	]+vphaddbw %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c1 3a[ 	]+vphaddbw \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c1 c2[ 	]+vphaddbw %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c1 e7[ 	]+vphaddbw %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c1 c7[ 	]+vphaddbw %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c1 f8[ 	]+vphaddbw %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 c1 22[ 	]+vphaddbw \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 c1 ff[ 	]+vphaddbw %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c1 00[ 	]+vphaddbw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c1 3c 24[ 	]+vphaddbw \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c1 20[ 	]+vphaddbw \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c1 02[ 	]+vphaddbw \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 cb fa[ 	]+vphadddq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 cb e0[ 	]+vphadddq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 cb 04 24[ 	]+vphadddq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 cb 38[ 	]+vphadddq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 cb c0[ 	]+vphadddq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 cb 3a[ 	]+vphadddq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 cb c2[ 	]+vphadddq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 cb e7[ 	]+vphadddq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 cb c7[ 	]+vphadddq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 cb f8[ 	]+vphadddq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 cb 22[ 	]+vphadddq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 cb ff[ 	]+vphadddq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 cb 00[ 	]+vphadddq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 cb 3c 24[ 	]+vphadddq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 cb 20[ 	]+vphadddq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 cb 02[ 	]+vphadddq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d2 fa[ 	]+vphaddubd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d2 e0[ 	]+vphaddubd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d2 04 24[ 	]+vphaddubd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d2 38[ 	]+vphaddubd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d2 c0[ 	]+vphaddubd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d2 3a[ 	]+vphaddubd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d2 c2[ 	]+vphaddubd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d2 e7[ 	]+vphaddubd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d2 c7[ 	]+vphaddubd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d2 f8[ 	]+vphaddubd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 d2 22[ 	]+vphaddubd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 d2 ff[ 	]+vphaddubd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d2 00[ 	]+vphaddubd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d2 3c 24[ 	]+vphaddubd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d2 20[ 	]+vphaddubd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d2 02[ 	]+vphaddubd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d3 fa[ 	]+vphaddubq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d3 e0[ 	]+vphaddubq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d3 04 24[ 	]+vphaddubq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d3 38[ 	]+vphaddubq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d3 c0[ 	]+vphaddubq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d3 3a[ 	]+vphaddubq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d3 c2[ 	]+vphaddubq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d3 e7[ 	]+vphaddubq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d3 c7[ 	]+vphaddubq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d3 f8[ 	]+vphaddubq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 d3 22[ 	]+vphaddubq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 d3 ff[ 	]+vphaddubq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d3 00[ 	]+vphaddubq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d3 3c 24[ 	]+vphaddubq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d3 20[ 	]+vphaddubq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d3 02[ 	]+vphaddubq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d1 fa[ 	]+vphaddubw %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d1 e0[ 	]+vphaddubw %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d1 04 24[ 	]+vphaddubw \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d1 38[ 	]+vphaddubw \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d1 c0[ 	]+vphaddubw %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d1 3a[ 	]+vphaddubw \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d1 c2[ 	]+vphaddubw %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d1 e7[ 	]+vphaddubw %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d1 c7[ 	]+vphaddubw %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d1 f8[ 	]+vphaddubw %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 d1 22[ 	]+vphaddubw \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 d1 ff[ 	]+vphaddubw %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d1 00[ 	]+vphaddubw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d1 3c 24[ 	]+vphaddubw \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d1 20[ 	]+vphaddubw \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d1 02[ 	]+vphaddubw \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 db fa[ 	]+vphaddudq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 db e0[ 	]+vphaddudq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 db 04 24[ 	]+vphaddudq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 db 38[ 	]+vphaddudq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 db c0[ 	]+vphaddudq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 db 3a[ 	]+vphaddudq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 db c2[ 	]+vphaddudq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 db e7[ 	]+vphaddudq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 db c7[ 	]+vphaddudq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 db f8[ 	]+vphaddudq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 db 22[ 	]+vphaddudq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 db ff[ 	]+vphaddudq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 db 00[ 	]+vphaddudq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 db 3c 24[ 	]+vphaddudq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 db 20[ 	]+vphaddudq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 db 02[ 	]+vphaddudq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d6 fa[ 	]+vphadduwd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d6 e0[ 	]+vphadduwd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d6 04 24[ 	]+vphadduwd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d6 38[ 	]+vphadduwd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d6 c0[ 	]+vphadduwd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d6 3a[ 	]+vphadduwd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d6 c2[ 	]+vphadduwd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d6 e7[ 	]+vphadduwd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d6 c7[ 	]+vphadduwd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d6 f8[ 	]+vphadduwd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 d6 22[ 	]+vphadduwd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 d6 ff[ 	]+vphadduwd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d6 00[ 	]+vphadduwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d6 3c 24[ 	]+vphadduwd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d6 20[ 	]+vphadduwd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d6 02[ 	]+vphadduwd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d7 fa[ 	]+vphadduwq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d7 e0[ 	]+vphadduwq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d7 04 24[ 	]+vphadduwq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d7 38[ 	]+vphadduwq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d7 c0[ 	]+vphadduwq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d7 3a[ 	]+vphadduwq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d7 c2[ 	]+vphadduwq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d7 e7[ 	]+vphadduwq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d7 c7[ 	]+vphadduwq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 d7 f8[ 	]+vphadduwq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 d7 22[ 	]+vphadduwq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 d7 ff[ 	]+vphadduwq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 d7 00[ 	]+vphadduwq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 d7 3c 24[ 	]+vphadduwq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 d7 20[ 	]+vphadduwq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 d7 02[ 	]+vphadduwq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c6 fa[ 	]+vphaddwd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c6 e0[ 	]+vphaddwd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c6 04 24[ 	]+vphaddwd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c6 38[ 	]+vphaddwd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c6 c0[ 	]+vphaddwd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c6 3a[ 	]+vphaddwd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c6 c2[ 	]+vphaddwd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c6 e7[ 	]+vphaddwd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c6 c7[ 	]+vphaddwd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c6 f8[ 	]+vphaddwd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 c6 22[ 	]+vphaddwd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 c6 ff[ 	]+vphaddwd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c6 00[ 	]+vphaddwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c6 3c 24[ 	]+vphaddwd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c6 20[ 	]+vphaddwd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c6 02[ 	]+vphaddwd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c7 fa[ 	]+vphaddwq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c7 e0[ 	]+vphaddwq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c7 04 24[ 	]+vphaddwq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c7 38[ 	]+vphaddwq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c7 c0[ 	]+vphaddwq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c7 3a[ 	]+vphaddwq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c7 c2[ 	]+vphaddwq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c7 e7[ 	]+vphaddwq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c7 c7[ 	]+vphaddwq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 c7 f8[ 	]+vphaddwq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 c7 22[ 	]+vphaddwq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 c7 ff[ 	]+vphaddwq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 c7 00[ 	]+vphaddwq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 c7 3c 24[ 	]+vphaddwq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 c7 20[ 	]+vphaddwq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 c7 02[ 	]+vphaddwq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e1 fa[ 	]+vphsubbw %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 e1 e0[ 	]+vphsubbw %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e1 04 24[ 	]+vphsubbw \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e1 38[ 	]+vphsubbw \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e1 c0[ 	]+vphsubbw %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e1 3a[ 	]+vphsubbw \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e1 c2[ 	]+vphsubbw %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e1 e7[ 	]+vphsubbw %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e1 c7[ 	]+vphsubbw %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e1 f8[ 	]+vphsubbw %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 e1 22[ 	]+vphsubbw \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 e1 ff[ 	]+vphsubbw %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e1 00[ 	]+vphsubbw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e1 3c 24[ 	]+vphsubbw \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 e1 20[ 	]+vphsubbw \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e1 02[ 	]+vphsubbw \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e3 fa[ 	]+vphsubdq %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 e3 e0[ 	]+vphsubdq %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e3 04 24[ 	]+vphsubdq \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e3 38[ 	]+vphsubdq \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e3 c0[ 	]+vphsubdq %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e3 3a[ 	]+vphsubdq \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e3 c2[ 	]+vphsubdq %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e3 e7[ 	]+vphsubdq %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e3 c7[ 	]+vphsubdq %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e3 f8[ 	]+vphsubdq %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 e3 22[ 	]+vphsubdq \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 e3 ff[ 	]+vphsubdq %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e3 00[ 	]+vphsubdq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e3 3c 24[ 	]+vphsubdq \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 e3 20[ 	]+vphsubdq \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e3 02[ 	]+vphsubdq \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e2 fa[ 	]+vphsubwd %xmm2,%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 e2 e0[ 	]+vphsubwd %xmm0,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e2 04 24[ 	]+vphsubwd \(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e2 38[ 	]+vphsubwd \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e2 c0[ 	]+vphsubwd %xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e2 3a[ 	]+vphsubwd \(%r10\),%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e2 c2[ 	]+vphsubwd %xmm2,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e2 e7[ 	]+vphsubwd %xmm15,%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e2 c7[ 	]+vphsubwd %xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 69 78 e2 f8[ 	]+vphsubwd %xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 49 78 e2 22[ 	]+vphsubwd \(%r10\),%xmm12
[ 	]*[a-f0-9]+:	8f 49 78 e2 ff[ 	]+vphsubwd %xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e9 78 e2 00[ 	]+vphsubwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 e2 3c 24[ 	]+vphsubwd \(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f 69 78 e2 20[ 	]+vphsubwd \(%rax\),%xmm12
[ 	]*[a-f0-9]+:	8f c9 78 e2 02[ 	]+vphsubwd \(%r10\),%xmm0
[ 	]*[a-f0-9]+:	8f c8 40 9e c7 00[ 	]+vpmacsdd %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 9e 01 20[ 	]+vpmacsdd %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 9e 01 f0[ 	]+vpmacsdd %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 9e d8 f0[ 	]+vpmacsdd %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 9e c4 f0[ 	]+vpmacsdd %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 9e fc f0[ 	]+vpmacsdd %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 9e 3c 24 f0[ 	]+vpmacsdd %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 9e f8 20[ 	]+vpmacsdd %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 9e 39 20[ 	]+vpmacsdd %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 9e fc 20[ 	]+vpmacsdd %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 9e 04 24 20[ 	]+vpmacsdd %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 9e 45 00 00[ 	]+vpmacsdd %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 9e 5d 00 f0[ 	]+vpmacsdd %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 9e 1c 24 f0[ 	]+vpmacsdd %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 9e c7 f0[ 	]+vpmacsdd %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 9e 19 20[ 	]+vpmacsdd %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 9f c7 00[ 	]+vpmacsdqh %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 9f 01 20[ 	]+vpmacsdqh %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 9f 01 f0[ 	]+vpmacsdqh %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 9f d8 f0[ 	]+vpmacsdqh %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 9f c4 f0[ 	]+vpmacsdqh %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 9f fc f0[ 	]+vpmacsdqh %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 9f 3c 24 f0[ 	]+vpmacsdqh %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 9f f8 20[ 	]+vpmacsdqh %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 9f 39 20[ 	]+vpmacsdqh %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 9f fc 20[ 	]+vpmacsdqh %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 9f 04 24 20[ 	]+vpmacsdqh %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 9f 45 00 00[ 	]+vpmacsdqh %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 9f 5d 00 f0[ 	]+vpmacsdqh %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 9f 1c 24 f0[ 	]+vpmacsdqh %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 9f c7 f0[ 	]+vpmacsdqh %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 9f 19 20[ 	]+vpmacsdqh %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 97 c7 00[ 	]+vpmacsdql %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 97 01 20[ 	]+vpmacsdql %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 97 01 f0[ 	]+vpmacsdql %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 97 d8 f0[ 	]+vpmacsdql %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 97 c4 f0[ 	]+vpmacsdql %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 97 fc f0[ 	]+vpmacsdql %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 97 3c 24 f0[ 	]+vpmacsdql %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 97 f8 20[ 	]+vpmacsdql %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 97 39 20[ 	]+vpmacsdql %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 97 fc 20[ 	]+vpmacsdql %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 97 04 24 20[ 	]+vpmacsdql %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 97 45 00 00[ 	]+vpmacsdql %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 97 5d 00 f0[ 	]+vpmacsdql %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 97 1c 24 f0[ 	]+vpmacsdql %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 97 c7 f0[ 	]+vpmacsdql %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 97 19 20[ 	]+vpmacsdql %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 8e c7 00[ 	]+vpmacssdd %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 8e 01 20[ 	]+vpmacssdd %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 8e 01 f0[ 	]+vpmacssdd %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 8e d8 f0[ 	]+vpmacssdd %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 8e c4 f0[ 	]+vpmacssdd %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 8e fc f0[ 	]+vpmacssdd %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 8e 3c 24 f0[ 	]+vpmacssdd %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 8e f8 20[ 	]+vpmacssdd %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 8e 39 20[ 	]+vpmacssdd %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 8e fc 20[ 	]+vpmacssdd %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 8e 04 24 20[ 	]+vpmacssdd %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 8e 45 00 00[ 	]+vpmacssdd %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 8e 5d 00 f0[ 	]+vpmacssdd %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 8e 1c 24 f0[ 	]+vpmacssdd %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 8e c7 f0[ 	]+vpmacssdd %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 8e 19 20[ 	]+vpmacssdd %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 8f c7 00[ 	]+vpmacssdqh %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 8f 01 20[ 	]+vpmacssdqh %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 8f 01 f0[ 	]+vpmacssdqh %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 8f d8 f0[ 	]+vpmacssdqh %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 8f c4 f0[ 	]+vpmacssdqh %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 8f fc f0[ 	]+vpmacssdqh %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 8f 3c 24 f0[ 	]+vpmacssdqh %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 8f f8 20[ 	]+vpmacssdqh %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 8f 39 20[ 	]+vpmacssdqh %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 8f fc 20[ 	]+vpmacssdqh %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 8f 04 24 20[ 	]+vpmacssdqh %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 8f 45 00 00[ 	]+vpmacssdqh %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 8f 5d 00 f0[ 	]+vpmacssdqh %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 8f 1c 24 f0[ 	]+vpmacssdqh %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 8f c7 f0[ 	]+vpmacssdqh %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 8f 19 20[ 	]+vpmacssdqh %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 87 c7 00[ 	]+vpmacssdql %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 87 01 20[ 	]+vpmacssdql %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 87 01 f0[ 	]+vpmacssdql %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 87 d8 f0[ 	]+vpmacssdql %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 87 c4 f0[ 	]+vpmacssdql %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 87 fc f0[ 	]+vpmacssdql %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 87 3c 24 f0[ 	]+vpmacssdql %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 87 f8 20[ 	]+vpmacssdql %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 87 39 20[ 	]+vpmacssdql %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 87 fc 20[ 	]+vpmacssdql %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 87 04 24 20[ 	]+vpmacssdql %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 87 45 00 00[ 	]+vpmacssdql %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 87 5d 00 f0[ 	]+vpmacssdql %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 87 1c 24 f0[ 	]+vpmacssdql %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 87 c7 f0[ 	]+vpmacssdql %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 87 19 20[ 	]+vpmacssdql %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 86 c7 00[ 	]+vpmacsswd %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 86 01 20[ 	]+vpmacsswd %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 86 01 f0[ 	]+vpmacsswd %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 86 d8 f0[ 	]+vpmacsswd %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 86 c4 f0[ 	]+vpmacsswd %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 86 fc f0[ 	]+vpmacsswd %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 86 3c 24 f0[ 	]+vpmacsswd %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 86 f8 20[ 	]+vpmacsswd %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 86 39 20[ 	]+vpmacsswd %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 86 fc 20[ 	]+vpmacsswd %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 86 04 24 20[ 	]+vpmacsswd %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 86 45 00 00[ 	]+vpmacsswd %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 86 5d 00 f0[ 	]+vpmacsswd %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 86 1c 24 f0[ 	]+vpmacsswd %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 86 c7 f0[ 	]+vpmacsswd %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 86 19 20[ 	]+vpmacsswd %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 85 c7 00[ 	]+vpmacssww %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 85 01 20[ 	]+vpmacssww %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 85 01 f0[ 	]+vpmacssww %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 85 d8 f0[ 	]+vpmacssww %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 85 c4 f0[ 	]+vpmacssww %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 85 fc f0[ 	]+vpmacssww %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 85 3c 24 f0[ 	]+vpmacssww %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 85 f8 20[ 	]+vpmacssww %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 85 39 20[ 	]+vpmacssww %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 85 fc 20[ 	]+vpmacssww %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 85 04 24 20[ 	]+vpmacssww %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 85 45 00 00[ 	]+vpmacssww %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 85 5d 00 f0[ 	]+vpmacssww %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 85 1c 24 f0[ 	]+vpmacssww %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 85 c7 f0[ 	]+vpmacssww %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 85 19 20[ 	]+vpmacssww %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 96 c7 00[ 	]+vpmacswd %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 96 01 20[ 	]+vpmacswd %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 96 01 f0[ 	]+vpmacswd %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 96 d8 f0[ 	]+vpmacswd %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 96 c4 f0[ 	]+vpmacswd %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 96 fc f0[ 	]+vpmacswd %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 96 3c 24 f0[ 	]+vpmacswd %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 96 f8 20[ 	]+vpmacswd %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 96 39 20[ 	]+vpmacswd %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 96 fc 20[ 	]+vpmacswd %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 96 04 24 20[ 	]+vpmacswd %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 96 45 00 00[ 	]+vpmacswd %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 96 5d 00 f0[ 	]+vpmacswd %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 96 1c 24 f0[ 	]+vpmacswd %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 96 c7 f0[ 	]+vpmacswd %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 96 19 20[ 	]+vpmacswd %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 95 c7 00[ 	]+vpmacsww %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 95 01 20[ 	]+vpmacsww %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 95 01 f0[ 	]+vpmacsww %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 95 d8 f0[ 	]+vpmacsww %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 95 c4 f0[ 	]+vpmacsww %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 95 fc f0[ 	]+vpmacsww %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 95 3c 24 f0[ 	]+vpmacsww %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 95 f8 20[ 	]+vpmacsww %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 95 39 20[ 	]+vpmacsww %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 95 fc 20[ 	]+vpmacsww %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 95 04 24 20[ 	]+vpmacsww %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 95 45 00 00[ 	]+vpmacsww %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 95 5d 00 f0[ 	]+vpmacsww %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 95 1c 24 f0[ 	]+vpmacsww %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 95 c7 f0[ 	]+vpmacsww %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 95 19 20[ 	]+vpmacsww %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 a6 c7 00[ 	]+vpmadcsswd %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 a6 01 20[ 	]+vpmadcsswd %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a6 01 f0[ 	]+vpmadcsswd %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a6 d8 f0[ 	]+vpmadcsswd %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a6 c4 f0[ 	]+vpmadcsswd %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 a6 fc f0[ 	]+vpmadcsswd %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 a6 3c 24 f0[ 	]+vpmadcsswd %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 a6 f8 20[ 	]+vpmadcsswd %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 a6 39 20[ 	]+vpmadcsswd %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 a6 fc 20[ 	]+vpmadcsswd %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 a6 04 24 20[ 	]+vpmadcsswd %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a6 45 00 00[ 	]+vpmadcsswd %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 a6 5d 00 f0[ 	]+vpmadcsswd %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 a6 1c 24 f0[ 	]+vpmadcsswd %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a6 c7 f0[ 	]+vpmadcsswd %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 a6 19 20[ 	]+vpmadcsswd %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 b6 c7 00[ 	]+vpmadcswd %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 b6 01 20[ 	]+vpmadcswd %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 b6 01 f0[ 	]+vpmadcswd %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 b6 d8 f0[ 	]+vpmadcswd %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 b6 c4 f0[ 	]+vpmadcswd %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 b6 fc f0[ 	]+vpmadcswd %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 b6 3c 24 f0[ 	]+vpmadcswd %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 b6 f8 20[ 	]+vpmadcswd %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 b6 39 20[ 	]+vpmadcswd %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 b6 fc 20[ 	]+vpmadcswd %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 b6 04 24 20[ 	]+vpmadcswd %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 b6 45 00 00[ 	]+vpmadcswd %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 b6 5d 00 f0[ 	]+vpmadcswd %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 b6 1c 24 f0[ 	]+vpmadcswd %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 b6 c7 f0[ 	]+vpmadcswd %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 b6 19 20[ 	]+vpmadcswd %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 a3 c4 00[ 	]+vpperm %xmm0,%xmm12,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f e8 f8 a3 00 f0[ 	]+vpperm \(%rax\),%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 80 a3 02 f0[ 	]+vpperm \(%r10\),%xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a3 d8 20[ 	]+vpperm %xmm2,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f e8 78 a3 c0 20[ 	]+vpperm %xmm2,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a3 f8 20[ 	]+vpperm %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 f8 a3 3a c0[ 	]+vpperm \(%r10\),%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 f8 a3 38 00[ 	]+vpperm \(%rax\),%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 80 a3 3c 24 f0[ 	]+vpperm \(%r12\),%xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 80 a3 3c 24 00[ 	]+vpperm \(%r12\),%xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 80 a3 00 c0[ 	]+vpperm \(%rax\),%xmm12,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a3 c7 f0[ 	]+vpperm %xmm15,%xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 c0 a3 1a f0[ 	]+vpperm \(%r10\),%xmm15,%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 a3 dc 20[ 	]+vpperm %xmm2,%xmm12,%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a3 c4 20[ 	]+vpperm %xmm2,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 80 a3 1c 24 f0[ 	]+vpperm \(%r12\),%xmm15,%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f c8 40 a3 c7 00[ 	]+vpperm %xmm0,%xmm15,%xmm7,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 a3 01 20[ 	]+vpperm %xmm2,\(%r9\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a3 01 f0[ 	]+vpperm %xmm15,\(%r9\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 a3 d8 f0[ 	]+vpperm %xmm15,%xmm0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a3 c4 f0[ 	]+vpperm %xmm15,%xmm12,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 a3 fc f0[ 	]+vpperm %xmm15,%xmm12,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 a3 3c 24 f0[ 	]+vpperm %xmm15,\(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 a3 f8 20[ 	]+vpperm %xmm2,%xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 a3 39 20[ 	]+vpperm %xmm2,\(%r9\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 a3 fc 20[ 	]+vpperm %xmm2,%xmm12,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 a3 04 24 20[ 	]+vpperm %xmm2,\(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 a3 45 00 00[ 	]+vpperm %xmm0,0x0\(%r13\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 40 a3 5d 00 f0[ 	]+vpperm %xmm15,0x0\(%r13\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f 48 40 a3 1c 24 f0[ 	]+vpperm %xmm15,\(%r12\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 a3 c7 f0[ 	]+vpperm %xmm15,%xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 a3 19 20[ 	]+vpperm %xmm2,\(%r9\),%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f 69 68 90 f8[ 	]+vprotb %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 90 fc[ 	]+vprotb %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 90 c0[ 	]+vprotb %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 90 39[ 	]+vprotb %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 90 c7[ 	]+vprotb %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 90 ff[ 	]+vprotb %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 90 c4[ 	]+vprotb %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 90 c4[ 	]+vprotb %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 90 3c 24[ 	]+vprotb %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 90 39[ 	]+vprotb %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 90 fc[ 	]+vprotb %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 90 39[ 	]+vprotb %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 90 45 00[ 	]+vprotb %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 90 39[ 	]+vprotb %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 90 04 24[ 	]+vprotb %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 90 7d 00[ 	]+vprotb %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 90 3c 24[ 	]+vprotb \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 90 3c 24[ 	]+vprotb \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 90 00[ 	]+vprotb \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 90 3a[ 	]+vprotb \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 90 c4[ 	]+vprotb %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 90 fc[ 	]+vprotb %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 90 c0[ 	]+vprotb %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 90 c0[ 	]+vprotb %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 90 38[ 	]+vprotb \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 90 ff[ 	]+vprotb %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 90 f8[ 	]+vprotb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 90 3c 24[ 	]+vprotb \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 90 04 24[ 	]+vprotb \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 90 3a[ 	]+vprotb \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 90 02[ 	]+vprotb \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 90 ff[ 	]+vprotb %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c0 fb 03[ 	]+vprotb \$0x3,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 c0 c0 ff[ 	]+vprotb \$0xff,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c0 e3 ff[ 	]+vprotb \$0xff,%xmm11,%xmm4
[ 	]*[a-f0-9]+:	8f c8 78 c0 e3 00[ 	]+vprotb \$0x0,%xmm11,%xmm4
[ 	]*[a-f0-9]+:	8f c8 78 c0 e7 00[ 	]+vprotb \$0x0,%xmm15,%xmm4
[ 	]*[a-f0-9]+:	8f 68 78 c0 f8 00[ 	]+vprotb \$0x0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 c0 c3 ff[ 	]+vprotb \$0xff,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 c0 03[ 	]+vprotb \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c0 c3 03[ 	]+vprotb \$0x3,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 e0 00[ 	]+vprotb \$0x0,%xmm0,%xmm4
[ 	]*[a-f0-9]+:	8f c8 78 c0 c7 ff[ 	]+vprotb \$0xff,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 c0 f8 ff[ 	]+vprotb \$0xff,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c0 ff ff[ 	]+vprotb \$0xff,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 c0 e7 03[ 	]+vprotb \$0x3,%xmm15,%xmm4
[ 	]*[a-f0-9]+:	8f 48 78 c0 fb ff[ 	]+vprotb \$0xff,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c0 f8 03[ 	]+vprotb \$0x3,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 92 f8[ 	]+vprotd %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 92 fc[ 	]+vprotd %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 92 c0[ 	]+vprotd %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 92 39[ 	]+vprotd %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 92 c7[ 	]+vprotd %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 92 ff[ 	]+vprotd %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 92 c4[ 	]+vprotd %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 92 c4[ 	]+vprotd %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 92 3c 24[ 	]+vprotd %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 92 39[ 	]+vprotd %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 92 fc[ 	]+vprotd %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 92 39[ 	]+vprotd %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 92 45 00[ 	]+vprotd %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 92 39[ 	]+vprotd %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 92 04 24[ 	]+vprotd %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 92 7d 00[ 	]+vprotd %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 92 3c 24[ 	]+vprotd \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 92 3c 24[ 	]+vprotd \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 92 00[ 	]+vprotd \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 92 3a[ 	]+vprotd \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 92 c4[ 	]+vprotd %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 92 fc[ 	]+vprotd %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 92 c0[ 	]+vprotd %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 92 c0[ 	]+vprotd %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 92 38[ 	]+vprotd \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 92 ff[ 	]+vprotd %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 92 f8[ 	]+vprotd %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 92 3c 24[ 	]+vprotd \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 92 04 24[ 	]+vprotd \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 92 3a[ 	]+vprotd \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 92 02[ 	]+vprotd \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 92 ff[ 	]+vprotd %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c2 ff 00[ 	]+vprotd \$0x0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c2 3e 00[ 	]+vprotd \$0x0,\(%rsi\),%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c2 d8 00[ 	]+vprotd \$0x0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 c2 c7 ff[ 	]+vprotd \$0xff,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 c0 03[ 	]+vprotd \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c2 c7 03[ 	]+vprotd \$0x3,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 c2 db 00[ 	]+vprotd \$0x0,%xmm11,%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c2 f8 00[ 	]+vprotd \$0x0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 c2 01 03[ 	]+vprotd \$0x3,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c2 06 ff[ 	]+vprotd \$0xff,\(%rsi\),%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 c2 3f 00[ 	]+vprotd \$0x0,\(%rdi\),%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c2 ff ff[ 	]+vprotd \$0xff,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c2 db ff[ 	]+vprotd \$0xff,%xmm11,%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c2 1e ff[ 	]+vprotd \$0xff,\(%rsi\),%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c2 3f 03[ 	]+vprotd \$0x3,\(%rdi\),%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c2 df 03[ 	]+vprotd \$0x3,%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f 69 68 93 f8[ 	]+vprotq %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 93 fc[ 	]+vprotq %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 93 c0[ 	]+vprotq %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 93 39[ 	]+vprotq %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 93 c7[ 	]+vprotq %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 93 ff[ 	]+vprotq %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 93 c4[ 	]+vprotq %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 93 c4[ 	]+vprotq %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 93 3c 24[ 	]+vprotq %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 93 39[ 	]+vprotq %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 93 fc[ 	]+vprotq %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 93 39[ 	]+vprotq %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 93 45 00[ 	]+vprotq %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 93 39[ 	]+vprotq %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 93 04 24[ 	]+vprotq %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 93 7d 00[ 	]+vprotq %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 93 3c 24[ 	]+vprotq \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 93 3c 24[ 	]+vprotq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 93 00[ 	]+vprotq \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 93 3a[ 	]+vprotq \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 93 c4[ 	]+vprotq %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 93 fc[ 	]+vprotq %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 93 c0[ 	]+vprotq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 93 c0[ 	]+vprotq %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 93 38[ 	]+vprotq \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 93 ff[ 	]+vprotq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 93 f8[ 	]+vprotq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 93 3c 24[ 	]+vprotq \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 93 04 24[ 	]+vprotq \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 93 3a[ 	]+vprotq \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 93 02[ 	]+vprotq \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 93 ff[ 	]+vprotq %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c3 ff 00[ 	]+vprotq \$0x0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c3 3e 00[ 	]+vprotq \$0x0,\(%rsi\),%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c3 d8 00[ 	]+vprotq \$0x0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 c3 c7 ff[ 	]+vprotq \$0xff,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 c0 03[ 	]+vprotq \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c3 c7 03[ 	]+vprotq \$0x3,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 c3 db 00[ 	]+vprotq \$0x0,%xmm11,%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c3 f8 00[ 	]+vprotq \$0x0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 c3 01 03[ 	]+vprotq \$0x3,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c3 06 ff[ 	]+vprotq \$0xff,\(%rsi\),%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 c3 3f 00[ 	]+vprotq \$0x0,\(%rdi\),%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c3 ff ff[ 	]+vprotq \$0xff,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c3 db ff[ 	]+vprotq \$0xff,%xmm11,%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c3 1e ff[ 	]+vprotq \$0xff,\(%rsi\),%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c3 3f 03[ 	]+vprotq \$0x3,\(%rdi\),%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c3 df 03[ 	]+vprotq \$0x3,%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f 69 68 91 f8[ 	]+vprotw %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 91 fc[ 	]+vprotw %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 91 c0[ 	]+vprotw %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 91 39[ 	]+vprotw %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 91 c7[ 	]+vprotw %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 91 ff[ 	]+vprotw %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 91 c4[ 	]+vprotw %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 91 c4[ 	]+vprotw %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 91 3c 24[ 	]+vprotw %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 91 39[ 	]+vprotw %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 91 fc[ 	]+vprotw %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 91 39[ 	]+vprotw %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 91 45 00[ 	]+vprotw %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 91 39[ 	]+vprotw %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 91 04 24[ 	]+vprotw %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 91 7d 00[ 	]+vprotw %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 91 3c 24[ 	]+vprotw \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 91 3c 24[ 	]+vprotw \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 91 00[ 	]+vprotw \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 91 3a[ 	]+vprotw \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 91 c4[ 	]+vprotw %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 91 fc[ 	]+vprotw %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 91 c0[ 	]+vprotw %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 91 c0[ 	]+vprotw %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 91 38[ 	]+vprotw \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 91 ff[ 	]+vprotw %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 91 f8[ 	]+vprotw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 91 3c 24[ 	]+vprotw \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 91 04 24[ 	]+vprotw \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 91 3a[ 	]+vprotw \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 91 02[ 	]+vprotw \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 91 ff[ 	]+vprotw %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c1 ff 00[ 	]+vprotw \$0x0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c1 3e 00[ 	]+vprotw \$0x0,\(%rsi\),%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 c1 d8 00[ 	]+vprotw \$0x0,%xmm0,%xmm11
[ 	]*[a-f0-9]+:	8f c8 78 c1 c7 ff[ 	]+vprotw \$0xff,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 c0 03[ 	]+vprotw \$0x3,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c1 c7 03[ 	]+vprotw \$0x3,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 c1 db 00[ 	]+vprotw \$0x0,%xmm11,%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c1 f8 00[ 	]+vprotw \$0x0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 c1 01 03[ 	]+vprotw \$0x3,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c1 06 ff[ 	]+vprotw \$0xff,\(%rsi\),%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 c1 3f 00[ 	]+vprotw \$0x0,\(%rdi\),%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c1 ff ff[ 	]+vprotw \$0xff,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c1 db ff[ 	]+vprotw \$0xff,%xmm11,%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c1 1e ff[ 	]+vprotw \$0xff,\(%rsi\),%xmm11
[ 	]*[a-f0-9]+:	8f 68 78 c1 3f 03[ 	]+vprotw \$0x3,\(%rdi\),%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 c1 df 03[ 	]+vprotw \$0x3,%xmm15,%xmm11
[ 	]*[a-f0-9]+:	8f 69 68 98 f8[ 	]+vpshab %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 98 fc[ 	]+vpshab %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 98 c0[ 	]+vpshab %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 98 39[ 	]+vpshab %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 98 c7[ 	]+vpshab %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 98 ff[ 	]+vpshab %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 98 c4[ 	]+vpshab %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 98 c4[ 	]+vpshab %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 98 3c 24[ 	]+vpshab %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 98 39[ 	]+vpshab %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 98 fc[ 	]+vpshab %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 98 39[ 	]+vpshab %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 98 45 00[ 	]+vpshab %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 98 39[ 	]+vpshab %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 98 04 24[ 	]+vpshab %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 98 7d 00[ 	]+vpshab %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 98 3c 24[ 	]+vpshab \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 98 3c 24[ 	]+vpshab \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 98 00[ 	]+vpshab \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 98 3a[ 	]+vpshab \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 98 c4[ 	]+vpshab %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 98 fc[ 	]+vpshab %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 98 c0[ 	]+vpshab %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 98 c0[ 	]+vpshab %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 98 38[ 	]+vpshab \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 98 ff[ 	]+vpshab %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 98 f8[ 	]+vpshab %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 98 3c 24[ 	]+vpshab \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 98 04 24[ 	]+vpshab \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 98 3a[ 	]+vpshab \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 98 02[ 	]+vpshab \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 98 ff[ 	]+vpshab %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 9a f8[ 	]+vpshad %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 9a fc[ 	]+vpshad %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 9a c0[ 	]+vpshad %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 9a 39[ 	]+vpshad %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 9a c7[ 	]+vpshad %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 9a ff[ 	]+vpshad %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 9a c4[ 	]+vpshad %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 9a c4[ 	]+vpshad %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 9a 3c 24[ 	]+vpshad %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 9a 39[ 	]+vpshad %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 9a fc[ 	]+vpshad %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 9a 39[ 	]+vpshad %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 9a 45 00[ 	]+vpshad %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 9a 39[ 	]+vpshad %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 9a 04 24[ 	]+vpshad %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 9a 7d 00[ 	]+vpshad %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 9a 3c 24[ 	]+vpshad \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 9a 3c 24[ 	]+vpshad \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 9a 00[ 	]+vpshad \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 9a 3a[ 	]+vpshad \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 9a c4[ 	]+vpshad %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 9a fc[ 	]+vpshad %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 9a c0[ 	]+vpshad %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 9a c0[ 	]+vpshad %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 9a 38[ 	]+vpshad \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 9a ff[ 	]+vpshad %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 9a f8[ 	]+vpshad %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 9a 3c 24[ 	]+vpshad \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 9a 04 24[ 	]+vpshad \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 9a 3a[ 	]+vpshad \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 9a 02[ 	]+vpshad \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 9a ff[ 	]+vpshad %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 9b f8[ 	]+vpshaq %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 9b fc[ 	]+vpshaq %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 9b c0[ 	]+vpshaq %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 9b 39[ 	]+vpshaq %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 9b c7[ 	]+vpshaq %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 9b ff[ 	]+vpshaq %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 9b c4[ 	]+vpshaq %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 9b c4[ 	]+vpshaq %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 9b 3c 24[ 	]+vpshaq %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 9b 39[ 	]+vpshaq %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 9b fc[ 	]+vpshaq %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 9b 39[ 	]+vpshaq %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 9b 45 00[ 	]+vpshaq %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 9b 39[ 	]+vpshaq %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 9b 04 24[ 	]+vpshaq %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 9b 7d 00[ 	]+vpshaq %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 9b 3c 24[ 	]+vpshaq \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 9b 3c 24[ 	]+vpshaq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 9b 00[ 	]+vpshaq \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 9b 3a[ 	]+vpshaq \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 9b c4[ 	]+vpshaq %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 9b fc[ 	]+vpshaq %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 9b c0[ 	]+vpshaq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 9b c0[ 	]+vpshaq %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 9b 38[ 	]+vpshaq \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 9b ff[ 	]+vpshaq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 9b f8[ 	]+vpshaq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 9b 3c 24[ 	]+vpshaq \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 9b 04 24[ 	]+vpshaq \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 9b 3a[ 	]+vpshaq \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 9b 02[ 	]+vpshaq \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 9b ff[ 	]+vpshaq %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 99 f8[ 	]+vpshaw %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 99 fc[ 	]+vpshaw %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 99 c0[ 	]+vpshaw %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 99 39[ 	]+vpshaw %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 99 c7[ 	]+vpshaw %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 99 ff[ 	]+vpshaw %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 99 c4[ 	]+vpshaw %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 99 c4[ 	]+vpshaw %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 99 3c 24[ 	]+vpshaw %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 99 39[ 	]+vpshaw %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 99 fc[ 	]+vpshaw %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 99 39[ 	]+vpshaw %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 99 45 00[ 	]+vpshaw %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 99 39[ 	]+vpshaw %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 99 04 24[ 	]+vpshaw %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 99 7d 00[ 	]+vpshaw %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 99 3c 24[ 	]+vpshaw \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 99 3c 24[ 	]+vpshaw \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 99 00[ 	]+vpshaw \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 99 3a[ 	]+vpshaw \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 99 c4[ 	]+vpshaw %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 99 fc[ 	]+vpshaw %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 99 c0[ 	]+vpshaw %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 99 c0[ 	]+vpshaw %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 99 38[ 	]+vpshaw \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 99 ff[ 	]+vpshaw %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 99 f8[ 	]+vpshaw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 99 3c 24[ 	]+vpshaw \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 99 04 24[ 	]+vpshaw \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 99 3a[ 	]+vpshaw \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 99 02[ 	]+vpshaw \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 99 ff[ 	]+vpshaw %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 94 f8[ 	]+vpshlb %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 94 fc[ 	]+vpshlb %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 94 c0[ 	]+vpshlb %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 94 39[ 	]+vpshlb %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 94 c7[ 	]+vpshlb %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 94 ff[ 	]+vpshlb %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 94 c4[ 	]+vpshlb %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 94 c4[ 	]+vpshlb %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 94 3c 24[ 	]+vpshlb %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 94 39[ 	]+vpshlb %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 94 fc[ 	]+vpshlb %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 94 39[ 	]+vpshlb %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 94 45 00[ 	]+vpshlb %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 94 39[ 	]+vpshlb %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 94 04 24[ 	]+vpshlb %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 94 7d 00[ 	]+vpshlb %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 94 3c 24[ 	]+vpshlb \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 94 3c 24[ 	]+vpshlb \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 94 00[ 	]+vpshlb \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 94 3a[ 	]+vpshlb \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 94 c4[ 	]+vpshlb %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 94 fc[ 	]+vpshlb %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 94 c0[ 	]+vpshlb %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 94 c0[ 	]+vpshlb %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 94 38[ 	]+vpshlb \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 94 ff[ 	]+vpshlb %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 94 f8[ 	]+vpshlb %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 94 3c 24[ 	]+vpshlb \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 94 04 24[ 	]+vpshlb \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 94 3a[ 	]+vpshlb \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 94 02[ 	]+vpshlb \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 94 ff[ 	]+vpshlb %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 96 f8[ 	]+vpshld %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 96 fc[ 	]+vpshld %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 96 c0[ 	]+vpshld %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 96 39[ 	]+vpshld %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 96 c7[ 	]+vpshld %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 96 ff[ 	]+vpshld %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 96 c4[ 	]+vpshld %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 96 c4[ 	]+vpshld %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 96 3c 24[ 	]+vpshld %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 96 39[ 	]+vpshld %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 96 fc[ 	]+vpshld %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 96 39[ 	]+vpshld %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 96 45 00[ 	]+vpshld %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 96 39[ 	]+vpshld %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 96 04 24[ 	]+vpshld %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 96 7d 00[ 	]+vpshld %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 96 3c 24[ 	]+vpshld \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 96 3c 24[ 	]+vpshld \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 96 00[ 	]+vpshld \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 96 3a[ 	]+vpshld \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 96 c4[ 	]+vpshld %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 96 fc[ 	]+vpshld %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 96 c0[ 	]+vpshld %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 96 c0[ 	]+vpshld %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 96 38[ 	]+vpshld \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 96 ff[ 	]+vpshld %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 96 f8[ 	]+vpshld %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 96 3c 24[ 	]+vpshld \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 96 04 24[ 	]+vpshld \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 96 3a[ 	]+vpshld \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 96 02[ 	]+vpshld \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 96 ff[ 	]+vpshld %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 97 f8[ 	]+vpshlq %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 97 fc[ 	]+vpshlq %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 97 c0[ 	]+vpshlq %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 97 39[ 	]+vpshlq %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 97 c7[ 	]+vpshlq %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 97 ff[ 	]+vpshlq %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 97 c4[ 	]+vpshlq %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 97 c4[ 	]+vpshlq %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 97 3c 24[ 	]+vpshlq %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 97 39[ 	]+vpshlq %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 97 fc[ 	]+vpshlq %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 97 39[ 	]+vpshlq %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 97 45 00[ 	]+vpshlq %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 97 39[ 	]+vpshlq %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 97 04 24[ 	]+vpshlq %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 97 7d 00[ 	]+vpshlq %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 97 3c 24[ 	]+vpshlq \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 97 3c 24[ 	]+vpshlq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 97 00[ 	]+vpshlq \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 97 3a[ 	]+vpshlq \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 97 c4[ 	]+vpshlq %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 97 fc[ 	]+vpshlq %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 97 c0[ 	]+vpshlq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 97 c0[ 	]+vpshlq %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 97 38[ 	]+vpshlq \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 97 ff[ 	]+vpshlq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 97 f8[ 	]+vpshlq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 97 3c 24[ 	]+vpshlq \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 97 04 24[ 	]+vpshlq \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 97 3a[ 	]+vpshlq \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 97 02[ 	]+vpshlq \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 97 ff[ 	]+vpshlq %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 69 68 95 f8[ 	]+vpshlw %xmm2,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 95 fc[ 	]+vpshlw %xmm2,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f e9 68 95 c0[ 	]+vpshlw %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 95 39[ 	]+vpshlw %xmm15,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 95 c7[ 	]+vpshlw %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 95 ff[ 	]+vpshlw %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 95 c4[ 	]+vpshlw %xmm0,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 95 c4[ 	]+vpshlw %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 68 95 3c 24[ 	]+vpshlw %xmm2,\(%r12\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 78 95 39[ 	]+vpshlw %xmm0,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 78 95 fc[ 	]+vpshlw %xmm0,%xmm12,%xmm7
[ 	]*[a-f0-9]+:	8f 49 68 95 39[ 	]+vpshlw %xmm2,\(%r9\),%xmm15
[ 	]*[a-f0-9]+:	8f c9 68 95 45 00[ 	]+vpshlw %xmm2,0x0\(%r13\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 00 95 39[ 	]+vpshlw %xmm15,\(%r9\),%xmm7
[ 	]*[a-f0-9]+:	8f c9 00 95 04 24[ 	]+vpshlw %xmm15,\(%r12\),%xmm0
[ 	]*[a-f0-9]+:	8f 49 78 95 7d 00[ 	]+vpshlw %xmm0,0x0\(%r13\),%xmm15
[ 	]*[a-f0-9]+:	8f 49 f8 95 3c 24[ 	]+vpshlw \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c9 f8 95 3c 24[ 	]+vpshlw \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e9 f8 95 00[ 	]+vpshlw \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 49 80 95 3a[ 	]+vpshlw \(%r10\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 95 c4[ 	]+vpshlw %xmm15,%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 95 fc[ 	]+vpshlw %xmm15,%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f e9 00 95 c0[ 	]+vpshlw %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 68 95 c0[ 	]+vpshlw %xmm2,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 69 98 95 38[ 	]+vpshlw \(%rax\),%xmm12,%xmm15
[ 	]*[a-f0-9]+:	8f c9 00 95 ff[ 	]+vpshlw %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e9 78 95 f8[ 	]+vpshlw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 49 80 95 3c 24[ 	]+vpshlw \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c9 80 95 04 24[ 	]+vpshlw \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c9 80 95 3a[ 	]+vpshlw \(%r10\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c9 98 95 02[ 	]+vpshlw \(%r10\),%xmm12,%xmm0
[ 	]*[a-f0-9]+:	8f 49 00 95 ff[ 	]+vpshlw %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cc c0 00[ 	]+vpcomltb %xmm8,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 cc 3c 24 00[ 	]+vpcomltb \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cc 04 0f 00[ 	]+vpcomltb \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 cc ff 00[ 	]+vpcomltb %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cc 3c 0f 00[ 	]+vpcomltb \(%rdi,%rcx,1\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 cc 04 0f 00[ 	]+vpcomltb \(%rdi,%rcx,1\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 08 00 cc 7c 59 06 00[ 	]+vpcomltb 0x6\(%r9,%r11,2\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cc 3c 83 00[ 	]+vpcomltb \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ce 3c 0f 00[ 	]+vpcomltd \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 ce c0 00[ 	]+vpcomltd %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ce 3c 24 00[ 	]+vpcomltd \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ce ff 00[ 	]+vpcomltd %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ce f8 00[ 	]+vpcomltd %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ce 04 24 00[ 	]+vpcomltd \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 ce f8 00[ 	]+vpcomltd %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ce c7 00[ 	]+vpcomltd %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cf f8 00[ 	]+vpcomltq %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cf ff 00[ 	]+vpcomltq %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cf c7 00[ 	]+vpcomltq %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 cf 3c 0f 00[ 	]+vpcomltq \(%rdi,%rcx,1\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 cf 44 59 06 00[ 	]+vpcomltq 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 cf 3c 24 00[ 	]+vpcomltq \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 cf c7 00[ 	]+vpcomltq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf 3c 0f 00[ 	]+vpcomltq \(%rdi,%rcx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 ec f8 00[ 	]+vpcomltub %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 08 00 ec 7c 59 06 00[ 	]+vpcomltub 0x6\(%r9,%r11,2\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 0f 00[ 	]+vpcomltub \(%rdi,%rcx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ec f8 00[ 	]+vpcomltub %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ec 3c 83 00[ 	]+vpcomltub \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ec 3c 0f 00[ 	]+vpcomltub \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ec f8 00[ 	]+vpcomltub %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ec 3c 24 00[ 	]+vpcomltub \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee f8 00[ 	]+vpcomltud %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ee c7 00[ 	]+vpcomltud %xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ee ff 00[ 	]+vpcomltud %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ee 3c 0f 00[ 	]+vpcomltud \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ee 3c 0f 00[ 	]+vpcomltud \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ee ff 00[ 	]+vpcomltud %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 78 ee 7c 59 06 00[ 	]+vpcomltud 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 88 78 ee 44 59 06 00[ 	]+vpcomltud 0x6\(%r9,%r11,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ef c7 00[ 	]+vpcomltuq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 3c 83 00[ 	]+vpcomltuq \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ef 3c 24 00[ 	]+vpcomltuq \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef 04 0f 00[ 	]+vpcomltuq \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ef c0 00[ 	]+vpcomltuq %xmm0,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ef 3c 0f 00[ 	]+vpcomltuq \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 ef 3c 0f 00[ 	]+vpcomltuq \(%rdi,%rcx,1\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef 3c 24 00[ 	]+vpcomltuq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ed 3c 83 00[ 	]+vpcomltuw \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ed 3c 83 00[ 	]+vpcomltuw \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ed f8 00[ 	]+vpcomltuw %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ed 3c 24 00[ 	]+vpcomltuw \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ed 3c 0f 00[ 	]+vpcomltuw \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 ed 3c 83 00[ 	]+vpcomltuw \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 ed f8 00[ 	]+vpcomltuw %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed c0 00[ 	]+vpcomltuw %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cd c0 00[ 	]+vpcomltw %xmm0,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 08 20 cd 7c 59 06 00[ 	]+vpcomltw 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 cd ff 00[ 	]+vpcomltw %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cd 3c 24 00[ 	]+vpcomltw \(%r12\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cd 3c 24 00[ 	]+vpcomltw \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 88 00 cd 44 59 06 00[ 	]+vpcomltw 0x6\(%r9,%r11,2\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cd 3c 0f 00[ 	]+vpcomltw \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cd f8 00[ 	]+vpcomltw %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 cc f8 01[ 	]+vpcomleb %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 08 78 cc 7c 59 06 01[ 	]+vpcomleb 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 cc f8 01[ 	]+vpcomleb %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 cc ff 01[ 	]+vpcomleb %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cc 3c 24 01[ 	]+vpcomleb \(%r12\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cc ff 01[ 	]+vpcomleb %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 cc 3c 83 01[ 	]+vpcomleb \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cc 3c 24 01[ 	]+vpcomleb \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 ce 3c 24 01[ 	]+vpcomled \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 ce f8 01[ 	]+vpcomled %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 88 78 ce 44 59 06 01[ 	]+vpcomled 0x6\(%r9,%r11,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ce f8 01[ 	]+vpcomled %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 00 ce ff 01[ 	]+vpcomled %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 88 78 ce 7c 59 06 01[ 	]+vpcomled 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ce 3c 83 01[ 	]+vpcomled \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 04 0f 01[ 	]+vpcomled \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 08 78 cf 7c 59 06 01[ 	]+vpcomleq 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 cf f8 01[ 	]+vpcomleq %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cf c7 01[ 	]+vpcomleq %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cf c0 01[ 	]+vpcomleq %xmm8,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 cf c0 01[ 	]+vpcomleq %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cf f8 01[ 	]+vpcomleq %xmm8,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 cf f8 01[ 	]+vpcomleq %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cf c7 01[ 	]+vpcomleq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ec 3c 0f 01[ 	]+vpcomleub \(%rdi,%rcx,1\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 88 20 ec 44 59 06 01[ 	]+vpcomleub 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 ec 3c 83 01[ 	]+vpcomleub \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 78 ec 7c 59 06 01[ 	]+vpcomleub 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 83 01[ 	]+vpcomleub \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ec 3c 24 01[ 	]+vpcomleub \(%r12\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 ec c0 01[ 	]+vpcomleub %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 20 ec 3c 83 01[ 	]+vpcomleub \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ee 3c 83 01[ 	]+vpcomleud \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ee c7 01[ 	]+vpcomleud %xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ee c7 01[ 	]+vpcomleud %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 3c 83 01[ 	]+vpcomleud \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ee f8 01[ 	]+vpcomleud %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 83 01[ 	]+vpcomleud \(%rbx,%rax,4\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 ee 44 59 06 01[ 	]+vpcomleud 0x6\(%r9,%r11,2\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 08 78 ee 7c 59 06 01[ 	]+vpcomleud 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 ef f8 01[ 	]+vpcomleuq %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 ef 3c 83 01[ 	]+vpcomleuq \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 ef ff 01[ 	]+vpcomleuq %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ef ff 01[ 	]+vpcomleuq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ef c7 01[ 	]+vpcomleuq %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 3c 83 01[ 	]+vpcomleuq \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 08 00 ef 7c 59 06 01[ 	]+vpcomleuq 0x6\(%r9,%r11,2\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef f8 01[ 	]+vpcomleuq %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 08 20 ed 7c 59 06 01[ 	]+vpcomleuw 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 ed 44 59 06 01[ 	]+vpcomleuw 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ed c0 01[ 	]+vpcomleuw %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ed 04 83 01[ 	]+vpcomleuw \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 ed f8 01[ 	]+vpcomleuw %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ed 3c 24 01[ 	]+vpcomleuw \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ed c0 01[ 	]+vpcomleuw %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ed 3c 83 01[ 	]+vpcomleuw \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 cd 3c 0f 01[ 	]+vpcomlew \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cd f8 01[ 	]+vpcomlew %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cd 04 83 01[ 	]+vpcomlew \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cd c0 01[ 	]+vpcomlew %xmm8,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cd 3c 0f 01[ 	]+vpcomlew \(%rdi,%rcx,1\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 cd 3c 0f 01[ 	]+vpcomlew \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 cd c0 01[ 	]+vpcomlew %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cd 3c 24 01[ 	]+vpcomlew \(%r12\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 00 cc 3c 24 02[ 	]+vpcomgtb \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 cc 3c 83 02[ 	]+vpcomgtb \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 cc 3c 24 02[ 	]+vpcomgtb \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 cc c7 02[ 	]+vpcomgtb %xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cc c0 02[ 	]+vpcomgtb %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cc 3c 83 02[ 	]+vpcomgtb \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 cc f8 02[ 	]+vpcomgtb %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 cc f8 02[ 	]+vpcomgtb %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ce 3c 0f 02[ 	]+vpcomgtd \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 ce ff 02[ 	]+vpcomgtd %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ce f8 02[ 	]+vpcomgtd %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 88 20 ce 44 59 06 02[ 	]+vpcomgtd 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 68 20 ce 3c 83 02[ 	]+vpcomgtd \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ce c0 02[ 	]+vpcomgtd %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ce 04 0f 02[ 	]+vpcomgtd \(%rdi,%rcx,1\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ce c7 02[ 	]+vpcomgtd %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 cf ff 02[ 	]+vpcomgtq %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cf c0 02[ 	]+vpcomgtq %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cf 3c 0f 02[ 	]+vpcomgtq \(%rdi,%rcx,1\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cf 3c 24 02[ 	]+vpcomgtq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cf ff 02[ 	]+vpcomgtq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 cf 3c 0f 02[ 	]+vpcomgtq \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 cf f8 02[ 	]+vpcomgtq %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 cf 3c 24 02[ 	]+vpcomgtq \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ec f8 02[ 	]+vpcomgtub %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ec c0 02[ 	]+vpcomgtub %xmm8,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 ec 3c 83 02[ 	]+vpcomgtub \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 ec 3c 24 02[ 	]+vpcomgtub \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ec 3c 83 02[ 	]+vpcomgtub \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ec c0 02[ 	]+vpcomgtub %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 ec ff 02[ 	]+vpcomgtub %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 88 00 ec 44 59 06 02[ 	]+vpcomgtub 0x6\(%r9,%r11,2\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ee 04 24 02[ 	]+vpcomgtud \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 ee ff 02[ 	]+vpcomgtud %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ee c7 02[ 	]+vpcomgtud %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 08 78 ee 7c 59 06 02[ 	]+vpcomgtud 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ee 04 83 02[ 	]+vpcomgtud \(%rbx,%rax,4\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee c0 02[ 	]+vpcomgtud %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 0f 02[ 	]+vpcomgtud \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 ee f8 02[ 	]+vpcomgtud %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ef ff 02[ 	]+vpcomgtuq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ef 3c 83 02[ 	]+vpcomgtuq \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ef 04 24 02[ 	]+vpcomgtuq \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 08 20 ef 7c 59 06 02[ 	]+vpcomgtuq 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 ef f8 02[ 	]+vpcomgtuq %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef c0 02[ 	]+vpcomgtuq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 00 ef f8 02[ 	]+vpcomgtuq %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 ef 3c 24 02[ 	]+vpcomgtuq \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ed 04 0f 02[ 	]+vpcomgtuw \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 88 20 ed 44 59 06 02[ 	]+vpcomgtuw 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 ed c7 02[ 	]+vpcomgtuw %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 ed 3c 24 02[ 	]+vpcomgtuw \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 ed f8 02[ 	]+vpcomgtuw %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ed ff 02[ 	]+vpcomgtuw %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ed ff 02[ 	]+vpcomgtuw %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ed ff 02[ 	]+vpcomgtuw %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cd c0 02[ 	]+vpcomgtw %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 00 cd 3c 83 02[ 	]+vpcomgtw \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 cd ff 02[ 	]+vpcomgtw %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cd ff 02[ 	]+vpcomgtw %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cd 3c 83 02[ 	]+vpcomgtw \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 cd 3c 24 02[ 	]+vpcomgtw \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 88 20 cd 7c 59 06 02[ 	]+vpcomgtw 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 cd 3c 0f 02[ 	]+vpcomgtw \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cc 04 83 03[ 	]+vpcomgeb \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 88 78 cc 7c 59 06 03[ 	]+vpcomgeb 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 cc 3c 0f 03[ 	]+vpcomgeb \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cc 3c 0f 03[ 	]+vpcomgeb \(%rdi,%rcx,1\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 cc ff 03[ 	]+vpcomgeb %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 cc ff 03[ 	]+vpcomgeb %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 cc f8 03[ 	]+vpcomgeb %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 cc c0 03[ 	]+vpcomgeb %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 ce ff 03[ 	]+vpcomged %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 ce f8 03[ 	]+vpcomged %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ce 3c 83 03[ 	]+vpcomged \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ce c7 03[ 	]+vpcomged %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 ce ff 03[ 	]+vpcomged %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ce 3c 0f 03[ 	]+vpcomged \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ce 3c 83 03[ 	]+vpcomged \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 ce f8 03[ 	]+vpcomged %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cf f8 03[ 	]+vpcomgeq %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cf ff 03[ 	]+vpcomgeq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cf f8 03[ 	]+vpcomgeq %xmm8,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cf c7 03[ 	]+vpcomgeq %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 20 cf 44 59 06 03[ 	]+vpcomgeq 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 cf 44 59 06 03[ 	]+vpcomgeq 0x6\(%r9,%r11,2\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 cf ff 03[ 	]+vpcomgeq %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cf 04 83 03[ 	]+vpcomgeq \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ec 3c 24 03[ 	]+vpcomgeub \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 08 20 ec 7c 59 06 03[ 	]+vpcomgeub 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ec 04 24 03[ 	]+vpcomgeub \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ec 04 83 03[ 	]+vpcomgeub \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 ec 7c 59 06 03[ 	]+vpcomgeub 0x6\(%r9,%r11,2\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ec f8 03[ 	]+vpcomgeub %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 ec c0 03[ 	]+vpcomgeub %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 ec ff 03[ 	]+vpcomgeub %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ee ff 03[ 	]+vpcomgeud %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ee 3c 0f 03[ 	]+vpcomgeud \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ee 04 83 03[ 	]+vpcomgeud \(%rbx,%rax,4\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ee 04 83 03[ 	]+vpcomgeud \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 20 ee f8 03[ 	]+vpcomgeud %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 ee f8 03[ 	]+vpcomgeud %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ee 04 24 03[ 	]+vpcomgeud \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ee 04 0f 03[ 	]+vpcomgeud \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ef c0 03[ 	]+vpcomgeuq %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 ef 3c 24 03[ 	]+vpcomgeuq \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef ff 03[ 	]+vpcomgeuq %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ef f8 03[ 	]+vpcomgeuq %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef 04 0f 03[ 	]+vpcomgeuq \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ef f8 03[ 	]+vpcomgeuq %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ef 04 24 03[ 	]+vpcomgeuq \(%r12\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 ef 7c 59 06 03[ 	]+vpcomgeuq 0x6\(%r9,%r11,2\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 00 ed f8 03[ 	]+vpcomgeuw %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ed ff 03[ 	]+vpcomgeuw %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 88 00 ed 7c 59 06 03[ 	]+vpcomgeuw 0x6\(%r9,%r11,2\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ed f8 03[ 	]+vpcomgeuw %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 00 ed 3c 24 03[ 	]+vpcomgeuw \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ed 3c 0f 03[ 	]+vpcomgeuw \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ed c7 03[ 	]+vpcomgeuw %xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ed f8 03[ 	]+vpcomgeuw %xmm8,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cd 3c 83 03[ 	]+vpcomgew \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cd c0 03[ 	]+vpcomgew %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cd f8 03[ 	]+vpcomgew %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cd f8 03[ 	]+vpcomgew %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 08 20 cd 7c 59 06 03[ 	]+vpcomgew 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cd f8 03[ 	]+vpcomgew %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 cd 3c 83 03[ 	]+vpcomgew \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cd 04 83 03[ 	]+vpcomgew \(%rbx,%rax,4\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 cc 3c 24 04[ 	]+vpcomeqb \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 cc 3c 24 04[ 	]+vpcomeqb \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cc ff 04[ 	]+vpcomeqb %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cc c0 04[ 	]+vpcomeqb %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 88 78 cc 7c 59 06 04[ 	]+vpcomeqb 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cc f8 04[ 	]+vpcomeqb %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc 3c 0f 04[ 	]+vpcomeqb \(%rdi,%rcx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 08 20 cc 7c 59 06 04[ 	]+vpcomeqb 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ce c7 04[ 	]+vpcomeqd %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ce c0 04[ 	]+vpcomeqd %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 04 83 04[ 	]+vpcomeqd \(%rbx,%rax,4\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 ce f8 04[ 	]+vpcomeqd %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ce f8 04[ 	]+vpcomeqd %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ce 3c 24 04[ 	]+vpcomeqd \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 88 78 ce 44 59 06 04[ 	]+vpcomeqd 0x6\(%r9,%r11,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 08 00 ce 7c 59 06 04[ 	]+vpcomeqd 0x6\(%r9,%r11,2\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 cf f8 04[ 	]+vpcomeqq %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 cf f8 04[ 	]+vpcomeqq %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 00 cf 44 59 06 04[ 	]+vpcomeqq 0x6\(%r9,%r11,2\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cf 3c 83 04[ 	]+vpcomeqq \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 cf 3c 24 04[ 	]+vpcomeqq \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cf f8 04[ 	]+vpcomeqq %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 00 cf ff 04[ 	]+vpcomeqq %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 08 78 cf 7c 59 06 04[ 	]+vpcomeqq 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 ec f8 04[ 	]+vpcomequb %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ec ff 04[ 	]+vpcomequb %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ec c0 04[ 	]+vpcomequb %xmm0,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ec 04 24 04[ 	]+vpcomequb \(%r12\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 ec 3c 24 04[ 	]+vpcomequb \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 83 04[ 	]+vpcomequb \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ec 04 24 04[ 	]+vpcomequb \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ec f8 04[ 	]+vpcomequb %xmm8,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ee 3c 83 04[ 	]+vpcomequd \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 08 00 ee 7c 59 06 04[ 	]+vpcomequd 0x6\(%r9,%r11,2\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 ee 04 83 04[ 	]+vpcomequd \(%rbx,%rax,4\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 ee 3c 83 04[ 	]+vpcomequd \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ee c7 04[ 	]+vpcomequd %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 68 00 ee 3c 83 04[ 	]+vpcomequd \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ee 3c 24 04[ 	]+vpcomequd \(%r12\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ee 3c 24 04[ 	]+vpcomequd \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ef 3c 24 04[ 	]+vpcomequq \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef ff 04[ 	]+vpcomequq %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ef 3c 24 04[ 	]+vpcomequq \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef 3c 0f 04[ 	]+vpcomequq \(%rdi,%rcx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ef f8 04[ 	]+vpcomequq %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef c7 04[ 	]+vpcomequq %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ef 3c 0f 04[ 	]+vpcomequq \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef f8 04[ 	]+vpcomequq %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ed 3c 0f 04[ 	]+vpcomequw \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ed ff 04[ 	]+vpcomequw %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 08 78 ed 7c 59 06 04[ 	]+vpcomequw 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 ed 3c 0f 04[ 	]+vpcomequw \(%rdi,%rcx,1\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ed 04 24 04[ 	]+vpcomequw \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ed c0 04[ 	]+vpcomequw %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 ed 3c 24 04[ 	]+vpcomequw \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 ed 3c 83 04[ 	]+vpcomequw \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 cd 7c 59 06 04[ 	]+vpcomeqw 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cd c0 04[ 	]+vpcomeqw %xmm0,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cd c7 04[ 	]+vpcomeqw %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cd f8 04[ 	]+vpcomeqw %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cd 3c 83 04[ 	]+vpcomeqw \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cd 3c 83 04[ 	]+vpcomeqw \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cd c7 04[ 	]+vpcomeqw %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 00 cd 3c 83 04[ 	]+vpcomeqw \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 cc 3c 24 05[ 	]+vpcomneqb \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 cc 7c 59 06 05[ 	]+vpcomneqb 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cc c0 05[ 	]+vpcomneqb %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 cc 3c 24 05[ 	]+vpcomneqb \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cc 3c 83 05[ 	]+vpcomneqb \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 cc 3c 83 05[ 	]+vpcomneqb \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cc 04 24 05[ 	]+vpcomneqb \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cc f8 05[ 	]+vpcomneqb %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 ce 3c 83 05[ 	]+vpcomneqd \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ce f8 05[ 	]+vpcomneqd %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ce c0 05[ 	]+vpcomneqd %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 88 78 ce 7c 59 06 05[ 	]+vpcomneqd 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ce c7 05[ 	]+vpcomneqd %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ce 04 0f 05[ 	]+vpcomneqd \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ce 04 24 05[ 	]+vpcomneqd \(%r12\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ce f8 05[ 	]+vpcomneqd %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 cf 3c 24 05[ 	]+vpcomneqq \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 cf c0 05[ 	]+vpcomneqq %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 cf 3c 0f 05[ 	]+vpcomneqq \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 88 78 cf 7c 59 06 05[ 	]+vpcomneqq 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cf f8 05[ 	]+vpcomneqq %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 cf f8 05[ 	]+vpcomneqq %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 cf c0 05[ 	]+vpcomneqq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 20 cf 3c 83 05[ 	]+vpcomneqq \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ec 3c 0f 05[ 	]+vpcomnequb \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ec 04 83 05[ 	]+vpcomnequb \(%rbx,%rax,4\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 ec ff 05[ 	]+vpcomnequb %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ec f8 05[ 	]+vpcomnequb %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ec c0 05[ 	]+vpcomnequb %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 ec 44 59 06 05[ 	]+vpcomnequb 0x6\(%r9,%r11,2\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 ec f8 05[ 	]+vpcomnequb %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ec ff 05[ 	]+vpcomnequb %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ee 3c 83 05[ 	]+vpcomnequd \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ee 3c 24 05[ 	]+vpcomnequd \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 ee ff 05[ 	]+vpcomnequd %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ee ff 05[ 	]+vpcomnequd %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ee f8 05[ 	]+vpcomnequd %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ee ff 05[ 	]+vpcomnequd %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ee 3c 24 05[ 	]+vpcomnequd \(%r12\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ee 04 24 05[ 	]+vpcomnequd \(%r12\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 ef 7c 59 06 05[ 	]+vpcomnequq 0x6\(%r9,%r11,2\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ef 3c 24 05[ 	]+vpcomnequq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ef f8 05[ 	]+vpcomnequq %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ef 3c 0f 05[ 	]+vpcomnequq \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ef 3c 83 05[ 	]+vpcomnequq \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef c0 05[ 	]+vpcomnequq %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 ef 3c 24 05[ 	]+vpcomnequq \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ef ff 05[ 	]+vpcomnequq %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ed ff 05[ 	]+vpcomnequw %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ed f8 05[ 	]+vpcomnequw %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ed ff 05[ 	]+vpcomnequw %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ed f8 05[ 	]+vpcomnequw %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ed f8 05[ 	]+vpcomnequw %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ed 3c 83 05[ 	]+vpcomnequw \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 08 20 ed 7c 59 06 05[ 	]+vpcomnequw 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 78 ed ff 05[ 	]+vpcomnequw %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 cd 04 83 05[ 	]+vpcomneqw \(%rbx,%rax,4\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cd 04 24 05[ 	]+vpcomneqw \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 cd c7 05[ 	]+vpcomneqw %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 cd 3c 24 05[ 	]+vpcomneqw \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cd f8 05[ 	]+vpcomneqw %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cd f8 05[ 	]+vpcomneqw %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 cd ff 05[ 	]+vpcomneqw %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 cd 3c 0f 05[ 	]+vpcomneqw \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 cc 3c 0f 06[ 	]+vpcomfalseb \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cc f8 06[ 	]+vpcomfalseb %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cc 04 24 06[ 	]+vpcomfalseb \(%r12\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 cc f8 06[ 	]+vpcomfalseb %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 cc 04 83 06[ 	]+vpcomfalseb \(%rbx,%rax,4\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cc 04 24 06[ 	]+vpcomfalseb \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 cc 3c 24 06[ 	]+vpcomfalseb \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 cc 3c 0f 06[ 	]+vpcomfalseb \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ce 3c 0f 06[ 	]+vpcomfalsed \(%rdi,%rcx,1\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ce f8 06[ 	]+vpcomfalsed %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 08 78 ce 7c 59 06 06[ 	]+vpcomfalsed 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ce ff 06[ 	]+vpcomfalsed %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ce c0 06[ 	]+vpcomfalsed %xmm0,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 ce ff 06[ 	]+vpcomfalsed %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ce f8 06[ 	]+vpcomfalsed %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 ce c7 06[ 	]+vpcomfalsed %xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cf ff 06[ 	]+vpcomfalseq %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cf 3c 0f 06[ 	]+vpcomfalseq \(%rdi,%rcx,1\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 68 78 cf 3c 83 06[ 	]+vpcomfalseq \(%rbx,%rax,4\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 cf 04 24 06[ 	]+vpcomfalseq \(%r12\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cf c0 06[ 	]+vpcomfalseq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 78 cf f8 06[ 	]+vpcomfalseq %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cf ff 06[ 	]+vpcomfalseq %xmm15,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cf 3c 24 06[ 	]+vpcomfalseq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ec 04 0f 06[ 	]+vpcomfalseub \(%rdi,%rcx,1\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ec 3c 83 06[ 	]+vpcomfalseub \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ec 04 83 06[ 	]+vpcomfalseub \(%rbx,%rax,4\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 20 ec 44 59 06 06[ 	]+vpcomfalseub 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ec ff 06[ 	]+vpcomfalseub %xmm15,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ec f8 06[ 	]+vpcomfalseub %xmm8,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 ec 3c 83 06[ 	]+vpcomfalseub \(%rbx,%rax,4\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ec f8 06[ 	]+vpcomfalseub %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 ee 44 59 06 06[ 	]+vpcomfalseud 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ee c0 06[ 	]+vpcomfalseud %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 88 20 ee 7c 59 06 06[ 	]+vpcomfalseud 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 88 78 ee 44 59 06 06[ 	]+vpcomfalseud 0x6\(%r9,%r11,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 ee f8 06[ 	]+vpcomfalseud %xmm0,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ee 04 24 06[ 	]+vpcomfalseud \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ee f8 06[ 	]+vpcomfalseud %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ee ff 06[ 	]+vpcomfalseud %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 78 ef ff 06[ 	]+vpcomfalseuq %xmm15,%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ef ff 06[ 	]+vpcomfalseuq %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ef 3c 24 06[ 	]+vpcomfalseuq \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ef c0 06[ 	]+vpcomfalseuq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 78 ef 3c 0f 06[ 	]+vpcomfalseuq \(%rdi,%rcx,1\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 ef 7c 59 06 06[ 	]+vpcomfalseuq 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ef 04 0f 06[ 	]+vpcomfalseuq \(%rdi,%rcx,1\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 ef f8 06[ 	]+vpcomfalseuq %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed c0 06[ 	]+vpcomfalseuw %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 20 ed 3c 24 06[ 	]+vpcomfalseuw \(%r12\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ed 3c 0f 06[ 	]+vpcomfalseuw \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 ed c0 06[ 	]+vpcomfalseuw %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 20 ed 3c 0f 06[ 	]+vpcomfalseuw \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ed c7 06[ 	]+vpcomfalseuw %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 00 ed 7c 59 06 06[ 	]+vpcomfalseuw 0x6\(%r9,%r11,2\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ed 04 0f 06[ 	]+vpcomfalseuw \(%rdi,%rcx,1\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 20 cd 44 59 06 06[ 	]+vpcomfalsew 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 08 78 cd 7c 59 06 06[ 	]+vpcomfalsew 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f 88 78 cd 7c 59 06 06[ 	]+vpcomfalsew 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 88 20 cd 7c 59 06 06[ 	]+vpcomfalsew 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 48 00 cd ff 06[ 	]+vpcomfalsew %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cd f8 06[ 	]+vpcomfalsew %xmm8,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 cd 04 83 06[ 	]+vpcomfalsew \(%rbx,%rax,4\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cd 04 24 06[ 	]+vpcomfalsew \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 00 cc 3c 83 07[ 	]+vpcomtrueb \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cc 3c 24 07[ 	]+vpcomtrueb \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cc f8 07[ 	]+vpcomtrueb %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cc 04 0f 07[ 	]+vpcomtrueb \(%rdi,%rcx,1\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cc c7 07[ 	]+vpcomtrueb %xmm15,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 cc 04 24 07[ 	]+vpcomtrueb \(%r12\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 cc ff 07[ 	]+vpcomtrueb %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 20 cc 3c 0f 07[ 	]+vpcomtrueb \(%rdi,%rcx,1\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 ce 04 24 07[ 	]+vpcomtrued \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 ce 3c 24 07[ 	]+vpcomtrued \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ce c7 07[ 	]+vpcomtrued %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 78 ce 7c 59 06 07[ 	]+vpcomtrued 0x6\(%r9,%r11,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 ce f8 07[ 	]+vpcomtrued %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 20 ce f8 07[ 	]+vpcomtrued %xmm0,%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 ce 3c 83 07[ 	]+vpcomtrued \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ce 3c 24 07[ 	]+vpcomtrued \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 cf ff 07[ 	]+vpcomtrueq %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cf 3c 24 07[ 	]+vpcomtrueq \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 cf f8 07[ 	]+vpcomtrueq %xmm0,%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 08 78 cf 7c 59 06 07[ 	]+vpcomtrueq 0x6\(%r9,%r11,2\),%xmm0,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 cf c7 07[ 	]+vpcomtrueq %xmm15,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 68 20 cf f8 07[ 	]+vpcomtrueq %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f e8 00 cf 04 0f 07[ 	]+vpcomtrueq \(%rdi,%rcx,1\),%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f 88 20 cf 7c 59 06 07[ 	]+vpcomtrueq 0x6\(%r9,%r11,2\),%xmm11,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ec f8 07[ 	]+vpcomtrueub %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ec c0 07[ 	]+vpcomtrueub %xmm8,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 ec c7 07[ 	]+vpcomtrueub %xmm15,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 00 ec 3c 24 07[ 	]+vpcomtrueub \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 78 ec 3c 24 07[ 	]+vpcomtrueub \(%r12\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 20 ec f8 07[ 	]+vpcomtrueub %xmm0,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 88 20 ec 44 59 06 07[ 	]+vpcomtrueub 0x6\(%r9,%r11,2\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 20 ec ff 07[ 	]+vpcomtrueub %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ee ff 07[ 	]+vpcomtrueud %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 08 20 ee 7c 59 06 07[ 	]+vpcomtrueud 0x6\(%r9,%r11,2\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ee f8 07[ 	]+vpcomtrueud %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ee c0 07[ 	]+vpcomtrueud %xmm8,%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 88 78 ee 44 59 06 07[ 	]+vpcomtrueud 0x6\(%r9,%r11,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 00 ee 3c 83 07[ 	]+vpcomtrueud \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ee ff 07[ 	]+vpcomtrueud %xmm15,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 00 ee 3c 0f 07[ 	]+vpcomtrueud \(%rdi,%rcx,1\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f e8 78 ef 04 0f 07[ 	]+vpcomtrueuq \(%rdi,%rcx,1\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 68 00 ef 3c 83 07[ 	]+vpcomtrueuq \(%rbx,%rax,4\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 20 ef 04 24 07[ 	]+vpcomtrueuq \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 ef 3c 83 07[ 	]+vpcomtrueuq \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ef f8 07[ 	]+vpcomtrueuq %xmm8,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 88 78 ef 44 59 06 07[ 	]+vpcomtrueuq 0x6\(%r9,%r11,2\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 ef f8 07[ 	]+vpcomtrueuq %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ef 3c 24 07[ 	]+vpcomtrueuq \(%r12\),%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 00 ed f8 07[ 	]+vpcomtrueuw %xmm8,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 88 00 ed 7c 59 06 07[ 	]+vpcomtrueuw 0x6\(%r9,%r11,2\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f 48 20 ed 3c 24 07[ 	]+vpcomtrueuw \(%r12\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 ed 3c 83 07[ 	]+vpcomtrueuw \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 00 ed f8 07[ 	]+vpcomtrueuw %xmm0,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f c8 78 ed f8 07[ 	]+vpcomtrueuw %xmm8,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 78 ed 3c 83 07[ 	]+vpcomtrueuw \(%rbx,%rax,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f e8 00 ed 3c 83 07[ 	]+vpcomtrueuw \(%rbx,%rax,4\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 20 cd 04 24 07[ 	]+vpcomtruew \(%r12\),%xmm11,%xmm0
[ 	]*[a-f0-9]+:	8f 48 00 cd ff 07[ 	]+vpcomtruew %xmm15,%xmm15,%xmm15
[ 	]*[a-f0-9]+:	8f 48 20 cd ff 07[ 	]+vpcomtruew %xmm15,%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f 68 20 cd 3c 83 07[ 	]+vpcomtruew \(%rbx,%rax,4\),%xmm11,%xmm15
[ 	]*[a-f0-9]+:	8f c8 00 cd 3c 24 07[ 	]+vpcomtruew \(%r12\),%xmm15,%xmm7
[ 	]*[a-f0-9]+:	8f c8 00 cd c0 07[ 	]+vpcomtruew %xmm8,%xmm15,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 cd f8 07[ 	]+vpcomtruew %xmm0,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	8f 68 78 cd 3c 83 07[ 	]+vpcomtruew \(%rbx,%rax,4\),%xmm0,%xmm15
#pass
