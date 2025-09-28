#source: x86-64-optimize-2.s
#as: -O
#objdump: -drw
#name: x86-64 optimized encoding 2a with -O

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 71 f5 4f 55 f9    	vandnpd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 55 f9          	vandnpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 55 f9          	vandnpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 55 f9          	vandnpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 55 c1    	vandnpd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 f5 28 55 c1    	vandnpd %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 f5 40 55 c9    	vandnpd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 f5 20 55 c9    	vandnpd %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 74 4f 55 f9    	vandnps %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 70 55 f9          	vandnps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 70 55 f9          	vandnps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 70 55 f9          	vandnps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 74 48 55 c1    	vandnps %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 74 28 55 c1    	vandnps %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 74 40 55 c9    	vandnps %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 74 20 55 c9    	vandnps %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 71 75 4f df f9    	vpandnd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 df c1    	vpandnd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 75 28 df c1    	vpandnd %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 75 40 df c9    	vpandnd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 75 20 df c9    	vpandnd %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 f5 4f df f9    	vpandnq %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 df c1    	vpandnq %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 f5 28 df c1    	vpandnq %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 f5 40 df c9    	vpandnq %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 f5 20 df c9    	vpandnq %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 f5 4f 57 f9    	vxorpd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 57 f9          	vxorpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 57 f9          	vxorpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 57 f9          	vxorpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 57 c1    	vxorpd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 f5 28 57 c1    	vxorpd %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 f5 40 57 c9    	vxorpd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 f5 20 57 c9    	vxorpd %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 74 4f 57 f9    	vxorps %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 70 57 f9          	vxorps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 70 57 f9          	vxorps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 70 57 f9          	vxorps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 74 48 57 c1    	vxorps %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 74 28 57 c1    	vxorps %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 74 40 57 c9    	vxorps %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 74 20 57 c9    	vxorps %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 71 75 4f ef f9    	vpxord %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 ef c1    	vpxord %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 75 28 ef c1    	vpxord %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 75 40 ef c9    	vpxord %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 75 20 ef c9    	vpxord %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 f5 4f ef f9    	vpxorq %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 ef c1    	vpxorq %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 f5 28 ef c1    	vpxorq %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 f5 40 ef c9    	vpxorq %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 f5 20 ef c9    	vpxorq %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 75 4f f8 f9    	vpsubb %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 f8 f9          	vpsubb %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 f8 f9          	vpsubb %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 f8 f9          	vpsubb %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 f8 c1    	vpsubb %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 75 28 f8 c1    	vpsubb %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 75 40 f8 c9    	vpsubb %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 75 20 f8 c9    	vpsubb %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 75 4f f9 f9    	vpsubw %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 f9 f9          	vpsubw %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 f9 f9          	vpsubw %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 f9 f9          	vpsubw %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 f9 c1    	vpsubw %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 75 28 f9 c1    	vpsubw %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 75 40 f9 c9    	vpsubw %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 75 20 f9 c9    	vpsubw %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 75 4f fa f9    	vpsubd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 fa f9          	vpsubd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 fa f9          	vpsubd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 fa f9          	vpsubd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 fa c1    	vpsubd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 75 28 fa c1    	vpsubd %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 75 40 fa c9    	vpsubd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 75 20 fa c9    	vpsubd %ymm17,%ymm17,%ymm1
 +[a-f0-9]+:	62 71 f5 4f fb f9    	vpsubq %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 fb f9          	vpsubq %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 fb f9          	vpsubq %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	c5 71 fb f9          	vpsubq %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 fb c1    	vpsubq %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 e1 f5 28 fb c1    	vpsubq %ymm1,%ymm1,%ymm16
 +[a-f0-9]+:	62 b1 f5 40 fb c9    	vpsubq %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 b1 f5 20 fb c9    	vpsubq %ymm17,%ymm17,%ymm1
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
 +[a-f0-9]+:	62 f1 7d 48 6f 10    	vmovdqa32 \(%rax\),%zmm2
 +[a-f0-9]+:	c5 .*	vpand  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c4 .*	vpand  %xmm12,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpandn %xmm2,%xmm13,%xmm4
 +[a-f0-9]+:	c5 .*	vpandn %xmm2,%xmm3,%xmm14
 +[a-f0-9]+:	c5 .*	vpor   %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	c4 .*	vpor   %xmm12,%xmm3,%xmm4
 +[a-f0-9]+:	c5 .*	vpxor  %xmm2,%xmm13,%xmm4
 +[a-f0-9]+:	c5 .*	vpxor  %xmm2,%xmm3,%xmm14
 +[a-f0-9]+:	c5 .*	vpand  %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c4 .*	vpand  %ymm12,%ymm3,%ymm4
 +[a-f0-9]+:	c5 .*	vpandn %ymm2,%ymm13,%ymm4
 +[a-f0-9]+:	c5 .*	vpandn %ymm2,%ymm3,%ymm14
 +[a-f0-9]+:	c5 .*	vpor   %ymm2,%ymm3,%ymm4
 +[a-f0-9]+:	c4 .*	vpor   %ymm12,%ymm3,%ymm4
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
#pass
