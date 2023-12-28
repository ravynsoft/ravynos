#objdump: -dw
#name: x86-64 AVX

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fc 77             	vzeroall
[ 	]*[a-f0-9]+:	c5 f8 77             	vzeroupper
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb \(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 a2 55 cf b4 f0 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb 0xfe0\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb 0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb -0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb -0x1020\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	c5 ff e6 21          	vcvtpd2dqy \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps %ymm4,%xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a 21          	vcvtpd2psy \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 21          	vcvttpd2dqy \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb \$0x7b,\(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 a3 d5 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb \$0x7b,0xfe0\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb \$0x7b,0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1020\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb \$0x7b,\(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 a3 d5 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb \$0x7b,0xfe0\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1020\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd %ymm4,\(%rcx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps %ymm4,\(%rcx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 \$0x7,%xmm4,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 \$0x7,\(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 \$0x7,%ymm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 \$0x7,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dqx \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2psx \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dqx \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb \(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 cf b4 f0 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb 0x7f0\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb 0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb -0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb -0x810\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestril? \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestril? \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 f9 61 f4 07    	vpcmpestriq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestril? \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrml? \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrml? \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 f9 60 f4 07    	vpcmpestrmq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrml? \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb \$0x7b,\(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 a3 d1 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb \$0x7b,0x7f0\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb \$0x7b,0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x810\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb \$0x7b,\(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 a3 d1 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x7f0\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x810\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  %rcx,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  %rcx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 fb 2d cc       	vcvtsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2d 09       	vcvtsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2c cc       	vcvttsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2c 09       	vcvttsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 db 2a f1       	vcvtsi2sd %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 db 2a 31       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 da 2a f1       	vcvtsi2ss %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 da 2a 31       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d9 22 f1 07    	vpinsrq \$0x7,%rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d9 22 31 07    	vpinsrq \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 f9 16 e1 07    	vpextrq \$0x7,%xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 07    	vpextrq \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 fa 2d cc       	vcvtss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2d 09       	vcvtss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2c cc       	vcvttss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2c 09       	vcvttss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb %xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sdl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ssl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 f7 f4          	vmaskmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 c8 12 d4          	vmovhlps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 16 d4          	vmovlhps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 72 f4 07       	vpslld \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 fc 07       	vpslldq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 f4 07       	vpsllq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 71 f4 07       	vpsllw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 72 e4 07       	vpsrad \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 71 e4 07       	vpsraw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 72 d4 07       	vpsrld \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 dc 07       	vpsrldq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 d4 07       	vpsrlq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 71 d4 07       	vpsrlw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 78 56 34 12 	vldmxcsr 0x12345678
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 78 56 34 12 	vmovdqa 0x12345678,%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 78 56 34 12 	vmovdqa %xmm8,0x12345678
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 78 56 34 12 	vmovd  %xmm8,0x12345678
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 78 56 34 12 	vcvtsd2si 0x12345678,%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 78 56 34 12 	vcvtdq2pd 0x12345678,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 78 56 34 12 	vcvtpd2psy 0x12345678,%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 78 56 34 12 	vpavgb 0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 78 56 34 12 07 	vaeskeygenassist \$0x7,0x12345678,%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 78 56 34 12 07 	vpextrb \$0x7,%xmm8,0x12345678
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 78 56 34 12 	vcvtsi2sdl 0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 78 56 34 12 07 	vpclmulqdq \$0x7,0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 78 56 34 12 80 	vblendvps %xmm8,0x12345678,%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 78 56 34 12 07 	vpinsrb \$0x7,0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 78 56 34 12 	vmovdqa 0x12345678,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 78 56 34 12 	vmovdqa %ymm8,0x12345678
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 78 56 34 12 	vpermilpd 0x12345678,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 78 56 34 12 07 	vroundpd \$0x7,0x12345678,%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 78 56 34 12 07 	vextractf128 \$0x7,%ymm8,0x12345678
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 78 56 34 12 07 	vperm2f128 \$0x7,0x12345678,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 78 56 34 12 80 	vblendvpd %ymm8,0x12345678,%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr 0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 6f 45 00       	vmovdqa 0x0\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 45 00       	vmovdqa %xmm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 7e 45 00       	vmovd  %xmm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 45 00       	vcvtsd2si 0x0\(%rbp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 45 00       	vcvtdq2pd 0x0\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 45 00       	vcvtpd2psy 0x0\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 7d 00       	vpavgb 0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 45 00 07 	vaeskeygenassist \$0x7,0x0\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 45 00 07 	vpextrb \$0x7,%xmm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 3b 2a 7d 00       	vcvtsi2sdl 0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 7d 00 07 	vpclmulqdq \$0x7,0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 75 00 80 	vblendvps %xmm8,0x0\(%rbp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 7d 00 07 	vpinsrb \$0x7,0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 45 00       	vmovdqa 0x0\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 45 00       	vmovdqa %ymm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d 7d 00    	vpermilpd 0x0\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 45 00 07 	vroundpd \$0x7,0x0\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 45 00 07 	vextractf128 \$0x7,%ymm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 7d 00 07 	vperm2f128 \$0x7,0x0\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 75 00 80 	vblendvpd %ymm8,0x0\(%rbp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 14 24       	vldmxcsr \(%rsp\)
[ 	]*[a-f0-9]+:	c5 79 6f 04 24       	vmovdqa \(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 24       	vmovdqa %xmm8,\(%rsp\)
[ 	]*[a-f0-9]+:	c5 79 7e 04 24       	vmovd  %xmm8,\(%rsp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 04 24       	vcvtsd2si \(%rsp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 24       	vcvtdq2pd \(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 24       	vcvtpd2psy \(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 24       	vpavgb \(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 24 07 	vaeskeygenassist \$0x7,\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 24 07 	vpextrb \$0x7,%xmm8,\(%rsp\)
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 24       	vcvtsi2sdl \(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 24 07 	vpclmulqdq \$0x7,\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 24 80 	vblendvps %xmm8,\(%rsp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 24 07 	vpinsrb \$0x7,\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 24       	vmovdqa \(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 24       	vmovdqa %ymm8,\(%rsp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 24    	vpermilpd \(%rsp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 24 07 	vroundpd \$0x7,\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 24 07 	vextractf128 \$0x7,%ymm8,\(%rsp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 24 07 	vperm2f128 \$0x7,\(%rsp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 24 80 	vblendvpd %ymm8,\(%rsp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 95 99 00 00 00 	vldmxcsr 0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 6f 85 99 00 00 00 	vmovdqa 0x99\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 85 99 00 00 00 	vmovdqa %xmm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 7e 85 99 00 00 00 	vmovd  %xmm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 85 99 00 00 00 	vcvtsd2si 0x99\(%rbp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 85 99 00 00 00 	vcvtdq2pd 0x99\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 85 99 00 00 00 	vcvtpd2psy 0x99\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bd 99 00 00 00 	vpavgb 0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 85 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 85 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 3b 2a bd 99 00 00 00 	vcvtsi2sdl 0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bd 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b5 99 00 00 00 80 	vblendvps %xmm8,0x99\(%rbp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bd 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 85 99 00 00 00 	vmovdqa 0x99\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 85 99 00 00 00 	vmovdqa %ymm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bd 99 00 00 00 	vpermilpd 0x99\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 85 99 00 00 00 07 	vroundpd \$0x7,0x99\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 85 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bd 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b5 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%rbp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 c1 78 ae 97 99 00 00 00 	vldmxcsr 0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 79 6f 87 99 00 00 00 	vmovdqa 0x99\(%r15\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7f 87 99 00 00 00 	vmovdqa %xmm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 79 7e 87 99 00 00 00 	vmovd  %xmm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 7b 2d 87 99 00 00 00 	vcvtsd2si 0x99\(%r15\),%r8d
[ 	]*[a-f0-9]+:	c4 41 7e e6 87 99 00 00 00 	vcvtdq2pd 0x99\(%r15\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a 87 99 00 00 00 	vcvtpd2psy 0x99\(%r15\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 39 e0 bf 99 00 00 00 	vpavgb 0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 79 df 87 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%r15\),%xmm8
[ 	]*[a-f0-9]+:	c4 43 79 14 87 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 3b 2a bf 99 00 00 00 	vcvtsi2sdl 0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 39 44 bf 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 19 4a b7 99 00 00 00 80 	vblendvps %xmm8,0x99\(%r15\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 43 39 20 bf 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7d 6f 87 99 00 00 00 	vmovdqa 0x99\(%r15\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 7f 87 99 00 00 00 	vmovdqa %ymm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 42 3d 0d bf 99 00 00 00 	vpermilpd 0x99\(%r15\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 7d 09 87 99 00 00 00 07 	vroundpd \$0x7,0x99\(%r15\),%ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 19 87 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 43 3d 06 bf 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%r15\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 1d 4b b7 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%r15\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 15 99 00 00 00 	vldmxcsr 0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 6f 05 99 00 00 00 	vmovdqa 0x99\(%rip\),%xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7f 05 99 00 00 00 	vmovdqa %xmm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7e 05 99 00 00 00 	vmovd  %xmm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7b 2d 05 99 00 00 00 	vcvtsd2si 0x99\(%rip\),%r8d        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7e e6 05 99 00 00 00 	vcvtdq2pd 0x99\(%rip\),%ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 5a 05 99 00 00 00 	vcvtpd2psy 0x99\(%rip\),%xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 39 e0 3d 99 00 00 00 	vpavgb 0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 df 05 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%rip\),%xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 14 05 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 3b 2a 3d 99 00 00 00 	vcvtsi2sdl 0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 44 3d 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 19 4a 35 99 00 00 00 80 	vblendvps %xmm8,0x99\(%rip\),%xmm12,%xmm14        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 20 3d 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 6f 05 99 00 00 00 	vmovdqa 0x99\(%rip\),%ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 7f 05 99 00 00 00 	vmovdqa %ymm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3d 99 00 00 00 	vpermilpd 0x99\(%rip\),%ymm8,%ymm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 09 05 99 00 00 00 07 	vroundpd \$0x7,0x99\(%rip\),%ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 19 05 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 3d 06 3d 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%rip\),%ymm8,%ymm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 1d 4b 35 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%rip\),%ymm12,%ymm14        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 f8 ae 94 24 99 00 00 00 	vldmxcsr 0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 79 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 84 24 99 00 00 00 	vmovdqa %xmm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 79 7e 84 24 99 00 00 00 	vmovd  %xmm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 84 24 99 00 00 00 	vcvtsd2si 0x99\(%rsp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 84 24 99 00 00 00 	vcvtdq2pd 0x99\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 84 24 99 00 00 00 	vcvtpd2psy 0x99\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bc 24 99 00 00 00 	vpavgb 0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 84 24 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 84 24 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 3b 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 24 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 24 99 00 00 00 80 	vblendvps %xmm8,0x99\(%rsp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 24 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 84 24 99 00 00 00 	vmovdqa %ymm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 24 99 00 00 00 	vpermilpd 0x99\(%rsp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 24 99 00 00 00 07 	vroundpd \$0x7,0x99\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 24 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 24 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%rsp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 24 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%rsp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 c1 78 ae 94 24 99 00 00 00 	vldmxcsr 0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 79 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%r12\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7f 84 24 99 00 00 00 	vmovdqa %xmm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 79 7e 84 24 99 00 00 00 	vmovd  %xmm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 7b 2d 84 24 99 00 00 00 	vcvtsd2si 0x99\(%r12\),%r8d
[ 	]*[a-f0-9]+:	c4 41 7e e6 84 24 99 00 00 00 	vcvtdq2pd 0x99\(%r12\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a 84 24 99 00 00 00 	vcvtpd2psy 0x99\(%r12\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 39 e0 bc 24 99 00 00 00 	vpavgb 0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 79 df 84 24 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%r12\),%xmm8
[ 	]*[a-f0-9]+:	c4 43 79 14 84 24 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 3b 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 39 44 bc 24 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 19 4a b4 24 99 00 00 00 80 	vblendvps %xmm8,0x99\(%r12\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 43 39 20 bc 24 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7d 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%r12\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 7f 84 24 99 00 00 00 	vmovdqa %ymm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 42 3d 0d bc 24 99 00 00 00 	vpermilpd 0x99\(%r12\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 7d 09 84 24 99 00 00 00 07 	vroundpd \$0x7,0x99\(%r12\),%ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 19 84 24 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 43 3d 06 bc 24 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%r12\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 1d 4b b4 24 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%r12\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 67 ff ff ff 	vldmxcsr 0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 67 ff ff ff 	vmovdqa 0xffffffffffffff67,%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 67 ff ff ff 	vmovdqa %xmm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 67 ff ff ff 	vmovd  %xmm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 67 ff ff ff 	vcvtsd2si 0xffffffffffffff67,%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 67 ff ff ff 	vcvtdq2pd 0xffffffffffffff67,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 67 ff ff ff 	vcvtpd2psy 0xffffffffffffff67,%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 67 ff ff ff 	vpavgb 0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 67 ff ff ff 07 	vaeskeygenassist \$0x7,0xffffffffffffff67,%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 67 ff ff ff 	vcvtsi2sdl 0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 67 ff ff ff 07 	vpclmulqdq \$0x7,0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 67 ff ff ff 80 	vblendvps %xmm8,0xffffffffffffff67,%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 67 ff ff ff 07 	vpinsrb \$0x7,0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 67 ff ff ff 	vmovdqa 0xffffffffffffff67,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 67 ff ff ff 	vmovdqa %ymm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 67 ff ff ff 	vpermilpd 0xffffffffffffff67,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 67 ff ff ff 07 	vroundpd \$0x7,0xffffffffffffff67,%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 67 ff ff ff 07 	vperm2f128 \$0x7,0xffffffffffffff67,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 67 ff ff ff 80 	vblendvpd %ymm8,0xffffffffffffff67,%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 14 65 67 ff ff ff 	vldmxcsr -0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 6f 04 65 67 ff ff ff 	vmovdqa -0x99\(,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 65 67 ff ff ff 	vmovdqa %xmm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 7e 04 65 67 ff ff ff 	vmovd  %xmm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 7b 2d 04 65 67 ff ff ff 	vcvtsd2si -0x99\(,%riz,2\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 65 67 ff ff ff 	vcvtdq2pd -0x99\(,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 65 67 ff ff ff 	vcvtpd2psy -0x99\(,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 65 67 ff ff ff 	vpavgb -0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 65 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 65 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 65 67 ff ff ff 	vcvtsi2sdl -0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 65 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 65 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(,%riz,2\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 65 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 65 67 ff ff ff 	vmovdqa -0x99\(,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 65 67 ff ff ff 	vmovdqa %ymm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 65 67 ff ff ff 	vpermilpd -0x99\(,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 65 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 65 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 65 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 65 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(,%riz,2\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 94 23 67 ff ff ff 	vldmxcsr -0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 79 6f 84 23 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,1\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 84 23 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 79 7e 84 23 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 7b 2d 84 23 67 ff ff ff 	vcvtsd2si -0x99\(%rbx,%riz,1\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 84 23 67 ff ff ff 	vcvtdq2pd -0x99\(%rbx,%riz,1\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 84 23 67 ff ff ff 	vcvtpd2psy -0x99\(%rbx,%riz,1\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bc 23 67 ff ff ff 	vpavgb -0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 84 23 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rbx,%riz,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 84 23 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 3b 2a bc 23 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 23 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 23 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rbx,%riz,1\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 23 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 84 23 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,1\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 84 23 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 23 67 ff ff ff 	vpermilpd -0x99\(%rbx,%riz,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 23 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rbx,%riz,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 23 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 23 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rbx,%riz,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 23 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rbx,%riz,1\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 94 63 67 ff ff ff 	vldmxcsr -0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 6f 84 63 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 84 63 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 7e 84 63 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 7b 2d 84 63 67 ff ff ff 	vcvtsd2si -0x99\(%rbx,%riz,2\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 84 63 67 ff ff ff 	vcvtdq2pd -0x99\(%rbx,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 84 63 67 ff ff ff 	vcvtpd2psy -0x99\(%rbx,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bc 63 67 ff ff ff 	vpavgb -0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 84 63 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rbx,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 84 63 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 3b 2a bc 63 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 63 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 63 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rbx,%riz,2\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 63 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 84 63 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 84 63 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 63 67 ff ff ff 	vpermilpd -0x99\(%rbx,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 63 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rbx,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 63 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 63 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rbx,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 63 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rbx,%riz,2\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 bc 67 ff ff ff 	vldmxcsr -0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 bc 67 ff ff ff 	vmovdqa -0x99\(%r12,%r15,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 bc 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 bc 67 ff ff ff 	vmovd  %xmm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 bc 67 ff ff ff 	vcvtsd2si -0x99\(%r12,%r15,4\),%r8d
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 bc 67 ff ff ff 	vcvtdq2pd -0x99\(%r12,%r15,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 bc 67 ff ff ff 	vcvtpd2psy -0x99\(%r12,%r15,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc bc 67 ff ff ff 	vpavgb -0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 79 df 84 bc 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%r12,%r15,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 03 79 14 84 bc 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc bc 67 ff ff ff 	vcvtsi2sdl -0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 39 44 bc bc 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 bc 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%r12,%r15,4\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 03 39 20 bc bc 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 bc 67 ff ff ff 	vmovdqa -0x99\(%r12,%r15,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 bc 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc bc 67 ff ff ff 	vpermilpd -0x99\(%r12,%r15,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 bc 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%r12,%r15,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 bc 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc bc 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%r12,%r15,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 bc 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%r12,%r15,4\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 f8 67 ff ff ff 	vldmxcsr -0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 f8 67 ff ff ff 	vmovdqa -0x99\(%r8,%r15,8\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 f8 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 f8 67 ff ff ff 	vmovd  %xmm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 f8 67 ff ff ff 	vcvtsd2si -0x99\(%r8,%r15,8\),%r8d
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 f8 67 ff ff ff 	vcvtdq2pd -0x99\(%r8,%r15,8\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 f8 67 ff ff ff 	vcvtpd2psy -0x99\(%r8,%r15,8\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc f8 67 ff ff ff 	vpavgb -0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 79 df 84 f8 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%r8,%r15,8\),%xmm8
[ 	]*[a-f0-9]+:	c4 03 79 14 84 f8 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc f8 67 ff ff ff 	vcvtsi2sdl -0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 39 44 bc f8 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 f8 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%r8,%r15,8\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 03 39 20 bc f8 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 f8 67 ff ff ff 	vmovdqa -0x99\(%r8,%r15,8\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 f8 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc f8 67 ff ff ff 	vpermilpd -0x99\(%r8,%r15,8\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 f8 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%r8,%r15,8\),%ymm8
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 f8 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc f8 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%r8,%r15,8\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 f8 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%r8,%r15,8\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 ad 67 ff ff ff 	vldmxcsr -0x99\(%rbp,%r13,4\)
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 ad 67 ff ff ff 	vmovdqa -0x99\(%rbp,%r13,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 ad 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rbp,%r13,4\)
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 ad 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rbp,%r13,4\)
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 ad 67 ff ff ff 	vcvtsd2si -0x99\(%rbp,%r13,4\),%r8d
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 ad 67 ff ff ff 	vcvtdq2pd -0x99\(%rbp,%r13,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 ad 67 ff ff ff 	vcvtpd2psy -0x99\(%rbp,%r13,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc ad 67 ff ff ff 	vpavgb -0x99\(%rbp,%r13,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 79 df 84 ad 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rbp,%r13,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 23 79 14 84 ad 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rbp,%r13,4\)
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc ad 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbp,%r13,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 39 44 bc ad 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rbp,%r13,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 ad 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rbp,%r13,4\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 23 39 20 bc ad 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rbp,%r13,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 ad 67 ff ff ff 	vmovdqa -0x99\(%rbp,%r13,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 ad 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rbp,%r13,4\)
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc ad 67 ff ff ff 	vpermilpd -0x99\(%rbp,%r13,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 ad 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rbp,%r13,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 ad 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rbp,%r13,4\)
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc ad 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rbp,%r13,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 ad 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rbp,%r13,4\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 24 67 ff ff ff 	vldmxcsr -0x99\(%rsp,%r12,1\)
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 24 67 ff ff ff 	vmovdqa -0x99\(%rsp,%r12,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 24 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rsp,%r12,1\)
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 24 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rsp,%r12,1\)
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 24 67 ff ff ff 	vcvtsd2si -0x99\(%rsp,%r12,1\),%r8d
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 24 67 ff ff ff 	vcvtdq2pd -0x99\(%rsp,%r12,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 24 67 ff ff ff 	vcvtpd2psy -0x99\(%rsp,%r12,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc 24 67 ff ff ff 	vpavgb -0x99\(%rsp,%r12,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 79 df 84 24 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rsp,%r12,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 23 79 14 84 24 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rsp,%r12,1\)
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc 24 67 ff ff ff 	vcvtsi2sdl -0x99\(%rsp,%r12,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 39 44 bc 24 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rsp,%r12,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 24 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rsp,%r12,1\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 23 39 20 bc 24 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rsp,%r12,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 24 67 ff ff ff 	vmovdqa -0x99\(%rsp,%r12,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 24 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rsp,%r12,1\)
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc 24 67 ff ff ff 	vpermilpd -0x99\(%rsp,%r12,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 24 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rsp,%r12,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 24 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rsp,%r12,1\)
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc 24 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rsp,%r12,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 24 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rsp,%r12,1\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 41 79 50 c0       	vmovmskpd %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 c1 01 72 f0 07    	vpslld \$0x7,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7c 50 c0       	vmovmskps %ymm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 79 6f f8       	vmovdqa %xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 79 7e c0       	vmovd  %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 7b 2d c0       	vcvtsd2si %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 7e e6 c0       	vcvtdq2pd %xmm8,%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a c0       	vcvtpd2ps %ymm8,%xmm8
[ 	]*[a-f0-9]+:	c4 43 79 df f8 07    	vaeskeygenassist \$0x7,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 79 14 c0 07    	vpextrb \$0x7,%xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 3b 2a f8       	vcvtsi2sd %r8d,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 01 44 e0 07    	vpclmulqdq \$0x7,%xmm8,%xmm15,%xmm12
[ 	]*[a-f0-9]+:	c4 43 19 4a f0 80    	vblendvps %xmm8,%xmm8,%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 43 39 20 f8 07    	vpinsrb \$0x7,%r8d,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7d 6f f8       	vmovdqa %ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 42 05 0d e0       	vpermilpd %ymm8,%ymm15,%ymm12
[ 	]*[a-f0-9]+:	c4 43 7d 09 f8 07    	vroundpd \$0x7,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 7d 19 c0 07    	vextractf128 \$0x7,%ymm8,%xmm8
[ 	]*[a-f0-9]+:	c4 43 05 06 e0 07    	vperm2f128 \$0x7,%ymm8,%ymm15,%ymm12
[ 	]*[a-f0-9]+:	c4 43 1d 4b f7 80    	vblendvpd %ymm8,%ymm15,%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 43 3d 18 f8 07    	vinsertf128 \$0x7,%xmm8,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 61 fb 2d 01       	vcvtsd2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c4 43 79 17 c0 0a    	vextractps \$0xa,%xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 61 fa 2d 01       	vcvtss2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c4 41 01 c4 c0 07    	vpinsrw \$0x7,%r8d,%xmm15,%xmm8
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb \(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb \(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 a2 55 cf b4 f0 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb 0xfe0\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb 0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb -0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb -0x1020\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	c5 ff e6 21          	vcvtpd2dqy \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps %ymm4,%xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a 21          	vcvtpd2psy \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 21          	vcvttpd2dqy \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps %ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb \$0x7b,\(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb \$0x7b,\(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 a3 d5 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb \$0x7b,0xfe0\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb \$0x7b,0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1020\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb \$0x7b,\(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb \$0x7b,\(%rcx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 a3 d5 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb \$0x7b,0xfe0\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1000\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1020\(%rdx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd %ymm4,\(%rcx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd %ymm4,\(%rcx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps %ymm4,\(%rcx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps %ymm4,\(%rcx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 \$0x7,%xmm4,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 \$0x7,\(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 \$0x7,\(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 \$0x7,%ymm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 \$0x7,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 \$0x7,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dqx \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2psx \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dqx \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb \(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb \(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 a2 51 cf b4 f0 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb 0x7f0\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb 0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb -0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb -0x810\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps \$0x7,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb \$0x7b,\(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb \$0x7b,\(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 a3 d1 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb \$0x7b,0x7f0\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb \$0x7b,0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x810\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb \$0x7b,\(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb \$0x7b,\(%rcx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 a3 d1 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%rax,%r14,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x7f0\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x800\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x810\(%rdx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  %rcx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  %rcx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 fb 2d cc       	vcvtsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2d 09       	vcvtsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2d 09       	vcvtsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2c cc       	vcvttsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2c 09       	vcvttsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fb 2c 09       	vcvttsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 db 2a f1       	vcvtsi2sd %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 db 2a 31       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 db 2a 31       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 da 2a f1       	vcvtsi2ss %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 da 2a 31       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 da 2a 31       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d9 22 f1 07    	vpinsrq \$0x7,%rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d9 22 31 07    	vpinsrq \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d9 22 31 07    	vpinsrq \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 f9 16 e1 07    	vpextrq \$0x7,%xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 07    	vpextrq \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 07    	vpextrq \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 fa 2d cc       	vcvtss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2d 09       	vcvtss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2d 09       	vcvtss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2c cc       	vcvttss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2c 09       	vcvttss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fa 2c 09       	vcvttss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb %xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sdl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ssl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb \$0x7,\(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb \$0x7,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 f7 f4          	vmaskmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 c8 12 d4          	vmovhlps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 16 d4          	vmovlhps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 72 f4 07       	vpslld \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 fc 07       	vpslldq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 f4 07       	vpsllq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 71 f4 07       	vpsllw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 72 e4 07       	vpsrad \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 71 e4 07       	vpsraw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 72 d4 07       	vpsrld \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 dc 07       	vpsrldq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 73 d4 07       	vpsrlq \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 71 d4 07       	vpsrlw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 78 56 34 12 	vldmxcsr 0x12345678
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 78 56 34 12 	vmovdqa 0x12345678,%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 78 56 34 12 	vmovdqa %xmm8,0x12345678
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 78 56 34 12 	vmovd  %xmm8,0x12345678
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 78 56 34 12 	vcvtsd2si 0x12345678,%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 78 56 34 12 	vcvtdq2pd 0x12345678,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 78 56 34 12 	vcvtpd2psy 0x12345678,%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 78 56 34 12 	vpavgb 0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 78 56 34 12 07 	vaeskeygenassist \$0x7,0x12345678,%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 78 56 34 12 07 	vpextrb \$0x7,%xmm8,0x12345678
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 78 56 34 12 	vcvtsi2sdl 0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 78 56 34 12 07 	vpclmulqdq \$0x7,0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 78 56 34 12 80 	vblendvps %xmm8,0x12345678,%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 78 56 34 12 07 	vpinsrb \$0x7,0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 78 56 34 12 	vmovdqa 0x12345678,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 78 56 34 12 	vmovdqa %ymm8,0x12345678
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 78 56 34 12 	vpermilpd 0x12345678,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 78 56 34 12 07 	vroundpd \$0x7,0x12345678,%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 78 56 34 12 07 	vextractf128 \$0x7,%ymm8,0x12345678
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 78 56 34 12 07 	vperm2f128 \$0x7,0x12345678,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 78 56 34 12 80 	vblendvpd %ymm8,0x12345678,%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr 0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 6f 45 00       	vmovdqa 0x0\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 45 00       	vmovdqa %xmm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 7e 45 00       	vmovd  %xmm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 45 00       	vcvtsd2si 0x0\(%rbp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 45 00       	vcvtdq2pd 0x0\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 45 00       	vcvtpd2psy 0x0\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 7d 00       	vpavgb 0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 45 00 07 	vaeskeygenassist \$0x7,0x0\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 45 00 07 	vpextrb \$0x7,%xmm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c5 3b 2a 7d 00       	vcvtsi2sdl 0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 7d 00 07 	vpclmulqdq \$0x7,0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 75 00 80 	vblendvps %xmm8,0x0\(%rbp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 7d 00 07 	vpinsrb \$0x7,0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 45 00       	vmovdqa 0x0\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 45 00       	vmovdqa %ymm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d 7d 00    	vpermilpd 0x0\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 45 00 07 	vroundpd \$0x7,0x0\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 45 00 07 	vextractf128 \$0x7,%ymm8,0x0\(%rbp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 7d 00 07 	vperm2f128 \$0x7,0x0\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 75 00 80 	vblendvpd %ymm8,0x0\(%rbp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 95 99 00 00 00 	vldmxcsr 0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 6f 85 99 00 00 00 	vmovdqa 0x99\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 85 99 00 00 00 	vmovdqa %xmm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 79 7e 85 99 00 00 00 	vmovd  %xmm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 85 99 00 00 00 	vcvtsd2si 0x99\(%rbp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 85 99 00 00 00 	vcvtdq2pd 0x99\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 85 99 00 00 00 	vcvtpd2psy 0x99\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bd 99 00 00 00 	vpavgb 0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 85 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%rbp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 85 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c5 3b 2a bd 99 00 00 00 	vcvtsi2sdl 0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bd 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b5 99 00 00 00 80 	vblendvps %xmm8,0x99\(%rbp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bd 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 85 99 00 00 00 	vmovdqa 0x99\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 85 99 00 00 00 	vmovdqa %ymm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bd 99 00 00 00 	vpermilpd 0x99\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 85 99 00 00 00 07 	vroundpd \$0x7,0x99\(%rbp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 85 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%rbp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bd 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%rbp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b5 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%rbp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 c1 78 ae 97 99 00 00 00 	vldmxcsr 0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 79 6f 87 99 00 00 00 	vmovdqa 0x99\(%r15\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7f 87 99 00 00 00 	vmovdqa %xmm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 79 7e 87 99 00 00 00 	vmovd  %xmm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 7b 2d 87 99 00 00 00 	vcvtsd2si 0x99\(%r15\),%r8d
[ 	]*[a-f0-9]+:	c4 41 7e e6 87 99 00 00 00 	vcvtdq2pd 0x99\(%r15\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a 87 99 00 00 00 	vcvtpd2psy 0x99\(%r15\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 39 e0 bf 99 00 00 00 	vpavgb 0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 79 df 87 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%r15\),%xmm8
[ 	]*[a-f0-9]+:	c4 43 79 14 87 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 41 3b 2a bf 99 00 00 00 	vcvtsi2sdl 0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 39 44 bf 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 19 4a b7 99 00 00 00 80 	vblendvps %xmm8,0x99\(%r15\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 43 39 20 bf 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7d 6f 87 99 00 00 00 	vmovdqa 0x99\(%r15\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 7f 87 99 00 00 00 	vmovdqa %ymm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 42 3d 0d bf 99 00 00 00 	vpermilpd 0x99\(%r15\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 7d 09 87 99 00 00 00 07 	vroundpd \$0x7,0x99\(%r15\),%ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 19 87 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%r15\)
[ 	]*[a-f0-9]+:	c4 43 3d 06 bf 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%r15\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 1d 4b b7 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%r15\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 15 99 00 00 00 	vldmxcsr 0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 6f 05 99 00 00 00 	vmovdqa 0x99\(%rip\),%xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7f 05 99 00 00 00 	vmovdqa %xmm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7e 05 99 00 00 00 	vmovd  %xmm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7b 2d 05 99 00 00 00 	vcvtsd2si 0x99\(%rip\),%r8d        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7e e6 05 99 00 00 00 	vcvtdq2pd 0x99\(%rip\),%ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 5a 05 99 00 00 00 	vcvtpd2psy 0x99\(%rip\),%xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 39 e0 3d 99 00 00 00 	vpavgb 0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 df 05 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%rip\),%xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 14 05 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 3b 2a 3d 99 00 00 00 	vcvtsi2sdl 0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 44 3d 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 19 4a 35 99 00 00 00 80 	vblendvps %xmm8,0x99\(%rip\),%xmm12,%xmm14        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 20 3d 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%rip\),%xmm8,%xmm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 6f 05 99 00 00 00 	vmovdqa 0x99\(%rip\),%ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 7f 05 99 00 00 00 	vmovdqa %ymm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3d 99 00 00 00 	vpermilpd 0x99\(%rip\),%ymm8,%ymm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 09 05 99 00 00 00 07 	vroundpd \$0x7,0x99\(%rip\),%ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 19 05 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 3d 06 3d 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%rip\),%ymm8,%ymm15        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 1d 4b 35 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%rip\),%ymm12,%ymm14        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 f8 ae 94 24 99 00 00 00 	vldmxcsr 0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 79 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 84 24 99 00 00 00 	vmovdqa %xmm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 79 7e 84 24 99 00 00 00 	vmovd  %xmm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 7b 2d 84 24 99 00 00 00 	vcvtsd2si 0x99\(%rsp\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 84 24 99 00 00 00 	vcvtdq2pd 0x99\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 84 24 99 00 00 00 	vcvtpd2psy 0x99\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bc 24 99 00 00 00 	vpavgb 0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 84 24 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%rsp\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 84 24 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c5 3b 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 24 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 24 99 00 00 00 80 	vblendvps %xmm8,0x99\(%rsp\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 24 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 84 24 99 00 00 00 	vmovdqa %ymm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 24 99 00 00 00 	vpermilpd 0x99\(%rsp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 24 99 00 00 00 07 	vroundpd \$0x7,0x99\(%rsp\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 24 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%rsp\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 24 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%rsp\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 24 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%rsp\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 c1 78 ae 94 24 99 00 00 00 	vldmxcsr 0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 79 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%r12\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7f 84 24 99 00 00 00 	vmovdqa %xmm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 79 7e 84 24 99 00 00 00 	vmovd  %xmm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 7b 2d 84 24 99 00 00 00 	vcvtsd2si 0x99\(%r12\),%r8d
[ 	]*[a-f0-9]+:	c4 41 7e e6 84 24 99 00 00 00 	vcvtdq2pd 0x99\(%r12\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a 84 24 99 00 00 00 	vcvtpd2psy 0x99\(%r12\),%xmm8
[ 	]*[a-f0-9]+:	c4 41 39 e0 bc 24 99 00 00 00 	vpavgb 0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 79 df 84 24 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%r12\),%xmm8
[ 	]*[a-f0-9]+:	c4 43 79 14 84 24 99 00 00 00 07 	vpextrb \$0x7,%xmm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 41 3b 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 39 44 bc 24 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 19 4a b4 24 99 00 00 00 80 	vblendvps %xmm8,0x99\(%r12\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 43 39 20 bc 24 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7d 6f 84 24 99 00 00 00 	vmovdqa 0x99\(%r12\),%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 7f 84 24 99 00 00 00 	vmovdqa %ymm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 42 3d 0d bc 24 99 00 00 00 	vpermilpd 0x99\(%r12\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 7d 09 84 24 99 00 00 00 07 	vroundpd \$0x7,0x99\(%r12\),%ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 19 84 24 99 00 00 00 07 	vextractf128 \$0x7,%ymm8,0x99\(%r12\)
[ 	]*[a-f0-9]+:	c4 43 3d 06 bc 24 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%r12\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 1d 4b b4 24 99 00 00 00 80 	vblendvpd %ymm8,0x99\(%r12\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 67 ff ff ff 	vldmxcsr 0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 67 ff ff ff 	vmovdqa 0xffffffffffffff67,%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 67 ff ff ff 	vmovdqa %xmm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 67 ff ff ff 	vmovd  %xmm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 67 ff ff ff 	vcvtsd2si 0xffffffffffffff67,%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 67 ff ff ff 	vcvtdq2pd 0xffffffffffffff67,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 67 ff ff ff 	vcvtpd2psy 0xffffffffffffff67,%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 67 ff ff ff 	vpavgb 0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 67 ff ff ff 07 	vaeskeygenassist \$0x7,0xffffffffffffff67,%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 67 ff ff ff 	vcvtsi2sdl 0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 67 ff ff ff 07 	vpclmulqdq \$0x7,0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 67 ff ff ff 80 	vblendvps %xmm8,0xffffffffffffff67,%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 67 ff ff ff 07 	vpinsrb \$0x7,0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 67 ff ff ff 	vmovdqa 0xffffffffffffff67,%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 67 ff ff ff 	vmovdqa %ymm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 67 ff ff ff 	vpermilpd 0xffffffffffffff67,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 67 ff ff ff 07 	vroundpd \$0x7,0xffffffffffffff67,%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 67 ff ff ff 07 	vperm2f128 \$0x7,0xffffffffffffff67,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 67 ff ff ff 80 	vblendvpd %ymm8,0xffffffffffffff67,%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 14 65 67 ff ff ff 	vldmxcsr -0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 6f 04 65 67 ff ff ff 	vmovdqa -0x99\(,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 04 65 67 ff ff ff 	vmovdqa %xmm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 7e 04 65 67 ff ff ff 	vmovd  %xmm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 7b 2d 04 65 67 ff ff ff 	vcvtsd2si -0x99\(,%riz,2\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 04 65 67 ff ff ff 	vcvtdq2pd -0x99\(,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 04 65 67 ff ff ff 	vcvtpd2psy -0x99\(,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 3c 65 67 ff ff ff 	vpavgb -0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 04 65 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 04 65 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 65 67 ff ff ff 	vcvtsi2sdl -0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 65 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 65 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(,%riz,2\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 65 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 04 65 67 ff ff ff 	vmovdqa -0x99\(,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 04 65 67 ff ff ff 	vmovdqa %ymm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 65 67 ff ff ff 	vpermilpd -0x99\(,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 65 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 65 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(,%riz,2\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 65 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 65 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(,%riz,2\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 94 23 67 ff ff ff 	vldmxcsr -0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 79 6f 84 23 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,1\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 84 23 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 79 7e 84 23 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 7b 2d 84 23 67 ff ff ff 	vcvtsd2si -0x99\(%rbx,%riz,1\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 84 23 67 ff ff ff 	vcvtdq2pd -0x99\(%rbx,%riz,1\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 84 23 67 ff ff ff 	vcvtpd2psy -0x99\(%rbx,%riz,1\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bc 23 67 ff ff ff 	vpavgb -0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 84 23 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rbx,%riz,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 84 23 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c5 3b 2a bc 23 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 23 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 23 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rbx,%riz,1\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 23 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 84 23 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,1\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 84 23 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 23 67 ff ff ff 	vpermilpd -0x99\(%rbx,%riz,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 23 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rbx,%riz,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 23 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rbx,%riz,1\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 23 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rbx,%riz,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 23 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rbx,%riz,1\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c5 f8 ae 94 63 67 ff ff ff 	vldmxcsr -0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 6f 84 63 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 79 7f 84 63 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 79 7e 84 63 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 7b 2d 84 63 67 ff ff ff 	vcvtsd2si -0x99\(%rbx,%riz,2\),%r8d
[ 	]*[a-f0-9]+:	c5 7e e6 84 63 67 ff ff ff 	vcvtdq2pd -0x99\(%rbx,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 5a 84 63 67 ff ff ff 	vcvtpd2psy -0x99\(%rbx,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c5 39 e0 bc 63 67 ff ff ff 	vpavgb -0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 79 df 84 63 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rbx,%riz,2\),%xmm8
[ 	]*[a-f0-9]+:	c4 63 79 14 84 63 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c5 3b 2a bc 63 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 63 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 63 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rbx,%riz,2\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 63 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 7d 6f 84 63 67 ff ff ff 	vmovdqa -0x99\(%rbx,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f 84 63 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 63 67 ff ff ff 	vpermilpd -0x99\(%rbx,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 63 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rbx,%riz,2\),%ymm8
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 63 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rbx,%riz,2\)
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 63 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rbx,%riz,2\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 63 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rbx,%riz,2\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 bc 67 ff ff ff 	vldmxcsr -0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 bc 67 ff ff ff 	vmovdqa -0x99\(%r12,%r15,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 bc 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 bc 67 ff ff ff 	vmovd  %xmm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 bc 67 ff ff ff 	vcvtsd2si -0x99\(%r12,%r15,4\),%r8d
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 bc 67 ff ff ff 	vcvtdq2pd -0x99\(%r12,%r15,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 bc 67 ff ff ff 	vcvtpd2psy -0x99\(%r12,%r15,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc bc 67 ff ff ff 	vpavgb -0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 79 df 84 bc 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%r12,%r15,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 03 79 14 84 bc 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc bc 67 ff ff ff 	vcvtsi2sdl -0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 39 44 bc bc 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 bc 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%r12,%r15,4\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 03 39 20 bc bc 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 bc 67 ff ff ff 	vmovdqa -0x99\(%r12,%r15,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 bc 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc bc 67 ff ff ff 	vpermilpd -0x99\(%r12,%r15,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 bc 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%r12,%r15,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 bc 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%r12,%r15,4\)
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc bc 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%r12,%r15,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 bc 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%r12,%r15,4\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 f8 67 ff ff ff 	vldmxcsr -0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 f8 67 ff ff ff 	vmovdqa -0x99\(%r8,%r15,8\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 f8 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 f8 67 ff ff ff 	vmovd  %xmm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 f8 67 ff ff ff 	vcvtsd2si -0x99\(%r8,%r15,8\),%r8d
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 f8 67 ff ff ff 	vcvtdq2pd -0x99\(%r8,%r15,8\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 f8 67 ff ff ff 	vcvtpd2psy -0x99\(%r8,%r15,8\),%xmm8
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc f8 67 ff ff ff 	vpavgb -0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 79 df 84 f8 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%r8,%r15,8\),%xmm8
[ 	]*[a-f0-9]+:	c4 03 79 14 84 f8 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc f8 67 ff ff ff 	vcvtsi2sdl -0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 39 44 bc f8 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 f8 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%r8,%r15,8\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 03 39 20 bc f8 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 f8 67 ff ff ff 	vmovdqa -0x99\(%r8,%r15,8\),%ymm8
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 f8 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc f8 67 ff ff ff 	vpermilpd -0x99\(%r8,%r15,8\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 f8 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%r8,%r15,8\),%ymm8
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 f8 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%r8,%r15,8\)
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc f8 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%r8,%r15,8\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 f8 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%r8,%r15,8\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 a5 67 ff ff ff 	vldmxcsr -0x99\(%rbp,%r12,4\)
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 a5 67 ff ff ff 	vmovdqa -0x99\(%rbp,%r12,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 a5 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rbp,%r12,4\)
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 a5 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rbp,%r12,4\)
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 a5 67 ff ff ff 	vcvtsd2si -0x99\(%rbp,%r12,4\),%r8d
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 a5 67 ff ff ff 	vcvtdq2pd -0x99\(%rbp,%r12,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 a5 67 ff ff ff 	vcvtpd2psy -0x99\(%rbp,%r12,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc a5 67 ff ff ff 	vpavgb -0x99\(%rbp,%r12,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 79 df 84 a5 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rbp,%r12,4\),%xmm8
[ 	]*[a-f0-9]+:	c4 23 79 14 84 a5 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rbp,%r12,4\)
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc a5 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbp,%r12,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 39 44 bc a5 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rbp,%r12,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 a5 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rbp,%r12,4\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 23 39 20 bc a5 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rbp,%r12,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 a5 67 ff ff ff 	vmovdqa -0x99\(%rbp,%r12,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 a5 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rbp,%r12,4\)
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc a5 67 ff ff ff 	vpermilpd -0x99\(%rbp,%r12,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 a5 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rbp,%r12,4\),%ymm8
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 a5 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rbp,%r12,4\)
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc a5 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rbp,%r12,4\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 a5 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rbp,%r12,4\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 2c 67 ff ff ff 	vldmxcsr -0x99\(%rsp,%r13,1\)
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 2c 67 ff ff ff 	vmovdqa -0x99\(%rsp,%r13,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 2c 67 ff ff ff 	vmovdqa %xmm8,-0x99\(%rsp,%r13,1\)
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 2c 67 ff ff ff 	vmovd  %xmm8,-0x99\(%rsp,%r13,1\)
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 2c 67 ff ff ff 	vcvtsd2si -0x99\(%rsp,%r13,1\),%r8d
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 2c 67 ff ff ff 	vcvtdq2pd -0x99\(%rsp,%r13,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 2c 67 ff ff ff 	vcvtpd2psy -0x99\(%rsp,%r13,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc 2c 67 ff ff ff 	vpavgb -0x99\(%rsp,%r13,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 79 df 84 2c 67 ff ff ff 07 	vaeskeygenassist \$0x7,-0x99\(%rsp,%r13,1\),%xmm8
[ 	]*[a-f0-9]+:	c4 23 79 14 84 2c 67 ff ff ff 07 	vpextrb \$0x7,%xmm8,-0x99\(%rsp,%r13,1\)
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc 2c 67 ff ff ff 	vcvtsi2sdl -0x99\(%rsp,%r13,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 39 44 bc 2c 67 ff ff ff 07 	vpclmulqdq \$0x7,-0x99\(%rsp,%r13,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 2c 67 ff ff ff 80 	vblendvps %xmm8,-0x99\(%rsp,%r13,1\),%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 23 39 20 bc 2c 67 ff ff ff 07 	vpinsrb \$0x7,-0x99\(%rsp,%r13,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 2c 67 ff ff ff 	vmovdqa -0x99\(%rsp,%r13,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 2c 67 ff ff ff 	vmovdqa %ymm8,-0x99\(%rsp,%r13,1\)
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc 2c 67 ff ff ff 	vpermilpd -0x99\(%rsp,%r13,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 2c 67 ff ff ff 07 	vroundpd \$0x7,-0x99\(%rsp,%r13,1\),%ymm8
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 2c 67 ff ff ff 07 	vextractf128 \$0x7,%ymm8,-0x99\(%rsp,%r13,1\)
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc 2c 67 ff ff ff 07 	vperm2f128 \$0x7,-0x99\(%rsp,%r13,1\),%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 2c 67 ff ff ff 80 	vblendvpd %ymm8,-0x99\(%rsp,%r13,1\),%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 41 79 50 c0       	vmovmskpd %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 c1 01 72 f0 07    	vpslld \$0x7,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7c 50 c0       	vmovmskps %ymm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 79 6f f8       	vmovdqa %xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 79 7e c0       	vmovd  %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 7b 2d c0       	vcvtsd2si %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 7e e6 c0       	vcvtdq2pd %xmm8,%ymm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a c0       	vcvtpd2ps %ymm8,%xmm8
[ 	]*[a-f0-9]+:	c4 43 79 df f8 07    	vaeskeygenassist \$0x7,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 79 14 c0 07    	vpextrb \$0x7,%xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 3b 2a f8       	vcvtsi2sd %r8d,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 43 01 44 e0 07    	vpclmulqdq \$0x7,%xmm8,%xmm15,%xmm12
[ 	]*[a-f0-9]+:	c4 43 19 4a f0 80    	vblendvps %xmm8,%xmm8,%xmm12,%xmm14
[ 	]*[a-f0-9]+:	c4 43 39 20 f8 07    	vpinsrb \$0x7,%r8d,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7d 6f f8       	vmovdqa %ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 42 05 0d e0       	vpermilpd %ymm8,%ymm15,%ymm12
[ 	]*[a-f0-9]+:	c4 43 7d 09 f8 07    	vroundpd \$0x7,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 43 7d 19 c0 07    	vextractf128 \$0x7,%ymm8,%xmm8
[ 	]*[a-f0-9]+:	c4 43 05 06 e0 07    	vperm2f128 \$0x7,%ymm8,%ymm15,%ymm12
[ 	]*[a-f0-9]+:	c4 43 1d 4b f7 80    	vblendvpd %ymm8,%ymm15,%ymm12,%ymm14
[ 	]*[a-f0-9]+:	c4 43 3d 18 f8 07    	vinsertf128 \$0x7,%xmm8,%ymm8,%ymm15
[ 	]*[a-f0-9]+:	c4 61 fb 2d 01       	vcvtsd2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c4 43 79 17 c0 0a    	vextractps \$0xa,%xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 61 fa 2d 01       	vcvtss2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c4 41 01 c4 c0 07    	vpinsrw \$0x7,%r8d,%xmm15,%xmm8
#pass
