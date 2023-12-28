#as:
#objdump: -dw -Msuffix
#name: i386 AVX512BW/VL opts insns
#source: avx512bw_vl-opts.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 7f ee[ 	]*vmovdqu8\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 7f ee[ 	]*vmovdqu8\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7f ee[ 	]*vmovdqu16\.s %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 %ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7f ee[ 	]*vmovdqu16\.s %ymm5,%ymm6\{%k7\}\{z\}
#pass
