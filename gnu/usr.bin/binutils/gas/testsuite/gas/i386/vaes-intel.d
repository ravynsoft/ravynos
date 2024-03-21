#objdump: -dwMintel
#name: i386 VAES (Intel disassembly)
#source: vaes.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 4d dc d4       	vaesenc ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d dc 39       	vaesenc ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d dd d4       	vaesenclast ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d dd 39       	vaesenclast ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d de d4       	vaesdec ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d de 39       	vaesdec ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d df d4       	vaesdeclast ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d df 39       	vaesdeclast ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d dc d4       	vaesenc ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d dc 39       	vaesenc ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d dc 39       	vaesenc ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d dd d4       	vaesenclast ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d dd 39       	vaesenclast ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d dd 39       	vaesenclast ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d de d4       	vaesdec ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d de 39       	vaesdec ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d de 39       	vaesdec ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d df d4       	vaesdeclast ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d df 39       	vaesdeclast ymm7,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d df 39       	vaesdeclast ymm7,ymm6,YMMWORD PTR \[ecx\]
#pass
