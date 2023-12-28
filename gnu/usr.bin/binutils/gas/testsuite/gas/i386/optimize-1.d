#as: -O2
#objdump: -drw
#name: optimized encoding 1 with -O2

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 f1 f5 4f 55 e9    	vandnpd %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 55 e9          	vandnpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 55 e9          	vandnpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 55 e9          	vandnpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 74 4f 55 e9    	vandnps %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f0 55 e9          	vandnps %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f0 55 e9          	vandnps %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f0 55 e9          	vandnps %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 75 4f df e9    	vpandnd %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 f5 4f df e9    	vpandnq %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 df e9          	vpandn %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 f5 4f 57 e9    	vxorpd %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 57 e9          	vxorpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 57 e9          	vxorpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 57 e9          	vxorpd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 74 4f 57 e9    	vxorps %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f0 57 e9          	vxorps %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f0 57 e9          	vxorps %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f0 57 e9          	vxorps %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 75 4f ef e9    	vpxord %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 f5 4f ef e9    	vpxorq %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 ef e9          	vpxor  %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 75 4f f8 e9    	vpsubb %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 f8 e9          	vpsubb %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 f8 e9          	vpsubb %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 f8 e9          	vpsubb %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 75 4f f9 e9    	vpsubw %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 f9 e9          	vpsubw %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 f9 e9          	vpsubw %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 f9 e9          	vpsubw %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 75 4f fa e9    	vpsubd %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 fa e9          	vpsubd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 fa e9          	vpsubd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 fa e9          	vpsubd %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	62 f1 f5 4f fb e9    	vpsubq %zmm1,%zmm1,%zmm5\{%k7\}
 +[a-f0-9]+:	c5 f1 fb e9          	vpsubq %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 fb e9          	vpsubq %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f1 fb e9          	vpsubq %xmm1,%xmm1,%xmm5
 +[a-f0-9]+:	c5 f4 47 e9          	kxorw  %k1,%k1,%k5
 +[a-f0-9]+:	c5 f4 47 e9          	kxorw  %k1,%k1,%k5
 +[a-f0-9]+:	c5 f4 42 e9          	kandnw %k1,%k1,%k5
 +[a-f0-9]+:	c5 f4 42 e9          	kandnw %k1,%k1,%k5
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
 +[a-f0-9]+:	62 f1 7d 48 6f 10    	vmovdqa32 \(%eax\),%zmm2
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
 +[a-f0-9]+:	0f ba e0 0f          	bt     \$0xf,%eax
 +[a-f0-9]+:	66 0f ba e0 10       	bt     \$0x10,%ax
 +[a-f0-9]+:	0f ba f8 0f          	btc    \$0xf,%eax
 +[a-f0-9]+:	0f ba f0 0f          	btr    \$0xf,%eax
 +[a-f0-9]+:	0f ba e8 0f          	bts    \$0xf,%eax
 +[a-f0-9]+:	0f ba e0 0f          	bt     \$0xf,%eax
 +[a-f0-9]+:	66 0f ba e0 10       	bt     \$0x10,%ax
 +[a-f0-9]+:	0f ba f8 0f          	btc    \$0xf,%eax
 +[a-f0-9]+:	0f ba f0 0f          	btr    \$0xf,%eax
 +[a-f0-9]+:	0f ba e8 0f          	bts    \$0xf,%eax
#pass
