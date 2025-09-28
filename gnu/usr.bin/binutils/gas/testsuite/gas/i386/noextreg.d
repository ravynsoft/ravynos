#as: -O0
#objdump: -dw
#name: ix86 no extended registers

.*:     file format .*

Disassembly of section .text:

0+ <ix86>:
[ 	]*[a-f0-9]+:	c5 f9 db c0          	vpand  %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 c1 79 db c0       	vpand  %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 c1 39 db c0       	vpand  %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 7d 08 db c0    	vpandd %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 d1 7d 08 db c0    	vpandd %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 3d 08 db c0    	vpandd %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f1 7d 00 db c0    	vpandd %xmm0,\(bad\),%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 4c c0 00    	vpblendvb %xmm0,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 c3 79 4c c0 00    	vpblendvb %xmm0,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 39 4c c0 00    	vpblendvb %xmm0,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 4c c0 80    	vpblendvb %xmm0,%xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	62 f2 7d 0f 90 0c 00 	vpgatherdd \(%eax,%xmm0,1\),%xmm1\{%k7\}
[ 	]*[a-f0-9]+:	62 d2 7d 0f 90 0c 00 	vpgatherdd \(%eax,%xmm0,1\),%xmm1\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d 07 90 0c 00 	vpgatherdd \(%eax,\(bad\),1\),%xmm1\{%k7\}
[ 	]*[a-f0-9]+:	c4 e2 78 f2 00       	andn   \(%eax\),%eax,%eax
[ 	]*[a-f0-9]+:	c4 e2 38 f2 00       	andn   \(%eax\),%eax,%eax
[ 	]*[a-f0-9]+:	c4 c2 78 f2 00       	andn   \(%eax\),%eax,%eax
[ 	]*[a-f0-9]+:	c4 e2 f8 f2 00       	andn   \(%eax\),%eax,%eax
[ 	]*[a-f0-9]+:	8f e9 78 01 20       	tzmsk  \(%eax\),%eax
[ 	]*[a-f0-9]+:	8f c9 78 01 20       	tzmsk  \(%eax\),%eax
[ 	]*[a-f0-9]+:	8f e9 38 01 20       	tzmsk  \(%eax\),%eax
[ 	]*[a-f0-9]+:	8f e9 f8 01 20       	tzmsk  \(%eax\),%eax
[ 	]*[a-f0-9]+:	8f e9 78 12 c0       	llwpcb %eax
[ 	]*[a-f0-9]+:	8f c9 78 12 c0       	llwpcb %eax
[ 	]*[a-f0-9]+:	8f e9 f8 12 c0       	llwpcb %eax
[ 	]*[a-f0-9]+:	8f e8 78 c0 c0 01    	vprotb \$0x1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c0 c0 01    	vprotb \$0x1,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e8 78 c0 00 01    	vprotb \$0x1,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f c8 78 c0 00 01    	vprotb \$0x1,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 90 c0       	vprotb %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c9 b8 90 c0       	vprotb %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 38 90 c0       	vprotb %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f e9 78 90 00       	vprotb %xmm0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f c9 78 90 00       	vprotb %xmm0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	8f e9 f8 90 00       	vprotb \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	8f c9 f8 90 00       	vprotb \(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 68 00 00    	vfmaddps %xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 39 68 00 00    	vfmaddps %xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 68 00 80    	vfmaddps %xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 68 00 0f    	vfmaddps %xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 48 00 00    	vpermil2ps \$0x0,%xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 39 48 00 00    	vpermil2ps \$0x0,%xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 48 00 80    	vpermil2ps \$0x0,%xmm0,\(%eax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
