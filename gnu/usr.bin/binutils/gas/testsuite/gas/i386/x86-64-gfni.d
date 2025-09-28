#as:
#objdump: -dw
#name: x86_64 GFNI insns
#source: x86-64-gfni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ec[ 	]*gf2p8mulb %xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 42 0f 38 cf ac f0 c0 1d fe ff[ 	]*gf2p8mulb -0x1e240\(%rax,%r14,8\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf aa f0 07 00 00[ 	]*gf2p8mulb 0x7f0\(%rdx\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce ec ab[ 	]*gf2p8affineqb \$0xab,%xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 42 0f 3a ce ac f0 c0 1d fe ff 7b[ 	]*gf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce aa f0 07 00 00 7b[ 	]*gf2p8affineqb \$0x7b,0x7f0\(%rdx\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf ec ab[ 	]*gf2p8affineinvqb \$0xab,%xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 42 0f 3a cf ac f0 c0 1d fe ff 7b[ 	]*gf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf aa f0 07 00 00 7b[ 	]*gf2p8affineinvqb \$0x7b,0x7f0\(%rdx\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ec[ 	]*gf2p8mulb %xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 42 0f 38 cf ac f0 c0 1d fe ff[ 	]*gf2p8mulb -0x1e240\(%rax,%r14,8\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf aa f0 07 00 00[ 	]*gf2p8mulb 0x7f0\(%rdx\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce ec ab[ 	]*gf2p8affineqb \$0xab,%xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 42 0f 3a ce ac f0 c0 1d fe ff 7b[ 	]*gf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce aa f0 07 00 00 7b[ 	]*gf2p8affineqb \$0x7b,0x7f0\(%rdx\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf ec ab[ 	]*gf2p8affineinvqb \$0xab,%xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 42 0f 3a cf ac f0 c0 1d fe ff 7b[ 	]*gf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf aa f0 07 00 00 7b[ 	]*gf2p8affineinvqb \$0x7b,0x7f0\(%rdx\),%xmm5
#pass
