#as: -Os
#objdump: -drw
#name: optimized encoding 2 with -Os

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	a8 7f                	test   \$0x7f,%al
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	f6 c3 7f             	test   \$0x7f,%bl
 +[a-f0-9]+:	f7 c7 7f 00 00 00    	test   \$0x7f,%edi
 +[a-f0-9]+:	66 f7 c7 7f 00       	test   \$0x7f,%di
 +[a-f0-9]+:	20 c9                	and    %cl,%cl
 +[a-f0-9]+:	66 21 d2             	and    %dx,%dx
 +[a-f0-9]+:	21 db                	and    %ebx,%ebx
 +[a-f0-9]+:	08 e4                	or     %ah,%ah
 +[a-f0-9]+:	66 09 ed             	or     %bp,%bp
 +[a-f0-9]+:	09 f6                	or     %esi,%esi
 +[a-f0-9]+:	87 0a                	xchg   %ecx,\(%edx\)
 +[a-f0-9]+:	87 11                	xchg   %edx,\(%ecx\)
 +[a-f0-9]+:	c5 f1 55 e9          	vandnpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f9 6f d1          	vmovdqa %xmm1,%xmm2
 +[a-f0-9]+:	c5 f9 6f d1          	vmovdqa %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 fa 6f d1          	vmovdqu %xmm1,%xmm2
 +[a-f0-9]+:	c5 f9 6f 50 7f       	vmovdqa 0x7f\(%eax\),%xmm2
 +[a-f0-9]+:	c5 f9 6f 50 7f       	vmovdqa 0x7f\(%eax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%eax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%eax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%eax\),%xmm2
 +[a-f0-9]+:	c5 fa 6f 50 7f       	vmovdqu 0x7f\(%eax\),%xmm2
 +[a-f0-9]+:	62 f1 7d 08 7f 48 08 	vmovdqa32 %xmm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 fd 08 7f 48 08 	vmovdqa64 %xmm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 7f 08 7f 48 08 	vmovdqu8 %xmm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 ff 08 7f 48 08 	vmovdqu16 %xmm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 7e 08 7f 48 08 	vmovdqu32 %xmm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 fe 08 7f 48 08 	vmovdqu64 %xmm1,0x80\(%eax\)
 +[a-f0-9]+:	c5 fd 6f d1          	vmovdqa %ymm1,%ymm2
 +[a-f0-9]+:	c5 fd 6f d1          	vmovdqa %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fe 6f d1          	vmovdqu %ymm1,%ymm2
 +[a-f0-9]+:	c5 fd 6f 50 7f       	vmovdqa 0x7f\(%eax\),%ymm2
 +[a-f0-9]+:	c5 fd 6f 50 7f       	vmovdqa 0x7f\(%eax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%eax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%eax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%eax\),%ymm2
 +[a-f0-9]+:	c5 fe 6f 50 7f       	vmovdqu 0x7f\(%eax\),%ymm2
 +[a-f0-9]+:	62 f1 7d 28 7f 48 04 	vmovdqa32 %ymm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 fd 28 7f 48 04 	vmovdqa64 %ymm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 7f 28 7f 48 04 	vmovdqu8 %ymm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 ff 28 7f 48 04 	vmovdqu16 %ymm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 7e 28 7f 48 04 	vmovdqu32 %ymm1,0x80\(%eax\)
 +[a-f0-9]+:	62 f1 fe 28 7f 48 04 	vmovdqu64 %ymm1,0x80\(%eax\)
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
 +[a-f0-9]+:	62 f1 7d 29 6f 10    	vmovdqa32 \(%eax\),%ymm2\{%k1\}
 +[a-f0-9]+:	62 f1 fd 29 6f 10    	vmovdqa64 \(%eax\),%ymm2\{%k1\}
 +[a-f0-9]+:	62 f1 7f 09 6f 10    	vmovdqu8 \(%eax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 ff 09 6f 10    	vmovdqu16 \(%eax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 7e 09 6f 10    	vmovdqu32 \(%eax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 fe 09 6f 10    	vmovdqu64 \(%eax\),%xmm2\{%k1\}
 +[a-f0-9]+:	62 f1 7d 29 7f 08    	vmovdqa32 %ymm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 fd 29 7f 08    	vmovdqa64 %ymm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7f 09 7f 08    	vmovdqu8 %xmm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 ff 09 7f 08    	vmovdqu16 %xmm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7e 09 7f 08    	vmovdqu32 %xmm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 fe 09 7f 08    	vmovdqu64 %xmm1,\(%eax\)\{%k1\}
 +[a-f0-9]+:	62 f1 7d 89 6f d1    	vmovdqa32 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 fd 89 6f d1    	vmovdqa64 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 7f 89 6f d1    	vmovdqu8 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 ff 89 6f d1    	vmovdqu16 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 7e 89 6f d1    	vmovdqu32 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	62 f1 fe 89 6f d1    	vmovdqu64 %xmm1,%xmm2\{%k1\}\{z\}
 +[a-f0-9]+:	c5 .*	vpand  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpand  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpandn %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpandn %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpor   %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpor   %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpxor  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpxor  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpand  %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpand  %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpandn %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpandn %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpor   %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpor   %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpxor  %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpxor  %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpand  0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpand  0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpandn 0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpandn 0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpor   0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpor   0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpxor  0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpxor  0x70\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandd 0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandq 0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnd 0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnq 0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpord  0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vporq  0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxord 0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxorq 0x80\(%eax\),%xmm2,%xmm3
 +[a-f0-9]+:	c5 .*	vpand  0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpand  0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpandn 0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpandn 0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpor   0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpor   0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpxor  0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	c5 .*	vpxor  0x60\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandd 0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandq 0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandnd 0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandnq 0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpord  0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vporq  0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpxord 0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpxorq 0x80\(%eax\),%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandd %xmm2,%xmm3,%xmm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandq %ymm2,%ymm3,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandnd %ymm2,%ymm3,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandnq %xmm2,%xmm3,%xmm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpord  %xmm2,%xmm3,%xmm4\{%k5\}
 +[a-f0-9]+:	62 .*	vporq  %ymm2,%ymm3,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpxord %ymm2,%ymm3,%ymm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpxorq %xmm2,%xmm3,%xmm4\{%k5\}
 +[a-f0-9]+:	62 .*	vpandd \(%eax\)\{1to8\},%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpandq \(%eax\)\{1to2\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnd \(%eax\)\{1to4\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpandnq \(%eax\)\{1to4\},%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vpord  \(%eax\)\{1to8\},%ymm2,%ymm3
 +[a-f0-9]+:	62 .*	vporq  \(%eax\)\{1to2\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxord \(%eax\)\{1to4\},%xmm2,%xmm3
 +[a-f0-9]+:	62 .*	vpxorq \(%eax\)\{1to4\},%ymm2,%ymm3
#pass
