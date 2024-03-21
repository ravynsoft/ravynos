#as:
#objdump: -dw
#name: x86_64 AVX/GFNI insns
#source: x86-64-avx_gfni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf f4[ 	]*vgf2p8mulb %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 a2 55 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf 72 7e[ 	]*vgf2p8mulb 0x7e\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce 72 7e 7b[ 	]*vgf2p8affineqb \$0x7b,0x7e\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf 72 7e 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7e\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf f4[ 	]*vgf2p8mulb %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 a2 51 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf 72 7e[ 	]*vgf2p8mulb 0x7e\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce 72 7e 7b[ 	]*vgf2p8affineqb \$0x7b,0x7e\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf 72 7e 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7e\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf f4[ 	]*vgf2p8mulb %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 a2 55 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf 72 7e[ 	]*vgf2p8mulb 0x7e\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce 72 7e 7b[ 	]*vgf2p8affineqb \$0x7b,0x7e\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf 72 7e 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7e\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf f4[ 	]*vgf2p8mulb %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 a2 51 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb -0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf 72 7e[ 	]*vgf2p8mulb 0x7e\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce f4 ab[ 	]*vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce 72 7e 7b[ 	]*vgf2p8affineqb \$0x7b,0x7e\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf f4 ab[ 	]*vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf 72 7e 7b[ 	]*vgf2p8affineinvqb \$0x7b,0x7e\(%rdx\),%xmm5,%xmm6
#pass
