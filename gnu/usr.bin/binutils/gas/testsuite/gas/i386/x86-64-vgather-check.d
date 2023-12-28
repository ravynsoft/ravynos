#as: -moperand-check=none
#objdump: -dw
#name: x86-64 vgather check (-moperand-check=none)

.*:     file format .*

Disassembly of section .text:

0+ <vgather>:
[ 	]*[a-f0-9]+:[ 	]+c4 e2 69 92 04 08[ 	]+vgatherdps %xmm2,\(%rax,%xmm1,1\),%xmm0
[ 	]*[a-f0-9]+:[ 	]+c4 e2 69 92 14 48[ 	]+vgatherdps %xmm2/\(bad\),\(%rax,%xmm1,2\),%xmm2/\(bad\)
[ 	]*[a-f0-9]+:[ 	]+c4 62 69 92 14 48[ 	]+vgatherdps %xmm2,\(%rax,%xmm1,2\),%xmm10
[ 	]*[a-f0-9]+:[ 	]+c4 62 29 92 14 48[ 	]+vgatherdps %xmm10/\(bad\),\(%rax,%xmm1,2\),%xmm10/\(bad\)
[ 	]*[a-f0-9]+:[ 	]+c4 e2 71 92 04 88[ 	]+vgatherdps %xmm1/\(bad\),\(%rax,%xmm1,4\)/\(bad\),%xmm0
[ 	]*[a-f0-9]+:[ 	]+c4 e2 31 92 04 88[ 	]+vgatherdps %xmm9,\(%rax,%xmm1,4\),%xmm0
[ 	]*[a-f0-9]+:[ 	]+c4 a2 31 92 04 88[ 	]+vgatherdps %xmm9/\(bad\),\(%rax,%xmm9,4\)/\(bad\),%xmm0
[ 	]*[a-f0-9]+:[ 	]+c4 e2 69 92 0c c8[ 	]+vgatherdps %xmm2,\(%rax,%xmm1,8\)/\(bad\),%xmm1/\(bad\)
[ 	]*[a-f0-9]+:[ 	]+c4 62 69 92 0c c8[ 	]+vgatherdps %xmm2,\(%rax,%xmm1,8\),%xmm9
[ 	]*[a-f0-9]+:[ 	]+c4 22 69 92 0c c8[ 	]+vgatherdps %xmm2,\(%rax,%xmm9,8\)/\(bad\),%xmm9/\(bad\)

[0-9a-f]+ <avx512vgather>:
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 92 84 cd 7b 00 00 00[ 	]+vgatherdpd 0x7b\(%rbp,%ymm17,8\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 92 84 c5 7b 00 00 00[ 	]+vgatherdpd 0x7b\(%rbp,%ymm16,8\)/\(bad\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 92 84 cd 7b 00 00 00[ 	]+vgatherdps 0x7b\(%rbp,%zmm17,8\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 92 84 c5 7b 00 00 00[ 	]+vgatherdps 0x7b\(%rbp,%zmm16,8\)/\(bad\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 93 84 cd 7b 00 00 00[ 	]+vgatherqpd 0x7b\(%rbp,%zmm17,8\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 93 84 c5 7b 00 00 00[ 	]+vgatherqpd 0x7b\(%rbp,%zmm16,8\)/\(bad\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 93 84 cd 7b 00 00 00[ 	]+vgatherqps 0x7b\(%rbp,%zmm17,8\),%ymm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 93 84 c5 7b 00 00 00[ 	]+vgatherqps 0x7b\(%rbp,%zmm16,8\)/\(bad\),%ymm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 90 84 cd 7b 00 00 00[ 	]+vpgatherdd 0x7b\(%rbp,%zmm17,8\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 90 84 c5 7b 00 00 00[ 	]+vpgatherdd 0x7b\(%rbp,%zmm16,8\)/\(bad\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 90 84 cd 7b 00 00 00[ 	]+vpgatherdq 0x7b\(%rbp,%ymm17,8\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 90 84 c5 7b 00 00 00[ 	]+vpgatherdq 0x7b\(%rbp,%ymm16,8\)/\(bad\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 91 84 cd 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%rbp,%zmm17,8\),%ymm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 41 91 84 c5 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%rbp,%zmm16,8\)/\(bad\),%ymm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 91 84 cd 7b 00 00 00[ 	]+vpgatherqq 0x7b\(%rbp,%zmm17,8\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 fd 41 91 84 c5 7b 00 00 00[ 	]+vpgatherqq 0x7b\(%rbp,%zmm16,8\)/\(bad\),%zmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 21 91 84 cd 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%rbp,%ymm17,8\),%xmm16\{%k1\}
[ 	]+[a-f0-9]+:[ 	]+62 e2 7d 21 91 84 c5 7b 00 00 00[ 	]+vpgatherqd 0x7b\(%rbp,%ymm16,8\)/\(bad\),%xmm16\{%k1\}
#pass
