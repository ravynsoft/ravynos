#as:
#objdump: -dw
#name: i386 AVX512F/GFNI insns
#source: avx512f_gfni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf f4[ 	]*vgf2p8mulb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f cf f4[ 	]*vgf2p8mulb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf cf f4[ 	]*vgf2p8mulb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf 72 7f[ 	]*vgf2p8mulb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 ce 72 7f 7b[ 	]*vgf2p8affineqb \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 cf 72 7f 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf f4[ 	]*vgf2p8mulb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f cf f4[ 	]*vgf2p8mulb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf cf f4[ 	]*vgf2p8mulb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf 72 7f[ 	]*vgf2p8mulb 0x1fc0\(%edx\),%zmm5,%zmm6
#pass
