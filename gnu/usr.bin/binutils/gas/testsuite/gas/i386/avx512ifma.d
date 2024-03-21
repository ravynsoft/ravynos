#as:
#objdump: -dw
#name: i386 AVX512IFMA insns
#source: avx512ifma.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 31[ 	]*vpmadd52luq \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 30[ 	]*vpmadd52luq \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 7f[ 	]*vpmadd52luq 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 00 20 00 00[ 	]*vpmadd52luq 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 80[ 	]*vpmadd52luq -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 c0 df ff ff[ 	]*vpmadd52luq -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 80[ 	]*vpmadd52luq -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 f4[ 	]*vpmadd52huq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b5 f4[ 	]*vpmadd52huq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b5 f4[ 	]*vpmadd52huq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 31[ 	]*vpmadd52huq \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 30[ 	]*vpmadd52huq \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 7f[ 	]*vpmadd52huq 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 00 20 00 00[ 	]*vpmadd52huq 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 80[ 	]*vpmadd52huq -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 c0 df ff ff[ 	]*vpmadd52huq -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 80[ 	]*vpmadd52huq -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b4 f4[ 	]*vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 31[ 	]*vpmadd52luq \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 30[ 	]*vpmadd52luq \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 7f[ 	]*vpmadd52luq 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 00 20 00 00[ 	]*vpmadd52luq 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 72 80[ 	]*vpmadd52luq -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b4 b2 c0 df ff ff[ 	]*vpmadd52luq -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 7f[ 	]*vpmadd52luq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 00 04 00 00[ 	]*vpmadd52luq 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 72 80[ 	]*vpmadd52luq -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 f4[ 	]*vpmadd52huq %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f b5 f4[ 	]*vpmadd52huq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf b5 f4[ 	]*vpmadd52huq %zmm4,%zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 31[ 	]*vpmadd52huq \(%ecx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 30[ 	]*vpmadd52huq \(%eax\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 7f[ 	]*vpmadd52huq 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 00 20 00 00[ 	]*vpmadd52huq 0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 72 80[ 	]*vpmadd52huq -0x2000\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 b5 b2 c0 df ff ff[ 	]*vpmadd52huq -0x2040\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 7f[ 	]*vpmadd52huq 0x3f8\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 00 04 00 00[ 	]*vpmadd52huq 0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 72 80[ 	]*vpmadd52huq -0x400\(%edx\)\{1to8\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq -0x408\(%edx\)\{1to8\},%zmm5,%zmm6
#pass
