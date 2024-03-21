#objdump: -dw
#name: i386 arch 14-1

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 58 f4[ 	]*vaddpd %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 31[ 	]*vbroadcastf32x8 \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 c4 f5[ 	]*vpconflictd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1c f5[ 	]*vpabsb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 58 f4[ 	]*vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 57 48 72 f4[ 	]*vcvtne2ps2bf16 %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 31[ 	]*vpcompressb %zmm6,\(%ecx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 e3[ 	]*vpdpwssd %zmm3,%zmm1,%zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ec[ 	]*vpshufbitqmb %zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ec[ 	]*gf2p8mulb %xmm4,%xmm5
#pass
