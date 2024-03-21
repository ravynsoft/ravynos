#source: ../x86-64-aes.s
#as: -J
#objdump: -dw
#name: x86-64 (ILP32) AES

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dc c1       	aesenc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd 01       	aesenclast \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd c1       	aesenclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de 01       	aesdec \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de c1       	aesdec %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df 01       	aesdeclast \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df c1       	aesdeclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db 01       	aesimc \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db c1       	aesimc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df 01 08    	aeskeygenassist \$0x8,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df c1 08    	aeskeygenassist \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dc c1       	aesenc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd 01       	aesenclast \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd c1       	aesenclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de 01       	aesdec \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de c1       	aesdec %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df 01       	aesdeclast \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df c1       	aesdeclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db 01       	aesimc \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db c1       	aesimc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df 01 08    	aeskeygenassist \$0x8,\(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df c1 08    	aeskeygenassist \$0x8,%xmm1,%xmm0
#pass
