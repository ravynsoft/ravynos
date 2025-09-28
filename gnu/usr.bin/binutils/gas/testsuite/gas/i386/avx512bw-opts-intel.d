#as:
#objdump: -dw -Mintel -Msuffix
#name: i386 AVX512BW opts insns (Intel disassembly)
#source: avx512bw-opts.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 6f f5[ 	]*vmovdqu8 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 7f ee[ 	]*vmovdqu8\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 6f f5[ 	]*vmovdqu8 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 6f f5[ 	]*vmovdqu8 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 6f f5[ 	]*vmovdqu8 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 7f ee[ 	]*vmovdqu8\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 6f f5[ 	]*vmovdqu8 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 6f f5[ 	]*vmovdqu8 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 6f f5[ 	]*vmovdqu16 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 7f ee[ 	]*vmovdqu16\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 6f f5[ 	]*vmovdqu16 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 6f f5[ 	]*vmovdqu16 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 6f f5[ 	]*vmovdqu16 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 7f ee[ 	]*vmovdqu16\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 6f f5[ 	]*vmovdqu16 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 6f f5[ 	]*vmovdqu16 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 6f f5[ 	]*vmovdqu8 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 7f ee[ 	]*vmovdqu8\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 6f f5[ 	]*vmovdqu8 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 6f f5[ 	]*vmovdqu8 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 6f f5[ 	]*vmovdqu8 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 48 7f ee[ 	]*vmovdqu8\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 6f f5[ 	]*vmovdqu8 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 4f 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 6f f5[ 	]*vmovdqu8 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f cf 7f ee[ 	]*vmovdqu8\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 6f f5[ 	]*vmovdqu16 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 7f ee[ 	]*vmovdqu16\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 6f f5[ 	]*vmovdqu16 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 6f f5[ 	]*vmovdqu16 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 6f f5[ 	]*vmovdqu16 zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 48 7f ee[ 	]*vmovdqu16\.s zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 6f f5[ 	]*vmovdqu16 zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 6f f5[ 	]*vmovdqu16 zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7f ee[ 	]*vmovdqu16\.s zmm6\{k7\}\{z\},zmm5
#pass
