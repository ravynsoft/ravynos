#as: -O2
#objdump: -dw
#name: x86-64 AVX/AVX2 w/ source swapping

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 c1 4d 58 d6       	vaddpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 58 d6       	vaddps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d d0 d6       	vaddsubpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4f d0 d6       	vaddsubps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 55 d6       	vandnpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 55 d6       	vandnps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d 54 d6          	vandpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c 54 d6          	vandps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 5e d6       	vdivpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 5e d6       	vdivps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 7c d6       	vhaddpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4f 7c d6       	vhaddps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 7d d6       	vhsubpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4f 7d d6       	vhsubps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 5f d6       	vmaxpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 5f d6       	vmaxps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 5d d6       	vminpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 5d d6       	vminps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 59 d6       	vmulpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 59 d6       	vmulps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d 56 d6          	vorpd  %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c 56 d6          	vorps  %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d fc d6          	vpaddb %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d fd d6          	vpaddw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d fe d6          	vpaddd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d d4 d6          	vpaddq %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d ec d6          	vpaddsb %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d ed d6          	vpaddsw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d dc d6          	vpaddusb %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d dd d6          	vpaddusw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d db d6          	vpand  %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d df d6       	vpandn %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d e0 d6          	vpavgb %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d e3 d6          	vpavgw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d 74 d6          	vpcmpeqb %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d 75 d6          	vpcmpeqw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d 76 d6          	vpcmpeqd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 29 d6       	vpcmpeqq %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 64 d6       	vpcmpgtb %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 65 d6       	vpcmpgtw %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 66 d6       	vpcmpgtd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 37 d6       	vpcmpgtq %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d f5 d6          	vpmaddwd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 3c d6       	vpmaxsb %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d ee d6          	vpmaxsw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 3d d6       	vpmaxsd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d de d6          	vpmaxub %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 3e d6       	vpmaxuw %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 3f d6       	vpmaxud %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 38 d6       	vpminsb %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d ea d6          	vpminsw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 39 d6       	vpminsd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d da d6          	vpminub %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 3a d6       	vpminuw %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 3b d6       	vpminud %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d e4 d6          	vpmulhuw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d e5 d6          	vpmulhw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d d5 d6          	vpmullw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 40 d6       	vpmulld %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d f4 d6          	vpmuludq %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c2 4d 28 d6       	vpmuldq %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d eb d6          	vpor   %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d f6 d6          	vpsadbw %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d f8 d6       	vpsubb %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d f9 d6       	vpsubw %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d fa d6       	vpsubd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d fb d6       	vpsubq %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d e8 d6       	vpsubsb %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d e9 d6       	vpsubsw %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d d8 d6       	vpsubusb %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d d9 d6       	vpsubusw %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d ef d6          	vpxor  %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d 5c d6       	vsubpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c 5c d6       	vsubps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d 57 d6          	vxorpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c 57 d6          	vxorps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 00       	vcmpeqpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 01    	vcmpltpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 02    	vcmplepd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 03       	vcmpunordpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 04       	vcmpneqpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 05    	vcmpnltpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 06    	vcmpnlepd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 07       	vcmpordpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 08       	vcmpeq_uqpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 09    	vcmpngepd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 0a    	vcmpngtpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 0b       	vcmpfalsepd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 0c       	vcmpneq_oqpd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 0d    	vcmpgepd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 0e    	vcmpgtpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 0f       	vcmptruepd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 10       	vcmpeq_ospd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 11    	vcmplt_oqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 12    	vcmple_oqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 13       	vcmpunord_spd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 14       	vcmpneq_uspd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 15    	vcmpnlt_uqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 16    	vcmpnle_uqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 17       	vcmpord_spd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 18       	vcmpeq_uspd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 19    	vcmpnge_uqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 1a    	vcmpngt_uqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 1b       	vcmpfalse_ospd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 1c       	vcmpneq_ospd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 1d    	vcmpge_oqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 1e    	vcmpgt_oqpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8d c2 d6 1f       	vcmptrue_uspd %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 00       	vcmpeqps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 01    	vcmpltps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 02    	vcmpleps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 03       	vcmpunordps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 04       	vcmpneqps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 05    	vcmpnltps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 06    	vcmpnleps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 07       	vcmpordps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 08       	vcmpeq_uqps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 09    	vcmpngeps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 0a    	vcmpngtps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 0b       	vcmpfalseps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 0c       	vcmpneq_oqps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 0d    	vcmpgeps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 0e    	vcmpgtps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 0f       	vcmptrueps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 10       	vcmpeq_osps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 11    	vcmplt_oqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 12    	vcmple_oqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 13       	vcmpunord_sps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 14       	vcmpneq_usps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 15    	vcmpnlt_uqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 16    	vcmpnle_uqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 17       	vcmpord_sps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 18       	vcmpeq_usps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 19    	vcmpnge_uqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 1a    	vcmpngt_uqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 1b       	vcmpfalse_osps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 1c       	vcmpneq_osps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 1d    	vcmpge_oqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 1e    	vcmpgt_oqps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 8c c2 d6 1f       	vcmptrue_usps %ymm6,%ymm14,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4d c2 d6 07    	vcmpordpd %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 4c c2 d6 07    	vcmpordps %ymm14,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 c1 49 58 d6       	vaddpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 58 d6       	vaddps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 d0 d6       	vaddsubpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b d0 d6       	vaddsubps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 55 d6       	vandnpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 55 d6       	vandnps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 54 d6          	vandpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 54 d6          	vandps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 5e d6       	vdivpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 5e d6       	vdivps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 7c d6       	vhaddpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 7c d6       	vhaddps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 7d d6       	vhsubpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 7d d6       	vhsubps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 5f d6       	vmaxpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 5f d6       	vmaxps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 5d d6       	vminpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 5d d6       	vminps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 59 d6       	vmulpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 59 d6       	vmulps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 56 d6          	vorpd  %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 56 d6          	vorps  %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 fc d6          	vpaddb %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 fd d6          	vpaddw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 fe d6          	vpaddd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 d4 d6          	vpaddq %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 ec d6          	vpaddsb %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 ed d6          	vpaddsw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 dc d6          	vpaddusb %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 dd d6          	vpaddusw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 db d6          	vpand  %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 df d6       	vpandn %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 e0 d6          	vpavgb %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 e3 d6          	vpavgw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 74 d6          	vpcmpeqb %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 75 d6          	vpcmpeqw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 76 d6          	vpcmpeqd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 29 d6       	vpcmpeqq %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 64 d6       	vpcmpgtb %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 65 d6       	vpcmpgtw %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 66 d6       	vpcmpgtd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 37 d6       	vpcmpgtq %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 f5 d6          	vpmaddwd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 3c d6       	vpmaxsb %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 ee d6          	vpmaxsw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 3d d6       	vpmaxsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 de d6          	vpmaxub %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 3e d6       	vpmaxuw %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 3f d6       	vpmaxud %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 38 d6       	vpminsb %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 ea d6          	vpminsw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 39 d6       	vpminsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 da d6          	vpminub %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 3a d6       	vpminuw %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 3b d6       	vpminud %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 e4 d6          	vpmulhuw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 e5 d6          	vpmulhw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 d5 d6          	vpmullw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 40 d6       	vpmulld %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 f4 d6          	vpmuludq %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c2 49 28 d6       	vpmuldq %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 eb d6          	vpor   %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 f6 d6          	vpsadbw %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 f8 d6       	vpsubb %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 f9 d6       	vpsubw %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 fa d6       	vpsubd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 fb d6       	vpsubq %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 e8 d6       	vpsubsb %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 e9 d6       	vpsubsw %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 d8 d6       	vpsubusb %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 d9 d6       	vpsubusw %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 ef d6          	vpxor  %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 5c d6       	vsubpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 5c d6       	vsubps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 57 d6          	vxorpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 57 d6          	vxorps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 00       	vcmpeqpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 01    	vcmpltpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 02    	vcmplepd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 03       	vcmpunordpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 04       	vcmpneqpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 05    	vcmpnltpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 06    	vcmpnlepd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 07       	vcmpordpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 08       	vcmpeq_uqpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 09    	vcmpngepd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 0a    	vcmpngtpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 0b       	vcmpfalsepd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 0c       	vcmpneq_oqpd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 0d    	vcmpgepd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 0e    	vcmpgtpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 0f       	vcmptruepd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 10       	vcmpeq_ospd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 11    	vcmplt_oqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 12    	vcmple_oqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 13       	vcmpunord_spd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 14       	vcmpneq_uspd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 15    	vcmpnlt_uqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 16    	vcmpnle_uqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 17       	vcmpord_spd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 18       	vcmpeq_uspd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 19    	vcmpnge_uqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 1a    	vcmpngt_uqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 1b       	vcmpfalse_ospd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 1c       	vcmpneq_ospd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 1d    	vcmpge_oqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 1e    	vcmpgt_oqpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 89 c2 d6 1f       	vcmptrue_uspd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 00       	vcmpeqps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 01    	vcmpltps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 02    	vcmpleps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 03       	vcmpunordps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 04       	vcmpneqps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 05    	vcmpnltps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 06    	vcmpnleps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 07       	vcmpordps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 08       	vcmpeq_uqps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 09    	vcmpngeps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 0a    	vcmpngtps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 0b       	vcmpfalseps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 0c       	vcmpneq_oqps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 0d    	vcmpgeps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 0e    	vcmpgtps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 0f       	vcmptrueps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 10       	vcmpeq_osps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 11    	vcmplt_oqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 12    	vcmple_oqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 13       	vcmpunord_sps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 14       	vcmpneq_usps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 15    	vcmpnlt_uqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 16    	vcmpnle_uqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 17       	vcmpord_sps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 18       	vcmpeq_usps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 19    	vcmpnge_uqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 1a    	vcmpngt_uqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 1b       	vcmpfalse_osps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 1c       	vcmpneq_osps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 1d    	vcmpge_oqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 1e    	vcmpgt_oqps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 88 c2 d6 1f       	vcmptrue_usps %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 49 c2 d6 07    	vcmpordpd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 48 c2 d6 07    	vcmpordps %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 79 2f f6       	vcomisd %xmm14,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 79 2e f6       	vucomisd %xmm14,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 07    	vcmpordsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 58 d6       	vaddsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 5e d6       	vdivsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 5f d6       	vmaxsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 5d d6       	vminsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 59 d6       	vmulsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 51 d6       	vsqrtsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b 5c d6       	vsubsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 00       	vcmpeqsd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 01    	vcmpltsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 02    	vcmplesd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 03       	vcmpunordsd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 04       	vcmpneqsd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 05    	vcmpnltsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 06    	vcmpnlesd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 07       	vcmpordsd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 08       	vcmpeq_uqsd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 09    	vcmpngesd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 0a    	vcmpngtsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 0b       	vcmpfalsesd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 0c       	vcmpneq_oqsd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 0d    	vcmpgesd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 0e    	vcmpgtsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 0f       	vcmptruesd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 10       	vcmpeq_ossd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 11    	vcmplt_oqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 12    	vcmple_oqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 13       	vcmpunord_ssd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 14       	vcmpneq_ussd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 15    	vcmpnlt_uqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 16    	vcmpnle_uqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 17       	vcmpord_ssd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 18       	vcmpeq_ussd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 19    	vcmpnge_uqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 1a    	vcmpngt_uqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 1b       	vcmpfalse_ossd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 1c       	vcmpneq_ossd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 1d    	vcmpge_oqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4b c2 d6 1e    	vcmpgt_oqsd %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8b c2 d6 1f       	vcmptrue_ussd %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 58 d6       	vaddss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 5e d6       	vdivss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 5f d6       	vmaxss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 5d d6       	vminss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 59 d6       	vmulss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 53 d6       	vrcpss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 52 d6       	vrsqrtss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 51 d6       	vsqrtss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a 5c d6       	vsubss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 00       	vcmpeqss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 01    	vcmpltss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 02    	vcmpless %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 03       	vcmpunordss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 04       	vcmpneqss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 05    	vcmpnltss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 06    	vcmpnless %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 07       	vcmpordss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 08       	vcmpeq_uqss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 09    	vcmpngess %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 0a    	vcmpngtss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 0b       	vcmpfalsess %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 0c       	vcmpneq_oqss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 0d    	vcmpgess %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 0e    	vcmpgtss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 0f       	vcmptruess %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 10       	vcmpeq_osss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 11    	vcmplt_oqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 12    	vcmple_oqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 13       	vcmpunord_sss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 14       	vcmpneq_usss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 15    	vcmpnlt_uqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 16    	vcmpnle_uqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 17       	vcmpord_sss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 18       	vcmpeq_usss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 19    	vcmpnge_uqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 1a    	vcmpngt_uqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 1b       	vcmpfalse_osss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 1c       	vcmpneq_osss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 1d    	vcmpge_oqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 1e    	vcmpgt_oqss %xmm14,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 8a c2 d6 1f       	vcmptrue_usss %xmm6,%xmm14,%xmm2
[ 	]*[a-f0-9]+:	c4 c1 78 2f f6       	vcomiss %xmm14,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 78 2e f6       	vucomiss %xmm14,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a c2 d6 07    	vcmpordss %xmm14,%xmm6,%xmm2
#pass
