#source: x86-64-default-suffix.s
#as: -msse2avx
#objdump: -dw
#name: x86-64 default suffix (AT&T mode)

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
 +[a-f0-9]+:	c5 fb 2a 00          	vcvtsi2sdl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	c5 fa 2a 00          	vcvtsi2ssl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	c5 fb 2a 00          	vcvtsi2sdl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 61 7f 08 2a 38    	vcvtsi2sdl \(%rax\),%xmm0,%xmm31
 +[a-f0-9]+:	c5 fa 2a 00          	vcvtsi2ssl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 61 7e 08 2a 38    	vcvtsi2ssl \(%rax\),%xmm0,%xmm31
 +[a-f0-9]+:	62 f1 7f 08 7b 00    	vcvtusi2sdl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 f1 7e 08 7b 00    	vcvtusi2ssl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 f5 7e 08 2a 00    	vcvtsi2shl \(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 65 7e 08 2a 38    	vcvtsi2shl \(%rax\),%xmm0,%xmm31
 +[a-f0-9]+:	62 f5 7e 08 7b 00    	vcvtusi2shl \(%rax\),%xmm0,%xmm0
#pass
