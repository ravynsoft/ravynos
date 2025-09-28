#as:
#objdump: -dw
#name: i386 AVX512VBMI insns
#source: avx512vbmi.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 31[ 	]*vpermb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b4 f4 c0 1d fe ff[ 	]*vpermb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 7f[ 	]*vpermb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 00 20 00 00[ 	]*vpermb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 80[ 	]*vpermb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 c0 df ff ff[ 	]*vpermb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 f4[ 	]*vpermi2b %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 75 f4[ 	]*vpermi2b %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 75 f4[ 	]*vpermi2b %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 31[ 	]*vpermi2b \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 7f[ 	]*vpermi2b 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 00 20 00 00[ 	]*vpermi2b 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 80[ 	]*vpermi2b -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 c0 df ff ff[ 	]*vpermi2b -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d f4[ 	]*vpermt2b %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 7d f4[ 	]*vpermt2b %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 7d f4[ 	]*vpermt2b %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 31[ 	]*vpermt2b \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 7f[ 	]*vpermt2b 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 00 20 00 00[ 	]*vpermt2b 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 80[ 	]*vpermt2b -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 c0 df ff ff[ 	]*vpermt2b -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 f4[ 	]*vpmultishiftqb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 83 f4[ 	]*vpmultishiftqb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 83 f4[ 	]*vpmultishiftqb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 31[ 	]*vpmultishiftqb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 30[ 	]*vpmultishiftqb \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 7f[ 	]*vpmultishiftqb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 00 20 00 00[ 	]*vpmultishiftqb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 80[ 	]*vpmultishiftqb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 c0 df ff ff[ 	]*vpmultishiftqb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 80[ 	]*vpmultishiftqb -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 8d f4[ 	]*vpermb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 31[ 	]*vpermb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b4 f4 c0 1d fe ff[ 	]*vpermb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 7f[ 	]*vpermb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 00 20 00 00[ 	]*vpermb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d 72 80[ 	]*vpermb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8d b2 c0 df ff ff[ 	]*vpermb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 f4[ 	]*vpermi2b %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 75 f4[ 	]*vpermi2b %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 75 f4[ 	]*vpermi2b %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 31[ 	]*vpermi2b \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 7f[ 	]*vpermi2b 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 00 20 00 00[ 	]*vpermi2b 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 72 80[ 	]*vpermi2b -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 75 b2 c0 df ff ff[ 	]*vpermi2b -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d f4[ 	]*vpermt2b %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 7d f4[ 	]*vpermt2b %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 7d f4[ 	]*vpermt2b %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 31[ 	]*vpermt2b \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 7f[ 	]*vpermt2b 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 00 20 00 00[ 	]*vpermt2b 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d 72 80[ 	]*vpermt2b -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 7d b2 c0 df ff ff[ 	]*vpermt2b -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 f4[ 	]*vpmultishiftqb %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 83 f4[ 	]*vpmultishiftqb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 83 f4[ 	]*vpmultishiftqb %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 31[ 	]*vpmultishiftqb \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 30[ 	]*vpmultishiftqb \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 7f[ 	]*vpmultishiftqb 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 00 20 00 00[ 	]*vpmultishiftqb 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 72 80[ 	]*vpmultishiftqb -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 83 b2 c0 df ff ff[ 	]*vpmultishiftqb -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 7f[ 	]*vpmultishiftqb 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 00 04 00 00[ 	]*vpmultishiftqb 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 72 80[ 	]*vpmultishiftqb -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
#pass
