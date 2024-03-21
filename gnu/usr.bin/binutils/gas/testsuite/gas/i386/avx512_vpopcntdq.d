#as:
#objdump: -dw
#name: i386 AVX512/VPOPCNTDQ insns
#source: avx512_vpopcntdq.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 31[ 	]*vpopcntd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 30[ 	]*vpopcntd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 80[ 	]*vpopcntd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 80[ 	]*vpopcntd -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 31[ 	]*vpopcntq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 30[ 	]*vpopcntq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 80[ 	]*vpopcntq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 80[ 	]*vpopcntq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 31[ 	]*vpopcntd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 30[ 	]*vpopcntd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 30[ 	]*vpopcntd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 80[ 	]*vpopcntd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 80[ 	]*vpopcntd -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 31[ 	]*vpopcntq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 30[ 	]*vpopcntq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 30[ 	]*vpopcntq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 80[ 	]*vpopcntq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 80[ 	]*vpopcntq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq -0x408\(%edx\)\{1to8\},%zmm6
#pass
