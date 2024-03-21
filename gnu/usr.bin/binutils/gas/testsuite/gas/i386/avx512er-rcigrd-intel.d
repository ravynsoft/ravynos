#as: -mevexrcig=rd
#objdump: -dw -Mintel
#name: i386 AVX512ER rcig insns (Intel disassembly)
#source: avx512er-rcig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 38 c8 f5[ 	]*vexp2ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 38 c8 f5[ 	]*vexp2pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 38 ca f5[ 	]*vrcp28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 38 ca f5[ 	]*vrcp28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f cb f4[ 	]*vrcp28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f cb f4[ 	]*vrcp28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 38 cc f5[ 	]*vrsqrt28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 38 cc f5[ 	]*vrsqrt28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f cd f4[ 	]*vrsqrt28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f cd f4[ 	]*vrsqrt28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 38 c8 f5[ 	]*vexp2ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 38 c8 f5[ 	]*vexp2pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 38 ca f5[ 	]*vrcp28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 38 ca f5[ 	]*vrcp28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f cb f4[ 	]*vrcp28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f cb f4[ 	]*vrcp28sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 38 cc f5[ 	]*vrsqrt28ps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 38 cc f5[ 	]*vrsqrt28pd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f cd f4[ 	]*vrsqrt28ss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f cd f4[ 	]*vrsqrt28sd xmm6\{k7\},xmm5,xmm4\{sae\}
#pass
