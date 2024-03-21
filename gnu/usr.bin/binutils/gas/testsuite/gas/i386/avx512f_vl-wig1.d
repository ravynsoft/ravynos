#as: -mevexwig=1
#objdump: -dw
#name: i386 AVX512F/VL wig insns
#source: avx512f_vl-wig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 f5[ 	]*vpmovsxbd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 21 f5[ 	]*vpmovsxbd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 31[ 	]*vpmovsxbd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 7f[ 	]*vpmovsxbd 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 00 02 00 00[ 	]*vpmovsxbd 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 80[ 	]*vpmovsxbd -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 fc fd ff ff[ 	]*vpmovsxbd -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 f5[ 	]*vpmovsxbd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 21 f5[ 	]*vpmovsxbd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 31[ 	]*vpmovsxbd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 7f[ 	]*vpmovsxbd 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 00 04 00 00[ 	]*vpmovsxbd 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 80[ 	]*vpmovsxbd -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 f8 fb ff ff[ 	]*vpmovsxbd -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 f5[ 	]*vpmovsxbq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 22 f5[ 	]*vpmovsxbq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 31[ 	]*vpmovsxbq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 7f[ 	]*vpmovsxbq 0xfe\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 00 01 00 00[ 	]*vpmovsxbq 0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 80[ 	]*vpmovsxbq -0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 fe fe ff ff[ 	]*vpmovsxbq -0x102\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 f5[ 	]*vpmovsxbq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 22 f5[ 	]*vpmovsxbq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 31[ 	]*vpmovsxbq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 7f[ 	]*vpmovsxbq 0x1fc\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 00 02 00 00[ 	]*vpmovsxbq 0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 80[ 	]*vpmovsxbq -0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 fc fd ff ff[ 	]*vpmovsxbq -0x204\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 f5[ 	]*vpmovsxwd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 23 f5[ 	]*vpmovsxwd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 31[ 	]*vpmovsxwd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 7f[ 	]*vpmovsxwd 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 00 04 00 00[ 	]*vpmovsxwd 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 80[ 	]*vpmovsxwd -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 f8 fb ff ff[ 	]*vpmovsxwd -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 f5[ 	]*vpmovsxwd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 23 f5[ 	]*vpmovsxwd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 31[ 	]*vpmovsxwd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 7f[ 	]*vpmovsxwd 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 00 08 00 00[ 	]*vpmovsxwd 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 80[ 	]*vpmovsxwd -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 f5[ 	]*vpmovsxwq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 24 f5[ 	]*vpmovsxwq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 31[ 	]*vpmovsxwq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 7f[ 	]*vpmovsxwq 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 00 02 00 00[ 	]*vpmovsxwq 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 80[ 	]*vpmovsxwq -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 fc fd ff ff[ 	]*vpmovsxwq -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 f5[ 	]*vpmovsxwq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 24 f5[ 	]*vpmovsxwq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 31[ 	]*vpmovsxwq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 7f[ 	]*vpmovsxwq 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 00 04 00 00[ 	]*vpmovsxwq 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 80[ 	]*vpmovsxwq -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 f8 fb ff ff[ 	]*vpmovsxwq -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 f5[ 	]*vpmovzxbd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 31 f5[ 	]*vpmovzxbd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 31[ 	]*vpmovzxbd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 7f[ 	]*vpmovzxbd 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 00 02 00 00[ 	]*vpmovzxbd 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 80[ 	]*vpmovzxbd -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 fc fd ff ff[ 	]*vpmovzxbd -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 f5[ 	]*vpmovzxbd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 31 f5[ 	]*vpmovzxbd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 31[ 	]*vpmovzxbd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 7f[ 	]*vpmovzxbd 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 00 04 00 00[ 	]*vpmovzxbd 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 80[ 	]*vpmovzxbd -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 f8 fb ff ff[ 	]*vpmovzxbd -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 f5[ 	]*vpmovzxbq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 32 f5[ 	]*vpmovzxbq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 31[ 	]*vpmovzxbq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 7f[ 	]*vpmovzxbq 0xfe\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 00 01 00 00[ 	]*vpmovzxbq 0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 80[ 	]*vpmovzxbq -0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 fe fe ff ff[ 	]*vpmovzxbq -0x102\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 f5[ 	]*vpmovzxbq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 32 f5[ 	]*vpmovzxbq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 31[ 	]*vpmovzxbq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 7f[ 	]*vpmovzxbq 0x1fc\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 00 02 00 00[ 	]*vpmovzxbq 0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 80[ 	]*vpmovzxbq -0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 fc fd ff ff[ 	]*vpmovzxbq -0x204\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 f5[ 	]*vpmovzxwd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 33 f5[ 	]*vpmovzxwd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 31[ 	]*vpmovzxwd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 7f[ 	]*vpmovzxwd 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 00 04 00 00[ 	]*vpmovzxwd 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 80[ 	]*vpmovzxwd -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 f8 fb ff ff[ 	]*vpmovzxwd -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 f5[ 	]*vpmovzxwd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 33 f5[ 	]*vpmovzxwd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 31[ 	]*vpmovzxwd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 7f[ 	]*vpmovzxwd 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 00 08 00 00[ 	]*vpmovzxwd 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 80[ 	]*vpmovzxwd -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 f5[ 	]*vpmovzxwq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 34 f5[ 	]*vpmovzxwq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 31[ 	]*vpmovzxwq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 7f[ 	]*vpmovzxwq 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 00 02 00 00[ 	]*vpmovzxwq 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 80[ 	]*vpmovzxwq -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 fc fd ff ff[ 	]*vpmovzxwq -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 f5[ 	]*vpmovzxwq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 34 f5[ 	]*vpmovzxwq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 31[ 	]*vpmovzxwq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 7f[ 	]*vpmovzxwq 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 00 04 00 00[ 	]*vpmovzxwq 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 80[ 	]*vpmovzxwq -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 f8 fb ff ff[ 	]*vpmovzxwq -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 f5[ 	]*vpmovsxbd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 21 f5[ 	]*vpmovsxbd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 31[ 	]*vpmovsxbd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 7f[ 	]*vpmovsxbd 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 00 02 00 00[ 	]*vpmovsxbd 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 80[ 	]*vpmovsxbd -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 fc fd ff ff[ 	]*vpmovsxbd -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 f5[ 	]*vpmovsxbd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 21 f5[ 	]*vpmovsxbd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 31[ 	]*vpmovsxbd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 7f[ 	]*vpmovsxbd 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 00 04 00 00[ 	]*vpmovsxbd 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 80[ 	]*vpmovsxbd -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 f8 fb ff ff[ 	]*vpmovsxbd -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 f5[ 	]*vpmovsxbq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 22 f5[ 	]*vpmovsxbq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 31[ 	]*vpmovsxbq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 7f[ 	]*vpmovsxbq 0xfe\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 00 01 00 00[ 	]*vpmovsxbq 0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 80[ 	]*vpmovsxbq -0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 fe fe ff ff[ 	]*vpmovsxbq -0x102\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 f5[ 	]*vpmovsxbq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 22 f5[ 	]*vpmovsxbq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 31[ 	]*vpmovsxbq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 7f[ 	]*vpmovsxbq 0x1fc\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 00 02 00 00[ 	]*vpmovsxbq 0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 80[ 	]*vpmovsxbq -0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 fc fd ff ff[ 	]*vpmovsxbq -0x204\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 f5[ 	]*vpmovsxwd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 23 f5[ 	]*vpmovsxwd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 31[ 	]*vpmovsxwd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 7f[ 	]*vpmovsxwd 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 00 04 00 00[ 	]*vpmovsxwd 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 80[ 	]*vpmovsxwd -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 f8 fb ff ff[ 	]*vpmovsxwd -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 f5[ 	]*vpmovsxwd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 23 f5[ 	]*vpmovsxwd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 31[ 	]*vpmovsxwd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 7f[ 	]*vpmovsxwd 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 00 08 00 00[ 	]*vpmovsxwd 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 80[ 	]*vpmovsxwd -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 f5[ 	]*vpmovsxwq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 24 f5[ 	]*vpmovsxwq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 31[ 	]*vpmovsxwq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 7f[ 	]*vpmovsxwq 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 00 02 00 00[ 	]*vpmovsxwq 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 80[ 	]*vpmovsxwq -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 fc fd ff ff[ 	]*vpmovsxwq -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 f5[ 	]*vpmovsxwq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 24 f5[ 	]*vpmovsxwq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 31[ 	]*vpmovsxwq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 7f[ 	]*vpmovsxwq 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 00 04 00 00[ 	]*vpmovsxwq 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 80[ 	]*vpmovsxwq -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 f8 fb ff ff[ 	]*vpmovsxwq -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 f5[ 	]*vpmovzxbd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 31 f5[ 	]*vpmovzxbd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 31[ 	]*vpmovzxbd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 7f[ 	]*vpmovzxbd 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 00 02 00 00[ 	]*vpmovzxbd 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 80[ 	]*vpmovzxbd -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 fc fd ff ff[ 	]*vpmovzxbd -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 f5[ 	]*vpmovzxbd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 31 f5[ 	]*vpmovzxbd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 31[ 	]*vpmovzxbd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 7f[ 	]*vpmovzxbd 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 00 04 00 00[ 	]*vpmovzxbd 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 80[ 	]*vpmovzxbd -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 f8 fb ff ff[ 	]*vpmovzxbd -0x408\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 f5[ 	]*vpmovzxbq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 32 f5[ 	]*vpmovzxbq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 31[ 	]*vpmovzxbq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 7f[ 	]*vpmovzxbq 0xfe\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 00 01 00 00[ 	]*vpmovzxbq 0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 80[ 	]*vpmovzxbq -0x100\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 fe fe ff ff[ 	]*vpmovzxbq -0x102\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 f5[ 	]*vpmovzxbq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 32 f5[ 	]*vpmovzxbq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 31[ 	]*vpmovzxbq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 7f[ 	]*vpmovzxbq 0x1fc\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 00 02 00 00[ 	]*vpmovzxbq 0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 80[ 	]*vpmovzxbq -0x200\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 fc fd ff ff[ 	]*vpmovzxbq -0x204\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 f5[ 	]*vpmovzxwd %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 33 f5[ 	]*vpmovzxwd %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 31[ 	]*vpmovzxwd \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 7f[ 	]*vpmovzxwd 0x3f8\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 00 04 00 00[ 	]*vpmovzxwd 0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 80[ 	]*vpmovzxwd -0x400\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 f8 fb ff ff[ 	]*vpmovzxwd -0x408\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 f5[ 	]*vpmovzxwd %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 33 f5[ 	]*vpmovzxwd %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 31[ 	]*vpmovzxwd \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 7f[ 	]*vpmovzxwd 0x7f0\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 00 08 00 00[ 	]*vpmovzxwd 0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 80[ 	]*vpmovzxwd -0x800\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd -0x810\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 f5[ 	]*vpmovzxwq %xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 34 f5[ 	]*vpmovzxwq %xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 31[ 	]*vpmovzxwq \(%ecx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq -0x1e240\(%esp,%esi,8\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 7f[ 	]*vpmovzxwq 0x1fc\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 00 02 00 00[ 	]*vpmovzxwq 0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 80[ 	]*vpmovzxwq -0x200\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 fc fd ff ff[ 	]*vpmovzxwq -0x204\(%edx\),%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 f5[ 	]*vpmovzxwq %xmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 34 f5[ 	]*vpmovzxwq %xmm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 31[ 	]*vpmovzxwq \(%ecx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq -0x1e240\(%esp,%esi,8\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 7f[ 	]*vpmovzxwq 0x3f8\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 00 04 00 00[ 	]*vpmovzxwq 0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 80[ 	]*vpmovzxwq -0x400\(%edx\),%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 f8 fb ff ff[ 	]*vpmovzxwq -0x408\(%edx\),%ymm6\{%k7\}
#pass
