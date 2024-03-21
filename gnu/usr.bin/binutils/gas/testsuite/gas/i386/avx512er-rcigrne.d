#as: -mevexrcig=rne
#objdump: -dw
#name: i386 AVX512ER rcig insns
#source: avx512er-rcig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 18 c8 f5[ 	]*vexp2ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 18 c8 f5[ 	]*vexp2pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 18 ca f5[ 	]*vrcp28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 18 ca f5[ 	]*vrcp28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f cb f4[ 	]*vrcp28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f cb f4[ 	]*vrcp28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 18 cc f5[ 	]*vrsqrt28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 18 cc f5[ 	]*vrsqrt28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f cd f4[ 	]*vrsqrt28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f cd f4[ 	]*vrsqrt28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 18 c8 f5[ 	]*vexp2ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 18 c8 f5[ 	]*vexp2pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 18 ca f5[ 	]*vrcp28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 18 ca f5[ 	]*vrcp28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f cb f4[ 	]*vrcp28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f cb f4[ 	]*vrcp28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 18 cc f5[ 	]*vrsqrt28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 18 cc f5[ 	]*vrsqrt28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f cd f4[ 	]*vrsqrt28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f cd f4[ 	]*vrsqrt28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
#pass
