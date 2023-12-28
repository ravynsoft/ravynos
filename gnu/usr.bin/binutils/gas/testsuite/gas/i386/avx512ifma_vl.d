#as:
#objdump: -dw
#name: i386 AVX512IFMA/VL insns
#source: avx512ifma_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 f4[ 	]*vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b4 f4[ 	]*vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 31[ 	]*vpmadd52luq \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 30[ 	]*vpmadd52luq \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 7f[ 	]*vpmadd52luq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 00 08 00 00[ 	]*vpmadd52luq 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 80[ 	]*vpmadd52luq -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 80[ 	]*vpmadd52luq -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 f4[ 	]*vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b4 f4[ 	]*vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 31[ 	]*vpmadd52luq \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 30[ 	]*vpmadd52luq \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 7f[ 	]*vpmadd52luq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 00 10 00 00[ 	]*vpmadd52luq 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 80[ 	]*vpmadd52luq -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 e0 ef ff ff[ 	]*vpmadd52luq -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 80[ 	]*vpmadd52luq -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 f4[ 	]*vpmadd52huq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b5 f4[ 	]*vpmadd52huq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 31[ 	]*vpmadd52huq \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 30[ 	]*vpmadd52huq \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 7f[ 	]*vpmadd52huq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 00 08 00 00[ 	]*vpmadd52huq 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 80[ 	]*vpmadd52huq -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 80[ 	]*vpmadd52huq -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 f4[ 	]*vpmadd52huq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b5 f4[ 	]*vpmadd52huq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 31[ 	]*vpmadd52huq \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 30[ 	]*vpmadd52huq \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 7f[ 	]*vpmadd52huq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 00 10 00 00[ 	]*vpmadd52huq 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 80[ 	]*vpmadd52huq -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 e0 ef ff ff[ 	]*vpmadd52huq -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 80[ 	]*vpmadd52huq -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 f4[ 	]*vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b4 f4[ 	]*vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 31[ 	]*vpmadd52luq \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 30[ 	]*vpmadd52luq \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 7f[ 	]*vpmadd52luq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 00 08 00 00[ 	]*vpmadd52luq 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 80[ 	]*vpmadd52luq -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 80[ 	]*vpmadd52luq -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 f4[ 	]*vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b4 f4[ 	]*vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 31[ 	]*vpmadd52luq \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 30[ 	]*vpmadd52luq \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 7f[ 	]*vpmadd52luq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 00 10 00 00[ 	]*vpmadd52luq 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 80[ 	]*vpmadd52luq -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 e0 ef ff ff[ 	]*vpmadd52luq -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 80[ 	]*vpmadd52luq -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 f4[ 	]*vpmadd52huq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b5 f4[ 	]*vpmadd52huq %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 31[ 	]*vpmadd52huq \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 30[ 	]*vpmadd52huq \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 7f[ 	]*vpmadd52huq 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 00 08 00 00[ 	]*vpmadd52huq 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 80[ 	]*vpmadd52huq -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 80[ 	]*vpmadd52huq -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 f4[ 	]*vpmadd52huq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b5 f4[ 	]*vpmadd52huq %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 31[ 	]*vpmadd52huq \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 30[ 	]*vpmadd52huq \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 7f[ 	]*vpmadd52huq 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 00 10 00 00[ 	]*vpmadd52huq 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 80[ 	]*vpmadd52huq -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 e0 ef ff ff[ 	]*vpmadd52huq -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 80[ 	]*vpmadd52huq -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
#pass
