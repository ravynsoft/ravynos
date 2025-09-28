#as:
#objdump: -dw
#name: i386 AVX512VBMI/VL insns
#source: avx512vbmi_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d f4[ 	]*vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 8d f4[ 	]*vpermb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 31[ 	]*vpermb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 7f[ 	]*vpermb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 00 08 00 00[ 	]*vpermb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 80[ 	]*vpermb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 f0 f7 ff ff[ 	]*vpermb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d f4[ 	]*vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 8d f4[ 	]*vpermb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 31[ 	]*vpermb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 7f[ 	]*vpermb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 00 10 00 00[ 	]*vpermb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 80[ 	]*vpermb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 e0 ef ff ff[ 	]*vpermb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 f4[ 	]*vpermi2b %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 75 f4[ 	]*vpermi2b %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 31[ 	]*vpermi2b \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 7f[ 	]*vpermi2b 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 00 08 00 00[ 	]*vpermi2b 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 80[ 	]*vpermi2b -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2b -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 f4[ 	]*vpermi2b %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 75 f4[ 	]*vpermi2b %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 31[ 	]*vpermi2b \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 7f[ 	]*vpermi2b 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 00 10 00 00[ 	]*vpermi2b 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 80[ 	]*vpermi2b -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 e0 ef ff ff[ 	]*vpermi2b -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d f4[ 	]*vpermt2b %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 7d f4[ 	]*vpermt2b %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 31[ 	]*vpermt2b \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 7f[ 	]*vpermt2b 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 00 08 00 00[ 	]*vpermt2b 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 80[ 	]*vpermt2b -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2b -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d f4[ 	]*vpermt2b %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 7d f4[ 	]*vpermt2b %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 31[ 	]*vpermt2b \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 7f[ 	]*vpermt2b 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 00 10 00 00[ 	]*vpermt2b 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 80[ 	]*vpermt2b -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 e0 ef ff ff[ 	]*vpermt2b -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 f4[ 	]*vpmultishiftqb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 83 f4[ 	]*vpmultishiftqb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 31[ 	]*vpmultishiftqb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 30[ 	]*vpmultishiftqb \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 7f[ 	]*vpmultishiftqb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 00 08 00 00[ 	]*vpmultishiftqb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 80[ 	]*vpmultishiftqb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 80[ 	]*vpmultishiftqb -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 f4[ 	]*vpmultishiftqb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 83 f4[ 	]*vpmultishiftqb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 31[ 	]*vpmultishiftqb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 30[ 	]*vpmultishiftqb \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 7f[ 	]*vpmultishiftqb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 00 10 00 00[ 	]*vpmultishiftqb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 80[ 	]*vpmultishiftqb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 80[ 	]*vpmultishiftqb -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d f4[ 	]*vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 8d f4[ 	]*vpermb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 31[ 	]*vpermb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 7f[ 	]*vpermb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 00 08 00 00[ 	]*vpermb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 80[ 	]*vpermb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 f0 f7 ff ff[ 	]*vpermb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d f4[ 	]*vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 8d f4[ 	]*vpermb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 31[ 	]*vpermb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 7f[ 	]*vpermb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 00 10 00 00[ 	]*vpermb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 80[ 	]*vpermb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 e0 ef ff ff[ 	]*vpermb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 f4[ 	]*vpermi2b %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 75 f4[ 	]*vpermi2b %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 31[ 	]*vpermi2b \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 7f[ 	]*vpermi2b 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 00 08 00 00[ 	]*vpermi2b 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 80[ 	]*vpermi2b -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2b -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 f4[ 	]*vpermi2b %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 75 f4[ 	]*vpermi2b %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 31[ 	]*vpermi2b \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 7f[ 	]*vpermi2b 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 00 10 00 00[ 	]*vpermi2b 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 80[ 	]*vpermi2b -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 e0 ef ff ff[ 	]*vpermi2b -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d f4[ 	]*vpermt2b %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 7d f4[ 	]*vpermt2b %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 31[ 	]*vpermt2b \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 7f[ 	]*vpermt2b 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 00 08 00 00[ 	]*vpermt2b 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 80[ 	]*vpermt2b -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2b -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d f4[ 	]*vpermt2b %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 7d f4[ 	]*vpermt2b %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 31[ 	]*vpermt2b \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 7f[ 	]*vpermt2b 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 00 10 00 00[ 	]*vpermt2b 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 80[ 	]*vpermt2b -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 e0 ef ff ff[ 	]*vpermt2b -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 f4[ 	]*vpmultishiftqb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 83 f4[ 	]*vpmultishiftqb %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 31[ 	]*vpmultishiftqb \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 30[ 	]*vpmultishiftqb \(%eax\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 7f[ 	]*vpmultishiftqb 0x7f0\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 00 08 00 00[ 	]*vpmultishiftqb 0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 80[ 	]*vpmultishiftqb -0x800\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb -0x810\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 80[ 	]*vpmultishiftqb -0x400\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%edx\)\{1to2\},%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 f4[ 	]*vpmultishiftqb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 83 f4[ 	]*vpmultishiftqb %ymm4,%ymm5,%ymm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 31[ 	]*vpmultishiftqb \(%ecx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 30[ 	]*vpmultishiftqb \(%eax\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 7f[ 	]*vpmultishiftqb 0xfe0\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 00 10 00 00[ 	]*vpmultishiftqb 0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 80[ 	]*vpmultishiftqb -0x1000\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb -0x1020\(%edx\),%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 80[ 	]*vpmultishiftqb -0x400\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%edx\)\{1to4\},%ymm5,%ymm6\{%k7\}
#pass
