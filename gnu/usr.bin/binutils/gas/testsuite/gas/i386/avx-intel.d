#objdump: -dwMintel
#name: i386 AVX (Intel disassembly)
#source: avx.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fc 77             	vzeroall
[ 	]*[a-f0-9]+:	c5 f8 77             	vzeroupper
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b4 f4 c0 1d fe ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 ff e6 21          	vcvtpd2dq xmm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5a 21          	vcvtpd2ps xmm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd e6 21          	vcvttpd2dq xmm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu ymm4,\[ecx\]
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 ymm6,ymm4,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 ymm6,ymm4,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 xmm4,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 XMMWORD PTR \[ecx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2ps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu xmm4,\[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b4 f4 c0 1d fe ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd xmm2,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps xmm2,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  xmm4,ecx
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd xmm6,xmm4,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb xmm6,xmm4,BYTE PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 f7 f4          	vmaskmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb ecx,xmm4
[ 	]*[a-f0-9]+:	c5 c8 12 d4          	vmovhlps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 16 d4          	vmovlhps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 72 f4 07       	vpslld xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 fc 07       	vpslldq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 f4 07       	vpsllq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 71 f4 07       	vpsllw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 72 e4 07       	vpsrad xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 71 e4 07       	vpsraw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 72 d4 07       	vpsrld xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 dc 07       	vpsrldq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 d4 07       	vpsrlq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 71 d4 07       	vpsrlw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f8 ae 15 34 12 00 00 	vldmxcsr DWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 f9 6f 05 34 12 00 00 	vmovdqa xmm0,XMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 f9 7f 05 34 12 00 00 	vmovdqa XMMWORD PTR ds:0x1234,xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 05 34 12 00 00 	vmovd  DWORD PTR ds:0x1234,xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 05 34 12 00 00 	vcvtsd2si eax,QWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 fe e6 05 34 12 00 00 	vcvtdq2pd ymm0,XMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 fd 5a 05 34 12 00 00 	vcvtpd2ps xmm0,YMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 f9 e0 3d 34 12 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c4 e3 79 df 05 34 12 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 05 34 12 00 00 07 	vpextrb BYTE PTR ds:0x1234,xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3d 34 12 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c4 e3 79 44 3d 34 12 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 35 34 12 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR ds:0x1234,xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3d 34 12 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 05 34 12 00 00 	vmovdqa ymm0,YMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 fd 7f 05 34 12 00 00 	vmovdqa YMMWORD PTR ds:0x1234,ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3d 34 12 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c4 e3 7d 09 05 34 12 00 00 07 	vroundpd ymm0,YMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 05 34 12 00 00 07 	vextractf128 XMMWORD PTR ds:0x1234,ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3d 34 12 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 35 34 12 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR ds:0x1234,ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr DWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 f9 6f 45 00       	vmovdqa xmm0,XMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 f9 7f 45 00       	vmovdqa XMMWORD PTR \[ebp\+0x0\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 45 00       	vmovd  DWORD PTR \[ebp\+0x0\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 45 00       	vcvtsd2si eax,QWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 fe e6 45 00       	vcvtdq2pd ymm0,XMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 fd 5a 45 00       	vcvtpd2ps xmm0,YMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 f9 e0 7d 00       	vpavgb xmm7,xmm0,XMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 45 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 45 00 07 	vpextrb BYTE PTR \[ebp\+0x0\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 7d 00       	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 7d 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 75 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[ebp\+0x0\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 7d 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 45 00       	vmovdqa ymm0,YMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 fd 7f 45 00       	vmovdqa YMMWORD PTR \[ebp\+0x0\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 7d 00    	vpermilpd ymm7,ymm0,YMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 45 00 07 	vroundpd ymm0,YMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 45 00 07 	vextractf128 XMMWORD PTR \[ebp\+0x0\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 7d 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 75 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[ebp\+0x0\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 14 24       	vldmxcsr DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 f9 6f 04 24       	vmovdqa xmm0,XMMWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 f9 7f 04 24       	vmovdqa XMMWORD PTR \[esp\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 04 24       	vmovd  DWORD PTR \[esp\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 04 24       	vcvtsd2si eax,QWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 fe e6 04 24       	vcvtdq2pd ymm0,XMMWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 fd 5a 04 24       	vcvtpd2ps xmm0,YMMWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 f9 e0 3c 24       	vpavgb xmm7,xmm0,XMMWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 04 24 07 	vaeskeygenassist xmm0,XMMWORD PTR \[esp\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 04 24 07 	vpextrb BYTE PTR \[esp\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3c 24       	vcvtsi2sd xmm7,xmm0,DWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 3c 24 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[esp\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 34 24 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[esp\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3c 24 07 	vpinsrb xmm7,xmm0,BYTE PTR \[esp\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 04 24       	vmovdqa ymm0,YMMWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c5 fd 7f 04 24       	vmovdqa YMMWORD PTR \[esp\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3c 24    	vpermilpd ymm7,ymm0,YMMWORD PTR \[esp\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 04 24 07 	vroundpd ymm0,YMMWORD PTR \[esp\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 04 24 07 	vextractf128 XMMWORD PTR \[esp\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3c 24 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[esp\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 34 24 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[esp\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 95 99 00 00 00 	vldmxcsr DWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 85 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 85 99 00 00 00 	vmovdqa XMMWORD PTR \[ebp\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 85 99 00 00 00 	vmovd  DWORD PTR \[ebp\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 85 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 85 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 85 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bd 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 85 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 85 99 00 00 00 07 	vpextrb BYTE PTR \[ebp\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bd 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bd 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b5 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[ebp\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bd 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 85 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 85 99 00 00 00 	vmovdqa YMMWORD PTR \[ebp\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bd 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 85 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 85 99 00 00 00 07 	vextractf128 XMMWORD PTR \[ebp\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bd 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b5 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[ebp\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 99 00 00 00 	vldmxcsr DWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 04 25 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 04 25 99 00 00 00 	vmovdqa XMMWORD PTR \[eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 04 25 99 00 00 00 	vmovd  DWORD PTR \[eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 04 25 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 04 25 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 04 25 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 3c 25 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 04 25 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 04 25 99 00 00 00 07 	vpextrb BYTE PTR \[eiz\*1\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3c 25 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 3c 25 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 34 25 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3c 25 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 04 25 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 04 25 99 00 00 00 	vmovdqa YMMWORD PTR \[eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3c 25 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 04 25 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 04 25 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eiz\*1\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3c 25 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 34 25 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 14 65 99 00 00 00 	vldmxcsr DWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 04 65 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 04 65 99 00 00 00 	vmovdqa XMMWORD PTR \[eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 04 65 99 00 00 00 	vmovd  DWORD PTR \[eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 04 65 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 04 65 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 04 65 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 3c 65 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 04 65 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 04 65 99 00 00 00 07 	vpextrb BYTE PTR \[eiz\*2\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3c 65 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 3c 65 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 34 65 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3c 65 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 04 65 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 04 65 99 00 00 00 	vmovdqa YMMWORD PTR \[eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3c 65 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 04 65 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 04 65 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eiz\*2\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3c 65 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 34 65 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 20 99 00 00 00 	vldmxcsr DWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 20 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 20 99 00 00 00 	vmovdqa XMMWORD PTR \[eax\+eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 20 99 00 00 00 	vmovd  DWORD PTR \[eax\+eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 20 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 20 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 20 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 20 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 20 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 20 99 00 00 00 07 	vpextrb BYTE PTR \[eax\+eiz\*1\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 20 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 20 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 20 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eax\+eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 20 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 20 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 20 99 00 00 00 	vmovdqa YMMWORD PTR \[eax\+eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 20 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 20 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 20 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eax\+eiz\*1\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 20 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 20 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eax\+eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 60 99 00 00 00 	vldmxcsr DWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 60 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 60 99 00 00 00 	vmovdqa XMMWORD PTR \[eax\+eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 60 99 00 00 00 	vmovd  DWORD PTR \[eax\+eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 60 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 60 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 60 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 60 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 60 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 60 99 00 00 00 07 	vpextrb BYTE PTR \[eax\+eiz\*2\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 60 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 60 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 60 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eax\+eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 60 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 60 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 60 99 00 00 00 	vmovdqa YMMWORD PTR \[eax\+eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 60 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 60 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 60 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eax\+eiz\*2\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 60 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 60 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eax\+eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 98 99 00 00 00 	vldmxcsr DWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 98 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 98 99 00 00 00 	vmovdqa XMMWORD PTR \[eax\+ebx\*4\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 98 99 00 00 00 	vmovd  DWORD PTR \[eax\+ebx\*4\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 98 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 98 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 98 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 98 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 98 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 98 99 00 00 00 07 	vpextrb BYTE PTR \[eax\+ebx\*4\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 98 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 98 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 98 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eax\+ebx\*4\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 98 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 98 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 98 99 00 00 00 	vmovdqa YMMWORD PTR \[eax\+ebx\*4\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 98 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 98 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 98 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eax\+ebx\*4\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 98 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 98 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eax\+ebx\*4\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 cc 99 00 00 00 	vldmxcsr DWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 cc 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 cc 99 00 00 00 	vmovdqa XMMWORD PTR \[esp\+ecx\*8\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 cc 99 00 00 00 	vmovd  DWORD PTR \[esp\+ecx\*8\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 cc 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 cc 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 cc 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc cc 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 cc 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 cc 99 00 00 00 07 	vpextrb BYTE PTR \[esp\+ecx\*8\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc cc 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc cc 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 cc 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[esp\+ecx\*8\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc cc 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 cc 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 cc 99 00 00 00 	vmovdqa YMMWORD PTR \[esp\+ecx\*8\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc cc 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 cc 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 cc 99 00 00 00 07 	vextractf128 XMMWORD PTR \[esp\+ecx\*8\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc cc 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 cc 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[esp\+ecx\*8\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 15 99 00 00 00 	vldmxcsr DWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 15 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 15 99 00 00 00 	vmovdqa XMMWORD PTR \[ebp\+edx\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 15 99 00 00 00 	vmovd  DWORD PTR \[ebp\+edx\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 15 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 15 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 15 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 15 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 15 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 15 99 00 00 00 07 	vpextrb BYTE PTR \[ebp\+edx\*1\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 15 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 15 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 15 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[ebp\+edx\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 15 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 15 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 15 99 00 00 00 	vmovdqa YMMWORD PTR \[ebp\+edx\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 15 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 15 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 15 99 00 00 00 07 	vextractf128 XMMWORD PTR \[ebp\+edx\*1\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 15 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 15 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[ebp\+edx\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f9 50 c0          	vmovmskpd eax,xmm0
[ 	]*[a-f0-9]+:	c5 c1 72 f0 07       	vpslld xmm7,xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fc 50 c0          	vmovmskps eax,ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps ymm6,ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps YMMWORD PTR \[ecx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b4 f4 c0 1d fe ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 ff e6 21          	vcvtpd2dq xmm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5a 21          	vcvtpd2ps xmm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd e6 21          	vcvttpd2dq xmm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu ymm4,\[ecx\]
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu ymm4,\[ecx\]
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 ymm6,ymm4,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 ymm6,ymm4,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 ymm6,ymm4,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 xmm4,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 XMMWORD PTR \[ecx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 XMMWORD PTR \[ecx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2ps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu xmm4,\[ecx\]
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu xmm4,\[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps xmm7,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b4 f4 c0 1d fe ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd xmm6,xmm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd XMMWORD PTR \[ecx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd xmm2,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd xmm2,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps xmm2,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps xmm2,xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps xmm2,xmm6,XMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b4 f4 c0 1d fe ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b4 f4 c0 1d fe ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb xmm7,xmm2,XMMWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  QWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si ecx,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps xmm6,xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  xmm4,ecx
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[ecx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si ecx,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd xmm6,xmm4,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd xmm6,xmm4,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq xmm4,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb xmm6,xmm4,BYTE PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb xmm6,xmm4,BYTE PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[ecx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 f7 f4          	vmaskmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb ecx,xmm4
[ 	]*[a-f0-9]+:	c5 c8 12 d4          	vmovhlps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 16 d4          	vmovlhps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 10 d4          	vmovsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 10 d4          	vmovss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 72 f4 07       	vpslld xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 fc 07       	vpslldq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 f4 07       	vpsllq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 71 f4 07       	vpsllw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 72 e4 07       	vpsrad xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 71 e4 07       	vpsraw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 72 d4 07       	vpsrld xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 dc 07       	vpsrldq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 73 d4 07       	vpsrlq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 71 d4 07       	vpsrlw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f8 ae 15 34 12 00 00 	vldmxcsr DWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 f9 6f 05 34 12 00 00 	vmovdqa xmm0,XMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 f9 7f 05 34 12 00 00 	vmovdqa XMMWORD PTR ds:0x1234,xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 05 34 12 00 00 	vmovd  DWORD PTR ds:0x1234,xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 05 34 12 00 00 	vcvtsd2si eax,QWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 fe e6 05 34 12 00 00 	vcvtdq2pd ymm0,XMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 fd 5a 05 34 12 00 00 	vcvtpd2ps xmm0,YMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 f9 e0 3d 34 12 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c4 e3 79 df 05 34 12 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 05 34 12 00 00 07 	vpextrb BYTE PTR ds:0x1234,xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3d 34 12 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c4 e3 79 44 3d 34 12 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 35 34 12 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR ds:0x1234,xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3d 34 12 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 05 34 12 00 00 	vmovdqa ymm0,YMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c5 fd 7f 05 34 12 00 00 	vmovdqa YMMWORD PTR ds:0x1234,ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3d 34 12 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR ds:0x1234
[ 	]*[a-f0-9]+:	c4 e3 7d 09 05 34 12 00 00 07 	vroundpd ymm0,YMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 05 34 12 00 00 07 	vextractf128 XMMWORD PTR ds:0x1234,ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3d 34 12 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR ds:0x1234,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 35 34 12 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR ds:0x1234,ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr DWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 f9 6f 45 00       	vmovdqa xmm0,XMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 f9 7f 45 00       	vmovdqa XMMWORD PTR \[ebp\+0x0\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 45 00       	vmovd  DWORD PTR \[ebp\+0x0\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 45 00       	vcvtsd2si eax,QWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 fe e6 45 00       	vcvtdq2pd ymm0,XMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 fd 5a 45 00       	vcvtpd2ps xmm0,YMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 f9 e0 7d 00       	vpavgb xmm7,xmm0,XMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 45 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 45 00 07 	vpextrb BYTE PTR \[ebp\+0x0\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 7d 00       	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 7d 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 75 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[ebp\+0x0\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 7d 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 45 00       	vmovdqa ymm0,YMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c5 fd 7f 45 00       	vmovdqa YMMWORD PTR \[ebp\+0x0\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 7d 00    	vpermilpd ymm7,ymm0,YMMWORD PTR \[ebp\+0x0\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 45 00 07 	vroundpd ymm0,YMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 45 00 07 	vextractf128 XMMWORD PTR \[ebp\+0x0\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 7d 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[ebp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 75 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[ebp\+0x0\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 95 99 00 00 00 	vldmxcsr DWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 85 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 85 99 00 00 00 	vmovdqa XMMWORD PTR \[ebp\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 85 99 00 00 00 	vmovd  DWORD PTR \[ebp\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 85 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 85 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 85 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bd 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 85 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 85 99 00 00 00 07 	vpextrb BYTE PTR \[ebp\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bd 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bd 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b5 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[ebp\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bd 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 85 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 85 99 00 00 00 	vmovdqa YMMWORD PTR \[ebp\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bd 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[ebp\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 85 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 85 99 00 00 00 07 	vextractf128 XMMWORD PTR \[ebp\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bd 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[ebp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b5 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[ebp\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 99 00 00 00 	vldmxcsr DWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 04 25 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 04 25 99 00 00 00 	vmovdqa XMMWORD PTR \[eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 04 25 99 00 00 00 	vmovd  DWORD PTR \[eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 04 25 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 04 25 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 04 25 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 3c 25 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 04 25 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 04 25 99 00 00 00 07 	vpextrb BYTE PTR \[eiz\*1\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3c 25 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 3c 25 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 34 25 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3c 25 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 04 25 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 04 25 99 00 00 00 	vmovdqa YMMWORD PTR \[eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3c 25 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 04 25 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 04 25 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eiz\*1\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3c 25 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 34 25 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 14 65 99 00 00 00 	vldmxcsr DWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 04 65 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 04 65 99 00 00 00 	vmovdqa XMMWORD PTR \[eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 04 65 99 00 00 00 	vmovd  DWORD PTR \[eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 04 65 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 04 65 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 04 65 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 3c 65 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 04 65 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 04 65 99 00 00 00 07 	vpextrb BYTE PTR \[eiz\*2\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a 3c 65 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 3c 65 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a 34 65 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 3c 65 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 04 65 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 04 65 99 00 00 00 	vmovdqa YMMWORD PTR \[eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d 3c 65 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 04 65 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 04 65 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eiz\*2\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 3c 65 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b 34 65 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 20 99 00 00 00 	vldmxcsr DWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 20 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 20 99 00 00 00 	vmovdqa XMMWORD PTR \[eax\+eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 20 99 00 00 00 	vmovd  DWORD PTR \[eax\+eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 20 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 20 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 20 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 20 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 20 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 20 99 00 00 00 07 	vpextrb BYTE PTR \[eax\+eiz\*1\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 20 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 20 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 20 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eax\+eiz\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 20 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 20 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 20 99 00 00 00 	vmovdqa YMMWORD PTR \[eax\+eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 20 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 20 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 20 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eax\+eiz\*1\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 20 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 20 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eax\+eiz\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 60 99 00 00 00 	vldmxcsr DWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 60 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 60 99 00 00 00 	vmovdqa XMMWORD PTR \[eax\+eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 60 99 00 00 00 	vmovd  DWORD PTR \[eax\+eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 60 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 60 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 60 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 60 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 60 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 60 99 00 00 00 07 	vpextrb BYTE PTR \[eax\+eiz\*2\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 60 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 60 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 60 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eax\+eiz\*2\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 60 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 60 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 60 99 00 00 00 	vmovdqa YMMWORD PTR \[eax\+eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 60 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 60 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 60 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eax\+eiz\*2\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 60 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eax\+eiz\*2\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 60 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eax\+eiz\*2\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 98 99 00 00 00 	vldmxcsr DWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 98 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 98 99 00 00 00 	vmovdqa XMMWORD PTR \[eax\+ebx\*4\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 98 99 00 00 00 	vmovd  DWORD PTR \[eax\+ebx\*4\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 98 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 98 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 98 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 98 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 98 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 98 99 00 00 00 07 	vpextrb BYTE PTR \[eax\+ebx\*4\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 98 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 98 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 98 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[eax\+ebx\*4\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 98 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 98 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 98 99 00 00 00 	vmovdqa YMMWORD PTR \[eax\+ebx\*4\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 98 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 98 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 98 99 00 00 00 07 	vextractf128 XMMWORD PTR \[eax\+ebx\*4\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 98 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[eax\+ebx\*4\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 98 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[eax\+ebx\*4\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 cc 99 00 00 00 	vldmxcsr DWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 cc 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 cc 99 00 00 00 	vmovdqa XMMWORD PTR \[esp\+ecx\*8\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 cc 99 00 00 00 	vmovd  DWORD PTR \[esp\+ecx\*8\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 cc 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 cc 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 cc 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc cc 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 cc 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 cc 99 00 00 00 07 	vpextrb BYTE PTR \[esp\+ecx\*8\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc cc 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc cc 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 cc 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[esp\+ecx\*8\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc cc 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 cc 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 cc 99 00 00 00 	vmovdqa YMMWORD PTR \[esp\+ecx\*8\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc cc 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 cc 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 cc 99 00 00 00 07 	vextractf128 XMMWORD PTR \[esp\+ecx\*8\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc cc 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[esp\+ecx\*8\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 cc 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[esp\+ecx\*8\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f8 ae 94 15 99 00 00 00 	vldmxcsr DWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 6f 84 15 99 00 00 00 	vmovdqa xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 7f 84 15 99 00 00 00 	vmovdqa XMMWORD PTR \[ebp\+edx\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 f9 7e 84 15 99 00 00 00 	vmovd  DWORD PTR \[ebp\+edx\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c5 fb 2d 84 15 99 00 00 00 	vcvtsd2si eax,QWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fe e6 84 15 99 00 00 00 	vcvtdq2pd ymm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 5a 84 15 99 00 00 00 	vcvtpd2ps xmm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 f9 e0 bc 15 99 00 00 00 	vpavgb xmm7,xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 df 84 15 99 00 00 00 07 	vaeskeygenassist xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 84 15 99 00 00 00 07 	vpextrb BYTE PTR \[ebp\+edx\*1\+0x99\],xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fb 2a bc 15 99 00 00 00 	vcvtsi2sd xmm7,xmm0,DWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 79 44 bc 15 99 00 00 00 07 	vpclmulqdq xmm7,xmm0,XMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 4a b4 15 99 00 00 00 00 	vblendvps xmm6,xmm4,XMMWORD PTR \[ebp\+edx\*1\+0x99\],xmm0
[ 	]*[a-f0-9]+:	c4 e3 79 20 bc 15 99 00 00 00 07 	vpinsrb xmm7,xmm0,BYTE PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 fd 6f 84 15 99 00 00 00 	vmovdqa ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c5 fd 7f 84 15 99 00 00 00 	vmovdqa YMMWORD PTR \[ebp\+edx\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c4 e2 7d 0d bc 15 99 00 00 00 	vpermilpd ymm7,ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\]
[ 	]*[a-f0-9]+:	c4 e3 7d 09 84 15 99 00 00 00 07 	vroundpd ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 84 15 99 00 00 00 07 	vextractf128 XMMWORD PTR \[ebp\+edx\*1\+0x99\],ymm0,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 06 bc 15 99 00 00 00 07 	vperm2f128 ymm7,ymm0,YMMWORD PTR \[ebp\+edx\*1\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 4b b4 15 99 00 00 00 00 	vblendvpd ymm6,ymm4,YMMWORD PTR \[ebp\+edx\*1\+0x99\],ymm0
[ 	]*[a-f0-9]+:	c5 f9 50 c0          	vmovmskpd eax,xmm0
[ 	]*[a-f0-9]+:	c5 c1 72 f0 07       	vpslld xmm7,xmm0,0x7
[ 	]*[a-f0-9]+:	c5 fc 50 c0          	vmovmskps eax,ymm0
#pass
