#as:
#objdump: -dw
#name: x86_64 AVX512VL/GFNI insns
#source: x86-64-avx512vl_gfni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 ce b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineqb \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 ce b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineqb \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 cf b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 cf b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 cf f4[ 	]*vgf2p8mulb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 cf f4[ 	]*vgf2p8mulb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 cf f4[ 	]*vgf2p8mulb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 cf b4 f0 23 01 00 00[ 	]*vgf2p8mulb 0x123\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 cf 72 7f[ 	]*vgf2p8mulb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 cf f4[ 	]*vgf2p8mulb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 cf f4[ 	]*vgf2p8mulb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 cf f4[ 	]*vgf2p8mulb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 cf b4 f0 23 01 00 00[ 	]*vgf2p8mulb 0x123\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 cf 72 7f[ 	]*vgf2p8mulb 0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 ce b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineqb \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 ce b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineqb \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 cf b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%rdx\)\{1to2\},%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 cf b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0xfe0\(%rdx\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 cf b2 00 04 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x400\(%rdx\)\{1to4\},%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 cf f4[ 	]*vgf2p8mulb %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 cf f4[ 	]*vgf2p8mulb %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 cf f4[ 	]*vgf2p8mulb %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 cf b4 f0 34 12 00 00[ 	]*vgf2p8mulb 0x1234\(%rax,%r14,8\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 cf 72 7f[ 	]*vgf2p8mulb 0x7f0\(%rdx\),%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 cf f4[ 	]*vgf2p8mulb %ymm28,%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 cf f4[ 	]*vgf2p8mulb %ymm28,%ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 cf f4[ 	]*vgf2p8mulb %ymm28,%ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 cf b4 f0 34 12 00 00[ 	]*vgf2p8mulb 0x1234\(%rax,%r14,8\),%ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 cf 72 7f[ 	]*vgf2p8mulb 0xfe0\(%rdx\),%ymm29,%ymm30
#pass
