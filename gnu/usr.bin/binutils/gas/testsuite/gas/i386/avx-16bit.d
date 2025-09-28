#as: -I${srcdir}/$subdir
#objdump: -dwMaddr16 -Mdata16
#name: i386 16-bit AVX

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fc 77             	vzeroall
[ 	]*[a-f0-9]+:	c5 f8 77             	vzeroupper
[ 	]*[a-f0-9]+:	67 c5 f8 ae 11       	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 19       	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 5d 2d 31    	vmaskmovpd \(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 4d 2f 21    	vmaskmovpd %ymm4,%ymm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 5d 2c 31    	vmaskmovps \(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 4d 2e 21    	vmaskmovps %ymm4,%ymm6,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 05 31 07 	vpermilpd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 04 31 07 	vpermilps \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 31 07 	vroundpd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 08 31 07 	vroundps \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 58 11       	vaddpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 58 11       	vaddps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd d0 11       	vaddsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf d0 11       	vaddsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 55 11       	vandnpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 55 11       	vandnps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 54 11       	vandpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 54 11       	vandps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5e 11       	vdivpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5e 11       	vdivps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 7c 11       	vhaddpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf 7c 11       	vhaddps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 7d 11       	vhsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf 7d 11       	vhsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5f 11       	vmaxpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5f 11       	vmaxps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5d 11       	vminpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5d 11       	vminps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 59 11       	vmulpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 59 11       	vmulps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 56 11       	vorpd  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 56 11       	vorps  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e2 4d 0d 11    	vpermilpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e2 4d 0c 11    	vpermilps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5c 11       	vsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5c 11       	vsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 15 11       	vunpckhpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 15 11       	vunpckhps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 14 11       	vunpcklpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 14 11       	vunpcklps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 57 11       	vxorpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 57 11       	vxorps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 00    	vcmpeqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 00    	vcmpeqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 01    	vcmpltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 01    	vcmpltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 02    	vcmplepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 02    	vcmplepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 03    	vcmpunordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 03    	vcmpunordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 04    	vcmpneqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 04    	vcmpneqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 05    	vcmpnltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 05    	vcmpnltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 06    	vcmpnlepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 06    	vcmpnlepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 08    	vcmpeq_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 09    	vcmpngepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 09    	vcmpngepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0a    	vcmpngtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0a    	vcmpngtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0b    	vcmpfalsepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0b    	vcmpfalsepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0c    	vcmpneq_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0d    	vcmpgepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0d    	vcmpgepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0e    	vcmpgtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0e    	vcmpgtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0f    	vcmptruepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0f    	vcmptruepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 10    	vcmpeq_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 11    	vcmplt_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 12    	vcmple_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 13    	vcmpunord_spd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 14    	vcmpneq_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 15    	vcmpnlt_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 16    	vcmpnle_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 17    	vcmpord_spd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 18    	vcmpeq_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 19    	vcmpnge_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1a    	vcmpngt_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1b    	vcmpfalse_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1c    	vcmpneq_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1d    	vcmpge_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1e    	vcmpgt_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1f    	vcmptrue_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 00    	vcmpeqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 00    	vcmpeqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 01    	vcmpltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 01    	vcmpltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 02    	vcmpleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 02    	vcmpleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 03    	vcmpunordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 03    	vcmpunordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 04    	vcmpneqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 04    	vcmpneqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 05    	vcmpnltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 05    	vcmpnltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 06    	vcmpnleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 06    	vcmpnleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 08    	vcmpeq_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 09    	vcmpngeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 09    	vcmpngeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0a    	vcmpngtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0a    	vcmpngtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0b    	vcmpfalseps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0b    	vcmpfalseps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0c    	vcmpneq_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0d    	vcmpgeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0d    	vcmpgeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0e    	vcmpgtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0e    	vcmpgtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0f    	vcmptrueps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0f    	vcmptrueps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 10    	vcmpeq_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 11    	vcmplt_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 12    	vcmple_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 13    	vcmpunord_sps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 14    	vcmpneq_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 15    	vcmpnlt_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 16    	vcmpnle_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 17    	vcmpord_sps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 18    	vcmpeq_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 19    	vcmpnge_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1a    	vcmpngt_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1b    	vcmpfalse_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1c    	vcmpneq_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1d    	vcmpge_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1e    	vcmpgt_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1f    	vcmptrue_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf 31    	vgf2p8mulb \(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b4 f4 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb 0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb -0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb -0x1020\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c5 ff e6 21       	vcvtpd2dqy \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps %ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c5 fd 5a 21       	vcvtpd2psy \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c5 fd e6 21       	vcvttpd2dqy \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 5b 21       	vcvtdq2ps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 5b 21       	vcvtps2dq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 5b 21       	vcvttps2dq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 28 21       	vmovapd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 28 21       	vmovaps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 6f 21       	vmovdqa \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 6f 21       	vmovdqu \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 ff 12 21       	vmovddup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 16 21       	vmovshdup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 12 21       	vmovsldup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 10 21       	vmovupd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 10 21       	vmovups \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 7d 17 21    	vptest \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 53 21       	vrcpps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 52 21       	vrsqrtps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 51 21       	vsqrtpd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 51 21       	vsqrtps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0f 21    	vtestpd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0e 21    	vtestps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 29 21       	vmovapd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 29 21       	vmovaps %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 7f 21       	vmovdqa %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 7f 21       	vmovdqu %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 11 21       	vmovupd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 11 21       	vmovups %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 ff f0 21       	vlddqu \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd e7 21       	vmovntdq %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd 2b 21       	vmovntpd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fc 2b 21       	vmovntps %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 0d 11 07 	vblendpd \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 0c 11 07 	vblendps \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 40 11 07 	vdpps  \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 06 11 07 	vperm2f128 \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c6 11 07    	vshufpd \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c6 11 07    	vshufps \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce 31 7b 	vgf2p8affineqb \$0x7b,\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb \$0x7b,0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf 31 7b 	vgf2p8affineinvqb \$0x7b,\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 6d 4b 39 40 	vblendvpd %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 6d 4a 39 40 	vblendvps %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 \$0x7,%xmm4,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 5d 18 31 07 	vinsertf128 \$0x7,\(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 \$0x7,%ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 21 07 	vextractf128 \$0x7,%ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 1a 21    	vbroadcastf128 \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 5b 21       	vcvtdq2ps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb e6 21       	vcvtpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 5a 21       	vcvtpd2psx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 5b 21       	vcvtps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 e6 21       	vcvttpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 5b 21       	vcvttps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 28 21       	vmovapd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 28 21       	vmovaps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 6f 21       	vmovdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 6f 21       	vmovdqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 16 21       	vmovshdup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 12 21       	vmovsldup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 10 21       	vmovupd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 10 21       	vmovups \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 1c 21    	vpabsb \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 1d 21    	vpabsw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 1e 21    	vpabsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 41 21    	vphminposuw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 17 21    	vptest \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 0e 21    	vtestps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 0f 21    	vtestpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 53 21       	vrcpps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 52 21       	vrsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 51 21       	vsqrtpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 51 21       	vsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 db 21    	vaesimc \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 29 21       	vmovapd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 29 21       	vmovaps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 7f 21       	vmovdqa %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 7f 21       	vmovdqu %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 11 21       	vmovupd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 11 21       	vmovups %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fb f0 21       	vlddqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 2a 21    	vmovntdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 e7 21       	vmovntdq %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 2b 21       	vmovntpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 2b 21       	vmovntps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe e6 21       	vcvtdq2pd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 5a 21       	vcvtps2pd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 58 39       	vaddpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 58 39       	vaddps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d0 39       	vaddsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb d0 39       	vaddsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 55 39       	vandnpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 55 39       	vandnps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 54 39       	vandpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 54 39       	vandps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5e 39       	vdivpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5e 39       	vdivps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 7c 39       	vhaddpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 7c 39       	vhaddps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 7d 39       	vhsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 7d 39       	vhsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5f 39       	vmaxpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5f 39       	vmaxps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5d 39       	vminpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5d 39       	vminps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 59 39       	vmulpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 59 39       	vmulps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 56 39       	vorpd  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 56 39       	vorps  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 63 39       	vpacksswb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6b 39       	vpackssdw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 67 39       	vpackuswb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 2b 39    	vpackusdw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fc 39       	vpaddb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fd 39       	vpaddw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fe 39       	vpaddd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d4 39       	vpaddq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ec 39       	vpaddsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ed 39       	vpaddsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 dc 39       	vpaddusb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 dd 39       	vpaddusw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 db 39       	vpand  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 df 39       	vpandn \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e0 39       	vpavgb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e3 39       	vpavgw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 00 	vpclmullqlqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 01 	vpclmulhqlqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 10 	vpclmullqhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 11 	vpclmulhqhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 74 39       	vpcmpeqb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 75 39       	vpcmpeqw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 76 39       	vpcmpeqd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 29 39    	vpcmpeqq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 64 39       	vpcmpgtb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 65 39       	vpcmpgtw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 66 39       	vpcmpgtd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 37 39    	vpcmpgtq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0d 39    	vpermilpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0c 39    	vpermilps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 01 39    	vphaddw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 02 39    	vphaddd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 03 39    	vphaddsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 05 39    	vphsubw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 06 39    	vphsubd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 07 39    	vphsubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f5 39       	vpmaddwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 04 39    	vpmaddubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3c 39    	vpmaxsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ee 39       	vpmaxsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3d 39    	vpmaxsd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 de 39       	vpmaxub \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3e 39    	vpmaxuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3f 39    	vpmaxud \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 38 39    	vpminsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ea 39       	vpminsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 39 39    	vpminsd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 da 39       	vpminub \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3a 39    	vpminuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3b 39    	vpminud \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e4 39       	vpmulhuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0b 39    	vpmulhrsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e5 39       	vpmulhw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d5 39       	vpmullw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 40 39    	vpmulld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f4 39       	vpmuludq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 28 39    	vpmuldq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 eb 39       	vpor   \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f6 39       	vpsadbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 00 39    	vpshufb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 08 39    	vpsignb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 09 39    	vpsignw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0a 39    	vpsignd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f1 39       	vpsllw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f2 39       	vpslld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f3 39       	vpsllq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e1 39       	vpsraw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e2 39       	vpsrad \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d1 39       	vpsrlw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d2 39       	vpsrld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d3 39       	vpsrlq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f8 39       	vpsubb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f9 39       	vpsubw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fa 39       	vpsubd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fb 39       	vpsubq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e8 39       	vpsubsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e9 39       	vpsubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d8 39       	vpsubusb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d9 39       	vpsubusw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 68 39       	vpunpckhbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 69 39       	vpunpckhwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6a 39       	vpunpckhdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6d 39       	vpunpckhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 60 39       	vpunpcklbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 61 39       	vpunpcklwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 62 39       	vpunpckldq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6c 39       	vpunpcklqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ef 39       	vpxor  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5c 39       	vsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5c 39       	vsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 15 39       	vunpckhpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 15 39       	vunpckhps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 14 39       	vunpcklpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 14 39       	vunpcklps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 57 39       	vxorpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 57 39       	vxorps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 dc 39    	vaesenc \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 dd 39    	vaesenclast \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 de 39    	vaesdec \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 df 39    	vaesdeclast \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 00    	vcmpeqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 01    	vcmpltpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 02    	vcmplepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 03    	vcmpunordpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 04    	vcmpneqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 05    	vcmpnltpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 06    	vcmpnlepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 07    	vcmpordpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 08    	vcmpeq_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 09    	vcmpngepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0a    	vcmpngtpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0b    	vcmpfalsepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0c    	vcmpneq_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0d    	vcmpgepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0e    	vcmpgtpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0f    	vcmptruepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 10    	vcmpeq_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 11    	vcmplt_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 12    	vcmple_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 13    	vcmpunord_spd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 14    	vcmpneq_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 15    	vcmpnlt_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 16    	vcmpnle_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 17    	vcmpord_spd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 18    	vcmpeq_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 19    	vcmpnge_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1a    	vcmpngt_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1b    	vcmpfalse_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1c    	vcmpneq_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1d    	vcmpge_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1e    	vcmpgt_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1f    	vcmptrue_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 00    	vcmpeqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 01    	vcmpltps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 02    	vcmpleps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 03    	vcmpunordps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 04    	vcmpneqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 05    	vcmpnltps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 06    	vcmpnleps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 07    	vcmpordps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 08    	vcmpeq_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 09    	vcmpngeps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0a    	vcmpngtps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0b    	vcmpfalseps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0c    	vcmpneq_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0d    	vcmpgeps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0e    	vcmpgtps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0f    	vcmptrueps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 10    	vcmpeq_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 11    	vcmplt_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 12    	vcmple_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 13    	vcmpunord_sps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 14    	vcmpneq_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 15    	vcmpnlt_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 16    	vcmpnle_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 17    	vcmpord_sps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 18    	vcmpeq_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 19    	vcmpnge_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1a    	vcmpngt_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1b    	vcmpfalse_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1c    	vcmpneq_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1d    	vcmpge_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1e    	vcmpgt_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1f    	vcmptrue_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf 31    	vgf2p8mulb \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b4 f4 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb 0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb -0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb -0x810\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 59 2c 31    	vmaskmovps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 59 2d 31    	vmaskmovpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 31 07 	vaeskeygenassist \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 61 31 07 	vpcmpestri \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 60 31 07 	vpcmpestrm \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 63 31 07 	vpcmpistri \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 62 31 07 	vpcmpistrm \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 05 31 07 	vpermilpd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 04 31 07 	vpermilps \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 70 31 07    	vpshufd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 70 31 07    	vpshufhw \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb 70 31 07    	vpshuflw \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 09 31 07 	vroundpd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 08 31 07 	vroundps \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 49 2e 21    	vmaskmovps %xmm4,%xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 49 2f 21    	vmaskmovpd %xmm4,%xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0d 11 07 	vblendpd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0c 11 07 	vblendps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 11 07    	vcmpordpd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 11 07    	vcmpordps \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 41 11 07 	vdppd  \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 40 11 07 	vdpps  \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 42 11 07 	vmpsadbw \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0f 11 07 	vpalignr \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0e 11 07 	vpblendw \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 11 07 	vpclmulqdq \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c6 11 07    	vshufpd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c6 11 07    	vshufps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce 31 7b 	vgf2p8affineqb \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb \$0x7b,0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x810\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf 31 7b 	vgf2p8affineinvqb \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x810\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4b 39 40 	vblendvpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4a 39 40 	vblendvps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4c 39 40 	vpblendvb %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 7d 19 21    	vbroadcastsd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 2f 21       	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa e6 21       	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 5a 21       	vcvtps2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb 12 21       	vmovddup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 20 21    	vpmovsxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 23 21    	vpmovsxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 25 21    	vpmovsxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 30 21    	vpmovzxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 33 21    	vpmovzxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 35 21    	vpmovzxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 2e 21       	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fb 10 21       	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 13 21       	vmovlpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 13 21       	vmovlps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 17 21       	vmovhpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 17 21       	vmovhps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fb 11 21       	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 d6 21       	vmovq  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fa 7e 21       	vmovq  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fb 2d 09       	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fb 2c 09       	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c5 d9 12 31       	vmovlpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d8 12 31       	vmovlps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 16 31       	vmovhpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d8 16 31       	vmovhps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0b 11 07 	vroundsd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 58 11       	vaddsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5a 11       	vcvtsd2ss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5e 11       	vdivsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5f 11       	vmaxsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5d 11       	vminsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 59 11       	vmulsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 51 11       	vsqrtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5c 11       	vsubsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 00    	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 00    	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 01    	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 01    	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 02    	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 02    	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 03    	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 03    	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 04    	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 04    	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 05    	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 05    	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 06    	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 06    	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 08    	vcmpeq_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 09    	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 09    	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0a    	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0a    	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0b    	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0b    	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0c    	vcmpneq_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0d    	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0d    	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0e    	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0e    	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0f    	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0f    	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 10    	vcmpeq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 11    	vcmplt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 12    	vcmple_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 13    	vcmpunord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 14    	vcmpneq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 15    	vcmpnlt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 16    	vcmpnle_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 17    	vcmpord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 18    	vcmpeq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 19    	vcmpnge_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1a    	vcmpngt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1b    	vcmpfalse_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1c    	vcmpneq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1d    	vcmpge_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1e    	vcmpgt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1f    	vcmptrue_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 f8 ae 11       	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 19       	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 58 11       	vaddss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5a 11       	vcvtss2sd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5e 11       	vdivss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5f 11       	vmaxss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5d 11       	vminss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 59 11       	vmulss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 53 11       	vrcpss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 52 11       	vrsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 51 11       	vsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5c 11       	vsubss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 00    	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 00    	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 01    	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 01    	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 02    	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 02    	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 03    	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 03    	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 04    	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 04    	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 05    	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 05    	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 06    	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 06    	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 08    	vcmpeq_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 09    	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 09    	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0a    	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0a    	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0b    	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0b    	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0c    	vcmpneq_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0d    	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0d    	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0e    	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0e    	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0f    	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0f    	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 10    	vcmpeq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 11    	vcmplt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 12    	vcmple_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 13    	vcmpunord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 14    	vcmpneq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 15    	vcmpnlt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 16    	vcmpnle_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 17    	vcmpord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 18    	vcmpeq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 19    	vcmpnge_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1a    	vcmpngt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1b    	vcmpfalse_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1c    	vcmpneq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1d    	vcmpge_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1e    	vcmpgt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1f    	vcmptrue_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 7d 18 21    	vbroadcastss \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 2f 21       	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 21 21    	vpmovsxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 24 21    	vpmovsxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 31 21    	vpmovzxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 34 21    	vpmovzxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 2e 21       	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 18 21    	vbroadcastss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 10 21       	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 11 21       	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 f9 7e 21       	vmovd  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 6e 21       	vmovd  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fa 2d 09       	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fa 2c 09       	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 17 21 07 	vextractps \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 16 21 07 	vpextrd \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 17 21 07 	vextractps \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 59 22 31 07 	vpinsrd \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 db 2a 31       	vcvtsi2sd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 da 2a 31       	vcvtsi2ss \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 21 11 07 	vinsertps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0a 11 07 	vroundss \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 22 21    	vpmovsxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 32 21    	vpmovzxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 15 21 07 	vpextrw \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 15 21 07 	vpextrw \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 c4 31 07    	vpinsrw \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 21 07 	vpextrb \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 59 20 31 07 	vpinsrb \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 21 07 	vpextrb \$0x7,%xmm4,\(%ecx\)
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
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 ae 16 34 12    	vldmxcsr 0x1234
[ 	]*[a-f0-9]+:	c5 f9 6f 06 34 12    	vmovdqa 0x1234,%xmm0
[ 	]*[a-f0-9]+:	c5 f9 7f 06 34 12    	vmovdqa %xmm0,0x1234
[ 	]*[a-f0-9]+:	c5 f9 7e 06 34 12    	vmovd  %xmm0,0x1234
[ 	]*[a-f0-9]+:	c5 fb 2d 06 34 12    	vcvtsd2si 0x1234,%eax
[ 	]*[a-f0-9]+:	c5 fe e6 06 34 12    	vcvtdq2pd 0x1234,%ymm0
[ 	]*[a-f0-9]+:	c5 fd 5a 06 34 12    	vcvtpd2psy 0x1234,%xmm0
[ 	]*[a-f0-9]+:	c5 f9 e0 3e 34 12    	vpavgb 0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 df 06 34 12 07 	vaeskeygenassist \$0x7,0x1234,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 14 06 34 12 07 	vpextrb \$0x7,%xmm0,0x1234
[ 	]*[a-f0-9]+:	c5 fb 2a 3e 34 12    	vcvtsi2sd 0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 44 3e 34 12 07 	vpclmulqdq \$0x7,0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 36 34 12 00 	vblendvps %xmm0,0x1234,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 20 3e 34 12 07 	vpinsrb \$0x7,0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 fd 6f 06 34 12    	vmovdqa 0x1234,%ymm0
[ 	]*[a-f0-9]+:	c5 fd 7f 06 34 12    	vmovdqa %ymm0,0x1234
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3e 34 12 	vpermilpd 0x1234,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 06 34 12 07 	vroundpd \$0x7,0x1234,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 7d 19 06 34 12 07 	vextractf128 \$0x7,%ymm0,0x1234
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3e 34 12 07 	vperm2f128 \$0x7,0x1234,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 36 34 12 00 	vblendvpd %ymm0,0x1234,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 55 00    	vldmxcsr 0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 45 00    	vmovdqa 0x0\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 45 00    	vmovdqa %xmm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 45 00    	vmovd  %xmm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 45 00    	vcvtsd2si 0x0\(%ebp\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 45 00    	vcvtdq2pd 0x0\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 45 00    	vcvtpd2psy 0x0\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 7d 00    	vpavgb 0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 45 00 07 	vaeskeygenassist \$0x7,0x0\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 45 00 07 	vpextrb \$0x7,%xmm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 7d 00    	vcvtsi2sd 0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 7d 00 07 	vpclmulqdq \$0x7,0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 75 00 00 	vblendvps %xmm0,0x0\(%ebp\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 7d 00 07 	vpinsrb \$0x7,0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 45 00    	vmovdqa 0x0\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 45 00    	vmovdqa %ymm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 7d 00 	vpermilpd 0x0\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 45 00 07 	vroundpd \$0x7,0x0\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 45 00 07 	vextractf128 \$0x7,%ymm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 7d 00 07 	vperm2f128 \$0x7,0x0\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 75 00 00 	vblendvpd %ymm0,0x0\(%ebp\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 14 24    	vldmxcsr \(%esp\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 04 24    	vmovdqa \(%esp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 04 24    	vmovdqa %xmm0,\(%esp\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 04 24    	vmovd  %xmm0,\(%esp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 04 24    	vcvtsd2si \(%esp\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 04 24    	vcvtdq2pd \(%esp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 04 24    	vcvtpd2psy \(%esp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 3c 24    	vpavgb \(%esp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 04 24 07 	vaeskeygenassist \$0x7,\(%esp\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 04 24 07 	vpextrb \$0x7,%xmm0,\(%esp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 3c 24    	vcvtsi2sd \(%esp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 3c 24 07 	vpclmulqdq \$0x7,\(%esp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 34 24 00 	vblendvps %xmm0,\(%esp\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 3c 24 07 	vpinsrb \$0x7,\(%esp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 04 24    	vmovdqa \(%esp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 04 24    	vmovdqa %ymm0,\(%esp\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 3c 24 	vpermilpd \(%esp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 04 24 07 	vroundpd \$0x7,\(%esp\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 04 24 07 	vextractf128 \$0x7,%ymm0,\(%esp\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 3c 24 07 	vperm2f128 \$0x7,\(%esp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 34 24 00 	vblendvpd %ymm0,\(%esp\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 95 99 00 00 00 	vldmxcsr 0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 85 99 00 00 00 	vmovdqa 0x99\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 85 99 00 00 00 	vmovdqa %xmm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 85 99 00 00 00 	vmovd  %xmm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 85 99 00 00 00 	vcvtsd2si 0x99\(%ebp\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 85 99 00 00 00 	vcvtdq2pd 0x99\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 85 99 00 00 00 	vcvtpd2psy 0x99\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bd 99 00 00 00 	vpavgb 0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 85 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 85 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bd 99 00 00 00 	vcvtsi2sd 0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bd 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b5 99 00 00 00 00 	vblendvps %xmm0,0x99\(%ebp\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bd 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 85 99 00 00 00 	vmovdqa 0x99\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 85 99 00 00 00 	vmovdqa %ymm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bd 99 00 00 00 	vpermilpd 0x99\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 85 99 00 00 00 07 	vroundpd \$0x7,0x99\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 85 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bd 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b5 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%ebp\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 14 25 99 00 00 00 	vldmxcsr 0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 04 25 99 00 00 00 	vmovdqa 0x99\(,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 04 25 99 00 00 00 	vmovdqa %xmm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 04 25 99 00 00 00 	vmovd  %xmm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 04 25 99 00 00 00 	vcvtsd2si 0x99\(,%eiz,1\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 04 25 99 00 00 00 	vcvtdq2pd 0x99\(,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 04 25 99 00 00 00 	vcvtpd2psy 0x99\(,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 3c 25 99 00 00 00 	vpavgb 0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 04 25 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 04 25 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 3c 25 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 3c 25 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 34 25 99 00 00 00 00 	vblendvps %xmm0,0x99\(,%eiz,1\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 3c 25 99 00 00 00 07 	vpinsrb \$0x7,0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 04 25 99 00 00 00 	vmovdqa 0x99\(,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 04 25 99 00 00 00 	vmovdqa %ymm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 3c 25 99 00 00 00 	vpermilpd 0x99\(,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 04 25 99 00 00 00 07 	vroundpd \$0x7,0x99\(,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 04 25 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 3c 25 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 34 25 99 00 00 00 00 	vblendvpd %ymm0,0x99\(,%eiz,1\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 14 65 99 00 00 00 	vldmxcsr 0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 04 65 99 00 00 00 	vmovdqa 0x99\(,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 04 65 99 00 00 00 	vmovdqa %xmm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 04 65 99 00 00 00 	vmovd  %xmm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 04 65 99 00 00 00 	vcvtsd2si 0x99\(,%eiz,2\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 04 65 99 00 00 00 	vcvtdq2pd 0x99\(,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 04 65 99 00 00 00 	vcvtpd2psy 0x99\(,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 3c 65 99 00 00 00 	vpavgb 0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 04 65 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 04 65 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 3c 65 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 3c 65 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 34 65 99 00 00 00 00 	vblendvps %xmm0,0x99\(,%eiz,2\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 3c 65 99 00 00 00 07 	vpinsrb \$0x7,0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 04 65 99 00 00 00 	vmovdqa 0x99\(,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 04 65 99 00 00 00 	vmovdqa %ymm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 3c 65 99 00 00 00 	vpermilpd 0x99\(,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 04 65 99 00 00 00 07 	vroundpd \$0x7,0x99\(,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 04 65 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 3c 65 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 34 65 99 00 00 00 00 	vblendvpd %ymm0,0x99\(,%eiz,2\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 20 99 00 00 00 	vldmxcsr 0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 20 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 20 99 00 00 00 	vmovdqa %xmm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 20 99 00 00 00 	vmovd  %xmm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 20 99 00 00 00 	vcvtsd2si 0x99\(%eax,%eiz,1\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 20 99 00 00 00 	vcvtdq2pd 0x99\(%eax,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 20 99 00 00 00 	vcvtpd2psy 0x99\(%eax,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 20 99 00 00 00 	vpavgb 0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 20 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%eax,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 20 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 20 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 20 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 20 99 00 00 00 00 	vblendvps %xmm0,0x99\(%eax,%eiz,1\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 20 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 20 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 20 99 00 00 00 	vmovdqa %ymm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 20 99 00 00 00 	vpermilpd 0x99\(%eax,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 20 99 00 00 00 07 	vroundpd \$0x7,0x99\(%eax,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 20 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 20 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%eax,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 20 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%eax,%eiz,1\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 60 99 00 00 00 	vldmxcsr 0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 60 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 60 99 00 00 00 	vmovdqa %xmm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 60 99 00 00 00 	vmovd  %xmm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 60 99 00 00 00 	vcvtsd2si 0x99\(%eax,%eiz,2\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 60 99 00 00 00 	vcvtdq2pd 0x99\(%eax,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 60 99 00 00 00 	vcvtpd2psy 0x99\(%eax,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 60 99 00 00 00 	vpavgb 0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 60 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%eax,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 60 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 60 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 60 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 60 99 00 00 00 00 	vblendvps %xmm0,0x99\(%eax,%eiz,2\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 60 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 60 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 60 99 00 00 00 	vmovdqa %ymm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 60 99 00 00 00 	vpermilpd 0x99\(%eax,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 60 99 00 00 00 07 	vroundpd \$0x7,0x99\(%eax,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 60 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 60 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%eax,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 60 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%eax,%eiz,2\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 98 99 00 00 00 	vldmxcsr 0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 98 99 00 00 00 	vmovdqa 0x99\(%eax,%ebx,4\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 98 99 00 00 00 	vmovdqa %xmm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 98 99 00 00 00 	vmovd  %xmm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 98 99 00 00 00 	vcvtsd2si 0x99\(%eax,%ebx,4\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 98 99 00 00 00 	vcvtdq2pd 0x99\(%eax,%ebx,4\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 98 99 00 00 00 	vcvtpd2psy 0x99\(%eax,%ebx,4\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 98 99 00 00 00 	vpavgb 0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 98 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%eax,%ebx,4\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 98 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 98 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 98 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 98 99 00 00 00 00 	vblendvps %xmm0,0x99\(%eax,%ebx,4\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 98 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 98 99 00 00 00 	vmovdqa 0x99\(%eax,%ebx,4\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 98 99 00 00 00 	vmovdqa %ymm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 98 99 00 00 00 	vpermilpd 0x99\(%eax,%ebx,4\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 98 99 00 00 00 07 	vroundpd \$0x7,0x99\(%eax,%ebx,4\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 98 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 98 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%eax,%ebx,4\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 98 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%eax,%ebx,4\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 cc 99 00 00 00 	vldmxcsr 0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 cc 99 00 00 00 	vmovdqa 0x99\(%esp,%ecx,8\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 cc 99 00 00 00 	vmovdqa %xmm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 cc 99 00 00 00 	vmovd  %xmm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 cc 99 00 00 00 	vcvtsd2si 0x99\(%esp,%ecx,8\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 cc 99 00 00 00 	vcvtdq2pd 0x99\(%esp,%ecx,8\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 cc 99 00 00 00 	vcvtpd2psy 0x99\(%esp,%ecx,8\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc cc 99 00 00 00 	vpavgb 0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 cc 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%esp,%ecx,8\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 cc 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc cc 99 00 00 00 	vcvtsi2sd 0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc cc 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 cc 99 00 00 00 00 	vblendvps %xmm0,0x99\(%esp,%ecx,8\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc cc 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 cc 99 00 00 00 	vmovdqa 0x99\(%esp,%ecx,8\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 cc 99 00 00 00 	vmovdqa %ymm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc cc 99 00 00 00 	vpermilpd 0x99\(%esp,%ecx,8\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 cc 99 00 00 00 07 	vroundpd \$0x7,0x99\(%esp,%ecx,8\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 cc 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc cc 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%esp,%ecx,8\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 cc 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%esp,%ecx,8\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 15 99 00 00 00 	vldmxcsr 0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 15 99 00 00 00 	vmovdqa 0x99\(%ebp,%edx,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 15 99 00 00 00 	vmovdqa %xmm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 15 99 00 00 00 	vmovd  %xmm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 15 99 00 00 00 	vcvtsd2si 0x99\(%ebp,%edx,1\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 15 99 00 00 00 	vcvtdq2pd 0x99\(%ebp,%edx,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 15 99 00 00 00 	vcvtpd2psy 0x99\(%ebp,%edx,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 15 99 00 00 00 	vpavgb 0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 15 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%ebp,%edx,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 15 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 15 99 00 00 00 	vcvtsi2sd 0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 15 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 15 99 00 00 00 00 	vblendvps %xmm0,0x99\(%ebp,%edx,1\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 15 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 15 99 00 00 00 	vmovdqa 0x99\(%ebp,%edx,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 15 99 00 00 00 	vmovdqa %ymm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 15 99 00 00 00 	vpermilpd 0x99\(%ebp,%edx,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 15 99 00 00 00 07 	vroundpd \$0x7,0x99\(%ebp,%edx,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 15 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 15 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%ebp,%edx,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 15 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%ebp,%edx,1\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 f9 50 c0          	vmovmskpd %xmm0,%eax
[ 	]*[a-f0-9]+:	c5 c1 72 f0 07       	vpslld \$0x7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 fc 50 c0          	vmovmskps %ymm0,%eax
[ 	]*[a-f0-9]+:	67 c5 f8 ae 11       	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 11       	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 19       	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 19       	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 5d 2d 31    	vmaskmovpd \(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 4d 2f 21    	vmaskmovpd %ymm4,%ymm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 5d 2d 31    	vmaskmovpd \(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 4d 2f 21    	vmaskmovpd %ymm4,%ymm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 5d 2c 31    	vmaskmovps \(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 4d 2e 21    	vmaskmovps %ymm4,%ymm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 5d 2c 31    	vmaskmovps \(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 4d 2e 21    	vmaskmovps %ymm4,%ymm6,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 05 31 07 	vpermilpd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 7d 05 31 07 	vpermilpd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 04 31 07 	vpermilps \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 7d 04 31 07 	vpermilps \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 31 07 	vroundpd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 31 07 	vroundpd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 7d 08 31 07 	vroundps \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 7d 08 31 07 	vroundps \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 58 11       	vaddpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 58 11       	vaddpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 58 11       	vaddps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 58 11       	vaddps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd d0 11       	vaddsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd d0 11       	vaddsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf d0 11       	vaddsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf d0 11       	vaddsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 55 11       	vandnpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 55 11       	vandnpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 55 11       	vandnps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 55 11       	vandnps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 54 11       	vandpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 54 11       	vandpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 54 11       	vandps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 54 11       	vandps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5e 11       	vdivpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5e 11       	vdivpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5e 11       	vdivps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5e 11       	vdivps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 7c 11       	vhaddpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 7c 11       	vhaddpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf 7c 11       	vhaddps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf 7c 11       	vhaddps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 7d 11       	vhsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 7d 11       	vhsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf 7d 11       	vhsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cf 7d 11       	vhsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5f 11       	vmaxpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5f 11       	vmaxpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5f 11       	vmaxps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5f 11       	vmaxps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5d 11       	vminpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5d 11       	vminpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5d 11       	vminps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5d 11       	vminps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 59 11       	vmulpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 59 11       	vmulpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 59 11       	vmulps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 59 11       	vmulps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 56 11       	vorpd  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 56 11       	vorpd  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 56 11       	vorps  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 56 11       	vorps  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e2 4d 0d 11    	vpermilpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e2 4d 0d 11    	vpermilpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e2 4d 0c 11    	vpermilps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e2 4d 0c 11    	vpermilps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5c 11       	vsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 5c 11       	vsubpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5c 11       	vsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 5c 11       	vsubps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 15 11       	vunpckhpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 15 11       	vunpckhpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 15 11       	vunpckhps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 15 11       	vunpckhps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 14 11       	vunpcklpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 14 11       	vunpcklpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 14 11       	vunpcklps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 14 11       	vunpcklps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 57 11       	vxorpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd 57 11       	vxorpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 57 11       	vxorps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc 57 11       	vxorps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 00    	vcmpeqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 00    	vcmpeqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 01    	vcmpltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 01    	vcmpltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 02    	vcmplepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 02    	vcmplepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 03    	vcmpunordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 03    	vcmpunordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 04    	vcmpneqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 04    	vcmpneqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 05    	vcmpnltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 05    	vcmpnltpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 06    	vcmpnlepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 06    	vcmpnlepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 08    	vcmpeq_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 08    	vcmpeq_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 09    	vcmpngepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 09    	vcmpngepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0a    	vcmpngtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0a    	vcmpngtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0b    	vcmpfalsepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0b    	vcmpfalsepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0c    	vcmpneq_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0c    	vcmpneq_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0d    	vcmpgepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0d    	vcmpgepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0e    	vcmpgtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0e    	vcmpgtpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0f    	vcmptruepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 0f    	vcmptruepd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 10    	vcmpeq_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 10    	vcmpeq_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 11    	vcmplt_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 11    	vcmplt_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 12    	vcmple_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 12    	vcmple_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 13    	vcmpunord_spd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 13    	vcmpunord_spd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 14    	vcmpneq_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 14    	vcmpneq_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 15    	vcmpnlt_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 15    	vcmpnlt_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 16    	vcmpnle_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 16    	vcmpnle_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 17    	vcmpord_spd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 17    	vcmpord_spd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 18    	vcmpeq_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 18    	vcmpeq_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 19    	vcmpnge_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 19    	vcmpnge_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1a    	vcmpngt_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1a    	vcmpngt_uqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1b    	vcmpfalse_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1b    	vcmpfalse_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1c    	vcmpneq_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1c    	vcmpneq_ospd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1d    	vcmpge_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1d    	vcmpge_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1e    	vcmpgt_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1e    	vcmpgt_oqpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1f    	vcmptrue_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 1f    	vcmptrue_uspd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 00    	vcmpeqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 00    	vcmpeqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 01    	vcmpltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 01    	vcmpltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 02    	vcmpleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 02    	vcmpleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 03    	vcmpunordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 03    	vcmpunordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 04    	vcmpneqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 04    	vcmpneqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 05    	vcmpnltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 05    	vcmpnltps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 06    	vcmpnleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 06    	vcmpnleps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 08    	vcmpeq_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 08    	vcmpeq_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 09    	vcmpngeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 09    	vcmpngeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0a    	vcmpngtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0a    	vcmpngtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0b    	vcmpfalseps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0b    	vcmpfalseps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0c    	vcmpneq_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0c    	vcmpneq_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0d    	vcmpgeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0d    	vcmpgeps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0e    	vcmpgtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0e    	vcmpgtps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0f    	vcmptrueps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 0f    	vcmptrueps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 10    	vcmpeq_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 10    	vcmpeq_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 11    	vcmplt_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 11    	vcmplt_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 12    	vcmple_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 12    	vcmple_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 13    	vcmpunord_sps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 13    	vcmpunord_sps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 14    	vcmpneq_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 14    	vcmpneq_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 15    	vcmpnlt_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 15    	vcmpnlt_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 16    	vcmpnle_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 16    	vcmpnle_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 17    	vcmpord_sps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 17    	vcmpord_sps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 18    	vcmpeq_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 18    	vcmpeq_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 19    	vcmpnge_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 19    	vcmpnge_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1a    	vcmpngt_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1a    	vcmpngt_uqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1b    	vcmpfalse_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1b    	vcmpfalse_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1c    	vcmpneq_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1c    	vcmpneq_osps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1d    	vcmpge_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1d    	vcmpge_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1e    	vcmpgt_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1e    	vcmpgt_oqps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1f    	vcmptrue_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 1f    	vcmptrue_usps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb %ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf 31    	vgf2p8mulb \(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf 31    	vgf2p8mulb \(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b4 f4 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb 0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb 0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb -0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb -0x1020\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c5 ff e6 21       	vcvtpd2dqy \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps %ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c5 fd 5a 21       	vcvtpd2psy \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq %ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c5 fd e6 21       	vcvttpd2dqy \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 5b 21       	vcvtdq2ps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 5b 21       	vcvtdq2ps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 5b 21       	vcvtps2dq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd 5b 21       	vcvtps2dq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 5b 21       	vcvttps2dq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe 5b 21       	vcvttps2dq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 28 21       	vmovapd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd 28 21       	vmovapd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 28 21       	vmovaps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 28 21       	vmovaps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 6f 21       	vmovdqa \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd 6f 21       	vmovdqa \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 6f 21       	vmovdqu \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe 6f 21       	vmovdqu \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 ff 12 21       	vmovddup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 ff 12 21       	vmovddup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 16 21       	vmovshdup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe 16 21       	vmovshdup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 12 21       	vmovsldup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe 12 21       	vmovsldup \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 10 21       	vmovupd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd 10 21       	vmovupd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 10 21       	vmovups \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 10 21       	vmovups \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 7d 17 21    	vptest \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c4 e2 7d 17 21    	vptest \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 53 21       	vrcpps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 53 21       	vrcpps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 52 21       	vrsqrtps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 52 21       	vrsqrtps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 51 21       	vsqrtpd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd 51 21       	vsqrtpd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 51 21       	vsqrtps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 51 21       	vsqrtps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0f 21    	vtestpd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0f 21    	vtestpd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0e 21    	vtestps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0e 21    	vtestps \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 29 21       	vmovapd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd 29 21       	vmovapd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 29 21       	vmovaps %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fc 29 21       	vmovaps %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 7f 21       	vmovdqa %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd 7f 21       	vmovdqa %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fe 7f 21       	vmovdqu %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fe 7f 21       	vmovdqu %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fd 11 21       	vmovupd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd 11 21       	vmovupd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups %ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 fc 11 21       	vmovups %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fc 11 21       	vmovups %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 ff f0 21       	vlddqu \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 ff f0 21       	vlddqu \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fd e7 21       	vmovntdq %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd e7 21       	vmovntdq %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd 2b 21       	vmovntpd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fd 2b 21       	vmovntpd %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fc 2b 21       	vmovntps %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fc 2b 21       	vmovntps %ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 0d 11 07 	vblendpd \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 0d 11 07 	vblendpd \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 0c 11 07 	vblendps \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 0c 11 07 	vblendps \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c2 11 07    	vcmpordpd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c2 11 07    	vcmpordps \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 40 11 07 	vdpps  \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 40 11 07 	vdpps  \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 06 11 07 	vperm2f128 \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c4 e3 4d 06 11 07 	vperm2f128 \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c6 11 07    	vshufpd \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cd c6 11 07    	vshufpd \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c6 11 07    	vshufps \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	67 c5 cc c6 11 07    	vshufps \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce 31 7b 	vgf2p8affineqb \$0x7b,\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce 31 7b 	vgf2p8affineqb \$0x7b,\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb \$0x7b,0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb \$0xab,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%ymm4,%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf 31 7b 	vgf2p8affineinvqb \$0x7b,\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf 31 7b 	vgf2p8affineinvqb \$0x7b,\(%ecx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb \$0x7b,0xfe0\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1000\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1020\(%edx\),%ymm5,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 6d 4b 39 40 	vblendvpd %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 6d 4b 39 40 	vblendvpd %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 6d 4a 39 40 	vblendvps %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 6d 4a 39 40 	vblendvps %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 \$0x7,%xmm4,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 5d 18 31 07 	vinsertf128 \$0x7,\(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c4 e3 5d 18 31 07 	vinsertf128 \$0x7,\(%ecx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 \$0x7,%ymm4,%xmm4
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 21 07 	vextractf128 \$0x7,%ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 21 07 	vextractf128 \$0x7,%ymm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 1a 21    	vbroadcastf128 \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c4 e2 7d 1a 21    	vbroadcastf128 \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 5b 21       	vcvtdq2ps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 5b 21       	vcvtdq2ps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb e6 21       	vcvtpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 5a 21       	vcvtpd2psx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 5b 21       	vcvtps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 5b 21       	vcvtps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 e6 21       	vcvttpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 5b 21       	vcvttps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 5b 21       	vcvttps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 28 21       	vmovapd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 28 21       	vmovapd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 28 21       	vmovaps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 28 21       	vmovaps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 6f 21       	vmovdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 6f 21       	vmovdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 6f 21       	vmovdqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 6f 21       	vmovdqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 16 21       	vmovshdup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 16 21       	vmovshdup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 12 21       	vmovsldup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 12 21       	vmovsldup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 10 21       	vmovupd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 10 21       	vmovupd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 10 21       	vmovups \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 10 21       	vmovups \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 1c 21    	vpabsb \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 1c 21    	vpabsb \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 1d 21    	vpabsw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 1d 21    	vpabsw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 1e 21    	vpabsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 1e 21    	vpabsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 41 21    	vphminposuw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 41 21    	vphminposuw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 17 21    	vptest \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 17 21    	vptest \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 0e 21    	vtestps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 0e 21    	vtestps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 0f 21    	vtestpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 0f 21    	vtestpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 53 21       	vrcpps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 53 21       	vrcpps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 52 21       	vrsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 52 21       	vrsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 51 21       	vsqrtpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 51 21       	vsqrtpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 51 21       	vsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 51 21       	vsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 db 21    	vaesimc \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 db 21    	vaesimc \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 29 21       	vmovapd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 29 21       	vmovapd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 29 21       	vmovaps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 29 21       	vmovaps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 7f 21       	vmovdqa %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 7f 21       	vmovdqa %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 7f 21       	vmovdqu %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fa 7f 21       	vmovdqu %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 11 21       	vmovupd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 11 21       	vmovupd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 11 21       	vmovups %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 11 21       	vmovups %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fb f0 21       	vlddqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fb f0 21       	vlddqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 2a 21    	vmovntdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 2a 21    	vmovntdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 e7 21       	vmovntdq %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 e7 21       	vmovntdq %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 2b 21       	vmovntpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 2b 21       	vmovntpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 2b 21       	vmovntps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 2b 21       	vmovntps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe e6 21       	vcvtdq2pd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fe e6 21       	vcvtdq2pd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 5a 21       	vcvtps2pd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c5 fc 5a 21       	vcvtps2pd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 58 39       	vaddpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 58 39       	vaddpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 58 39       	vaddps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 58 39       	vaddps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d0 39       	vaddsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d0 39       	vaddsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb d0 39       	vaddsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 cb d0 39       	vaddsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 55 39       	vandnpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 55 39       	vandnpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 55 39       	vandnps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 55 39       	vandnps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 54 39       	vandpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 54 39       	vandpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 54 39       	vandps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 54 39       	vandps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5e 39       	vdivpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 5e 39       	vdivpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5e 39       	vdivps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 5e 39       	vdivps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 7c 39       	vhaddpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 7c 39       	vhaddpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 7c 39       	vhaddps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 cb 7c 39       	vhaddps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 7d 39       	vhsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 7d 39       	vhsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 7d 39       	vhsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 cb 7d 39       	vhsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5f 39       	vmaxpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 5f 39       	vmaxpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5f 39       	vmaxps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 5f 39       	vmaxps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5d 39       	vminpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 5d 39       	vminpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5d 39       	vminps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 5d 39       	vminps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 59 39       	vmulpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 59 39       	vmulpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 59 39       	vmulps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 59 39       	vmulps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 56 39       	vorpd  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 56 39       	vorpd  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 56 39       	vorps  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 56 39       	vorps  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 63 39       	vpacksswb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 63 39       	vpacksswb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6b 39       	vpackssdw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 6b 39       	vpackssdw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 67 39       	vpackuswb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 67 39       	vpackuswb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 2b 39    	vpackusdw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 2b 39    	vpackusdw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fc 39       	vpaddb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 fc 39       	vpaddb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fd 39       	vpaddw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 fd 39       	vpaddw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fe 39       	vpaddd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 fe 39       	vpaddd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d4 39       	vpaddq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d4 39       	vpaddq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ec 39       	vpaddsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 ec 39       	vpaddsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ed 39       	vpaddsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 ed 39       	vpaddsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 dc 39       	vpaddusb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 dc 39       	vpaddusb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 dd 39       	vpaddusw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 dd 39       	vpaddusw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 db 39       	vpand  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 db 39       	vpand  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 df 39       	vpandn \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 df 39       	vpandn \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e0 39       	vpavgb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e0 39       	vpavgb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e3 39       	vpavgw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e3 39       	vpavgw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 00 	vpclmullqlqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 00 	vpclmullqlqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 01 	vpclmulhqlqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 01 	vpclmulhqlqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 10 	vpclmullqhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 10 	vpclmullqhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 11 	vpclmulhqhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 39 11 	vpclmulhqhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 74 39       	vpcmpeqb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 74 39       	vpcmpeqb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 75 39       	vpcmpeqw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 75 39       	vpcmpeqw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 76 39       	vpcmpeqd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 76 39       	vpcmpeqd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 29 39    	vpcmpeqq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 29 39    	vpcmpeqq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 64 39       	vpcmpgtb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 64 39       	vpcmpgtb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 65 39       	vpcmpgtw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 65 39       	vpcmpgtw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 66 39       	vpcmpgtd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 66 39       	vpcmpgtd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 37 39    	vpcmpgtq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 37 39    	vpcmpgtq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0d 39    	vpermilpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 0d 39    	vpermilpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0c 39    	vpermilps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 0c 39    	vpermilps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 01 39    	vphaddw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 01 39    	vphaddw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 02 39    	vphaddd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 02 39    	vphaddd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 03 39    	vphaddsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 03 39    	vphaddsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 05 39    	vphsubw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 05 39    	vphsubw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 06 39    	vphsubd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 06 39    	vphsubd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 07 39    	vphsubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 07 39    	vphsubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f5 39       	vpmaddwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f5 39       	vpmaddwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 04 39    	vpmaddubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 04 39    	vpmaddubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3c 39    	vpmaxsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 3c 39    	vpmaxsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ee 39       	vpmaxsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 ee 39       	vpmaxsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3d 39    	vpmaxsd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 3d 39    	vpmaxsd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 de 39       	vpmaxub \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 de 39       	vpmaxub \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3e 39    	vpmaxuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 3e 39    	vpmaxuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3f 39    	vpmaxud \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 3f 39    	vpmaxud \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 38 39    	vpminsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 38 39    	vpminsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ea 39       	vpminsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 ea 39       	vpminsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 39 39    	vpminsd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 39 39    	vpminsd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 da 39       	vpminub \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 da 39       	vpminub \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3a 39    	vpminuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 3a 39    	vpminuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 3b 39    	vpminud \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 3b 39    	vpminud \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e4 39       	vpmulhuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e4 39       	vpmulhuw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0b 39    	vpmulhrsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 0b 39    	vpmulhrsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e5 39       	vpmulhw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e5 39       	vpmulhw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d5 39       	vpmullw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d5 39       	vpmullw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 40 39    	vpmulld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 40 39    	vpmulld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f4 39       	vpmuludq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f4 39       	vpmuludq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 28 39    	vpmuldq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 28 39    	vpmuldq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 eb 39       	vpor   \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 eb 39       	vpor   \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f6 39       	vpsadbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f6 39       	vpsadbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 00 39    	vpshufb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 00 39    	vpshufb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 08 39    	vpsignb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 08 39    	vpsignb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 09 39    	vpsignw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 09 39    	vpsignw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 0a 39    	vpsignd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 0a 39    	vpsignd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f1 39       	vpsllw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f1 39       	vpsllw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f2 39       	vpslld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f2 39       	vpslld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f3 39       	vpsllq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f3 39       	vpsllq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e1 39       	vpsraw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e1 39       	vpsraw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e2 39       	vpsrad \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e2 39       	vpsrad \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d1 39       	vpsrlw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d1 39       	vpsrlw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d2 39       	vpsrld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d2 39       	vpsrld \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d3 39       	vpsrlq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d3 39       	vpsrlq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f8 39       	vpsubb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f8 39       	vpsubb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 f9 39       	vpsubw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 f9 39       	vpsubw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fa 39       	vpsubd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 fa 39       	vpsubd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 fb 39       	vpsubq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 fb 39       	vpsubq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e8 39       	vpsubsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e8 39       	vpsubsb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 e9 39       	vpsubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 e9 39       	vpsubsw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d8 39       	vpsubusb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d8 39       	vpsubusb \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 d9 39       	vpsubusw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 d9 39       	vpsubusw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 68 39       	vpunpckhbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 68 39       	vpunpckhbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 69 39       	vpunpckhwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 69 39       	vpunpckhwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6a 39       	vpunpckhdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 6a 39       	vpunpckhdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6d 39       	vpunpckhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 6d 39       	vpunpckhqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 60 39       	vpunpcklbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 60 39       	vpunpcklbw \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 61 39       	vpunpcklwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 61 39       	vpunpcklwd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 62 39       	vpunpckldq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 62 39       	vpunpckldq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 6c 39       	vpunpcklqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 6c 39       	vpunpcklqdq \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 ef 39       	vpxor  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 ef 39       	vpxor  \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 5c 39       	vsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 5c 39       	vsubpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 5c 39       	vsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 5c 39       	vsubps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 15 39       	vunpckhpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 15 39       	vunpckhpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 15 39       	vunpckhps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 15 39       	vunpckhps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 14 39       	vunpcklpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 14 39       	vunpcklpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 14 39       	vunpcklps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 14 39       	vunpcklps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 57 39       	vxorpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 57 39       	vxorpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 57 39       	vxorps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 57 39       	vxorps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 dc 39    	vaesenc \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 dc 39    	vaesenc \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 dd 39    	vaesenclast \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 dd 39    	vaesenclast \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 de 39    	vaesdec \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 de 39    	vaesdec \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 49 df 39    	vaesdeclast \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 49 df 39    	vaesdeclast \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 00    	vcmpeqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 00    	vcmpeqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 01    	vcmpltpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 01    	vcmpltpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 02    	vcmplepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 02    	vcmplepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 03    	vcmpunordpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 03    	vcmpunordpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 04    	vcmpneqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 04    	vcmpneqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 05    	vcmpnltpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 05    	vcmpnltpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 06    	vcmpnlepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 06    	vcmpnlepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 07    	vcmpordpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 07    	vcmpordpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 08    	vcmpeq_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 08    	vcmpeq_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 09    	vcmpngepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 09    	vcmpngepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0a    	vcmpngtpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0a    	vcmpngtpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0b    	vcmpfalsepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0b    	vcmpfalsepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0c    	vcmpneq_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0c    	vcmpneq_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0d    	vcmpgepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0d    	vcmpgepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0e    	vcmpgtpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0e    	vcmpgtpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0f    	vcmptruepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 0f    	vcmptruepd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 10    	vcmpeq_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 10    	vcmpeq_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 11    	vcmplt_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 11    	vcmplt_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 12    	vcmple_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 12    	vcmple_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 13    	vcmpunord_spd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 13    	vcmpunord_spd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 14    	vcmpneq_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 14    	vcmpneq_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 15    	vcmpnlt_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 15    	vcmpnlt_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 16    	vcmpnle_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 16    	vcmpnle_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 17    	vcmpord_spd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 17    	vcmpord_spd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 18    	vcmpeq_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 18    	vcmpeq_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 19    	vcmpnge_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 19    	vcmpnge_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1a    	vcmpngt_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1a    	vcmpngt_uqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1b    	vcmpfalse_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1b    	vcmpfalse_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1c    	vcmpneq_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1c    	vcmpneq_ospd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1d    	vcmpge_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1d    	vcmpge_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1e    	vcmpgt_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1e    	vcmpgt_oqpd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1f    	vcmptrue_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c9 c2 39 1f    	vcmptrue_uspd \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 00    	vcmpeqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 00    	vcmpeqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 01    	vcmpltps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 01    	vcmpltps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 02    	vcmpleps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 02    	vcmpleps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 03    	vcmpunordps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 03    	vcmpunordps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 04    	vcmpneqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 04    	vcmpneqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 05    	vcmpnltps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 05    	vcmpnltps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 06    	vcmpnleps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 06    	vcmpnleps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 07    	vcmpordps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 07    	vcmpordps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 08    	vcmpeq_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 08    	vcmpeq_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 09    	vcmpngeps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 09    	vcmpngeps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0a    	vcmpngtps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0a    	vcmpngtps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0b    	vcmpfalseps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0b    	vcmpfalseps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0c    	vcmpneq_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0c    	vcmpneq_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0d    	vcmpgeps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0d    	vcmpgeps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0e    	vcmpgtps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0e    	vcmpgtps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0f    	vcmptrueps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 0f    	vcmptrueps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 10    	vcmpeq_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 10    	vcmpeq_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 11    	vcmplt_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 11    	vcmplt_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 12    	vcmple_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 12    	vcmple_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 13    	vcmpunord_sps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 13    	vcmpunord_sps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 14    	vcmpneq_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 14    	vcmpneq_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 15    	vcmpnlt_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 15    	vcmpnlt_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 16    	vcmpnle_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 16    	vcmpnle_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 17    	vcmpord_sps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 17    	vcmpord_sps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 18    	vcmpeq_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 18    	vcmpeq_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 19    	vcmpnge_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 19    	vcmpnge_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1a    	vcmpngt_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1a    	vcmpngt_uqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1b    	vcmpfalse_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1b    	vcmpfalse_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1c    	vcmpneq_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1c    	vcmpneq_osps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1d    	vcmpge_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1d    	vcmpge_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1e    	vcmpgt_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1e    	vcmpgt_oqps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1f    	vcmptrue_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	67 c5 c8 c2 39 1f    	vcmptrue_usps \(%ecx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb %xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf 31    	vgf2p8mulb \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf 31    	vgf2p8mulb \(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b4 f4 c0 1d fe ff 	vgf2p8mulb -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb 0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb 0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb -0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb -0x810\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 59 2c 31    	vmaskmovps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 59 2c 31    	vmaskmovps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 59 2d 31    	vmaskmovpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 59 2d 31    	vmaskmovpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 31 07 	vaeskeygenassist \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 31 07 	vaeskeygenassist \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 61 31 07 	vpcmpestri \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 61 31 07 	vpcmpestri \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 60 31 07 	vpcmpestrm \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 60 31 07 	vpcmpestrm \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 63 31 07 	vpcmpistri \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 63 31 07 	vpcmpistri \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 62 31 07 	vpcmpistrm \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 62 31 07 	vpcmpistrm \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 05 31 07 	vpermilpd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 05 31 07 	vpermilpd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 04 31 07 	vpermilps \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 04 31 07 	vpermilps \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 70 31 07    	vpshufd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 70 31 07    	vpshufd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 70 31 07    	vpshufhw \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa 70 31 07    	vpshufhw \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb 70 31 07    	vpshuflw \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb 70 31 07    	vpshuflw \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 09 31 07 	vroundpd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 09 31 07 	vroundpd \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps \$0x7,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 08 31 07 	vroundps \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 08 31 07 	vroundps \$0x7,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 49 2e 21    	vmaskmovps %xmm4,%xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 49 2e 21    	vmaskmovps %xmm4,%xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 49 2f 21    	vmaskmovpd %xmm4,%xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e2 49 2f 21    	vmaskmovpd %xmm4,%xmm6,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0d 11 07 	vblendpd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0d 11 07 	vblendpd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0c 11 07 	vblendps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0c 11 07 	vblendps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 11 07    	vcmpordpd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c2 11 07    	vcmpordpd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 11 07    	vcmpordps \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c2 11 07    	vcmpordps \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 41 11 07 	vdppd  \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 41 11 07 	vdppd  \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 40 11 07 	vdpps  \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 40 11 07 	vdpps  \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 42 11 07 	vmpsadbw \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 42 11 07 	vmpsadbw \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0f 11 07 	vpalignr \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0f 11 07 	vpalignr \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0e 11 07 	vpblendw \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0e 11 07 	vpblendw \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 11 07 	vpclmulqdq \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 44 11 07 	vpclmulqdq \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c6 11 07    	vshufpd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c9 c6 11 07    	vshufpd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c6 11 07    	vshufps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 c8 c6 11 07    	vshufps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce 31 7b 	vgf2p8affineqb \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce 31 7b 	vgf2p8affineqb \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb \$0x7b,0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb \$0x7b,-0x810\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb \$0xab,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb \$0x7b,%xmm4,%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf 31 7b 	vgf2p8affineinvqb \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf 31 7b 	vgf2p8affineinvqb \$0x7b,\(%ecx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb \$0x7b,-0x1e240\(%esp,%esi,8\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x7f0\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb \$0x7b,0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x800\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb \$0x7b,-0x810\(%edx\),%xmm5,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4b 39 40 	vblendvpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4b 39 40 	vblendvpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4a 39 40 	vblendvps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4a 39 40 	vblendvps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4c 39 40 	vpblendvb %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 69 4c 39 40 	vpblendvb %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e2 7d 19 21    	vbroadcastsd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c4 e2 7d 19 21    	vbroadcastsd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 2f 21       	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 2f 21       	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fa e6 21       	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa e6 21       	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 5a 21       	vcvtps2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 5a 21       	vcvtps2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 fb 12 21       	vmovddup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fb 12 21       	vmovddup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 20 21    	vpmovsxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 20 21    	vpmovsxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 23 21    	vpmovsxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 23 21    	vpmovsxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 25 21    	vpmovsxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 25 21    	vpmovsxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 30 21    	vpmovzxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 30 21    	vpmovzxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 33 21    	vpmovzxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 33 21    	vpmovzxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 35 21    	vpmovzxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 35 21    	vpmovzxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f9 2e 21       	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 2e 21       	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fb 10 21       	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fb 10 21       	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 13 21       	vmovlpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 13 21       	vmovlpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 13 21       	vmovlps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 13 21       	vmovlps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 17 21       	vmovhpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 17 21       	vmovhpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 17 21       	vmovhps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 17 21       	vmovhps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fb 11 21       	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fb 11 21       	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 d6 21       	vmovq  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fa 7e 21       	vmovq  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 d6 21       	vmovq  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fa 7e 21       	vmovq  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fb 2d 09       	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c5 fb 2d 09       	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fb 2c 09       	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c5 fb 2c 09       	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c5 d9 12 31       	vmovlpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 12 31       	vmovlpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d8 12 31       	vmovlps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d8 12 31       	vmovlps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 16 31       	vmovhpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 16 31       	vmovhpd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d8 16 31       	vmovhps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d8 16 31       	vmovhps \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0b 11 07 	vroundsd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0b 11 07 	vroundsd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 58 11       	vaddsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 58 11       	vaddsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5a 11       	vcvtsd2ss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5a 11       	vcvtsd2ss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5e 11       	vdivsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5e 11       	vdivsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5f 11       	vmaxsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5f 11       	vmaxsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5d 11       	vminsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5d 11       	vminsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 59 11       	vmulsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 59 11       	vmulsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 51 11       	vsqrtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 51 11       	vsqrtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5c 11       	vsubsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb 5c 11       	vsubsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 00    	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 00    	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 01    	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 01    	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 02    	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 02    	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 03    	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 03    	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 04    	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 04    	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 05    	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 05    	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 06    	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 06    	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 07    	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 08    	vcmpeq_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 08    	vcmpeq_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 09    	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 09    	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0a    	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0a    	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0b    	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0b    	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0c    	vcmpneq_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0c    	vcmpneq_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0d    	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0d    	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0e    	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0e    	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0f    	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 0f    	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 10    	vcmpeq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 10    	vcmpeq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 11    	vcmplt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 11    	vcmplt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 12    	vcmple_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 12    	vcmple_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 13    	vcmpunord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 13    	vcmpunord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 14    	vcmpneq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 14    	vcmpneq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 15    	vcmpnlt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 15    	vcmpnlt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 16    	vcmpnle_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 16    	vcmpnle_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 17    	vcmpord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 17    	vcmpord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 18    	vcmpeq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 18    	vcmpeq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 19    	vcmpnge_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 19    	vcmpnge_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1a    	vcmpngt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1a    	vcmpngt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1b    	vcmpfalse_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1b    	vcmpfalse_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1c    	vcmpneq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1c    	vcmpneq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1d    	vcmpge_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1d    	vcmpge_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1e    	vcmpgt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1e    	vcmpgt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1f    	vcmptrue_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 cb c2 11 1f    	vcmptrue_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 f8 ae 11       	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 11       	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 19       	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f8 ae 19       	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 58 11       	vaddss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 58 11       	vaddss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5a 11       	vcvtss2sd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5a 11       	vcvtss2sd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5e 11       	vdivss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5e 11       	vdivss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5f 11       	vmaxss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5f 11       	vmaxss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5d 11       	vminss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5d 11       	vminss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 59 11       	vmulss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 59 11       	vmulss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 53 11       	vrcpss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 53 11       	vrcpss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 52 11       	vrsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 52 11       	vrsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 51 11       	vsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 51 11       	vsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5c 11       	vsubss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca 5c 11       	vsubss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 00    	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 00    	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 01    	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 01    	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 02    	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 02    	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 03    	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 03    	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 04    	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 04    	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 05    	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 05    	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 06    	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 06    	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 08    	vcmpeq_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 08    	vcmpeq_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 09    	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 09    	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0a    	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0a    	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0b    	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0b    	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0c    	vcmpneq_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0c    	vcmpneq_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0d    	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0d    	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0e    	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0e    	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0f    	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 0f    	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 10    	vcmpeq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 10    	vcmpeq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 11    	vcmplt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 11    	vcmplt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 12    	vcmple_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 12    	vcmple_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 13    	vcmpunord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 13    	vcmpunord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 14    	vcmpneq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 14    	vcmpneq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 15    	vcmpnlt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 15    	vcmpnlt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 16    	vcmpnle_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 16    	vcmpnle_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 17    	vcmpord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 17    	vcmpord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 18    	vcmpeq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 18    	vcmpeq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 19    	vcmpnge_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 19    	vcmpnge_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1a    	vcmpngt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1a    	vcmpngt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1b    	vcmpfalse_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1b    	vcmpfalse_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1c    	vcmpneq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1c    	vcmpneq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1d    	vcmpge_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1d    	vcmpge_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1e    	vcmpgt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1e    	vcmpgt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1f    	vcmptrue_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 1f    	vcmptrue_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e2 7d 18 21    	vbroadcastss \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	67 c4 e2 7d 18 21    	vbroadcastss \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 2f 21       	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 2f 21       	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 21 21    	vpmovsxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 21 21    	vpmovsxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 24 21    	vpmovsxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 24 21    	vpmovsxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 31 21    	vpmovzxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 31 21    	vpmovzxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 34 21    	vpmovzxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 34 21    	vpmovzxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 f8 2e 21       	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f8 2e 21       	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 18 21    	vbroadcastss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 18 21    	vbroadcastss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 10 21       	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 10 21       	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 fa 11 21       	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 fa 11 21       	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 f9 7e 21       	vmovd  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 6e 21       	vmovd  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c5 f9 7e 21       	vmovd  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c5 f9 6e 21       	vmovd  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fa 2d 09       	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c5 fa 2d 09       	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c5 fa 2c 09       	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c5 fa 2c 09       	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 17 21 07 	vextractps \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 17 21 07 	vextractps \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 16 21 07 	vpextrd \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 16 21 07 	vpextrd \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 17 21 07 	vextractps \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 17 21 07 	vextractps \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 59 22 31 07 	vpinsrd \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 59 22 31 07 	vpinsrd \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 db 2a 31       	vcvtsi2sd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 db 2a 31       	vcvtsi2sd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 da 2a 31       	vcvtsi2ss \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 da 2a 31       	vcvtsi2ss \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c5 ca c2 11 07    	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 21 11 07 	vinsertps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 21 11 07 	vinsertps \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0a 11 07 	vroundss \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	67 c4 e3 49 0a 11 07 	vroundss \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 22 21    	vpmovsxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 22 21    	vpmovsxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e2 79 32 21    	vpmovzxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	67 c4 e2 79 32 21    	vpmovzxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 15 21 07 	vpextrw \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 15 21 07 	vpextrw \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 15 21 07 	vpextrw \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 15 21 07 	vpextrw \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 c4 31 07    	vpinsrw \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c5 d9 c4 31 07    	vpinsrw \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb \$0x7,%xmm4,%ecx
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 21 07 	vpextrb \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 21 07 	vpextrb \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb \$0x7,%ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 59 20 31 07 	vpinsrb \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 59 20 31 07 	vpinsrb \$0x7,\(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 21 07 	vpextrb \$0x7,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 21 07 	vpextrb \$0x7,%xmm4,\(%ecx\)
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
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq %ymm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 ae 16 34 12    	vldmxcsr 0x1234
[ 	]*[a-f0-9]+:	c5 f9 6f 06 34 12    	vmovdqa 0x1234,%xmm0
[ 	]*[a-f0-9]+:	c5 f9 7f 06 34 12    	vmovdqa %xmm0,0x1234
[ 	]*[a-f0-9]+:	c5 f9 7e 06 34 12    	vmovd  %xmm0,0x1234
[ 	]*[a-f0-9]+:	c5 fb 2d 06 34 12    	vcvtsd2si 0x1234,%eax
[ 	]*[a-f0-9]+:	c5 fe e6 06 34 12    	vcvtdq2pd 0x1234,%ymm0
[ 	]*[a-f0-9]+:	c5 fd 5a 06 34 12    	vcvtpd2psy 0x1234,%xmm0
[ 	]*[a-f0-9]+:	c5 f9 e0 3e 34 12    	vpavgb 0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 df 06 34 12 07 	vaeskeygenassist \$0x7,0x1234,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 14 06 34 12 07 	vpextrb \$0x7,%xmm0,0x1234
[ 	]*[a-f0-9]+:	c5 fb 2a 3e 34 12    	vcvtsi2sd 0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 79 44 3e 34 12 07 	vpclmulqdq \$0x7,0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 36 34 12 00 	vblendvps %xmm0,0x1234,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 20 3e 34 12 07 	vpinsrb \$0x7,0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 fd 6f 06 34 12    	vmovdqa 0x1234,%ymm0
[ 	]*[a-f0-9]+:	c5 fd 7f 06 34 12    	vmovdqa %ymm0,0x1234
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3e 34 12 	vpermilpd 0x1234,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 06 34 12 07 	vroundpd \$0x7,0x1234,%ymm0
[ 	]*[a-f0-9]+:	c4 e3 7d 19 06 34 12 07 	vextractf128 \$0x7,%ymm0,0x1234
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3e 34 12 07 	vperm2f128 \$0x7,0x1234,%ymm0,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 36 34 12 00 	vblendvpd %ymm0,0x1234,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 55 00    	vldmxcsr 0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 45 00    	vmovdqa 0x0\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 45 00    	vmovdqa %xmm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 45 00    	vmovd  %xmm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 45 00    	vcvtsd2si 0x0\(%ebp\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 45 00    	vcvtdq2pd 0x0\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 45 00    	vcvtpd2psy 0x0\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 7d 00    	vpavgb 0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 45 00 07 	vaeskeygenassist \$0x7,0x0\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 45 00 07 	vpextrb \$0x7,%xmm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 7d 00    	vcvtsi2sd 0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 7d 00 07 	vpclmulqdq \$0x7,0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 75 00 00 	vblendvps %xmm0,0x0\(%ebp\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 7d 00 07 	vpinsrb \$0x7,0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 45 00    	vmovdqa 0x0\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 45 00    	vmovdqa %ymm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 7d 00 	vpermilpd 0x0\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 45 00 07 	vroundpd \$0x7,0x0\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 45 00 07 	vextractf128 \$0x7,%ymm0,0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 7d 00 07 	vperm2f128 \$0x7,0x0\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 75 00 00 	vblendvpd %ymm0,0x0\(%ebp\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 95 99 00 00 00 	vldmxcsr 0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 85 99 00 00 00 	vmovdqa 0x99\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 85 99 00 00 00 	vmovdqa %xmm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 85 99 00 00 00 	vmovd  %xmm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 85 99 00 00 00 	vcvtsd2si 0x99\(%ebp\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 85 99 00 00 00 	vcvtdq2pd 0x99\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 85 99 00 00 00 	vcvtpd2psy 0x99\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bd 99 00 00 00 	vpavgb 0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 85 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%ebp\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 85 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bd 99 00 00 00 	vcvtsi2sd 0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bd 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b5 99 00 00 00 00 	vblendvps %xmm0,0x99\(%ebp\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bd 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 85 99 00 00 00 	vmovdqa 0x99\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 85 99 00 00 00 	vmovdqa %ymm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bd 99 00 00 00 	vpermilpd 0x99\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 85 99 00 00 00 07 	vroundpd \$0x7,0x99\(%ebp\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 85 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%ebp\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bd 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%ebp\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b5 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%ebp\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 14 25 99 00 00 00 	vldmxcsr 0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 04 25 99 00 00 00 	vmovdqa 0x99\(,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 04 25 99 00 00 00 	vmovdqa %xmm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 04 25 99 00 00 00 	vmovd  %xmm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 04 25 99 00 00 00 	vcvtsd2si 0x99\(,%eiz,1\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 04 25 99 00 00 00 	vcvtdq2pd 0x99\(,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 04 25 99 00 00 00 	vcvtpd2psy 0x99\(,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 3c 25 99 00 00 00 	vpavgb 0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 04 25 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 04 25 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 3c 25 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 3c 25 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 34 25 99 00 00 00 00 	vblendvps %xmm0,0x99\(,%eiz,1\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 3c 25 99 00 00 00 07 	vpinsrb \$0x7,0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 04 25 99 00 00 00 	vmovdqa 0x99\(,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 04 25 99 00 00 00 	vmovdqa %ymm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 3c 25 99 00 00 00 	vpermilpd 0x99\(,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 04 25 99 00 00 00 07 	vroundpd \$0x7,0x99\(,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 04 25 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 3c 25 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 34 25 99 00 00 00 00 	vblendvpd %ymm0,0x99\(,%eiz,1\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 14 65 99 00 00 00 	vldmxcsr 0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 04 65 99 00 00 00 	vmovdqa 0x99\(,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 04 65 99 00 00 00 	vmovdqa %xmm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 04 65 99 00 00 00 	vmovd  %xmm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 04 65 99 00 00 00 	vcvtsd2si 0x99\(,%eiz,2\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 04 65 99 00 00 00 	vcvtdq2pd 0x99\(,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 04 65 99 00 00 00 	vcvtpd2psy 0x99\(,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 3c 65 99 00 00 00 	vpavgb 0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 04 65 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 04 65 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a 3c 65 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 3c 65 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a 34 65 99 00 00 00 00 	vblendvps %xmm0,0x99\(,%eiz,2\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 3c 65 99 00 00 00 07 	vpinsrb \$0x7,0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 04 65 99 00 00 00 	vmovdqa 0x99\(,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 04 65 99 00 00 00 	vmovdqa %ymm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d 3c 65 99 00 00 00 	vpermilpd 0x99\(,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 04 65 99 00 00 00 07 	vroundpd \$0x7,0x99\(,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 04 65 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 3c 65 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b 34 65 99 00 00 00 00 	vblendvpd %ymm0,0x99\(,%eiz,2\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 20 99 00 00 00 	vldmxcsr 0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 20 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 20 99 00 00 00 	vmovdqa %xmm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 20 99 00 00 00 	vmovd  %xmm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 20 99 00 00 00 	vcvtsd2si 0x99\(%eax,%eiz,1\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 20 99 00 00 00 	vcvtdq2pd 0x99\(%eax,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 20 99 00 00 00 	vcvtpd2psy 0x99\(%eax,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 20 99 00 00 00 	vpavgb 0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 20 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%eax,%eiz,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 20 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 20 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 20 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 20 99 00 00 00 00 	vblendvps %xmm0,0x99\(%eax,%eiz,1\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 20 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 20 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 20 99 00 00 00 	vmovdqa %ymm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 20 99 00 00 00 	vpermilpd 0x99\(%eax,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 20 99 00 00 00 07 	vroundpd \$0x7,0x99\(%eax,%eiz,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 20 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%eax,%eiz,1\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 20 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%eax,%eiz,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 20 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%eax,%eiz,1\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 60 99 00 00 00 	vldmxcsr 0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 60 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 60 99 00 00 00 	vmovdqa %xmm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 60 99 00 00 00 	vmovd  %xmm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 60 99 00 00 00 	vcvtsd2si 0x99\(%eax,%eiz,2\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 60 99 00 00 00 	vcvtdq2pd 0x99\(%eax,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 60 99 00 00 00 	vcvtpd2psy 0x99\(%eax,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 60 99 00 00 00 	vpavgb 0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 60 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%eax,%eiz,2\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 60 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 60 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 60 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 60 99 00 00 00 00 	vblendvps %xmm0,0x99\(%eax,%eiz,2\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 60 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 60 99 00 00 00 	vmovdqa 0x99\(%eax,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 60 99 00 00 00 	vmovdqa %ymm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 60 99 00 00 00 	vpermilpd 0x99\(%eax,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 60 99 00 00 00 07 	vroundpd \$0x7,0x99\(%eax,%eiz,2\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 60 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%eax,%eiz,2\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 60 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%eax,%eiz,2\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 60 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%eax,%eiz,2\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 98 99 00 00 00 	vldmxcsr 0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 98 99 00 00 00 	vmovdqa 0x99\(%eax,%ebx,4\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 98 99 00 00 00 	vmovdqa %xmm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 98 99 00 00 00 	vmovd  %xmm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 98 99 00 00 00 	vcvtsd2si 0x99\(%eax,%ebx,4\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 98 99 00 00 00 	vcvtdq2pd 0x99\(%eax,%ebx,4\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 98 99 00 00 00 	vcvtpd2psy 0x99\(%eax,%ebx,4\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 98 99 00 00 00 	vpavgb 0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 98 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%eax,%ebx,4\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 98 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 98 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 98 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 98 99 00 00 00 00 	vblendvps %xmm0,0x99\(%eax,%ebx,4\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 98 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 98 99 00 00 00 	vmovdqa 0x99\(%eax,%ebx,4\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 98 99 00 00 00 	vmovdqa %ymm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 98 99 00 00 00 	vpermilpd 0x99\(%eax,%ebx,4\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 98 99 00 00 00 07 	vroundpd \$0x7,0x99\(%eax,%ebx,4\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 98 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%eax,%ebx,4\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 98 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%eax,%ebx,4\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 98 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%eax,%ebx,4\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 cc 99 00 00 00 	vldmxcsr 0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 cc 99 00 00 00 	vmovdqa 0x99\(%esp,%ecx,8\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 cc 99 00 00 00 	vmovdqa %xmm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 cc 99 00 00 00 	vmovd  %xmm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 cc 99 00 00 00 	vcvtsd2si 0x99\(%esp,%ecx,8\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 cc 99 00 00 00 	vcvtdq2pd 0x99\(%esp,%ecx,8\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 cc 99 00 00 00 	vcvtpd2psy 0x99\(%esp,%ecx,8\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc cc 99 00 00 00 	vpavgb 0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 cc 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%esp,%ecx,8\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 cc 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc cc 99 00 00 00 	vcvtsi2sd 0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc cc 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 cc 99 00 00 00 00 	vblendvps %xmm0,0x99\(%esp,%ecx,8\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc cc 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 cc 99 00 00 00 	vmovdqa 0x99\(%esp,%ecx,8\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 cc 99 00 00 00 	vmovdqa %ymm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc cc 99 00 00 00 	vpermilpd 0x99\(%esp,%ecx,8\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 cc 99 00 00 00 07 	vroundpd \$0x7,0x99\(%esp,%ecx,8\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 cc 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%esp,%ecx,8\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc cc 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%esp,%ecx,8\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 cc 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%esp,%ecx,8\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	67 c5 f8 ae 94 15 99 00 00 00 	vldmxcsr 0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 6f 84 15 99 00 00 00 	vmovdqa 0x99\(%ebp,%edx,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 7f 84 15 99 00 00 00 	vmovdqa %xmm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 f9 7e 84 15 99 00 00 00 	vmovd  %xmm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2d 84 15 99 00 00 00 	vcvtsd2si 0x99\(%ebp,%edx,1\),%eax
[ 	]*[a-f0-9]+:	67 c5 fe e6 84 15 99 00 00 00 	vcvtdq2pd 0x99\(%ebp,%edx,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 5a 84 15 99 00 00 00 	vcvtpd2psy 0x99\(%ebp,%edx,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c5 f9 e0 bc 15 99 00 00 00 	vpavgb 0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 df 84 15 99 00 00 00 07 	vaeskeygenassist \$0x7,0x99\(%ebp,%edx,1\),%xmm0
[ 	]*[a-f0-9]+:	67 c4 e3 79 14 84 15 99 00 00 00 07 	vpextrb \$0x7,%xmm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c5 fb 2a bc 15 99 00 00 00 	vcvtsi2sd 0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 79 44 bc 15 99 00 00 00 07 	vpclmulqdq \$0x7,0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c4 e3 59 4a b4 15 99 00 00 00 00 	vblendvps %xmm0,0x99\(%ebp,%edx,1\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	67 c4 e3 79 20 bc 15 99 00 00 00 07 	vpinsrb \$0x7,0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 c5 fd 6f 84 15 99 00 00 00 	vmovdqa 0x99\(%ebp,%edx,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c5 fd 7f 84 15 99 00 00 00 	vmovdqa %ymm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c4 e2 7d 0d bc 15 99 00 00 00 	vpermilpd 0x99\(%ebp,%edx,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 7d 09 84 15 99 00 00 00 07 	vroundpd \$0x7,0x99\(%ebp,%edx,1\),%ymm0
[ 	]*[a-f0-9]+:	67 c4 e3 7d 19 84 15 99 00 00 00 07 	vextractf128 \$0x7,%ymm0,0x99\(%ebp,%edx,1\)
[ 	]*[a-f0-9]+:	67 c4 e3 7d 06 bc 15 99 00 00 00 07 	vperm2f128 \$0x7,0x99\(%ebp,%edx,1\),%ymm0,%ymm7
[ 	]*[a-f0-9]+:	67 c4 e3 5d 4b b4 15 99 00 00 00 00 	vblendvpd %ymm0,0x99\(%ebp,%edx,1\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c5 f9 50 c0          	vmovmskpd %xmm0,%eax
[ 	]*[a-f0-9]+:	c5 c1 72 f0 07       	vpslld \$0x7,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 fc 50 c0          	vmovmskps %ymm0,%eax
[ 	]*[a-f0-9]+:	c5 cc 58 17          	vaddps \(%bx\),%ymm6,%ymm2
#pass
