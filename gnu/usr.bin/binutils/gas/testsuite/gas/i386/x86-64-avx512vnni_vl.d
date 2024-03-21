#as:
#objdump: -dw
#name: x86_64 AVX512VNNI/VL insns
#source: x86-64-avx512vnni_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 00 52 d4[ 	]*vpdpwssd %xmm20,%xmm22,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 03 52 d4[ 	]*vpdpwssd %xmm20,%xmm22,%xmm26\{%k3\}
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 83 52 d4[ 	]*vpdpwssd %xmm20,%xmm22,%xmm26\{%k3\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 4d 00 52 94 f0 23 01 00 00[ 	]*vpdpwssd 0x123\(%rax,%r14,8\),%xmm22,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 62 4d 00 52 52 7f[ 	]*vpdpwssd 0x7f0\(%rdx\),%xmm22,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 62 4d 10 52 52 7f[ 	]*vpdpwssd 0x1fc\(%rdx\)\{1to4\},%xmm22,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 20 52 e2[ 	]*vpdpwssd %ymm18,%ymm20,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 25 52 e2[ 	]*vpdpwssd %ymm18,%ymm20,%ymm20\{%k5\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d a5 52 e2[ 	]*vpdpwssd %ymm18,%ymm20,%ymm20\{%k5\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 20 52 a4 f0 23 01 00 00[ 	]*vpdpwssd 0x123\(%rax,%r14,8\),%ymm20,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 20 52 62 7f[ 	]*vpdpwssd 0xfe0\(%rdx\),%ymm20,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 30 52 62 7f[ 	]*vpdpwssd 0x1fc\(%rdx\)\{1to8\},%ymm20,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 00 53 f7[ 	]*vpdpwssds %xmm23,%xmm19,%xmm22
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 07 53 f7[ 	]*vpdpwssds %xmm23,%xmm19,%xmm22\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 87 53 f7[ 	]*vpdpwssds %xmm23,%xmm19,%xmm22\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 65 00 53 b4 f0 23 01 00 00[ 	]*vpdpwssds 0x123\(%rax,%r14,8\),%xmm19,%xmm22
[ 	]*[a-f0-9]+:[ 	]*62 e2 65 00 53 72 7f[ 	]*vpdpwssds 0x7f0\(%rdx\),%xmm19,%xmm22
[ 	]*[a-f0-9]+:[ 	]*62 e2 65 10 53 72 7f[ 	]*vpdpwssds 0x1fc\(%rdx\)\{1to4\},%xmm19,%xmm22
[ 	]*[a-f0-9]+:[ 	]*62 82 45 20 53 fc[ 	]*vpdpwssds %ymm28,%ymm23,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 82 45 23 53 fc[ 	]*vpdpwssds %ymm28,%ymm23,%ymm23\{%k3\}
[ 	]*[a-f0-9]+:[ 	]*62 82 45 a3 53 fc[ 	]*vpdpwssds %ymm28,%ymm23,%ymm23\{%k3\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 45 20 53 bc f0 23 01 00 00[ 	]*vpdpwssds 0x123\(%rax,%r14,8\),%ymm23,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 e2 45 20 53 7a 7f[ 	]*vpdpwssds 0xfe0\(%rdx\),%ymm23,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 e2 45 30 53 7a 7f[ 	]*vpdpwssds 0x1fc\(%rdx\)\{1to8\},%ymm23,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 82 15 00 50 d4[ 	]*vpdpbusd %xmm28,%xmm29,%xmm18
[ 	]*[a-f0-9]+:[ 	]*62 82 15 03 50 d4[ 	]*vpdpbusd %xmm28,%xmm29,%xmm18\{%k3\}
[ 	]*[a-f0-9]+:[ 	]*62 82 15 83 50 d4[ 	]*vpdpbusd %xmm28,%xmm29,%xmm18\{%k3\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 15 00 50 94 f0 23 01 00 00[ 	]*vpdpbusd 0x123\(%rax,%r14,8\),%xmm29,%xmm18
[ 	]*[a-f0-9]+:[ 	]*62 e2 15 00 50 52 7f[ 	]*vpdpbusd 0x7f0\(%rdx\),%xmm29,%xmm18
[ 	]*[a-f0-9]+:[ 	]*62 e2 15 10 50 52 7f[ 	]*vpdpbusd 0x1fc\(%rdx\)\{1to4\},%xmm29,%xmm18
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 20 50 e1[ 	]*vpdpbusd %ymm17,%ymm18,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 22 50 e1[ 	]*vpdpbusd %ymm17,%ymm18,%ymm20\{%k2\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d a2 50 e1[ 	]*vpdpbusd %ymm17,%ymm18,%ymm20\{%k2\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 20 50 a4 f0 23 01 00 00[ 	]*vpdpbusd 0x123\(%rax,%r14,8\),%ymm18,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 20 50 62 7f[ 	]*vpdpbusd 0xfe0\(%rdx\),%ymm18,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 30 50 62 7f[ 	]*vpdpbusd 0x1fc\(%rdx\)\{1to8\},%ymm18,%ymm20
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 00 51 c3[ 	]*vpdpbusds %xmm27,%xmm26,%xmm24
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 04 51 c3[ 	]*vpdpbusds %xmm27,%xmm26,%xmm24\{%k4\}
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 84 51 c3[ 	]*vpdpbusds %xmm27,%xmm26,%xmm24\{%k4\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 2d 00 51 84 f0 23 01 00 00[ 	]*vpdpbusds 0x123\(%rax,%r14,8\),%xmm26,%xmm24
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 00 51 42 7f[ 	]*vpdpbusds 0x7f0\(%rdx\),%xmm26,%xmm24
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 10 51 42 7f[ 	]*vpdpbusds 0x1fc\(%rdx\)\{1to4\},%xmm26,%xmm24
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 51 f1[ 	]*vpdpbusds %ymm25,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 21 51 f1[ 	]*vpdpbusds %ymm25,%ymm29,%ymm30\{%k1\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a1 51 f1[ 	]*vpdpbusds %ymm25,%ymm29,%ymm30\{%k1\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 51 b4 f0 23 01 00 00[ 	]*vpdpbusds 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 51 72 7f[ 	]*vpdpbusds 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 51 72 7f[ 	]*vpdpbusds 0x1fc\(%rdx\)\{1to8\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 00 52 ef[ 	]*vpdpwssd %xmm23,%xmm20,%xmm21
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 06 52 ef[ 	]*vpdpwssd %xmm23,%xmm20,%xmm21\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 86 52 ef[ 	]*vpdpwssd %xmm23,%xmm20,%xmm21\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 5d 00 52 ac f0 34 12 00 00[ 	]*vpdpwssd 0x1234\(%rax,%r14,8\),%xmm20,%xmm21
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 00 52 6a 7f[ 	]*vpdpwssd 0x7f0\(%rdx\),%xmm20,%xmm21
[ 	]*[a-f0-9]+:[ 	]*62 e2 5d 10 52 6a 7f[ 	]*vpdpwssd 0x1fc\(%rdx\)\{1to4\},%xmm20,%xmm21
[ 	]*[a-f0-9]+:[ 	]*62 22 25 20 52 c9[ 	]*vpdpwssd %ymm17,%ymm27,%ymm25
[ 	]*[a-f0-9]+:[ 	]*62 22 25 26 52 c9[ 	]*vpdpwssd %ymm17,%ymm27,%ymm25\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 22 25 a6 52 c9[ 	]*vpdpwssd %ymm17,%ymm27,%ymm25\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 25 20 52 8c f0 34 12 00 00[ 	]*vpdpwssd 0x1234\(%rax,%r14,8\),%ymm27,%ymm25
[ 	]*[a-f0-9]+:[ 	]*62 62 25 20 52 4a 7f[ 	]*vpdpwssd 0xfe0\(%rdx\),%ymm27,%ymm25
[ 	]*[a-f0-9]+:[ 	]*62 62 25 30 52 4a 7f[ 	]*vpdpwssd 0x1fc\(%rdx\)\{1to8\},%ymm27,%ymm25
[ 	]*[a-f0-9]+:[ 	]*62 22 35 00 53 f5[ 	]*vpdpwssds %xmm21,%xmm25,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 35 06 53 f5[ 	]*vpdpwssds %xmm21,%xmm25,%xmm30\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 22 35 86 53 f5[ 	]*vpdpwssds %xmm21,%xmm25,%xmm30\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 35 00 53 b4 f0 34 12 00 00[ 	]*vpdpwssds 0x1234\(%rax,%r14,8\),%xmm25,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 35 00 53 72 7f[ 	]*vpdpwssds 0x7f0\(%rdx\),%xmm25,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 35 10 53 72 7f[ 	]*vpdpwssds 0x1fc\(%rdx\)\{1to4\},%xmm25,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 25 20 53 e3[ 	]*vpdpwssds %ymm27,%ymm27,%ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 25 27 53 e3[ 	]*vpdpwssds %ymm27,%ymm27,%ymm28\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 25 a7 53 e3[ 	]*vpdpwssds %ymm27,%ymm27,%ymm28\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 25 20 53 a4 f0 34 12 00 00[ 	]*vpdpwssds 0x1234\(%rax,%r14,8\),%ymm27,%ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 25 20 53 62 7f[ 	]*vpdpwssds 0xfe0\(%rdx\),%ymm27,%ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 25 30 53 62 7f[ 	]*vpdpwssds 0x1fc\(%rdx\)\{1to8\},%ymm27,%ymm28
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 00 50 d3[ 	]*vpdpbusd %xmm19,%xmm18,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 06 50 d3[ 	]*vpdpbusd %xmm19,%xmm18,%xmm26\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 86 50 d3[ 	]*vpdpbusd %xmm19,%xmm18,%xmm26\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 6d 00 50 94 f0 34 12 00 00[ 	]*vpdpbusd 0x1234\(%rax,%r14,8\),%xmm18,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 62 6d 00 50 52 7f[ 	]*vpdpbusd 0x7f0\(%rdx\),%xmm18,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 62 6d 10 50 52 7f[ 	]*vpdpbusd 0x1fc\(%rdx\)\{1to4\},%xmm18,%xmm26
[ 	]*[a-f0-9]+:[ 	]*62 82 75 20 50 eb[ 	]*vpdpbusd %ymm27,%ymm17,%ymm21
[ 	]*[a-f0-9]+:[ 	]*62 82 75 22 50 eb[ 	]*vpdpbusd %ymm27,%ymm17,%ymm21\{%k2\}
[ 	]*[a-f0-9]+:[ 	]*62 82 75 a2 50 eb[ 	]*vpdpbusd %ymm27,%ymm17,%ymm21\{%k2\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 75 20 50 ac f0 34 12 00 00[ 	]*vpdpbusd 0x1234\(%rax,%r14,8\),%ymm17,%ymm21
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 20 50 6a 7f[ 	]*vpdpbusd 0xfe0\(%rdx\),%ymm17,%ymm21
[ 	]*[a-f0-9]+:[ 	]*62 e2 75 30 50 6a 7f[ 	]*vpdpbusd 0x1fc\(%rdx\)\{1to8\},%ymm17,%ymm21
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 00 51 e0[ 	]*vpdpbusds %xmm24,%xmm26,%xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 01 51 e0[ 	]*vpdpbusds %xmm24,%xmm26,%xmm28\{%k1\}
[ 	]*[a-f0-9]+:[ 	]*62 02 2d 81 51 e0[ 	]*vpdpbusds %xmm24,%xmm26,%xmm28\{%k1\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 2d 00 51 a4 f0 34 12 00 00[ 	]*vpdpbusds 0x1234\(%rax,%r14,8\),%xmm26,%xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 00 51 62 7f[ 	]*vpdpbusds 0x7f0\(%rdx\),%xmm26,%xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 2d 10 51 62 7f[ 	]*vpdpbusds 0x1fc\(%rdx\)\{1to4\},%xmm26,%xmm28
[ 	]*[a-f0-9]+:[ 	]*62 82 6d 20 51 fb[ 	]*vpdpbusds %ymm27,%ymm18,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 82 6d 26 51 fb[ 	]*vpdpbusds %ymm27,%ymm18,%ymm23\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 82 6d a6 51 fb[ 	]*vpdpbusds %ymm27,%ymm18,%ymm23\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 20 51 bc f0 34 12 00 00[ 	]*vpdpbusds 0x1234\(%rax,%r14,8\),%ymm18,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 20 51 7a 7f[ 	]*vpdpbusds 0xfe0\(%rdx\),%ymm18,%ymm23
[ 	]*[a-f0-9]+:[ 	]*62 e2 6d 30 51 7a 7f[ 	]*vpdpbusds 0x1fc\(%rdx\)\{1to8\},%ymm18,%ymm23
#pass
