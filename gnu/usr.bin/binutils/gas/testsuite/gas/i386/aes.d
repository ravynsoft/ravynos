#objdump: -dw
#name: i386 AES

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dc c1       	aesenc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd 01       	aesenclast \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd c1       	aesenclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de 01       	aesdec \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de c1       	aesdec %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df 01       	aesdeclast \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df c1       	aesdeclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db 01       	aesimc \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db c1       	aesimc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df 01 08    	aeskeygenassist \$0x8,\(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df c1 08    	aeskeygenassist \$0x8,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dc 01       	aesenc \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dc c1       	aesenc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd 01       	aesenclast \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 dd c1       	aesenclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de 01       	aesdec \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 de c1       	aesdec %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df 01       	aesdeclast \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 df c1       	aesdeclast %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db 01       	aesimc \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 db c1       	aesimc %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df 01 08    	aeskeygenassist \$0x8,\(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a df c1 08    	aeskeygenassist \$0x8,%xmm1,%xmm0
#pass
