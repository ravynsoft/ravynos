#as:
#objdump: -dw
#name: x86_64 AVX512F/GFNI insns
#source: x86-64-avx512f_gfni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 ce b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineqb \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 cf b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 cf f4[ 	]*vgf2p8mulb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 cf f4[ 	]*vgf2p8mulb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 cf f4[ 	]*vgf2p8mulb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 cf b4 f0 23 01 00 00[ 	]*vgf2p8mulb 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 cf 72 7f[ 	]*vgf2p8mulb 0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 ce b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineqb \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 cf b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1fc0\(%rdx\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 cf b2 00 04 00 00 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x400\(%rdx\)\{1to8\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 cf f4[ 	]*vgf2p8mulb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 cf f4[ 	]*vgf2p8mulb %zmm28,%zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 cf f4[ 	]*vgf2p8mulb %zmm28,%zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 cf b4 f0 34 12 00 00[ 	]*vgf2p8mulb 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 cf 72 7f[ 	]*vgf2p8mulb 0x1fc0\(%rdx\),%zmm29,%zmm30
#pass
