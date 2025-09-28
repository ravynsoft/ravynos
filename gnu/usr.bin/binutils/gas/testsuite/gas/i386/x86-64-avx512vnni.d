#as:
#objdump: -dw
#name: x86_64 AVX512VNNI insns
#source: x86-64-avx512vnni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 40 52 d1[ 	]*vpdpwssd %zmm17,%zmm18,%zmm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 45 52 d1[ 	]*vpdpwssd %zmm17,%zmm18,%zmm18\{%k5\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d c5 52 d1[ 	]*vpdpwssd %zmm17,%zmm18,%zmm18\{%k5\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 40 52 94 f0 23 01 00 00[ 	]*vpdpwssd 0x123\(%rax,%r14,8\),%zmm18,%zmm18
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 40 52 52 7f[ 	]*vpdpwssd 0x1fc0\(%rdx\),%zmm18,%zmm18
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 50 52 52 7f[ 	]*vpdpwssd 0x1fc\(%rdx\)\{1to16\},%zmm18,%zmm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 53 e9[ 	]*vpdpwssds %zmm17,%zmm21,%zmm21
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 44 53 e9[ 	]*vpdpwssds %zmm17,%zmm21,%zmm21\{%k4\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 c4 53 e9[ 	]*vpdpwssds %zmm17,%zmm21,%zmm21\{%k4\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 53 ac f0 23 01 00 00[ 	]*vpdpwssds 0x123\(%rax,%r14,8\),%zmm21,%zmm21
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 40 53 6a 7f[ 	]*vpdpwssds 0x1fc0\(%rdx\),%zmm21,%zmm21
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 50 53 6a 7f[ 	]*vpdpwssds 0x1fc\(%rdx\)\{1to16\},%zmm21,%zmm21
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 50 fa[ 	]*vpdpbusd %zmm18,%zmm21,%zmm23
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 44 50 fa[ 	]*vpdpbusd %zmm18,%zmm21,%zmm23\{%k4\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 c4 50 fa[ 	]*vpdpbusd %zmm18,%zmm21,%zmm23\{%k4\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 55 40 50 bc f0 23 01 00 00[ 	]*vpdpbusd 0x123\(%rax,%r14,8\),%zmm21,%zmm23
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 40 50 7a 7f[ 	]*vpdpbusd 0x1fc0\(%rdx\),%zmm21,%zmm23
[ 	]*[a-f0-9]+:[ 	]*62 e2 55 50 50 7a 7f[ 	]*vpdpbusd 0x1fc\(%rdx\)\{1to16\},%zmm21,%zmm23
[ 	]*[a-f0-9]+:[ 	]*62 02 3d 40 51 c1[ 	]*vpdpbusds %zmm25,%zmm24,%zmm24
[ 	]*[a-f0-9]+:[ 	]*62 02 3d 47 51 c1[ 	]*vpdpbusds %zmm25,%zmm24,%zmm24\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 3d c7 51 c1[ 	]*vpdpbusds %zmm25,%zmm24,%zmm24\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 40 51 84 f0 23 01 00 00[ 	]*vpdpbusds 0x123\(%rax,%r14,8\),%zmm24,%zmm24
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 40 51 42 7f[ 	]*vpdpbusds 0x1fc0\(%rdx\),%zmm24,%zmm24
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 50 51 42 7f[ 	]*vpdpbusds 0x1fc\(%rdx\)\{1to16\},%zmm24,%zmm24
[ 	]*[a-f0-9]+:[ 	]*62 22 25 40 52 e1[ 	]*vpdpwssd %zmm17,%zmm27,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 25 47 52 e1[ 	]*vpdpwssd %zmm17,%zmm27,%zmm28\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 22 25 c7 52 e1[ 	]*vpdpwssd %zmm17,%zmm27,%zmm28\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 25 40 52 a4 f0 34 12 00 00[ 	]*vpdpwssd 0x1234\(%rax,%r14,8\),%zmm27,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 25 40 52 62 7f[ 	]*vpdpwssd 0x1fc0\(%rdx\),%zmm27,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 25 50 52 62 7f[ 	]*vpdpwssd 0x1fc\(%rdx\)\{1to16\},%zmm27,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 1d 40 53 e9[ 	]*vpdpwssds %zmm17,%zmm28,%zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 1d 43 53 e9[ 	]*vpdpwssds %zmm17,%zmm28,%zmm29\{%k3\}
[ 	]*[a-f0-9]+:[ 	]*62 22 1d c3 53 e9[ 	]*vpdpwssds %zmm17,%zmm28,%zmm29\{%k3\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 1d 40 53 ac f0 34 12 00 00[ 	]*vpdpwssds 0x1234\(%rax,%r14,8\),%zmm28,%zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 1d 40 53 6a 7f[ 	]*vpdpwssds 0x1fc0\(%rdx\),%zmm28,%zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 1d 50 53 6a 7f[ 	]*vpdpwssds 0x1fc\(%rdx\)\{1to16\},%zmm28,%zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 40 50 e5[ 	]*vpdpbusd %zmm21,%zmm24,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 46 50 e5[ 	]*vpdpbusd %zmm21,%zmm24,%zmm28\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 22 3d c6 50 e5[ 	]*vpdpbusd %zmm21,%zmm24,%zmm28\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 3d 40 50 a4 f0 34 12 00 00[ 	]*vpdpbusd 0x1234\(%rax,%r14,8\),%zmm24,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 40 50 62 7f[ 	]*vpdpbusd 0x1fc0\(%rdx\),%zmm24,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 3d 50 50 62 7f[ 	]*vpdpbusd 0x1fc\(%rdx\)\{1to16\},%zmm24,%zmm28
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 40 51 e4[ 	]*vpdpbusds %zmm20,%zmm17,%zmm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 42 51 e4[ 	]*vpdpbusds %zmm20,%zmm17,%zmm20\{%k2\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 c2 51 e4[ 	]*vpdpbusds %zmm20,%zmm17,%zmm20\{%k2\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 40 51 a4 f0 34 12 00 00[ 	]*vpdpbusds 0x1234\(%rax,%r14,8\),%zmm17,%zmm20
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 40 51 62 7f[ 	]*vpdpbusds 0x1fc0\(%rdx\),%zmm17,%zmm20
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 50 51 62 7f[ 	]*vpdpbusds 0x1fc\(%rdx\)\{1to16\},%zmm17,%zmm20
#pass
