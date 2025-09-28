#as: -moperand-check=none
#objdump: -dw
#name: i386 vgather check (-moperand-check=none)

.*:     file format .*

Disassembly of section .text:

0+ <vgather>:
[ 	]*[a-f0-9]+:[ 	]+c4 e2 69 92 04 08[ 	]+vgatherdps %xmm2,\(%eax,%xmm1,1\),%xmm0
[ 	]*[a-f0-9]+:[ 	]+c4 e2 69 92 14 48[ 	]+vgatherdps %xmm2/\(bad\),\(%eax,%xmm1,2\),%xmm2/\(bad\)
[ 	]*[a-f0-9]+:[ 	]+c4 e2 71 92 04 88[ 	]+vgatherdps %xmm1/\(bad\),\(%eax,%xmm1,4\)/\(bad\),%xmm0
[ 	]*[a-f0-9]+:[ 	]+c4 e2 69 92 0c c8[ 	]+vgatherdps %xmm2,\(%eax,%xmm1,8\)/\(bad\),%xmm1/\(bad\)

00000018 <avx512vgather>:
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 92 b4 fd 7b 00 00 00[ 	]+vgatherdpd 0x7b\(%ebp,%ymm7,8\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 92 b4 f5 7b 00 00 00[ 	]+vgatherdpd 0x7b\(%ebp,%ymm6,8\)/\(bad\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 92 b4 fd 7b 00 00 00[ 	]+vgatherdps 0x7b\(%ebp,%zmm7,8\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 92 b4 f5 7b 00 00 00[ 	]+vgatherdps 0x7b\(%ebp,%zmm6,8\)/\(bad\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 93 b4 fd 7b 00 00 00[ 	]+vgatherqpd 0x7b\(%ebp,%zmm7,8\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 93 b4 f5 7b 00 00 00[ 	]+vgatherqpd 0x7b\(%ebp,%zmm6,8\)/\(bad\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 93 b4 fd 7b 00 00 00[ 	]+vgatherqps 0x7b\(%ebp,%zmm7,8\),%ymm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 93 b4 f5 7b 00 00 00[ 	]+vgatherqps 0x7b\(%ebp,%zmm6,8\)/\(bad\),%ymm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 90 b4 fd 7b 00 00 00[ 	]+vpgatherdd 0x7b\(%ebp,%zmm7,8\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 90 b4 f5 7b 00 00 00[ 	]+vpgatherdd 0x7b\(%ebp,%zmm6,8\)/\(bad\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 90 b4 fd 7b 00 00 00[ 	]+vpgatherdq 0x7b\(%ebp,%ymm7,8\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 90 b4 f5 7b 00 00 00[ 	]+vpgatherdq 0x7b\(%ebp,%ymm6,8\)/\(bad\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 91 b4 fd 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%ebp,%zmm7,8\),%ymm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 49 91 b4 f5 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%ebp,%zmm6,8\)/\(bad\),%ymm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 91 b4 fd 7b 00 00 00[ 	]+vpgatherqq 0x7b\(%ebp,%zmm7,8\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 fd 49 91 b4 f5 7b 00 00 00[ 	]+vpgatherqq 0x7b\(%ebp,%zmm6,8\)/\(bad\),%zmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 29 91 b4 fd 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%ebp,%ymm7,8\),%xmm6\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 f2 7d 29 91 b4 f5 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%ebp,%ymm6,8\)/\(bad\),%xmm6\{%k1\}
#pass
