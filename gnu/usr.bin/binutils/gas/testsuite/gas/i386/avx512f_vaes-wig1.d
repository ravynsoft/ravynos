#as: -mevexwig=1
#objdump: -dw
#name: i386 AVX512F/VAES wig insns
#source: avx512f_vaes.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 de f4[ 	]*vaesdec %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 de b4 f4 c0 1d fe ff[ 	]*vaesdec -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 de 72 7f[ 	]*vaesdec 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 df f4[ 	]*vaesdeclast %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 df 72 7f[ 	]*vaesdeclast 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dc f4[ 	]*vaesenc %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dc b4 f4 c0 1d fe ff[ 	]*vaesenc -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dc 72 7f[ 	]*vaesenc 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dd f4[ 	]*vaesenclast %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dd 72 7f[ 	]*vaesenclast 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 de f4[ 	]*vaesdec %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 de b4 f4 c0 1d fe ff[ 	]*vaesdec -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 de 72 7f[ 	]*vaesdec 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 df f4[ 	]*vaesdeclast %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 df 72 7f[ 	]*vaesdeclast 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dc f4[ 	]*vaesenc %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dc b4 f4 c0 1d fe ff[ 	]*vaesenc -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dc 72 7f[ 	]*vaesenc 0x1fc0\(%edx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dd f4[ 	]*vaesenclast %zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast -0x1e240\(%esp,%esi,8\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 dd 72 7f[ 	]*vaesenclast 0x1fc0\(%edx\),%zmm5,%zmm6
#pass
