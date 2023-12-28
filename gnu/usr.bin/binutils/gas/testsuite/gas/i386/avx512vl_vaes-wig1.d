#as: -mevexwig=1
#objdump: -dw
#name: i386 AVX512VL/VAES wig insns
#source: avx512vl_vaes.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de f4[ 	]*vaesdec %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b4 f4 c0 1d fe ff[ 	]*vaesdec -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b2 f0 07 00 00[ 	]*vaesdec 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de f4[ 	]*vaesdec %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b4 f4 c0 1d fe ff[ 	]*vaesdec -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b2 e0 0f 00 00[ 	]*vaesdec 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df f4[ 	]*vaesdeclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b2 f0 07 00 00[ 	]*vaesdeclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df f4[ 	]*vaesdeclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b2 e0 0f 00 00[ 	]*vaesdeclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc f4[ 	]*vaesenc %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b4 f4 c0 1d fe ff[ 	]*vaesenc -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b2 f0 07 00 00[ 	]*vaesenc 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc f4[ 	]*vaesenc %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b4 f4 c0 1d fe ff[ 	]*vaesenc -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b2 e0 0f 00 00[ 	]*vaesenc 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd f4[ 	]*vaesenclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b2 f0 07 00 00[ 	]*vaesenclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd f4[ 	]*vaesenclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b2 e0 0f 00 00[ 	]*vaesenclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 de f4[ 	]*\{evex\} vaesdec %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 de 72 7f[ 	]*\{evex\} vaesdec 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 de f4[ 	]*\{evex\} vaesdec %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 de 72 7f[ 	]*\{evex\} vaesdec 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 df f4[ 	]*\{evex\} vaesdeclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 df 72 7f[ 	]*\{evex\} vaesdeclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 df f4[ 	]*\{evex\} vaesdeclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 df 72 7f[ 	]*\{evex\} vaesdeclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dc f4[ 	]*\{evex\} vaesenc %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dc 72 7f[ 	]*\{evex\} vaesenc 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dc f4[ 	]*\{evex\} vaesenc %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dc 72 7f[ 	]*\{evex\} vaesenc 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dd f4[ 	]*\{evex\} vaesenclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dd 72 7f[ 	]*\{evex\} vaesenclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dd f4[ 	]*\{evex\} vaesenclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dd 72 7f[ 	]*\{evex\} vaesenclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de f4[ 	]*vaesdec %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b4 f4 c0 1d fe ff[ 	]*vaesdec -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b2 f0 07 00 00[ 	]*vaesdec 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de f4[ 	]*vaesdec %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b4 f4 c0 1d fe ff[ 	]*vaesdec -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b2 e0 0f 00 00[ 	]*vaesdec 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df f4[ 	]*vaesdeclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b2 f0 07 00 00[ 	]*vaesdeclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df f4[ 	]*vaesdeclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b2 e0 0f 00 00[ 	]*vaesdeclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc f4[ 	]*vaesenc %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b4 f4 c0 1d fe ff[ 	]*vaesenc -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b2 f0 07 00 00[ 	]*vaesenc 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc f4[ 	]*vaesenc %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b4 f4 c0 1d fe ff[ 	]*vaesenc -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b2 e0 0f 00 00[ 	]*vaesenc 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd f4[ 	]*vaesenclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b2 f0 07 00 00[ 	]*vaesenclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd f4[ 	]*vaesenclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b2 e0 0f 00 00[ 	]*vaesenclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 de f4[ 	]*\{evex\} vaesdec %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 de 72 7f[ 	]*\{evex\} vaesdec 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 de f4[ 	]*\{evex\} vaesdec %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 de 72 7f[ 	]*\{evex\} vaesdec 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 df f4[ 	]*\{evex\} vaesdeclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 df 72 7f[ 	]*\{evex\} vaesdeclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 df f4[ 	]*\{evex\} vaesdeclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 df 72 7f[ 	]*\{evex\} vaesdeclast 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dc f4[ 	]*\{evex\} vaesenc %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dc 72 7f[ 	]*\{evex\} vaesenc 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dc f4[ 	]*\{evex\} vaesenc %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dc 72 7f[ 	]*\{evex\} vaesenc 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dd f4[ 	]*\{evex\} vaesenclast %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 08 dd 72 7f[ 	]*\{evex\} vaesenclast 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dd f4[ 	]*\{evex\} vaesenclast %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 28 dd 72 7f[ 	]*\{evex\} vaesenclast 0xfe0\(%edx\),%ymm5,%ymm6
#pass
