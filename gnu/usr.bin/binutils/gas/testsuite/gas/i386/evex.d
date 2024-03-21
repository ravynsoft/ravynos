#objdump: -dw -Msuffix
#name: i386 EVEX insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 f1 d6 38 2a f0    	vcvtsi2ssl %eax,\{rd-sae\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 57 38 2a f0    	vcvtsi2sdl %eax,\{rd-bad\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d7 38 2a f0    	vcvtsi2sdl %eax,\{rd-bad\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d6 08 7b f0    	vcvtusi2ssl %eax,%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 57 08 7b f0    	vcvtusi2sdl %eax,%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d7 08 7b f0    	vcvtusi2sdl %eax,%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d6 38 7b f0    	vcvtusi2ssl %eax,\{rd-sae\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 57 38 7b f0    	vcvtusi2sdl %eax,\{rd-bad\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d7 38 7b f0    	vcvtusi2sdl %eax,\{rd-bad\},%xmm5,%xmm6
 +[a-f0-9]+:	62 e1 7e 08 2d c0    	\{evex\} vcvtss2si %xmm0,%eax
 +[a-f0-9]+:	62 e1 7c 08 c2 c0 00 	vcmpeqps %xmm0,%xmm0,%k0
#pass
