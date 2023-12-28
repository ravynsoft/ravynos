#as:
#objdump: -dw
#name: x86_64 AVX512F/VAES insns
#source: x86-64-avx512f_vaes.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 de f4[ 	]*vaesdec %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 de b4 f0 23 01 00 00[ 	]*vaesdec 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de 72 7f[ 	]*vaesdec 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 df f4[ 	]*vaesdeclast %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 df b4 f0 23 01 00 00[ 	]*vaesdeclast 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df 72 7f[ 	]*vaesdeclast 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 dc f4[ 	]*vaesenc %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 dc b4 f0 23 01 00 00[ 	]*vaesenc 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc 72 7f[ 	]*vaesenc 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 dd f4[ 	]*vaesenclast %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 dd b4 f0 23 01 00 00[ 	]*vaesenclast 0x123\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd 72 7f[ 	]*vaesenclast 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 de f4[ 	]*vaesdec %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 de b4 f0 34 12 00 00[ 	]*vaesdec 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de 72 7f[ 	]*vaesdec 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 df f4[ 	]*vaesdeclast %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 df b4 f0 34 12 00 00[ 	]*vaesdeclast 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df 72 7f[ 	]*vaesdeclast 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 dc f4[ 	]*vaesenc %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 dc b4 f0 34 12 00 00[ 	]*vaesenc 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc 72 7f[ 	]*vaesenc 0x1fc0\(%rdx\),%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 dd f4[ 	]*vaesenclast %zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 dd b4 f0 34 12 00 00[ 	]*vaesenclast 0x1234\(%rax,%r14,8\),%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd 72 7f[ 	]*vaesenclast 0x1fc0\(%rdx\),%zmm5,%zmm6
#pass
