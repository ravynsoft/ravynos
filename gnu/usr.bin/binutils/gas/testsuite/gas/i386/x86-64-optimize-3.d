#as: -Os
#objdump: -drw
#name: x86-64 optimized encoding 3 with -Os

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	40 f6 c7 7f          	test   \$0x7f,%dil
 +[a-f0-9]+:	40 f6 c7 7f          	test   \$0x7f,%dil
 +[a-f0-9]+:	40 f6 c7 7f          	test   \$0x7f,%dil
 +[a-f0-9]+:	40 f6 c7 7f          	test   \$0x7f,%dil
 +[a-f0-9]+:	41 f6 c1 7f          	test   \$0x7f,%r9b
 +[a-f0-9]+:	41 f6 c1 7f          	test   \$0x7f,%r9b
 +[a-f0-9]+:	41 f6 c1 7f          	test   \$0x7f,%r9b
 +[a-f0-9]+:	41 f6 c1 7f          	test   \$0x7f,%r9b
 +[a-f0-9]+:	41 f6 c4 7f          	test   \$0x7f,%r12b
 +[a-f0-9]+:	41 f6 c4 7f          	test   \$0x7f,%r12b
 +[a-f0-9]+:	41 f6 c4 7f          	test   \$0x7f,%r12b
 +[a-f0-9]+:	41 f6 c4 7f          	test   \$0x7f,%r12b
 +[a-f0-9]+:	20 c9                	and    %cl,%cl
 +[a-f0-9]+:	66 21 d2             	and    %dx,%dx
 +[a-f0-9]+:	21 db                	and    %ebx,%ebx
 +[a-f0-9]+:	48 21 e4             	and    %rsp,%rsp
 +[a-f0-9]+:	40 08 ed             	or     %bpl,%bpl
 +[a-f0-9]+:	66 09 f6             	or     %si,%si
 +[a-f0-9]+:	09 ff                	or     %edi,%edi
 +[a-f0-9]+:	4d 09 c0             	or     %r8,%r8
 +[a-f0-9]+:	c5 f1 55 e9          	vandnpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f9 6f d1          	vmovdqa %xmm1,%xmm2
 +[a-f0-9]+:	c5 f9 6f d1          	vmovdqa %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c4 41 79 6f e3       	vmovdqa %xmm11,%xmm12
 +[a-f0-9]+:	c4 41 79 6f e3       	vmovdqa %xmm11,%xmm12
 +[a-f0-9]+:	c4 41 7a 6f e3       	vmovdqu %xmm11,%xmm12
 +[a-f0-9]+:	c4 41 7a 6f e3       	vmovdqu %xmm11,%xmm12
 +[a-f0-9]+:	c4 41 7a 6f e3       	vmovdqu %xmm11,%xmm12
 +[a-f0-9]+:	c4 41 7a 6f e3       	vmovdqu %xmm11,%xmm12
 +[a-f0-9]+:	c5 f9 6f 50 7f       	vmovdqa 0x7f\(%rax\),%xmm2
 +[a-f0-9]+:	c5 f9 6f 50 7f       	vmovdqa 0x7f\(%rax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%rax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%rax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%rax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%rax\),%xmm2
 +[a-f0-9]+:	62 f1 7d 08 7f 48 08 	vmovdqa32 %xmm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 fd 08 7f 48 08 	vmovdqa64 %xmm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 7f 08 7f 48 08 	vmovdqu8 %xmm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 ff 08 7f 48 08 	vmovdqu16 %xmm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 7e 08 7f 48 08 	vmovdqu32 %xmm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 fe 08 7f 48 08 	vmovdqu64 %xmm1,0x80\(%rax\)
 +[a-f0-9]+:	c5 fd 6f d1          	vmovdqa %ymm1,%ymm2
 +[a-f0-9]+:	c5 fd 6f d1          	vmovdqa %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c4 41 7d 6f e3       	vmovdqa %ymm11,%ymm12
 +[a-f0-9]+:	c4 41 7d 6f e3       	vmovdqa %ymm11,%ymm12
 +[a-f0-9]+:	c4 41 7e 6f e3       	vmovdqu %ymm11,%ymm12
 +[a-f0-9]+:	c4 41 7e 6f e3       	vmovdqu %ymm11,%ymm12
 +[a-f0-9]+:	c4 41 7e 6f e3       	vmovdqu %ymm11,%ymm12
 +[a-f0-9]+:	c4 41 7e 6f e3       	vmovdqu %ymm11,%ymm12
 +[a-f0-9]+:	c5 fd 6f 50 7f       	vmovdqa 0x7f\(%rax\),%ymm2
 +[a-f0-9]+:	c5 fd 6f 50 7f       	vmovdqa 0x7f\(%rax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%rax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%rax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%rax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%rax\),%ymm2
 +[a-f0-9]+:	62 f1 7d 28 7f 48 04 	vmovdqa32 %ymm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 fd 28 7f 48 04 	vmovdqa64 %ymm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 7f 28 7f 48 04 	vmovdqu8 %ymm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 ff 28 7f 48 04 	vmovdqu16 %ymm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 7e 28 7f 48 04 	vmovdqu32 %ymm1,0x80\(%rax\)
 +[a-f0-9]+:	62 f1 fe 28 7f 48 04 	vmovdqu64 %ymm1,0x80\(%rax\)
 +[a-f0-9]+:	62 b1 7d 08 6f d5    	vmovdqa32 %xmm21,%xmm2
 +[a-f0-9]+:	62 b1 fd 08 6f d5    	vmovdqa64 %xmm21,%xmm2
 +[a-f0-9]+:	62 b1 7f 08 6f d5    	vmovdqu8 %xmm21,%xmm2
 +[a-f0-9]+:	62 b1 ff 08 6f d5    	vmovdqu16 %xmm21,%xmm2
 +[a-f0-9]+:	62 b1 7e 08 6f d5    	vmovdqu32 %xmm21,%xmm2
 +[a-f0-9]+:	62 b1 fe 08 6f d5    	vmovdqu64 %xmm21,%xmm2
 +[a-f0-9]+:	62 f1 7d 48 6f d1    	vmovdqa32 %zmm1,%zmm2
 +[a-f0-9]+:	62 f1 fd 48 6f d1    	vmovdqa64 %zmm1,%zmm2
 +[a-f0-9]+:	62 f1 7f 48 6f d1    	vmovdqu8 %zmm1,%zmm2
 +[a-f0-9]+:	62 f1 ff 48 6f d1    	vmovdqu16 %zmm1,%zmm2
 +[a-f0-9]+:	62 f1 7e 48 6f d1    	vmovdqu32 %zmm1,%zmm2
 +[a-f0-9]+:	62 f1 fe 48 6f d1    	vmovdqu64 %zmm1,%zmm2
 +[a-f0-9]+:	62 f1 7d 28 6f d1    	vmovdqa32 %ymm1,%ymm2
 +[a-f0-9]+:	62 f1 fd 28 6f d1    	vmovdqa64 %ymm1,%ymm2
 +[a-f0-9]+:	62 f1 7f 08 6f d1    	vmovdqu8 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 ff 08 6f d1    	vmovdqu16 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 7e 08 6f d1    	vmovdqu32 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 fe 08 6f d1    	vmovdqu64 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 7d 29 6f d1    	vmovdqa32 %ymm1,%ymm2\{%k1\}
 +[a-f0-9]+:	62 f1 fd 29 6f d1    	vmovdqa64 %ymm1,%ymm2\{%k1\}
 +[a-f0-9]+:	62 f1 7f 09 6f d1    	vmovdqu8 %xmm1,%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 ff 09 6f d1    	vmovdqu16 %xmm1,%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 7e 09 6f d1    	vmovdqu32 %xmm1,%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 fe 09 6f d1    	vmovdqu64 %xmm1,%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 7d 29 6f 10    	vmovdqa32 \(%rax\),%ymm2\{%k1\}
 +[a-f0-9]+:	62 f1 fd 29 6f 10    	vmovdqa64 \(%rax\),%ymm2\{%k1\}
 +[a-f0-9]+:	62 f1 7f 09 6f 10    	vmovdqu8 \(%rax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 ff 09 6f 10    	vmovdqu16 \(%rax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 7e 09 6f 10    	vmovdqu32 \(%rax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 fe 09 6f 10    	vmovdqu64 \(%rax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 7d 29 7f 08    	vmovdqa32 %ymm1,\(%rax\)\{%k1\}
 +[a-f0-9]+:	62 f1 fd 29 7f 08    	vmovdqa64 %ymm1,\(%rax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7f 09 7f 08    	vmovdqu8 %xmm1,\(%rax\)\{%k1\}
 +[a-f0-9]+:	62 f1 ff 09 7f 08    	vmovdqu16 %xmm1,\(%rax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7e 09 7f 08    	vmovdqu32 %xmm1,\(%rax\)\{%k1\}
 +[a-f0-9]+:	62 f1 fe 09 7f 08    	vmovdqu64 %xmm1,\(%rax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7d 89 6f d1    	vmovdqa32 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 fd 89 6f d1    	vmovdqa64 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 7f 89 6f d1    	vmovdqu8 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 ff 89 6f d1    	vmovdqu16 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 7e 89 6f d1    	vmovdqu32 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 fe 89 6f d1    	vmovdqu64 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	c5 .*	vpand  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpand  %xmm3,%xmm12,%xmm4
 +[a-f0-9]+:	c5 .*	vpandn %xmm2,%xmm13,%xmm4
 +[a-f0-9]+:	c5 .*	vpandn %xmm2,%xmm3,%xmm14
 +[a-f0-9]+:	c5 .*	vpor   %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpor   %xmm3,%xmm12,%xmm4
 +[a-f0-9]+:	c5 .*	vpxor  %xmm2,%xmm13,%xmm4
 +[a-f0-9]+:	c5 .*	vpxor  %xmm2,%xmm3,%xmm14
 +[a-f0-9]+:	c5 .*	vpand  %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpand  %ymm3,%ymm12,%ymm4
 +[a-f0-9]+:	c5 .*	vpandn %ymm2,%ymm13,%ymm4
 +[a-f0-9]+:	c5 .*	vpandn %ymm2,%ymm3,%ymm14
 +[a-f0-9]+:	c5 .*	vpor   %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpor   %ymm3,%ymm12,%ymm4
 +[a-f0-9]+:	c5 .*	vpxor  %ymm2,%ymm13,%ymm4
 +[a-f0-9]+:	c5 .*	vpxor  %ymm2,%ymm3,%ymm14
 +[a-f0-9]+:	c5 .*	vpand  0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpand  0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpandn 0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpandn 0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpor   0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpor   0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpxor  0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpxor  0x70\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandd 0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandq 0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnd 0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnq 0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpord  0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vporq  0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxord 0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxorq 0x80\(%rax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpand  0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpand  0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpandn 0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpandn 0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpor   0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpor   0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpxor  0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpxor  0x60\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandd 0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandq 0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandnd 0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandnq 0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpord  0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vporq  0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpxord 0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpxorq 0x80\(%rax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandd %xmm22,%xmm23,%xmm24
 +[a-f0-9]+:	62 .*	vpandq %ymm22,%ymm3,%ymm4
 +[a-f0-9]+:	62 .*	vpandnd %ymm2,%ymm23,%ymm4
 +[a-f0-9]+:	62 .*	vpandnq %xmm2,%xmm3,%xmm24
 +[a-f0-9]+:	62 .*	vpord  %xmm22,%xmm23,%xmm24
 +[a-f0-9]+:	62 .*	vporq  %ymm22,%ymm3,%ymm4
 +[a-f0-9]+:	62 .*	vpxord %ymm2,%ymm23,%ymm4
 +[a-f0-9]+:	62 .*	vpxorq %xmm2,%xmm3,%xmm24
 +[a-f0-9]+:	62 .*	vpandd %xmm2,%xmm3,%xmm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandq %ymm12,%ymm3,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandnd %ymm2,%ymm13,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandnq %xmm2,%xmm3,%xmm14\{%k5\}
 +[a-f0-9]+:	62 .*	vpord  %xmm2,%xmm3,%xmm4\{%k5\}
 +[a-f0-9]+:	62 .*	vporq  %ymm12,%ymm3,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpxord %ymm2,%ymm13,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpxorq %xmm2,%xmm3,%xmm14\{%k5\}
 +[a-f0-9]+:	62 .*	vpandd \(%rax\)\{1to8\},%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandq \(%rax\)\{1to2\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnd \(%rax\)\{1to4\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnq \(%rax\)\{1to4\},%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpord  \(%rax\)\{1to8\},%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vporq  \(%rax\)\{1to2\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxord \(%rax\)\{1to4\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxorq \(%rax\)\{1to4\},%ymm2,%ymm3
#pass
