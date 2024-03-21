#as: -mvexwig=1
#objdump: -dw
#name: i386 AVX WIG insns with -mvexwig=1

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	c4 e2 f8 f2 00       	andn   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e2 f8 f7 00       	bextr  %eax,\(%eax\),%eax
 +[a-f0-9]+:	8f ea f8 10 00 00 00 00 00 	bextr  \$0x0,\(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 01 08       	blcfill \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 02 30       	blci   \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 01 28       	blcic  \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 02 08       	blcmsk \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 01 18       	blcs   \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 01 10       	blsfill \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 f8 f3 18       	blsi   \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 01 30       	blsic  \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 f8 f3 10       	blsmsk \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 f8 f3 08       	blsr   \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 f8 f5 00       	bzhi   %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e1 fb 92 c0       	kmovd  %eax,%k0
 +[a-f0-9]+:	c4 e1 fb 93 c0       	kmovd  %k0,%eax
 +[a-f0-9]+:	8f e9 f8 12 c0       	llwpcb %eax
 +[a-f0-9]+:	8f ea f8 12 00 00 00 00 00 	lwpins \$0x0,\(%eax\),%eax
 +[a-f0-9]+:	8f ea f8 12 08 00 00 00 00 	lwpval \$0x0,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 fb f6 00       	mulx   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e2 fb f5 00       	pdep   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e2 fa f5 00       	pext   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e3 fb f0 00 00    	rorx   \$0x0,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 fa f7 00       	sarx   %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 f9 f7 00       	shlx   %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 fb f7 00       	shrx   %eax,\(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 12 c8       	slwpcb %eax
 +[a-f0-9]+:	8f e9 f8 01 38       	t1mskc \(%eax\),%eax
 +[a-f0-9]+:	8f e9 f8 01 20       	tzmsk  \(%eax\),%eax
 +[a-f0-9]+:	c4 e1 cd 58 d4       	vaddpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 58 d4       	vaddps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb 58 d4       	vaddsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 58 d4       	vaddss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 cd d0 d4       	vaddsubpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cf d0 d4       	vaddsubps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 c9 de d4       	vaesdec %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 df d4       	vaesdeclast %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 dc d4       	vaesenc %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 dd d4       	vaesenclast %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 f9 db f4       	vaesimc %xmm4,%xmm6
 +[a-f0-9]+:	c4 e3 f9 df f4 07    	vaeskeygenassist \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 cd 55 d4       	vandnpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 55 d4       	vandnps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 54 d4       	vandpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 54 d4       	vandps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 cd 0d d4 07    	vblendpd \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 cd 0c d4 07    	vblendps \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd c2 d4 00    	vcmpeqpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc c2 d4 00    	vcmpeqps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb c2 d4 00    	vcmpeqsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca c2 d4 00    	vcmpeqss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 cd c2 d4 07    	vcmpordpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc c2 d4 07    	vcmpordps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb c2 d4 07    	vcmpordsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca c2 d4 07    	vcmpordss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 f9 2f f4       	vcomisd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f8 2f f4       	vcomiss %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 fe e6 e4       	vcvtdq2pd %xmm4,%ymm4
 +[a-f0-9]+:	c4 e1 fc 5b f4       	vcvtdq2ps %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 ff e6 e4       	vcvtpd2dq %ymm4,%xmm4
 +[a-f0-9]+:	c4 e1 fb e6 f4       	vcvtpd2dq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 ff e6 e4       	vcvtpd2dq %ymm4,%xmm4
 +[a-f0-9]+:	c4 e1 fd 5a e4       	vcvtpd2ps %ymm4,%xmm4
 +[a-f0-9]+:	c4 e1 f9 5a f4       	vcvtpd2ps %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 fd 5a e4       	vcvtpd2ps %ymm4,%xmm4
 +[a-f0-9]+:	c4 e1 fd 5b f4       	vcvtps2dq %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fc 5a e4       	vcvtps2pd %xmm4,%ymm4
 +[a-f0-9]+:	c4 e1 cb 5a d4       	vcvtsd2ss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 fa 2a c0       	vcvtsi2ss %eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 fa 2a 00       	vcvtsi2ssl? \(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 fb 2a c0       	vcvtsi2sd %eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 fb 2a 00       	vcvtsi2sdl? \(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 ca 5a d4       	vcvtss2sd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 fa 2d c0       	vcvtss2si %xmm0,%eax
 +[a-f0-9]+:	c4 e1 fb 2d c0       	vcvtsd2si %xmm0,%eax
 +[a-f0-9]+:	c4 e1 fd e6 e4       	vcvttpd2dq %ymm4,%xmm4
 +[a-f0-9]+:	c4 e1 f9 e6 f4       	vcvttpd2dq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 fd e6 e4       	vcvttpd2dq %ymm4,%xmm4
 +[a-f0-9]+:	c4 e1 fe 5b f4       	vcvttps2dq %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fa 2c c0       	vcvttss2si %xmm0,%eax
 +[a-f0-9]+:	c4 e1 fb 2c c0       	vcvttsd2si %xmm0,%eax
 +[a-f0-9]+:	c4 e1 cd 5e d4       	vdivpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 5e d4       	vdivps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb 5e d4       	vdivsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 5e d4       	vdivss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 41 d4 07    	vdppd  \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 cd 40 d4 07    	vdpps  \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 f9 17 21 07    	vextractps \$0x7,%xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 cd 7c d4       	vhaddpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cf 7c d4       	vhaddps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 7d d4       	vhsubpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cf 7d d4       	vhsubps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 c9 21 d4 07    	vinsertps \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ff f0 21       	vlddqu \(%ecx\),%ymm4
 +[a-f0-9]+:	c4 e1 f8 ae 11       	vldmxcsr \(%ecx\)
 +[a-f0-9]+:	c4 e1 f9 f7 f4       	vmaskmovdqu %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 cd 5f d4       	vmaxpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 5f d4       	vmaxps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb 5f d4       	vmaxsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 5f d4       	vmaxss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 cd 5d d4       	vminpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 5d d4       	vminps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb 5d d4       	vminsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 5d d4       	vminss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 fd 28 f4       	vmovapd %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fc 28 f4       	vmovaps %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fd 29 e6       	vmovapd %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fc 29 e6       	vmovaps %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 f9 6e c0       	vmovd  %eax,%xmm0
 +[a-f0-9]+:	c4 e1 f9 6e 00       	vmovd  \(%eax\),%xmm0
 +[a-f0-9]+:	c4 e1 f9 7e c0       	vmovd  %xmm0,%eax
 +[a-f0-9]+:	c4 e1 f9 7e 00       	vmovd  %xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e1 ff 12 f4       	vmovddup %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fd 6f f4       	vmovdqa %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fe 6f f4       	vmovdqu %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fd 7f e6       	vmovdqa %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fe 7f e6       	vmovdqu %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 c8 12 d4       	vmovhlps %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 d9 16 31       	vmovhpd \(%ecx\),%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f9 17 21       	vmovhpd %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 d8 16 31       	vmovhps \(%ecx\),%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f8 17 21       	vmovhps %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 c8 16 d4       	vmovlhps %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 d9 12 31       	vmovlpd \(%ecx\),%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f9 13 21       	vmovlpd %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 d8 12 31       	vmovlps \(%ecx\),%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f8 13 21       	vmovlps %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 f9 50 cc       	vmovmskpd %xmm4,%ecx
 +[a-f0-9]+:	c4 e1 f8 50 cc       	vmovmskps %xmm4,%ecx
 +[a-f0-9]+:	c4 e1 fd e7 21       	vmovntdq %ymm4,\(%ecx\)
 +[a-f0-9]+:	c4 e2 f9 2a 21       	vmovntdqa \(%ecx\),%xmm4
 +[a-f0-9]+:	c4 e1 fd 2b 21       	vmovntpd %ymm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 fc 2b 21       	vmovntps %ymm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 fa 7e f4       	vmovq  %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f9 d6 21       	vmovq  %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 fb 10 21       	vmovsd \(%ecx\),%xmm4
 +[a-f0-9]+:	c4 e1 fb 11 21       	vmovsd %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 fe 16 f4       	vmovshdup %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fe 12 f4       	vmovsldup %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fa 10 21       	vmovss \(%ecx\),%xmm4
 +[a-f0-9]+:	c4 e1 fa 11 21       	vmovss %xmm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 fd 10 f4       	vmovupd %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fd 11 21       	vmovupd %ymm4,\(%ecx\)
 +[a-f0-9]+:	c4 e1 fc 10 f4       	vmovups %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fc 11 21       	vmovups %ymm4,\(%ecx\)
 +[a-f0-9]+:	c4 e3 c9 42 d4 07    	vmpsadbw \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 cd 59 d4       	vmulpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 59 d4       	vmulps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb 59 d4       	vmulsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 59 d4       	vmulss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 cd 56 d4       	vorpd  %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 56 d4       	vorps  %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 f9 1c f4       	vpabsb %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 1e f4       	vpabsd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 1d f4       	vpabsw %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 c9 6b d4       	vpackssdw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 63 d4       	vpacksswb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 2b d4       	vpackusdw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 67 d4       	vpackuswb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 fc d4       	vpaddb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 fe d4       	vpaddd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 d4 d4       	vpaddq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 ec d4       	vpaddsb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 ed d4       	vpaddsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 dc d4       	vpaddusb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 dd d4       	vpaddusw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 fd d4       	vpaddw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 0f d4 07    	vpalignr \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 db d4       	vpand  %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 df d4       	vpandn %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e0 d4       	vpavgb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e3 d4       	vpavgw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 0e d4 07    	vpblendw \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 44 d4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 44 d4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 44 d4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 44 d4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 44 d4 07    	vpclmulqdq \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 74 d4       	vpcmpeqb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 76 d4       	vpcmpeqd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 29 d4       	vpcmpeqq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 75 d4       	vpcmpeqw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 f9 61 c0 00    	vpcmpestri \$0x0,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 f9 60 c0 00    	vpcmpestrm \$0x0,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 c9 64 d4       	vpcmpgtb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 66 d4       	vpcmpgtd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 37 d4       	vpcmpgtq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 65 d4       	vpcmpgtw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 f9 63 f4 07    	vpcmpistri \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e3 f9 62 f4 07    	vpcmpistrm \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e3 f9 14 c0 00    	vpextrb \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 f9 14 00 00    	vpextrb \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e3 f9 16 c0 00    	vpextrd \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 f9 16 00 00    	vpextrd \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e1 f9 c5 c0 00    	vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 f9 15 c0 00    	vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 f9 15 00 00    	vpextrw \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e2 c9 02 d4       	vphaddd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 03 d4       	vphaddsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 01 d4       	vphaddw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 f9 41 f4       	vphminposuw %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 c9 06 d4       	vphsubd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 07 d4       	vphsubsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 05 d4       	vphsubw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 f9 20 c0 00    	vpinsrb \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 f9 20 00 00    	vpinsrb \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 f9 22 c0 00    	vpinsrd \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 f9 22 00 00    	vpinsrd \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 f9 c4 c0 00    	vpinsrw \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 f9 c4 00 00    	vpinsrw \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e2 c9 04 d4       	vpmaddubsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f5 d4       	vpmaddwd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 3c d4       	vpmaxsb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 3d d4       	vpmaxsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 ee d4       	vpmaxsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 de d4       	vpmaxub %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 3f d4       	vpmaxud %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 3e d4       	vpmaxuw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 38 d4       	vpminsb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 39 d4       	vpminsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 ea d4       	vpminsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 da d4       	vpminub %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 3b d4       	vpminud %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 3a d4       	vpminuw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 f9 d7 cc       	vpmovmskb %xmm4,%ecx
 +[a-f0-9]+:	c4 e2 f9 21 f4       	vpmovsxbd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 22 f4       	vpmovsxbq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 20 f4       	vpmovsxbw %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 25 f4       	vpmovsxdq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 23 f4       	vpmovsxwd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 24 f4       	vpmovsxwq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 31 f4       	vpmovzxbd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 32 f4       	vpmovzxbq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 30 f4       	vpmovzxbw %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 35 f4       	vpmovzxdq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 33 f4       	vpmovzxwd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 f9 34 f4       	vpmovzxwq %xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 c9 28 d4       	vpmuldq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 0b d4       	vpmulhrsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e4 d4       	vpmulhuw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e5 d4       	vpmulhw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 40 d4       	vpmulld %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 d5 d4       	vpmullw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f4 d4       	vpmuludq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 eb d4       	vpor   %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f6 d4       	vpsadbw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 00 d4       	vpshufb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 f9 70 f4 07    	vpshufd \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 fa 70 f4 07    	vpshufhw \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 fb 70 f4 07    	vpshuflw \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e2 c9 08 d4       	vpsignb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 0a d4       	vpsignd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 c9 09 d4       	vpsignw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f2 d4       	vpslld %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 73 fc 07    	vpslldq \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 c9 f3 d4       	vpsllq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f1 d4       	vpsllw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e2 d4       	vpsrad %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e1 d4       	vpsraw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 d2 d4       	vpsrld %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 73 dc 07    	vpsrldq \$0x7,%xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 c9 d3 d4       	vpsrlq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 d1 d4       	vpsrlw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f8 d4       	vpsubb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 fa d4       	vpsubd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 fb d4       	vpsubq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e8 d4       	vpsubsb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 e9 d4       	vpsubsw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 d8 d4       	vpsubusb %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 d9 d4       	vpsubusw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 f9 d4       	vpsubw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e2 fd 17 f4       	vptest %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 c9 68 d4       	vpunpckhbw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 6a d4       	vpunpckhdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 6d d4       	vpunpckhqdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 69 d4       	vpunpckhwd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 60 d4       	vpunpcklbw %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 62 d4       	vpunpckldq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 6c d4       	vpunpcklqdq %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 61 d4       	vpunpcklwd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 c9 ef d4       	vpxor  %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 fc 53 f4       	vrcpps %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 ca 53 d4       	vrcpss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 fd 09 d6 07    	vroundpd \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 fd 08 d6 07    	vroundps \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 c9 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e3 c9 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 fc 52 f4       	vrsqrtps %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 ca 52 d4       	vrsqrtss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 cd c6 d4 07    	vshufpd \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc c6 d4 07    	vshufps \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 fd 51 f4       	vsqrtpd %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 fc 51 f4       	vsqrtps %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 cb 51 d4       	vsqrtsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 51 d4       	vsqrtss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 f8 ae 19       	vstmxcsr \(%ecx\)
 +[a-f0-9]+:	c4 e1 cd 5c d4       	vsubpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 5c d4       	vsubps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cb 5c d4       	vsubsd %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 ca 5c d4       	vsubss %xmm4,%xmm6,%xmm2
 +[a-f0-9]+:	c4 e1 f9 2e f4       	vucomisd %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 f8 2e f4       	vucomiss %xmm4,%xmm6
 +[a-f0-9]+:	c4 e1 cd 15 d4       	vunpckhpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 15 d4       	vunpckhps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 14 d4       	vunpcklpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 14 d4       	vunpcklps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 57 d4       	vxorpd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cc 57 d4       	vxorps %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 fc 77          	vzeroall
 +[a-f0-9]+:	c4 e1 f8 77          	vzeroupper
#pass
