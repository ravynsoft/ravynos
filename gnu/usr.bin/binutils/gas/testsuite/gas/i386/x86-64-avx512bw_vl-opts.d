#as:
#objdump: -dw -Msuffix
#name: x86_64 AVX512BW/VL opts insns
#source: x86-64-avx512bw_vl-opts.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 6f f5[ 	]*vmovdqu8 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 6f f5[ 	]*vmovdqu8 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 7f ee[ 	]*vmovdqu8\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 6f f5[ 	]*vmovdqu16 %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 6f f5[ 	]*vmovdqu16 %ymm29,%ymm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7f ee[ 	]*vmovdqu16\.s %ymm29,%ymm30\{%k7\}\{z\}
#pass
