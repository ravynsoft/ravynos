#as: -mvexwig=1
#objdump: -dw
#name: x86_64 AVX/VAES wig insns
#source: x86-64-vaes.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 cd dc d4       	vaesenc %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd dc 39       	vaesenc \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd dd d4       	vaesenclast %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd dd 39       	vaesenclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd de d4       	vaesdec %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd de 39       	vaesdec \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd df d4       	vaesdeclast %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd df 39       	vaesdeclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd dc d4       	vaesenc %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd dc 39       	vaesenc \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd dc 39       	vaesenc \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd dd d4       	vaesenclast %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd dd 39       	vaesenclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd dd 39       	vaesenclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd de d4       	vaesdec %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd de 39       	vaesdec \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd de 39       	vaesdec \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd df d4       	vaesdeclast %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd df 39       	vaesdeclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 cd df 39       	vaesdeclast \(%rcx\),%ymm6,%ymm7
#pass
