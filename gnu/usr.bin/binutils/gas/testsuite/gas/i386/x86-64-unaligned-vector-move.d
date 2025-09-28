#source: unaligned-vector-move.s
#as: -muse-unaligned-vector-move
#objdump: -dw
#name: x86-64 (Encode aligned vector move as unaligned vector move)

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	0f 10 d1             	movups %xmm1,%xmm2
 +[a-f0-9]+:	67 0f 10 10          	movups \(%eax\),%xmm2
 +[a-f0-9]+:	67 0f 11 08          	movups %xmm1,\(%eax\)
 +[a-f0-9]+:	66 0f 10 d1          	movupd %xmm1,%xmm2
 +[a-f0-9]+:	67 66 0f 10 10       	movupd \(%eax\),%xmm2
 +[a-f0-9]+:	67 66 0f 11 08       	movupd %xmm1,\(%eax\)
 +[a-f0-9]+:	f3 0f 6f d1          	movdqu %xmm1,%xmm2
 +[a-f0-9]+:	67 f3 0f 6f 10       	movdqu \(%eax\),%xmm2
 +[a-f0-9]+:	67 f3 0f 7f 08       	movdqu %xmm1,\(%eax\)
 +[a-f0-9]+:	c5 f8 10 d1          	vmovups %xmm1,%xmm2
 +[a-f0-9]+:	67 c5 f8 10 10       	vmovups \(%eax\),%xmm2
 +[a-f0-9]+:	67 c5 f8 11 08       	vmovups %xmm1,\(%eax\)
 +[a-f0-9]+:	c5 f9 10 d1          	vmovupd %xmm1,%xmm2
 +[a-f0-9]+:	67 c5 f9 10 10       	vmovupd \(%eax\),%xmm2
 +[a-f0-9]+:	67 c5 f9 11 08       	vmovupd %xmm1,\(%eax\)
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	67 c5 fa 6f 10       	vmovdqu \(%eax\),%xmm2
 +[a-f0-9]+:	67 c5 fa 7f 08       	vmovdqu %xmm1,\(%eax\)
 +[a-f0-9]+:	62 f1 7c 09 10 d1    	vmovups %xmm1,%xmm2\{%k1\}
 +[a-f0-9]+:	67 62 f1 7c 09 10 10 	vmovups \(%eax\),%xmm2\{%k1\}
 +[a-f0-9]+:	67 62 f1 7c 09 11 08 	vmovups %xmm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 fd 09 10 d1    	vmovupd %xmm1,%xmm2\{%k1\}
 +[a-f0-9]+:	67 62 f1 fd 09 10 10 	vmovupd \(%eax\),%xmm2\{%k1\}
 +[a-f0-9]+:	67 62 f1 fd 09 11 08 	vmovupd %xmm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7e 08 6f d1    	vmovdqu32 %xmm1,%xmm2
 +[a-f0-9]+:	67 62 f1 7e 08 6f 10 	vmovdqu32 \(%eax\),%xmm2
 +[a-f0-9]+:	67 62 f1 7e 08 7f 08 	vmovdqu32 %xmm1,\(%eax\)
 +[a-f0-9]+:	62 f1 fe 08 6f d1    	vmovdqu64 %xmm1,%xmm2
 +[a-f0-9]+:	67 62 f1 fe 08 6f 10 	vmovdqu64 \(%eax\),%xmm2
 +[a-f0-9]+:	67 62 f1 fe 08 7f 08 	vmovdqu64 %xmm1,\(%eax\)
#pass
