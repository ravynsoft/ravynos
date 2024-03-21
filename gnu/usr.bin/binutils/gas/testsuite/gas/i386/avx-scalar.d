#as: -mavxscalar=256
#objdump: -dw
#name: i386 AVX scalar insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fd 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 ff 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 fc 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 df 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ss \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ff 2a 3d 34 12 00 00 	vcvtsi2sd 0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 7d 00       	vcvtsi2sd 0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 24       	vcvtsi2sd \(%esp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bd 99 00 00 00 	vcvtsi2sd 0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 25 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 65 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 20 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 60 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 98 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc cc 99 00 00 00 	vcvtsi2sd 0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 15 99 00 00 00 	vcvtsi2sd 0x99\(%ebp,%edx,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 fd 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 ff 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 fc 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fe 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 df 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sd \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ss \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ss \(%ecx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss \$0x7,\(%ecx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ff 2a 3d 34 12 00 00 	vcvtsi2sd 0x1234,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 7d 00       	vcvtsi2sd 0x0\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bd 99 00 00 00 	vcvtsi2sd 0x99\(%ebp\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 25 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 65 99 00 00 00 	vcvtsi2sd 0x99\(,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 20 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,1\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 60 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%eiz,2\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 98 99 00 00 00 	vcvtsi2sd 0x99\(%eax,%ebx,4\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc cc 99 00 00 00 	vcvtsi2sd 0x99\(%esp,%ecx,8\),%xmm0,%xmm7
[ 	]*[a-f0-9]+:	c5 ff 2a bc 15 99 00 00 00 	vcvtsi2sd 0x99\(%ebp,%edx,1\),%xmm0,%xmm7
#pass
