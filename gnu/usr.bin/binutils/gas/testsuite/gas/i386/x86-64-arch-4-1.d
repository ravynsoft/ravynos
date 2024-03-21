#objdump: -dw
#name: x86-64 arch 4-1

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 58 f4[ 	]*vaddpd %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 31[ 	]*vbroadcastf32x8 \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b4 f4[ 	]*vpmadd52luq %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 c4 f5[ 	]*vpconflictd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 1c f5[ 	]*vpabsb %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 58 f4[ 	]*vaddpd %xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 17 40 72 f4[ 	]*vcvtne2ps2bf16 %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 8d f4[ 	]*vpermb %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 63 31[ 	]*vpcompressb %zmm30,\(%rcx\)\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 a2 6d 40 52 d1[ 	]*vpdpwssd %zmm17,%zmm18,%zmm18
[ 	]*[a-f0-9]+:[ 	]*62 92 15 40 8f ec[ 	]*vpshufbitqmb %zmm28,%zmm29,%k5
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ec[ 	]*gf2p8mulb %xmm4,%xmm5
[ 	]*[a-f0-9]+:[ 	]*f3 0f 01 fd[ 	]*rmpquery
#pass
