#as: -mavxscalar=256
#objdump: -dwMintel
#name: i386 AVX scalar insns (Intel disassembly)
#source: avx-scalar.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fd 2f f4          	vcomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 2e f4          	vucomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 ff 2d cc          	vcvtsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 2c cc          	vcvttsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 0b d4 07    	vroundsd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cf 58 d4          	vaddsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5a d4          	vcvtsd2ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5e d4          	vdivsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5f d4          	vmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5d d4          	vminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 59 d4          	vmulsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 51 d4          	vsqrtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5c d4          	vsubsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 08       	vcmpeq_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0c       	vcmpneq_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 10       	vcmpeq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 11       	vcmplt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 12       	vcmple_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 13       	vcmpunord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 14       	vcmpneq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 15       	vcmpnlt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 16       	vcmpnle_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 17       	vcmpord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 18       	vcmpeq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 19       	vcmpnge_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1a       	vcmpngt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1b       	vcmpfalse_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1c       	vcmpneq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1d       	vcmpge_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1e       	vcmpgt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1f       	vcmptrue_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 58 d4          	vaddss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5a d4          	vcvtss2sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5e d4          	vdivss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5f d4          	vmaxss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5d d4          	vminss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 59 d4          	vmulss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 53 d4          	vrcpss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 52 d4          	vrsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 51 d4          	vsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5c d4          	vsubss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 08       	vcmpeq_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0c       	vcmpneq_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 10       	vcmpeq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 11       	vcmplt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 12       	vcmple_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 13       	vcmpunord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 14       	vcmpneq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 15       	vcmpnlt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 16       	vcmpnle_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 17       	vcmpord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 18       	vcmpeq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 19       	vcmpnge_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1a       	vcmpngt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1b       	vcmpfalse_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1c       	vcmpneq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1d       	vcmpge_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1e       	vcmpgt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1f       	vcmptrue_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 2f f4          	vcomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 2e f4          	vucomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe 2d cc          	vcvtss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 2c cc          	vcvttss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 df 2a f1          	vcvtsi2sd xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 de 2a f1          	vcvtsi2ss xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 0a d4 07    	vroundss xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cf 10 d4          	vmovsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 10 d4          	vmovss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ff 2a 3d 34 12 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 ff 2a 7d 00       	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 24       	vcvtsi2sd xmm7,xmm0,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 ff 2a bd 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 25 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 65 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 20 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 60 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 98 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc cc 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 15 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 2f f4          	vcomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 2f 21          	vcomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 2e f4          	vucomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 2e 21          	vucomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 10 21          	vmovsd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 ff 11 21          	vmovsd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 ff 2d cc          	vcvtsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 2d 09          	vcvtsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 2c cc          	vcvttsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 2c 09          	vcvttsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 0b d4 07    	vroundsd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cf 58 d4          	vaddsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5a d4          	vcvtsd2ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5e d4          	vdivsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5f d4          	vmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5d d4          	vminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 59 d4          	vmulsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 51 d4          	vsqrtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5c d4          	vsubsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 08       	vcmpeq_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0c       	vcmpneq_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 10       	vcmpeq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 11       	vcmplt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 12       	vcmple_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 13       	vcmpunord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 14       	vcmpneq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 15       	vcmpnlt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 16       	vcmpnle_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 17       	vcmpord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 18       	vcmpeq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 19       	vcmpnge_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1a       	vcmpngt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1b       	vcmpfalse_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1c       	vcmpneq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1d       	vcmpge_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1e       	vcmpgt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 d4 1f       	vcmptrue_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 58 d4          	vaddss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 58 11          	vaddss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5a d4          	vcvtss2sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5e d4          	vdivss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5f d4          	vmaxss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5d d4          	vminss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5d 11          	vminss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 59 d4          	vmulss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 59 11          	vmulss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 53 d4          	vrcpss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 52 d4          	vrsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 51 d4          	vsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5c d4          	vsubss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 08       	vcmpeq_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0c       	vcmpneq_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 10       	vcmpeq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 11       	vcmplt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 12       	vcmple_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 13       	vcmpunord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 14       	vcmpneq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 15       	vcmpnlt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 16       	vcmpnle_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 17       	vcmpord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 18       	vcmpeq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 19       	vcmpnge_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1a       	vcmpngt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1b       	vcmpfalse_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1c       	vcmpneq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1d       	vcmpge_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1e       	vcmpgt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 1f       	vcmptrue_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 2f f4          	vcomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 2f 21          	vcomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 2e f4          	vucomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 2e 21          	vucomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 10 21          	vmovss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe 11 21          	vmovss DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe 2d cc          	vcvtss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 2d 09          	vcvtss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 2c cc          	vcvttss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 2c 09          	vcvttss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 df 2a f1          	vcvtsi2sd xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 df 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 de 2a f1          	vcvtsi2ss xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 de 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ce c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 0a d4 07    	vroundss xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cf 10 d4          	vmovsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ce 10 d4          	vmovss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ff 2a 3d 34 12 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 ff 2a 7d 00       	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 ff 2a bd 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 25 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a 3c 65 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 20 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 60 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 98 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc cc 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 ff 2a bc 15 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+edx\*1\+0x99\]
#pass
