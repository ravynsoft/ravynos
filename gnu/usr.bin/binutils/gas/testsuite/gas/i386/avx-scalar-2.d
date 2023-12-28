#as: -mavxscalar=256 -msse2avx
#objdump: -dw
#name: i386 VEX.128 scalar insns with -mavxscalar=256 -msse2avx

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%ecx\)
 +[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
 +[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%ecx\),%xmm4
 +[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
 +[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%ecx\)
 +[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
 +[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%ecx\),%xmm4
 +[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
 +[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%ecx\)
 +[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%ecx\),%xmm4
 +[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%ecx\)
 +[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%ecx\),%xmm4
 +[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
#pass
