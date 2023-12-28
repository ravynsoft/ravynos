#as:
#objdump: -dw
#name: i386 AVX512VL/GFNI insns
#source: avx512vl_gfni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce f4 7b[ 	]*vgf2p8affineqb \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce f4 7b[ 	]*vgf2p8affineqb \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf f4 7b[ 	]*vgf2p8affineinvqb \$0x7b,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf f4 7b[ 	]*vgf2p8affineinvqb \$0x7b,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf f4[ 	]*vgf2p8mulb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f cf f4[ 	]*vgf2p8mulb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf 72 7f[ 	]*vgf2p8mulb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf f4[ 	]*vgf2p8mulb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af cf f4[ 	]*vgf2p8mulb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf 72 7f[ 	]*vgf2p8mulb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f cf 30 7b[ 	]*vgf2p8affineinvqb \$0x7b,\(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf f4[ 	]*vgf2p8mulb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f cf f4[ 	]*vgf2p8mulb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf 72 7f[ 	]*vgf2p8mulb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf f4[ 	]*vgf2p8mulb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af cf f4[ 	]*vgf2p8mulb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf 72 7f[ 	]*vgf2p8mulb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
#pass
