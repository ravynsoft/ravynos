#as: -mevexwig=1
#objdump: -dw
#name: i386 AVX512 wig insns
#source: evex-wig.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f1 fe 08 2a c0    	\{evex\} vcvtsi2ss %eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 2a 40 01 	\{evex\} vcvtsi2ss 0x4\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 2a c0    	\{evex\} vcvtsi2sd %eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 2a 40 01 	\{evex\} vcvtsi2sd 0x4\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 2d c0    	\{evex\} vcvtss2si %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 ff 08 2d c0    	\{evex\} vcvtsd2si %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 fe 08 2c c0    	\{evex\} vcvttss2si %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 ff 08 2c c0    	\{evex\} vcvttsd2si %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 fe 08 7b c0    	vcvtusi2ss %eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 7b 40 01 	vcvtusi2ss 0x4\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 7b c0    	vcvtusi2sd %eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 ff 08 7b 40 01 	vcvtusi2sd 0x4\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fe 08 79 c0    	vcvtss2usi %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 ff 08 79 c0    	vcvtsd2usi %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 fe 08 78 c0    	vcvttss2usi %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 ff 08 78 c0    	vcvttsd2usi %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 17 c0 00 	\{evex\} vextractps \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 17 40 01 00 	\{evex\} vextractps \$0x0,%xmm0,0x4\(%eax\)
[ 	]*[a-f0-9]+:	62 f1 fd 08 6e c0    	\{evex\} vmovd %eax,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 6e 40 01 	\{evex\} vmovd 0x4\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 7e c0    	\{evex\} vmovd %xmm0,%eax
[ 	]*[a-f0-9]+:	62 f1 fd 08 7e 40 01 	\{evex\} vmovd %xmm0,0x4\(%eax\)
[ 	]*[a-f0-9]+:	62 f2 fd 08 7c c0    	vpbroadcastd %eax,%xmm0
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 c0 00 	\{evex\} vpextrb \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 14 40 01 00 	\{evex\} vpextrb \$0x0,%xmm0,0x1\(%eax\)
[ 	]*[a-f0-9]+:	62 f3 fd 08 16 c0 00 	\{evex\} vpextrd \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 16 40 01 00 	\{evex\} vpextrd \$0x0,%xmm0,0x4\(%eax\)
[ 	]*[a-f0-9]+:	62 f1 fd 08 c5 c0 00 	\{evex\} vpextrw \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 c0 00 	\{evex\} vpextrw \$0x0,%xmm0,%eax
[ 	]*[a-f0-9]+:	62 f3 fd 08 15 40 01 00 	\{evex\} vpextrw \$0x0,%xmm0,0x2\(%eax\)
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 c0 00 	\{evex\} vpinsrb \$0x0,%eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f3 fd 08 20 40 01 00 	\{evex\} vpinsrb \$0x0,0x1\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f3 fd 08 22 c0 00 	\{evex\} vpinsrd \$0x0,%eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f3 fd 08 22 40 01 00 	\{evex\} vpinsrd \$0x0,0x4\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 c0 00 	\{evex\} vpinsrw \$0x0,%eax,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 fd 08 c4 40 01 00 	\{evex\} vpinsrw \$0x0,0x2\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 7e 0f 10 c0    	vmovss %xmm0,%xmm0,%xmm0\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 0f 10 00    	vmovss \(%eax\),%xmm0\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 7e 0f 11 00    	vmovss %xmm0,\(%eax\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 0f 10 c0    	vmovsd %xmm0,%xmm0,%xmm0\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 0f 10 00    	vmovsd \(%eax\),%xmm0\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 ff 0f 11 00    	vmovsd %xmm0,\(%eax\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f5 7e 0f 10 c0    	vmovsh %xmm0,%xmm0,%xmm0\{%k7\}
[ 	]*[a-f0-9]+:	62 f5 7e 0f 10 00    	vmovsh \(%eax\),%xmm0\{%k7\}
[ 	]*[a-f0-9]+:	62 f5 7e 0f 11 00    	vmovsh %xmm0,\(%eax\)\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 f5    	vpmovsxbd %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 21 f5    	vpmovsxbd %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 31    	vpmovsxbd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b4 f4 c0 1d fe ff 	vpmovsxbd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 7f 	vpmovsxbd 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 00 08 00 00 	vpmovsxbd 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 80 	vpmovsxbd -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 f5    	vpmovsxbq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 22 f5    	vpmovsxbq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 31    	vpmovsxbq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b4 f4 c0 1d fe ff 	vpmovsxbq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 7f 	vpmovsxbq 0x3f8\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 00 04 00 00 	vpmovsxbq 0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 80 	vpmovsxbq -0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq -0x408\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 f5    	vpmovsxwd %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 23 f5    	vpmovsxwd %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 31    	vpmovsxwd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b4 f4 c0 1d fe ff 	vpmovsxwd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 7f 	vpmovsxwd 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 00 10 00 00 	vpmovsxwd 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 80 	vpmovsxwd -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 f5    	vpmovsxwq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 24 f5    	vpmovsxwq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 31    	vpmovsxwq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b4 f4 c0 1d fe ff 	vpmovsxwq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 7f 	vpmovsxwq 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 00 08 00 00 	vpmovsxwq 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 80 	vpmovsxwq -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 f5    	vpmovzxbd %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 31 f5    	vpmovzxbd %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 31    	vpmovzxbd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b4 f4 c0 1d fe ff 	vpmovzxbd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 7f 	vpmovzxbd 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 00 08 00 00 	vpmovzxbd 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 80 	vpmovzxbd -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 f5    	vpmovzxbq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 32 f5    	vpmovzxbq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 31    	vpmovzxbq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b4 f4 c0 1d fe ff 	vpmovzxbq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 7f 	vpmovzxbq 0x3f8\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 00 04 00 00 	vpmovzxbq 0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 80 	vpmovzxbq -0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq -0x408\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 f5    	vpmovzxwd %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 33 f5    	vpmovzxwd %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 31    	vpmovzxwd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b4 f4 c0 1d fe ff 	vpmovzxwd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 7f 	vpmovzxwd 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 00 10 00 00 	vpmovzxwd 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 80 	vpmovzxwd -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 f5    	vpmovzxwq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 34 f5    	vpmovzxwq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 31    	vpmovzxwq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b4 f4 c0 1d fe ff 	vpmovzxwq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 7f 	vpmovzxwq 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 00 08 00 00 	vpmovzxwq 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 80 	vpmovzxwq -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 f5    	vpmovsxbd %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 21 f5    	vpmovsxbd %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 31    	vpmovsxbd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b4 f4 c0 1d fe ff 	vpmovsxbd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 7f 	vpmovsxbd 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 00 08 00 00 	vpmovsxbd 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 72 80 	vpmovsxbd -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 21 b2 f0 f7 ff ff 	vpmovsxbd -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 f5    	vpmovsxbq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 22 f5    	vpmovsxbq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 31    	vpmovsxbq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b4 f4 c0 1d fe ff 	vpmovsxbq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 7f 	vpmovsxbq 0x3f8\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 00 04 00 00 	vpmovsxbq 0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 72 80 	vpmovsxbq -0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 22 b2 f8 fb ff ff 	vpmovsxbq -0x408\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 f5    	vpmovsxwd %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 23 f5    	vpmovsxwd %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 31    	vpmovsxwd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b4 f4 c0 1d fe ff 	vpmovsxwd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 7f 	vpmovsxwd 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 00 10 00 00 	vpmovsxwd 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 72 80 	vpmovsxwd -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 23 b2 e0 ef ff ff 	vpmovsxwd -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 f5    	vpmovsxwq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 24 f5    	vpmovsxwq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 31    	vpmovsxwq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b4 f4 c0 1d fe ff 	vpmovsxwq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 7f 	vpmovsxwq 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 00 08 00 00 	vpmovsxwq 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 72 80 	vpmovsxwq -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 24 b2 f0 f7 ff ff 	vpmovsxwq -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 f5    	vpmovzxbd %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 31 f5    	vpmovzxbd %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 31    	vpmovzxbd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b4 f4 c0 1d fe ff 	vpmovzxbd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 7f 	vpmovzxbd 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 00 08 00 00 	vpmovzxbd 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 72 80 	vpmovzxbd -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 31 b2 f0 f7 ff ff 	vpmovzxbd -0x810\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 f5    	vpmovzxbq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 32 f5    	vpmovzxbq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 31    	vpmovzxbq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b4 f4 c0 1d fe ff 	vpmovzxbq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 7f 	vpmovzxbq 0x3f8\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 00 04 00 00 	vpmovzxbq 0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 72 80 	vpmovzxbq -0x400\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 32 b2 f8 fb ff ff 	vpmovzxbq -0x408\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 f5    	vpmovzxwd %ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 33 f5    	vpmovzxwd %ymm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 31    	vpmovzxwd \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b4 f4 c0 1d fe ff 	vpmovzxwd -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 7f 	vpmovzxwd 0xfe0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 00 10 00 00 	vpmovzxwd 0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 72 80 	vpmovzxwd -0x1000\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 33 b2 e0 ef ff ff 	vpmovzxwd -0x1020\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 f5    	vpmovzxwq %xmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 34 f5    	vpmovzxwq %xmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 31    	vpmovzxwq \(%ecx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b4 f4 c0 1d fe ff 	vpmovzxwq -0x1e240\(%esp,%esi,8\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 7f 	vpmovzxwq 0x7f0\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 00 08 00 00 	vpmovzxwq 0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 72 80 	vpmovzxwq -0x800\(%edx\),%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd 4f 34 b2 f0 f7 ff ff 	vpmovzxwq -0x810\(%edx\),%zmm6\{%k7\}
#pass
