#as: -mavxscalar=256
#objdump: -dw
#name: x86-64 AVX scalar insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fd 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 ff 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 ff 2d cc       	vcvtsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2d 09       	vcvtsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2c cc       	vcvttsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2c 09       	vcvttsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 df 2a f1       	vcvtsi2sd %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 df 2a 31       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 de 2a f1       	vcvtsi2ss %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 de 2a 31       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 fc 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 fe 2d cc       	vcvtss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2d 09       	vcvtss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2c cc       	vcvttss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2c 09       	vcvttss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c5 df 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sdl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ssl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 25 78 56 34 12 	vcvtsi2sdl 0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 7d 00       	vcvtsi2sdl 0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 24       	vcvtsi2sdl \(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a bd 99 00 00 00 	vcvtsi2sdl 0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 3f 2a bf 99 00 00 00 	vcvtsi2sdl 0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3d 99 00 00 00 	vcvtsi2sdl 0x99\(%rip\),%xmm8,%xmm15        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
[ 	]*[a-f0-9]+:	c5 3f 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 3f 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 25 67 ff ff ff 	vcvtsi2sdl 0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 65 67 ff ff ff 	vcvtsi2sdl -0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a bc 23 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a bc 63 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 3f 2a bc bc 67 ff ff ff 	vcvtsi2sdl -0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 3f 2a bc f8 67 ff ff ff 	vcvtsi2sdl -0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 3f 2a bc ad 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbp,%r13,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 3f 2a bc 24 67 ff ff ff 	vcvtsi2sdl -0x99\(%rsp,%r12,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7f 2d c0       	vcvtsd2si %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 3f 2a f8       	vcvtsi2sd %r8d,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 61 ff 2d 01       	vcvtsd2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c4 61 fe 2d 01       	vcvtss2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c5 fd 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 ff 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 ff 2d cc       	vcvtsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2d 09       	vcvtsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2d 09       	vcvtsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2c cc       	vcvttsd2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2c 09       	vcvttsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 ff 2c 09       	vcvttsd2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 df 2a f1       	vcvtsi2sd %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 df 2a 31       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 df 2a 31       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 de 2a f1       	vcvtsi2ss %rcx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 de 2a 31       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e1 de 2a 31       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b d4 07    	vroundsd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 d4          	vaddsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a d4          	vcvtsd2ss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e d4          	vdivsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f d4          	vmaxsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d d4          	vminsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 d4          	vmulsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 d4          	vsqrtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c d4          	vsubsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 00       	vcmpeqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 01       	vcmpltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 02       	vcmplesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 03       	vcmpunordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 04       	vcmpneqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 05       	vcmpnltsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 06       	vcmpnlesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 08       	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 09       	vcmpngesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0a       	vcmpngtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0b       	vcmpfalsesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0c       	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0d       	vcmpgesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0e       	vcmpgtsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0f       	vcmptruesd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 10       	vcmpeq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 11       	vcmplt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 12       	vcmple_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 13       	vcmpunord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 14       	vcmpneq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 15       	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 16       	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 17       	vcmpord_ssd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 18       	vcmpeq_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 19       	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1a       	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1b       	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1c       	vcmpneq_ossd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1d       	vcmpge_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1e       	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1f       	vcmptrue_ussd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 d4          	vaddss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a d4          	vcvtss2sd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e d4          	vdivss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f d4          	vmaxss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d d4          	vminss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 d4          	vmulss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 d4          	vrcpss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 d4          	vrsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 d4          	vsqrtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c d4          	vsubss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 00       	vcmpeqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 01       	vcmpltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 02       	vcmpless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 03       	vcmpunordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 04       	vcmpneqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 05       	vcmpnltss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 06       	vcmpnless %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 08       	vcmpeq_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 09       	vcmpngess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0a       	vcmpngtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0b       	vcmpfalsess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0c       	vcmpneq_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0d       	vcmpgess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0e       	vcmpgtss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0f       	vcmptruess %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 10       	vcmpeq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 11       	vcmplt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 12       	vcmple_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 13       	vcmpunord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 14       	vcmpneq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 15       	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 16       	vcmpnle_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 17       	vcmpord_sss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 18       	vcmpeq_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 19       	vcmpnge_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1a       	vcmpngt_uqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1b       	vcmpfalse_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1c       	vcmpneq_osss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1d       	vcmpge_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1e       	vcmpgt_oqss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1f       	vcmptrue_usss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 fc 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss %xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 fe 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si \(%rcx\),%ecx
[ 	]*[a-f0-9]+:	c4 e1 fe 2d cc       	vcvtss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2d 09       	vcvtss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2d 09       	vcvtss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2c cc       	vcvttss2si %xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2c 09       	vcvttss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c4 e1 fe 2c 09       	vcvttss2si \(%rcx\),%rcx
[ 	]*[a-f0-9]+:	c5 df 2a f1          	vcvtsi2sd %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sdl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a f1          	vcvtsi2ss %ecx,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ssl \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss \(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a d4 07    	vroundss \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 cf 10 d4          	vmovsd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 ce 10 d4          	vmovss %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 25 78 56 34 12 	vcvtsi2sdl 0x12345678,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 7d 00       	vcvtsi2sdl 0x0\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a bd 99 00 00 00 	vcvtsi2sdl 0x99\(%rbp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 3f 2a bf 99 00 00 00 	vcvtsi2sdl 0x99\(%r15\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3d 99 00 00 00 	vcvtsi2sdl 0x99\(%rip\),%xmm8,%xmm15        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
[ 	]*[a-f0-9]+:	c5 3f 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%rsp\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 3f 2a bc 24 99 00 00 00 	vcvtsi2sdl 0x99\(%r12\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 25 67 ff ff ff 	vcvtsi2sdl 0xffffffffffffff67,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a 3c 65 67 ff ff ff 	vcvtsi2sdl -0x99\(,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a bc 23 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c5 3f 2a bc 63 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbx,%riz,2\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 3f 2a bc bc 67 ff ff ff 	vcvtsi2sdl -0x99\(%r12,%r15,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 01 3f 2a bc f8 67 ff ff ff 	vcvtsi2sdl -0x99\(%r8,%r15,8\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 3f 2a bc a5 67 ff ff ff 	vcvtsi2sdl -0x99\(%rbp,%r12,4\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 21 3f 2a bc 2c 67 ff ff ff 	vcvtsi2sdl -0x99\(%rsp,%r13,1\),%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 41 7f 2d c0       	vcvtsd2si %xmm8,%r8d
[ 	]*[a-f0-9]+:	c4 41 3f 2a f8       	vcvtsi2sd %r8d,%xmm8,%xmm15
[ 	]*[a-f0-9]+:	c4 61 ff 2d 01       	vcvtsd2si \(%rcx\),%r8
[ 	]*[a-f0-9]+:	c4 61 fe 2d 01       	vcvtss2si \(%rcx\),%r8
#pass
