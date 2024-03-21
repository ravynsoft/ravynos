#objdump: -drwMintel
#name: x86-64 AVX (Intel mode)
#source: x86-64-avx.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fc 77             	vzeroall
[ 	]*[a-f0-9]+:	c5 f8 77             	vzeroupper
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd ymm6,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd YMMWORD PTR \[rcx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps ymm6,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps YMMWORD PTR \[rcx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 a2 55 cf b4 f0 c0 1d fe ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 ff e6 21          	vcvtpd2dq xmm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5a 21          	vcvtpd2ps xmm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd e6 21          	vcvttpd2dq xmm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu ymm4,\[rcx\]
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d5 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d5 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd ymm7,ymm2,YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps ymm7,ymm2,YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 ymm6,ymm4,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 ymm6,ymm4,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 xmm4,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 XMMWORD PTR \[rcx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2ps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu xmm4,\[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 a2 51 cf b4 f0 c0 1d fe ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps xmm6,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd xmm6,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 61 f4 07    	vpcmpestriq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 60 f4 07    	vpcmpestrmq xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps XMMWORD PTR \[rcx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd XMMWORD PTR \[rcx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd xmm2,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps xmm2,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d1 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d1 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd ymm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  xmm4,rcx
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  xmm4,rcx
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si ecx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si ecx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fb 2d cc       	vcvtsd2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fb 2d 09       	vcvtsd2si rcx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fb 2c cc       	vcvttsd2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fb 2c 09       	vcvttsd2si rcx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 db 2a f1       	vcvtsi2sd xmm6,xmm4,rcx
[ 	]*[a-f0-9]+:	c4 e1 db 2a 31       	vcvtsi2sd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 da 2a f1       	vcvtsi2ss xmm6,xmm4,rcx
[ 	]*[a-f0-9]+:	c4 e1 da 2a 31       	vcvtsi2ss xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 d9 22 f1 07    	vpinsrq xmm6,xmm4,rcx,0x7
[ 	]*[a-f0-9]+:	c4 e3 d9 22 31 07    	vpinsrq xmm6,xmm4,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 16 e1 07    	vpextrq rcx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 07    	vpextrq QWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss ymm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  xmm4,ecx
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si ecx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si ecx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fa 2d cc       	vcvtss2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fa 2d 09       	vcvtss2si rcx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fa 2c cc       	vcvttss2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fa 2c 09       	vcvttss2si rcx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb ecx,xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd xmm6,xmm4,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps xmm2,xmm6,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq xmm4,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq xmm4,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb xmm6,xmm4,BYTE PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[rcx\],xmm4,0x7
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
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 78 56 34 12 	vldmxcsr DWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 78 56 34 12 	vmovdqa xmm8,XMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 78 56 34 12 	vmovdqa XMMWORD PTR ds:0x12345678,xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 78 56 34 12 	vmovd  DWORD PTR ds:0x12345678,xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 78 56 34 12 	vcvtsd2si r8d,QWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 78 56 34 12 	vcvtdq2pd ymm8,XMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 78 56 34 12 	vcvtpd2ps xmm8,YMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 78 56 34 12 	vpavgb xmm15,xmm8,XMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 78 56 34 12 07 	vaeskeygenassist xmm8,XMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 78 56 34 12 07 	vpextrb BYTE PTR ds:0x12345678,xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 78 56 34 12 	vcvtsi2sd xmm15,xmm8,DWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 78 56 34 12 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 78 56 34 12 80 	vblendvps xmm14,xmm12,XMMWORD PTR ds:0x12345678,xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 78 56 34 12 07 	vpinsrb xmm15,xmm8,BYTE PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 78 56 34 12 	vmovdqa ymm8,YMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 78 56 34 12 	vmovdqa YMMWORD PTR ds:0x12345678,ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 78 56 34 12 	vpermilpd ymm15,ymm8,YMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 78 56 34 12 07 	vroundpd ymm8,YMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 78 56 34 12 07 	vextractf128 XMMWORD PTR ds:0x12345678,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 78 56 34 12 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 78 56 34 12 80 	vblendvpd ymm14,ymm12,YMMWORD PTR ds:0x12345678,ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr DWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 79 6f 45 00       	vmovdqa xmm8,XMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 79 7f 45 00       	vmovdqa XMMWORD PTR \[rbp\+0x0\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 45 00       	vmovd  DWORD PTR \[rbp\+0x0\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 45 00       	vcvtsd2si r8d,QWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 7e e6 45 00       	vcvtdq2pd ymm8,XMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 7d 5a 45 00       	vcvtpd2ps xmm8,YMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 39 e0 7d 00       	vpavgb xmm15,xmm8,XMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c4 63 79 df 45 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 45 00 07 	vpextrb BYTE PTR \[rbp\+0x0\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 7d 00       	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c4 63 39 44 7d 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 75 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbp\+0x0\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 7d 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 45 00       	vmovdqa ymm8,YMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 7d 7f 45 00       	vmovdqa YMMWORD PTR \[rbp\+0x0\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 7d 00    	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 45 00 07 	vroundpd ymm8,YMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 45 00 07 	vextractf128 XMMWORD PTR \[rbp\+0x0\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 7d 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 75 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbp\+0x0\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 14 24       	vldmxcsr DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c5 79 6f 04 24       	vmovdqa xmm8,XMMWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c5 79 7f 04 24       	vmovdqa XMMWORD PTR \[rsp\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 24       	vmovd  DWORD PTR \[rsp\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 24       	vcvtsd2si r8d,QWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c5 7e e6 04 24       	vcvtdq2pd ymm8,XMMWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c5 7d 5a 04 24       	vcvtpd2ps xmm8,YMMWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c5 39 e0 3c 24       	vpavgb xmm15,xmm8,XMMWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c4 63 79 df 04 24 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rsp\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 24 07 	vpextrb BYTE PTR \[rsp\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 24       	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 24 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rsp\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 24 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rsp\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 24 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rsp\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 24       	vmovdqa ymm8,YMMWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c5 7d 7f 04 24       	vmovdqa YMMWORD PTR \[rsp\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 24    	vpermilpd ymm15,ymm8,YMMWORD PTR \[rsp\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 24 07 	vroundpd ymm8,YMMWORD PTR \[rsp\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 24 07 	vextractf128 XMMWORD PTR \[rsp\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 24 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rsp\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 24 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rsp\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 95 99 00 00 00 	vldmxcsr DWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 85 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 85 99 00 00 00 	vmovdqa XMMWORD PTR \[rbp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 85 99 00 00 00 	vmovd  DWORD PTR \[rbp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 85 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 85 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 85 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bd 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 85 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 85 99 00 00 00 07 	vpextrb BYTE PTR \[rbp\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bd 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bd 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b5 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bd 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 85 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 85 99 00 00 00 	vmovdqa YMMWORD PTR \[rbp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bd 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 85 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 85 99 00 00 00 07 	vextractf128 XMMWORD PTR \[rbp\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bd 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b5 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 c1 78 ae 97 99 00 00 00 	vldmxcsr DWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 6f 87 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 7f 87 99 00 00 00 	vmovdqa XMMWORD PTR \[r15\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7e 87 99 00 00 00 	vmovd  DWORD PTR \[r15\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 7b 2d 87 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7e e6 87 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 5a 87 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 39 e0 bf 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 79 df 87 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 79 14 87 99 00 00 00 07 	vpextrb BYTE PTR \[r15\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 3b 2a bf 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 39 44 bf 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 19 4a b7 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r15\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 43 39 20 bf 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 41 7d 6f 87 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 7f 87 99 00 00 00 	vmovdqa YMMWORD PTR \[r15\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 42 3d 0d bf 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 7d 09 87 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 7d 19 87 99 00 00 00 07 	vextractf128 XMMWORD PTR \[r15\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 3d 06 bf 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 1d 4b b7 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r15\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 15 99 00 00 00 	vldmxcsr DWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 6f 05 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7f 05 99 00 00 00 	vmovdqa XMMWORD PTR \[rip\+0x99\],xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7e 05 99 00 00 00 	vmovd  DWORD PTR \[rip\+0x99\],xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7b 2d 05 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7e e6 05 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 5a 05 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 39 e0 3d 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 df 05 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 14 05 99 00 00 00 07 	vpextrb BYTE PTR \[rip\+0x99\],xmm8,0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 3b 2a 3d 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 44 3d 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 19 4a 35 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rip\+0x99\],xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 20 3d 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 6f 05 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 7f 05 99 00 00 00 	vmovdqa YMMWORD PTR \[rip\+0x99\],ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3d 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 09 05 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 19 05 99 00 00 00 07 	vextractf128 XMMWORD PTR \[rip\+0x99\],ymm8,0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 3d 06 3d 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 1d 4b 35 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rip\+0x99\],ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 f8 ae 94 24 99 00 00 00 	vldmxcsr DWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 84 24 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 84 24 99 00 00 00 	vmovdqa XMMWORD PTR \[rsp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 84 24 99 00 00 00 	vmovd  DWORD PTR \[rsp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 84 24 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 84 24 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 84 24 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bc 24 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 84 24 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 84 24 99 00 00 00 07 	vpextrb BYTE PTR \[rsp\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bc 24 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 24 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 24 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rsp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 24 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 84 24 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 84 24 99 00 00 00 	vmovdqa YMMWORD PTR \[rsp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 24 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 24 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 24 99 00 00 00 07 	vextractf128 XMMWORD PTR \[rsp\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 24 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 24 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rsp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 c1 78 ae 94 24 99 00 00 00 	vldmxcsr DWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 6f 84 24 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 7f 84 24 99 00 00 00 	vmovdqa XMMWORD PTR \[r12\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7e 84 24 99 00 00 00 	vmovd  DWORD PTR \[r12\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 7b 2d 84 24 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7e e6 84 24 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 5a 84 24 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 39 e0 bc 24 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 79 df 84 24 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 79 14 84 24 99 00 00 00 07 	vpextrb BYTE PTR \[r12\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 3b 2a bc 24 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 39 44 bc 24 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 19 4a b4 24 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r12\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 43 39 20 bc 24 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 41 7d 6f 84 24 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 7f 84 24 99 00 00 00 	vmovdqa YMMWORD PTR \[r12\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 42 3d 0d bc 24 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 7d 09 84 24 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 7d 19 84 24 99 00 00 00 07 	vextractf128 XMMWORD PTR \[r12\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 3d 06 bc 24 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 1d 4b b4 24 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r12\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 67 ff ff ff 	vldmxcsr DWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 67 ff ff ff 	vmovdqa XMMWORD PTR ds:0xffffffffffffff67,xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 67 ff ff ff 	vmovd  DWORD PTR ds:0xffffffffffffff67,xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 67 ff ff ff 07 	vpextrb BYTE PTR ds:0xffffffffffffff67,xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR ds:0xffffffffffffff67,xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 67 ff ff ff 	vmovdqa YMMWORD PTR ds:0xffffffffffffff67,ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 67 ff ff ff 07 	vextractf128 XMMWORD PTR ds:0xffffffffffffff67,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR ds:0xffffffffffffff67,ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 14 65 67 ff ff ff 	vldmxcsr DWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 04 65 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 04 65 67 ff ff ff 	vmovdqa XMMWORD PTR \[riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 65 67 ff ff ff 	vmovd  DWORD PTR \[riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 65 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 04 65 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 04 65 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 3c 65 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 04 65 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 65 67 ff ff ff 07 	vpextrb BYTE PTR \[riz\*2-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 65 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 65 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 65 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 65 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 65 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 04 65 67 ff ff ff 	vmovdqa YMMWORD PTR \[riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 65 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 65 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 65 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[riz\*2-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 65 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 65 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 94 23 67 ff ff ff 	vldmxcsr DWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 84 23 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 84 23 67 ff ff ff 	vmovdqa XMMWORD PTR \[rbx\+riz\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 84 23 67 ff ff ff 	vmovd  DWORD PTR \[rbx\+riz\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 84 23 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 84 23 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 84 23 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bc 23 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 84 23 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 84 23 67 ff ff ff 07 	vpextrb BYTE PTR \[rbx\+riz\*1-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bc 23 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 23 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 23 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbx\+riz\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 23 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 84 23 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 84 23 67 ff ff ff 	vmovdqa YMMWORD PTR \[rbx\+riz\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 23 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 23 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 23 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rbx\+riz\*1-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 23 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 23 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbx\+riz\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 94 63 67 ff ff ff 	vldmxcsr DWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 84 63 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 84 63 67 ff ff ff 	vmovdqa XMMWORD PTR \[rbx\+riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 84 63 67 ff ff ff 	vmovd  DWORD PTR \[rbx\+riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 84 63 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 84 63 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 84 63 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bc 63 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 84 63 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 84 63 67 ff ff ff 07 	vpextrb BYTE PTR \[rbx\+riz\*2-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bc 63 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 63 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 63 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbx\+riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 63 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 84 63 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 84 63 67 ff ff ff 	vmovdqa YMMWORD PTR \[rbx\+riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 63 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 63 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 63 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rbx\+riz\*2-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 63 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 63 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbx\+riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 bc 67 ff ff ff 	vldmxcsr DWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 bc 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 bc 67 ff ff ff 	vmovdqa XMMWORD PTR \[r12\+r15\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 bc 67 ff ff ff 	vmovd  DWORD PTR \[r12\+r15\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 bc 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 bc 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 bc 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc bc 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 03 79 df 84 bc 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 79 14 84 bc 67 ff ff ff 07 	vpextrb BYTE PTR \[r12\+r15\*4-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc bc 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 03 39 44 bc bc 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 bc 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r12\+r15\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 03 39 20 bc bc 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 bc 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 bc 67 ff ff ff 	vmovdqa YMMWORD PTR \[r12\+r15\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc bc 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 bc 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 bc 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[r12\+r15\*4-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc bc 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 bc 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r12\+r15\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 f8 67 ff ff ff 	vldmxcsr DWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 f8 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 f8 67 ff ff ff 	vmovdqa XMMWORD PTR \[r8\+r15\*8-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 f8 67 ff ff ff 	vmovd  DWORD PTR \[r8\+r15\*8-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 f8 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 f8 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 f8 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc f8 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 03 79 df 84 f8 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 79 14 84 f8 67 ff ff ff 07 	vpextrb BYTE PTR \[r8\+r15\*8-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc f8 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 03 39 44 bc f8 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 f8 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r8\+r15\*8-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 03 39 20 bc f8 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 f8 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 f8 67 ff ff ff 	vmovdqa YMMWORD PTR \[r8\+r15\*8-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc f8 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 f8 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 f8 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[r8\+r15\*8-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc f8 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 f8 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r8\+r15\*8-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 ad 67 ff ff ff 	vldmxcsr DWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 ad 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 ad 67 ff ff ff 	vmovdqa XMMWORD PTR \[rbp\+r13\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 ad 67 ff ff ff 	vmovd  DWORD PTR \[rbp\+r13\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 ad 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 ad 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 ad 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc ad 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 23 79 df 84 ad 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbp\+r13\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 79 14 84 ad 67 ff ff ff 07 	vpextrb BYTE PTR \[rbp\+r13\*4-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc ad 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 23 39 44 bc ad 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbp\+r13\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 ad 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbp\+r13\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 23 39 20 bc ad 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbp\+r13\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 ad 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 ad 67 ff ff ff 	vmovdqa YMMWORD PTR \[rbp\+r13\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc ad 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbp\+r13\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 ad 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rbp\+r13\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 ad 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rbp\+r13\*4-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc ad 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbp\+r13\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 ad 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbp\+r13\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 24 67 ff ff ff 	vldmxcsr DWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 24 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 24 67 ff ff ff 	vmovdqa XMMWORD PTR \[rsp\+r12\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 24 67 ff ff ff 	vmovd  DWORD PTR \[rsp\+r12\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 24 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 24 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 24 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc 24 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 23 79 df 84 24 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rsp\+r12\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 79 14 84 24 67 ff ff ff 07 	vpextrb BYTE PTR \[rsp\+r12\*1-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc 24 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 23 39 44 bc 24 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rsp\+r12\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 24 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rsp\+r12\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 23 39 20 bc 24 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rsp\+r12\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 24 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 24 67 ff ff ff 	vmovdqa YMMWORD PTR \[rsp\+r12\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc 24 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rsp\+r12\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 24 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rsp\+r12\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 24 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rsp\+r12\*1-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc 24 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rsp\+r12\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 24 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rsp\+r12\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 41 79 50 c0       	vmovmskpd r8d,xmm8
[ 	]*[a-f0-9]+:	c4 c1 01 72 f0 07    	vpslld xmm15,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 7c 50 c0       	vmovmskps r8d,ymm8
[ 	]*[a-f0-9]+:	c4 41 79 6f f8       	vmovdqa xmm15,xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7e c0       	vmovd  r8d,xmm8
[ 	]*[a-f0-9]+:	c4 41 7b 2d c0       	vcvtsd2si r8d,xmm8
[ 	]*[a-f0-9]+:	c4 41 7e e6 c0       	vcvtdq2pd ymm8,xmm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a c0       	vcvtpd2ps xmm8,ymm8
[ 	]*[a-f0-9]+:	c4 43 79 df f8 07    	vaeskeygenassist xmm15,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 43 79 14 c0 07    	vpextrb r8d,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 3b 2a f8       	vcvtsi2sd xmm15,xmm8,r8d
[ 	]*[a-f0-9]+:	c4 43 01 44 e0 07    	vpclmulqdq xmm12,xmm15,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 43 19 4a f0 80    	vblendvps xmm14,xmm12,xmm8,xmm8
[ 	]*[a-f0-9]+:	c4 43 39 20 f8 07    	vpinsrb xmm15,xmm8,r8d,0x7
[ 	]*[a-f0-9]+:	c4 41 7d 6f f8       	vmovdqa ymm15,ymm8
[ 	]*[a-f0-9]+:	c4 42 05 0d e0       	vpermilpd ymm12,ymm15,ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 09 f8 07    	vroundpd ymm15,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 7d 19 c0 07    	vextractf128 xmm8,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 05 06 e0 07    	vperm2f128 ymm12,ymm15,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 1d 4b f7 80    	vblendvpd ymm14,ymm12,ymm15,ymm8
[ 	]*[a-f0-9]+:	c4 43 3d 18 f8 07    	vinsertf128 ymm15,ymm8,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 61 fb 2d 01       	vcvtsd2si r8,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 43 79 17 c0 0a    	vextractps r8d,xmm8,0xa
[ 	]*[a-f0-9]+:	c4 61 fa 2d 01       	vcvtss2si r8,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 41 01 c4 c0 07    	vpinsrw xmm8,xmm15,r8d,0x7
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd ymm6,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd YMMWORD PTR \[rcx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2d 31       	vmaskmovpd ymm6,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2f 21       	vmaskmovpd YMMWORD PTR \[rcx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps ymm6,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps YMMWORD PTR \[rcx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 5d 2c 31       	vmaskmovps ymm6,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2e 21       	vmaskmovps YMMWORD PTR \[rcx\],ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 7d 05 d6 07    	vpermilpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 05 31 07    	vpermilpd ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 d6 07    	vpermilps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 04 31 07    	vpermilps ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 d6 07    	vroundpd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 09 31 07    	vroundpd ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 d6 07    	vroundps ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 08 31 07    	vroundps ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd 58 d4          	vaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 58 11          	vaddpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 58 d4          	vaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 58 11          	vaddps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd d0 d4          	vaddsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd d0 11          	vaddsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf d0 d4          	vaddsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf d0 11          	vaddsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 55 d4          	vandnpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 55 11          	vandnpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 55 d4          	vandnps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 55 11          	vandnps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 54 d4          	vandpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 54 11          	vandpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 54 d4          	vandps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 54 11          	vandps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5e d4          	vdivpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5e 11          	vdivpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5e d4          	vdivps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5e 11          	vdivps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 7c d4          	vhaddpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 7c 11          	vhaddpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf 7c d4          	vhaddps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf 7c 11          	vhaddps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 7d d4          	vhsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 7d 11          	vhsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf 7d d4          	vhsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cf 7d 11          	vhsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5f d4          	vmaxpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5f 11          	vmaxpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5f d4          	vmaxps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5f 11          	vmaxps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5d d4          	vminpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5d 11          	vminpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5d d4          	vminps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5d 11          	vminps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 59 d4          	vmulpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 59 11          	vmulpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 59 d4          	vmulps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 59 11          	vmulps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 56 d4          	vorpd  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 56 11          	vorpd  ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 56 d4          	vorps  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 56 11          	vorps  ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0d d4       	vpermilpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0d 11       	vpermilpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0c d4       	vpermilps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0c 11       	vpermilps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5c d4          	vsubpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 5c 11          	vsubpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5c d4          	vsubps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 5c 11          	vsubps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 15 d4          	vunpckhpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 15 11          	vunpckhpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 15 d4          	vunpckhps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 15 11          	vunpckhps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 14 d4          	vunpcklpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 14 11          	vunpcklpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 14 d4          	vunpcklps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 14 11          	vunpcklps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 57 d4          	vxorpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd 57 11          	vxorpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 57 d4          	vxorps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc 57 11          	vxorps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 00       	vcmpeqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 00       	vcmpeqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 01       	vcmpltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 01       	vcmpltpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 02       	vcmplepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 02       	vcmplepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 03       	vcmpunordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 03       	vcmpunordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 04       	vcmpneqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 04       	vcmpneqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 05       	vcmpnltpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 05       	vcmpnltpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 06       	vcmpnlepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 06       	vcmpnlepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 08       	vcmpeq_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 08       	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 09       	vcmpngepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 09       	vcmpngepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0a       	vcmpngtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0a       	vcmpngtpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0b       	vcmpfalsepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0b       	vcmpfalsepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0c       	vcmpneq_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0c       	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0d       	vcmpgepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0d       	vcmpgepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0e       	vcmpgtpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0e       	vcmpgtpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 0f       	vcmptruepd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 0f       	vcmptruepd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 10       	vcmpeq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 10       	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 11       	vcmplt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 11       	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 12       	vcmple_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 12       	vcmple_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 13       	vcmpunord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 13       	vcmpunord_spd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 14       	vcmpneq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 14       	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 15       	vcmpnlt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 15       	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 16       	vcmpnle_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 16       	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 17       	vcmpord_spd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 17       	vcmpord_spd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 18       	vcmpeq_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 18       	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 19       	vcmpnge_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 19       	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1a       	vcmpngt_uqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1a       	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1b       	vcmpfalse_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1b       	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1c       	vcmpneq_ospd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1c       	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1d       	vcmpge_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1d       	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1e       	vcmpgt_oqpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1e       	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 d4 1f       	vcmptrue_uspd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 1f       	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 00       	vcmpeqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 00       	vcmpeqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 01       	vcmpltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 01       	vcmpltps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 02       	vcmpleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 02       	vcmpleps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 03       	vcmpunordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 03       	vcmpunordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 04       	vcmpneqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 04       	vcmpneqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 05       	vcmpnltps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 05       	vcmpnltps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 06       	vcmpnleps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 06       	vcmpnleps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 08       	vcmpeq_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 08       	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 09       	vcmpngeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 09       	vcmpngeps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0a       	vcmpngtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0a       	vcmpngtps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0b       	vcmpfalseps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0b       	vcmpfalseps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0c       	vcmpneq_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0c       	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0d       	vcmpgeps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0d       	vcmpgeps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0e       	vcmpgtps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0e       	vcmpgtps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 0f       	vcmptrueps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 0f       	vcmptrueps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 10       	vcmpeq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 10       	vcmpeq_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 11       	vcmplt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 11       	vcmplt_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 12       	vcmple_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 12       	vcmple_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 13       	vcmpunord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 13       	vcmpunord_sps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 14       	vcmpneq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 14       	vcmpneq_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 15       	vcmpnlt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 15       	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 16       	vcmpnle_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 16       	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 17       	vcmpord_sps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 17       	vcmpord_sps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 18       	vcmpeq_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 18       	vcmpeq_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 19       	vcmpnge_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 19       	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1a       	vcmpngt_uqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1a       	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1b       	vcmpfalse_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1b       	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1c       	vcmpneq_osps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1c       	vcmpneq_osps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1d       	vcmpge_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1d       	vcmpge_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1e       	vcmpgt_oqps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1e       	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 1f       	vcmptrue_usps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 1f       	vcmptrue_usps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf f4       	vgf2p8mulb ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf 31       	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 a2 55 cf b4 f0 c0 1d fe ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 0f 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 10 00 00 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 00 f0 ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:	c4 e2 55 cf b2 e0 ef ff ff 	vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:	c5 ff e6 e4          	vcvtpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 ff e6 21          	vcvtpd2dq xmm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 5a e4          	vcvtpd2ps xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5a 21          	vcvtpd2ps xmm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd e6 e4          	vcvttpd2dq xmm4,ymm4
[ 	]*[a-f0-9]+:	c5 fd e6 21          	vcvttpd2dq xmm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 5b f4          	vcvtdq2ps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 5b 21          	vcvtdq2ps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 5b f4          	vcvtps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 5b 21          	vcvtps2dq ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 5b f4          	vcvttps2dq ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 5b 21          	vcvttps2dq ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 28 21          	vmovapd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 28 21          	vmovaps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 6f 21          	vmovdqa ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 6f 21          	vmovdqu ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ff 12 f4          	vmovddup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ff 12 21          	vmovddup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 16 f4          	vmovshdup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 16 21          	vmovshdup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 12 f4          	vmovsldup ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe 12 21          	vmovsldup ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 10 21          	vmovupd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 10 21          	vmovups ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 17 f4       	vptest ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 17 21       	vptest ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 53 f4          	vrcpps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 53 21          	vrcpps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 52 f4          	vrsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 52 21          	vrsqrtps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 51 f4          	vsqrtpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 51 21          	vsqrtpd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 51 f4          	vsqrtps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 51 21          	vsqrtps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0f f4       	vtestpd ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0f 21       	vtestpd ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0e f4       	vtestps ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 0e 21       	vtestps ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fd 28 f4          	vmovapd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 29 21          	vmovapd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 28 f4          	vmovaps ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 29 21          	vmovaps YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 6f f4          	vmovdqa ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 7f 21          	vmovdqa YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fe 6f f4          	vmovdqu ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fe 7f 21          	vmovdqu YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 10 f4          	vmovupd ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 11 21          	vmovupd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 10 f4          	vmovups ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 11 21          	vmovups YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu ymm4,\[rcx\]
[ 	]*[a-f0-9]+:	c5 ff f0 21          	vlddqu ymm4,\[rcx\]
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd e7 21          	vmovntdq YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fd 2b 21          	vmovntpd YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c5 fc 2b 21          	vmovntps YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 4d 0d d4 07    	vblendpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0d 11 07    	vblendpd ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c d4 07    	vblendps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0c 11 07    	vblendps ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c2 d4 07       	vcmpordpd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cd c2 11 07       	vcmpordpd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 d4 07       	vcmpordps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cc c2 11 07       	vcmpordps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 40 d4 07    	vdpps  ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 40 11 07    	vdpps  ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 d4 07    	vperm2f128 ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 06 11 07    	vperm2f128 ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c6 d4 07       	vshufpd ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cd c6 11 07       	vshufpd ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cc c6 d4 07       	vshufps ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cc c6 11 07       	vshufps ymm2,ymm6,YMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 ab    	vgf2p8affineqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 ce f4 7b    	vgf2p8affineqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce 31 7b    	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d5 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 0f 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 10 00 00 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 00 f0 ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 ce b2 e0 ef ff ff 7b 	vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 ab    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d5 cf f4 7b    	vgf2p8affineinvqb ymm6,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf 31 7b    	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d5 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 0f 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 10 00 00 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 00 f0 ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d5 cf b2 e0 ef ff ff 7b 	vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 6d 4b fe 40    	vblendvpd ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd ymm7,ymm2,YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4b 39 40    	vblendvpd ymm7,ymm2,YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a fe 40    	vblendvps ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps ymm7,ymm2,YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4a 39 40    	vblendvps ymm7,ymm2,YMMWORD PTR \[rcx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 5d 18 f4 07    	vinsertf128 ymm6,ymm4,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 ymm6,ymm4,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 5d 18 31 07    	vinsertf128 ymm6,ymm4,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 e4 07    	vextractf128 xmm4,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 XMMWORD PTR \[rcx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 7d 19 21 07    	vextractf128 XMMWORD PTR \[rcx\],ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1a 21       	vbroadcastf128 ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2ps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0e f4       	vtestps xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0e 21       	vtestps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0f f4       	vtestpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 0f 21       	vtestpd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu xmm4,\[rcx\]
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu xmm4,\[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 e4          	vcvtdq2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fe e6 21          	vcvtdq2pd ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 5a e4          	vcvtps2pd ymm4,xmm4
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fc 5a 21          	vcvtps2pd ymm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 58 d4          	vaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 58 39          	vaddpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 58 d4          	vaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 58 39          	vaddps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d0 d4          	vaddsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d0 39          	vaddsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb d0 d4          	vaddsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb d0 39          	vaddsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 55 d4          	vandnpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 55 39          	vandnpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 55 d4          	vandnps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 55 39          	vandnps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 54 d4          	vandpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 54 39          	vandpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 54 d4          	vandps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 54 39          	vandps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5e d4          	vdivpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5e 39          	vdivpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5e d4          	vdivps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5e 39          	vdivps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 7c d4          	vhaddpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 7c 39          	vhaddpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 7c d4          	vhaddps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 7c 39          	vhaddps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 7d d4          	vhsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 7d 39          	vhsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 7d d4          	vhsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 7d 39          	vhsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5f d4          	vmaxpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5f 39          	vmaxpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5f d4          	vmaxps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5f 39          	vmaxps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5d d4          	vminpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5d 39          	vminpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5d d4          	vminps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5d 39          	vminps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 59 d4          	vmulpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 59 39          	vmulpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 59 d4          	vmulps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 59 39          	vmulps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 56 d4          	vorpd  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 56 39          	vorpd  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 56 d4          	vorps  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 56 39          	vorps  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 63 d4          	vpacksswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 63 39          	vpacksswb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6b d4          	vpackssdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6b 39          	vpackssdw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 67 d4          	vpackuswb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 67 39          	vpackuswb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 2b d4       	vpackusdw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 2b 39       	vpackusdw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fc d4          	vpaddb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fc 39          	vpaddb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fd d4          	vpaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fd 39          	vpaddw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fe d4          	vpaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fe 39          	vpaddd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d4 d4          	vpaddq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d4 39          	vpaddq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ec d4          	vpaddsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ec 39          	vpaddsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ed d4          	vpaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ed 39          	vpaddsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 dc d4          	vpaddusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 dc 39          	vpaddusb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 dd d4          	vpaddusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 dd 39          	vpaddusw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 db d4          	vpand  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 db 39          	vpand  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 df d4          	vpandn xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 df 39          	vpandn xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e0 d4          	vpavgb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e0 39          	vpavgb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e3 d4          	vpavgw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e3 39          	vpavgw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 00    	vpclmullqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 00    	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 01    	vpclmulhqlqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 01    	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 10    	vpclmullqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 10    	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 11    	vpclmulhqhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 44 39 11    	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 74 d4          	vpcmpeqb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 74 39          	vpcmpeqb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 75 d4          	vpcmpeqw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 75 39          	vpcmpeqw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 76 d4          	vpcmpeqd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 76 39          	vpcmpeqd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 29 d4       	vpcmpeqq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 29 39       	vpcmpeqq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 64 d4          	vpcmpgtb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 64 39          	vpcmpgtb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 65 d4          	vpcmpgtw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 65 39          	vpcmpgtw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 66 d4          	vpcmpgtd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 66 39          	vpcmpgtd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 37 d4       	vpcmpgtq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 37 39       	vpcmpgtq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0d d4       	vpermilpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0d 39       	vpermilpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0c d4       	vpermilps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0c 39       	vpermilps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 01 d4       	vphaddw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 01 39       	vphaddw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 02 d4       	vphaddd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 02 39       	vphaddd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 03 d4       	vphaddsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 03 39       	vphaddsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 05 d4       	vphsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 05 39       	vphsubw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 06 d4       	vphsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 06 39       	vphsubd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 07 d4       	vphsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 07 39       	vphsubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f5 d4          	vpmaddwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f5 39          	vpmaddwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 04 d4       	vpmaddubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 04 39       	vpmaddubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3c d4       	vpmaxsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3c 39       	vpmaxsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ee d4          	vpmaxsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ee 39          	vpmaxsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3d d4       	vpmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3d 39       	vpmaxsd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 de d4          	vpmaxub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 de 39          	vpmaxub xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3e d4       	vpmaxuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3e 39       	vpmaxuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3f d4       	vpmaxud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3f 39       	vpmaxud xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 38 d4       	vpminsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 38 39       	vpminsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ea d4          	vpminsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ea 39          	vpminsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 39 d4       	vpminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 39 39       	vpminsd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 da d4          	vpminub xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 da 39          	vpminub xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3a d4       	vpminuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3a 39       	vpminuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3b d4       	vpminud xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 3b 39       	vpminud xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e4 d4          	vpmulhuw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e4 39          	vpmulhuw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0b d4       	vpmulhrsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0b 39       	vpmulhrsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e5 d4          	vpmulhw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e5 39          	vpmulhw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d5 d4          	vpmullw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d5 39          	vpmullw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 40 d4       	vpmulld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 40 39       	vpmulld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f4 d4          	vpmuludq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f4 39          	vpmuludq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 28 d4       	vpmuldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 28 39       	vpmuldq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 eb d4          	vpor   xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 eb 39          	vpor   xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f6 d4          	vpsadbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f6 39          	vpsadbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 00 d4       	vpshufb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 00 39       	vpshufb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 08 d4       	vpsignb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 08 39       	vpsignb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 09 d4       	vpsignw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 09 39       	vpsignw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0a d4       	vpsignd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 0a 39       	vpsignd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f1 d4          	vpsllw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f1 39          	vpsllw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f2 d4          	vpslld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f2 39          	vpslld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f3 d4          	vpsllq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f3 39          	vpsllq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e1 d4          	vpsraw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e1 39          	vpsraw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e2 d4          	vpsrad xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e2 39          	vpsrad xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d1 d4          	vpsrlw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d1 39          	vpsrlw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d2 d4          	vpsrld xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d2 39          	vpsrld xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d3 d4          	vpsrlq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d3 39          	vpsrlq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f8 d4          	vpsubb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f8 39          	vpsubb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f9 d4          	vpsubw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 f9 39          	vpsubw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fa d4          	vpsubd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fa 39          	vpsubd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fb d4          	vpsubq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 fb 39          	vpsubq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e8 d4          	vpsubsb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e8 39          	vpsubsb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e9 d4          	vpsubsw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 e9 39          	vpsubsw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d8 d4          	vpsubusb xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d8 39          	vpsubusb xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d9 d4          	vpsubusw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 d9 39          	vpsubusw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 68 d4          	vpunpckhbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 68 39          	vpunpckhbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 69 d4          	vpunpckhwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 69 39          	vpunpckhwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6a d4          	vpunpckhdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6a 39          	vpunpckhdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6d d4          	vpunpckhqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6d 39          	vpunpckhqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 60 d4          	vpunpcklbw xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 60 39          	vpunpcklbw xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 61 d4          	vpunpcklwd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 61 39          	vpunpcklwd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 62 d4          	vpunpckldq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 62 39          	vpunpckldq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6c d4          	vpunpcklqdq xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 6c 39          	vpunpcklqdq xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ef d4          	vpxor  xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 ef 39          	vpxor  xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5c d4          	vsubpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 5c 39          	vsubpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5c d4          	vsubps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 5c 39          	vsubps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 15 d4          	vunpckhpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 15 39          	vunpckhpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 15 d4          	vunpckhps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 15 39          	vunpckhps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 14 d4          	vunpcklpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 14 39          	vunpcklpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 14 d4          	vunpcklps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 14 39          	vunpcklps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 57 d4          	vxorpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 57 39          	vxorpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 57 d4          	vxorps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 57 39          	vxorps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dc d4       	vaesenc xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dc 39       	vaesenc xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dd d4       	vaesenclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 dd 39       	vaesenclast xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 de d4       	vaesdec xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 de 39       	vaesdec xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 df d4       	vaesdeclast xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 df 39       	vaesdeclast xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 00       	vcmpeqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 00       	vcmpeqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 01       	vcmpltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 01       	vcmpltpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 02       	vcmplepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 02       	vcmplepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 03       	vcmpunordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 03       	vcmpunordpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 04       	vcmpneqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 04       	vcmpneqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 05       	vcmpnltpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 05       	vcmpnltpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 06       	vcmpnlepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 06       	vcmpnlepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 07       	vcmpordpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 08       	vcmpeq_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 08       	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 09       	vcmpngepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 09       	vcmpngepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0a       	vcmpngtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0a       	vcmpngtpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0b       	vcmpfalsepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0b       	vcmpfalsepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0c       	vcmpneq_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0c       	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0d       	vcmpgepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0d       	vcmpgepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0e       	vcmpgtpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0e       	vcmpgtpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 0f       	vcmptruepd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 0f       	vcmptruepd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 10       	vcmpeq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 10       	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 11       	vcmplt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 11       	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 12       	vcmple_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 12       	vcmple_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 13       	vcmpunord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 13       	vcmpunord_spd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 14       	vcmpneq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 14       	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 15       	vcmpnlt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 15       	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 16       	vcmpnle_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 16       	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 17       	vcmpord_spd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 17       	vcmpord_spd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 18       	vcmpeq_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 18       	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 19       	vcmpnge_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 19       	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1a       	vcmpngt_uqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1a       	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1b       	vcmpfalse_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1b       	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1c       	vcmpneq_ospd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1c       	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1d       	vcmpge_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1d       	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1e       	vcmpgt_oqpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1e       	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 1f       	vcmptrue_uspd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 39 1f       	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 00       	vcmpeqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 00       	vcmpeqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 01       	vcmpltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 01       	vcmpltps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 02       	vcmpleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 02       	vcmpleps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 03       	vcmpunordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 03       	vcmpunordps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 04       	vcmpneqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 04       	vcmpneqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 05       	vcmpnltps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 05       	vcmpnltps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 06       	vcmpnleps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 06       	vcmpnleps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 07       	vcmpordps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 08       	vcmpeq_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 08       	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 09       	vcmpngeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 09       	vcmpngeps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0a       	vcmpngtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0a       	vcmpngtps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0b       	vcmpfalseps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0b       	vcmpfalseps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0c       	vcmpneq_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0c       	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0d       	vcmpgeps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0d       	vcmpgeps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0e       	vcmpgtps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0e       	vcmpgtps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 0f       	vcmptrueps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 0f       	vcmptrueps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 10       	vcmpeq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 10       	vcmpeq_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 11       	vcmplt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 11       	vcmplt_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 12       	vcmple_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 12       	vcmple_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 13       	vcmpunord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 13       	vcmpunord_sps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 14       	vcmpneq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 14       	vcmpneq_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 15       	vcmpnlt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 15       	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 16       	vcmpnle_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 16       	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 17       	vcmpord_sps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 17       	vcmpord_sps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 18       	vcmpeq_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 18       	vcmpeq_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 19       	vcmpnge_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 19       	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1a       	vcmpngt_uqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1a       	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1b       	vcmpfalse_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1b       	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1c       	vcmpneq_osps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1c       	vcmpneq_osps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1d       	vcmpge_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1d       	vcmpge_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1e       	vcmpgt_oqps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1e       	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 1f       	vcmptrue_usps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 39 1f       	vcmptrue_usps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf f4       	vgf2p8mulb xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf 31       	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 a2 51 cf b4 f0 c0 1d fe ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 07 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 08 00 00 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 00 f8 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:	c4 e2 51 cf b2 f0 f7 ff ff 	vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps xmm6,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2c 31       	vmaskmovps xmm6,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd xmm6,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 59 2d 31       	vmaskmovpd xmm6,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 07    	vaeskeygenassist xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 07    	vaeskeygenassist xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 07    	vpcmpestri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 07    	vpcmpestri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 07    	vpcmpestrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 07    	vpcmpestrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 07    	vpcmpistri xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 07    	vpcmpistri xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 07    	vpcmpistrm xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 07    	vpcmpistrm xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 f4 07    	vpermilpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 05 31 07    	vpermilpd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 f4 07    	vpermilps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 04 31 07    	vpermilps xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 70 f4 07       	vpshufd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 70 31 07       	vpshufd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 fa 70 f4 07       	vpshufhw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 fa 70 31 07       	vpshufhw xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 fb 70 f4 07       	vpshuflw xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 fb 70 31 07       	vpshuflw xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 07    	vroundpd xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 07    	vroundpd xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 07    	vroundps xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 07    	vroundps xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps XMMWORD PTR \[rcx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2e 21       	vmaskmovps XMMWORD PTR \[rcx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd XMMWORD PTR \[rcx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 2f 21       	vmaskmovpd XMMWORD PTR \[rcx\],xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 49 0d d4 07    	vblendpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0d 11 07    	vblendpd xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c d4 07    	vblendps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0c 11 07    	vblendps xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c2 d4 07       	vcmpordpd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd xmm2,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c9 c2 11 07       	vcmpordpd xmm2,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 d4 07       	vcmpordps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps xmm2,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 c8 c2 11 07       	vcmpordps xmm2,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 41 d4 07    	vdppd  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 41 11 07    	vdppd  xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 d4 07    	vdpps  xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 40 11 07    	vdpps  xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 d4 07    	vmpsadbw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 42 11 07    	vmpsadbw xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f d4 07    	vpalignr xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0f 11 07    	vpalignr xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e d4 07    	vpblendw xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0e 11 07    	vpblendw xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 d4 07    	vpclmulqdq xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 44 11 07    	vpclmulqdq xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 d4 07       	vshufpd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c9 c6 11 07       	vshufpd xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 d4 07       	vshufps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 c8 c6 11 07       	vshufps xmm2,xmm6,XMMWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 ab    	vgf2p8affineqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 ce f4 7b    	vgf2p8affineqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce 31 7b    	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d1 ce b4 f0 c0 1d fe ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 07 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 08 00 00 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 00 f8 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 ce b2 f0 f7 ff ff 7b 	vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 ab    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:	c4 e3 d1 cf f4 7b    	vgf2p8affineinvqb xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf 31 7b    	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:	c4 a3 d1 cf b4 f0 c0 1d fe ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 07 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 08 00 00 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 00 f8 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 d1 cf b2 f0 f7 ff ff 7b 	vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:	c4 e3 69 4b fe 40    	vblendvpd xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4b 39 40    	vblendvpd xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a fe 40    	vblendvps xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4a 39 40    	vblendvps xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c fe 40    	vpblendvb xmm7,xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e3 69 4c 39 40    	vpblendvb xmm7,xmm2,XMMWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd ymm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 19 21       	vbroadcastsd ymm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  xmm4,rcx
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 f9 7e e1       	vmovq  rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 f9 6e e1       	vmovq  xmm4,rcx
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  QWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si ecx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si ecx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si ecx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si ecx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fb 2d cc       	vcvtsd2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fb 2d 09       	vcvtsd2si rcx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fb 2d 09       	vcvtsd2si rcx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fb 2c cc       	vcvttsd2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fb 2c 09       	vcvttsd2si rcx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fb 2c 09       	vcvttsd2si rcx,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 db 2a f1       	vcvtsi2sd xmm6,xmm4,rcx
[ 	]*[a-f0-9]+:	c4 e1 db 2a 31       	vcvtsi2sd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 db 2a 31       	vcvtsi2sd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 da 2a f1       	vcvtsi2ss xmm6,xmm4,rcx
[ 	]*[a-f0-9]+:	c4 e1 da 2a 31       	vcvtsi2ss xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 da 2a 31       	vcvtsi2ss xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 d9 22 f1 07    	vpinsrq xmm6,xmm4,rcx,0x7
[ 	]*[a-f0-9]+:	c4 e3 d9 22 31 07    	vpinsrq xmm6,xmm4,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 d9 22 31 07    	vpinsrq xmm6,xmm4,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 16 e1 07    	vpextrq rcx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 07    	vpextrq QWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 07    	vpextrq QWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d9 12 31          	vmovlpd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d8 12 31          	vmovlps xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d9 16 31          	vmovhpd xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 d8 16 31          	vmovhps xmm6,xmm4,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 0b d4 07    	vroundsd xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0b 11 07    	vroundsd xmm2,xmm6,QWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 cb 58 d4          	vaddsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 58 11          	vaddsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5a d4          	vcvtsd2ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5a 11          	vcvtsd2ss xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5e d4          	vdivsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5e 11          	vdivsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5f d4          	vmaxsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5f 11          	vmaxsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5d d4          	vminsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5d 11          	vminsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 59 d4          	vmulsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 59 11          	vmulsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 51 d4          	vsqrtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 51 11          	vsqrtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5c d4          	vsubsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb 5c 11          	vsubsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 00       	vcmpeqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 00       	vcmpeqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 01       	vcmpltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 01       	vcmpltsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 02       	vcmplesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 02       	vcmplesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 03       	vcmpunordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 03       	vcmpunordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 04       	vcmpneqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 04       	vcmpneqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 05       	vcmpnltsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 05       	vcmpnltsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 06       	vcmpnlesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 06       	vcmpnlesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 07       	vcmpordsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 07       	vcmpordsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 08       	vcmpeq_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 08       	vcmpeq_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 09       	vcmpngesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 09       	vcmpngesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0a       	vcmpngtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0a       	vcmpngtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0b       	vcmpfalsesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0b       	vcmpfalsesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0c       	vcmpneq_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0c       	vcmpneq_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0d       	vcmpgesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0d       	vcmpgesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0e       	vcmpgtsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0e       	vcmpgtsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 0f       	vcmptruesd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 0f       	vcmptruesd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 10       	vcmpeq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 10       	vcmpeq_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 11       	vcmplt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 11       	vcmplt_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 12       	vcmple_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 12       	vcmple_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 13       	vcmpunord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 13       	vcmpunord_ssd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 14       	vcmpneq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 14       	vcmpneq_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 15       	vcmpnlt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 15       	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 16       	vcmpnle_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 16       	vcmpnle_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 17       	vcmpord_ssd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 17       	vcmpord_ssd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 18       	vcmpeq_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 18       	vcmpeq_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 19       	vcmpnge_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 19       	vcmpnge_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1a       	vcmpngt_uqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1a       	vcmpngt_uqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1b       	vcmpfalse_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1b       	vcmpfalse_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1c       	vcmpneq_ossd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1c       	vcmpneq_ossd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1d       	vcmpge_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1d       	vcmpge_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1e       	vcmpgt_oqsd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1e       	vcmpgt_oqsd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 d4 1f       	vcmptrue_ussd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 cb c2 11 1f       	vcmptrue_ussd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 58 d4          	vaddss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 58 11          	vaddss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5a d4          	vcvtss2sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5a 11          	vcvtss2sd xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5e d4          	vdivss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5e 11          	vdivss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5f d4          	vmaxss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5f 11          	vmaxss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5d d4          	vminss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5d 11          	vminss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 59 d4          	vmulss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 59 11          	vmulss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 53 d4          	vrcpss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 53 11          	vrcpss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 52 d4          	vrsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 52 11          	vrsqrtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 51 d4          	vsqrtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 51 11          	vsqrtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5c d4          	vsubss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca 5c 11          	vsubss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 00       	vcmpeqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 00       	vcmpeqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 01       	vcmpltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 01       	vcmpltss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 02       	vcmpless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 02       	vcmpless xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 03       	vcmpunordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 03       	vcmpunordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 04       	vcmpneqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 04       	vcmpneqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 05       	vcmpnltss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 05       	vcmpnltss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 06       	vcmpnless xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 06       	vcmpnless xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 08       	vcmpeq_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 08       	vcmpeq_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 09       	vcmpngess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 09       	vcmpngess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0a       	vcmpngtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0a       	vcmpngtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0b       	vcmpfalsess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0b       	vcmpfalsess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0c       	vcmpneq_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0c       	vcmpneq_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0d       	vcmpgess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0d       	vcmpgess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0e       	vcmpgtss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0e       	vcmpgtss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 0f       	vcmptruess xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 0f       	vcmptruess xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 10       	vcmpeq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 10       	vcmpeq_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 11       	vcmplt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 11       	vcmplt_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 12       	vcmple_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 12       	vcmple_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 13       	vcmpunord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 13       	vcmpunord_sss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 14       	vcmpneq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 14       	vcmpneq_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 15       	vcmpnlt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 15       	vcmpnlt_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 16       	vcmpnle_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 16       	vcmpnle_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 17       	vcmpord_sss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 17       	vcmpord_sss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 18       	vcmpeq_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 18       	vcmpeq_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 19       	vcmpnge_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 19       	vcmpnge_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1a       	vcmpngt_uqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1a       	vcmpngt_uqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1b       	vcmpfalse_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1b       	vcmpfalse_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1c       	vcmpneq_osss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1c       	vcmpneq_osss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1d       	vcmpge_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1d       	vcmpge_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1e       	vcmpgt_oqss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1e       	vcmpgt_oqss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 1f       	vcmptrue_usss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 1f       	vcmptrue_usss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss ymm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 18 21       	vbroadcastss ymm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 18 21       	vbroadcastss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  xmm4,ecx
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  DWORD PTR \[rcx\],xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si ecx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si ecx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si ecx,xmm4
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si ecx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si ecx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fa 2d cc       	vcvtss2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fa 2d 09       	vcvtss2si rcx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fa 2d 09       	vcvtss2si rcx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fa 2c cc       	vcvttss2si rcx,xmm4
[ 	]*[a-f0-9]+:	c4 e1 fa 2c 09       	vcvttss2si rcx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e1 fa 2c 09       	vcvttss2si rcx,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps ecx,xmm4
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb ecx,xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 07    	vpextrd ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 07    	vpextrd DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 07    	vextractps ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 07    	vextractps DWORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 f1 07    	vpinsrd xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd xmm6,xmm4,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 22 31 07    	vpinsrd xmm6,xmm4,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 db 2a f1          	vcvtsi2sd xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 db 2a 31          	vcvtsi2sd xmm6,xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 da 2a f1          	vcvtsi2ss xmm6,xmm4,ecx
[ 	]*[a-f0-9]+:	c5 da 2a 31          	vcvtsi2ss xmm6,xmm4,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 d4 07       	vcmpordss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 ca c2 11 07       	vcmpordss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e3 49 21 d4 07    	vinsertps xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps xmm2,xmm6,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 21 11 07    	vinsertps xmm2,xmm6,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a d4 07    	vroundss xmm2,xmm6,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 49 0a 11 07    	vroundss xmm2,xmm6,DWORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq xmm4,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq xmm4,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq xmm4,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq xmm4,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 07    	vpextrw WORD PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 f1 07       	vpinsrw xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 d9 c4 31 07       	vpinsrw xmm6,xmm4,WORD PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 f1 07    	vpinsrb xmm6,xmm4,ecx,0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb xmm6,xmm4,BYTE PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 59 20 31 07    	vpinsrb xmm6,xmm4,BYTE PTR \[rcx\],0x7
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 07       	vpextrw ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 07    	vpextrb ecx,xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[rcx\],xmm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 07    	vpextrb BYTE PTR \[rcx\],xmm4,0x7
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
[ 	]*[a-f0-9]+:	c5 fd 50 cc          	vmovmskpd ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fc 50 cc          	vmovmskps ecx,ymm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 ff e6 f4          	vcvtpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd 5a f4          	vcvtpd2ps xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq xmm6,xmm4
[ 	]*[a-f0-9]+:	c5 fd e6 f4          	vcvttpd2dq xmm6,ymm4
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 78 56 34 12 	vldmxcsr DWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 78 56 34 12 	vmovdqa xmm8,XMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 78 56 34 12 	vmovdqa XMMWORD PTR ds:0x12345678,xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 78 56 34 12 	vmovd  DWORD PTR ds:0x12345678,xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 78 56 34 12 	vcvtsd2si r8d,QWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 78 56 34 12 	vcvtdq2pd ymm8,XMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 78 56 34 12 	vcvtpd2ps xmm8,YMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 78 56 34 12 	vpavgb xmm15,xmm8,XMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 78 56 34 12 07 	vaeskeygenassist xmm8,XMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 78 56 34 12 07 	vpextrb BYTE PTR ds:0x12345678,xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 78 56 34 12 	vcvtsi2sd xmm15,xmm8,DWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 78 56 34 12 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 78 56 34 12 80 	vblendvps xmm14,xmm12,XMMWORD PTR ds:0x12345678,xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 78 56 34 12 07 	vpinsrb xmm15,xmm8,BYTE PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 78 56 34 12 	vmovdqa ymm8,YMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 78 56 34 12 	vmovdqa YMMWORD PTR ds:0x12345678,ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 78 56 34 12 	vpermilpd ymm15,ymm8,YMMWORD PTR ds:0x12345678
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 78 56 34 12 07 	vroundpd ymm8,YMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 78 56 34 12 07 	vextractf128 XMMWORD PTR ds:0x12345678,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 78 56 34 12 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR ds:0x12345678,0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 78 56 34 12 80 	vblendvpd ymm14,ymm12,YMMWORD PTR ds:0x12345678,ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 55 00       	vldmxcsr DWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 79 6f 45 00       	vmovdqa xmm8,XMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 79 7f 45 00       	vmovdqa XMMWORD PTR \[rbp\+0x0\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 45 00       	vmovd  DWORD PTR \[rbp\+0x0\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 45 00       	vcvtsd2si r8d,QWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 7e e6 45 00       	vcvtdq2pd ymm8,XMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 7d 5a 45 00       	vcvtpd2ps xmm8,YMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 39 e0 7d 00       	vpavgb xmm15,xmm8,XMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c4 63 79 df 45 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 45 00 07 	vpextrb BYTE PTR \[rbp\+0x0\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 7d 00       	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c4 63 39 44 7d 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 75 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbp\+0x0\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 7d 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 45 00       	vmovdqa ymm8,YMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c5 7d 7f 45 00       	vmovdqa YMMWORD PTR \[rbp\+0x0\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 7d 00    	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbp\+0x0\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 45 00 07 	vroundpd ymm8,YMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 45 00 07 	vextractf128 XMMWORD PTR \[rbp\+0x0\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 7d 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbp\+0x0\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 75 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbp\+0x0\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 95 99 00 00 00 	vldmxcsr DWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 85 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 85 99 00 00 00 	vmovdqa XMMWORD PTR \[rbp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 85 99 00 00 00 	vmovd  DWORD PTR \[rbp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 85 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 85 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 85 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bd 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 85 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 85 99 00 00 00 07 	vpextrb BYTE PTR \[rbp\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bd 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bd 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b5 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bd 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 85 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 85 99 00 00 00 	vmovdqa YMMWORD PTR \[rbp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bd 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 85 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 85 99 00 00 00 07 	vextractf128 XMMWORD PTR \[rbp\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bd 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b5 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 c1 78 ae 97 99 00 00 00 	vldmxcsr DWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 6f 87 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 7f 87 99 00 00 00 	vmovdqa XMMWORD PTR \[r15\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7e 87 99 00 00 00 	vmovd  DWORD PTR \[r15\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 7b 2d 87 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7e e6 87 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 5a 87 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 39 e0 bf 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 79 df 87 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 79 14 87 99 00 00 00 07 	vpextrb BYTE PTR \[r15\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 3b 2a bf 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 39 44 bf 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 19 4a b7 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r15\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 43 39 20 bf 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 41 7d 6f 87 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 7f 87 99 00 00 00 	vmovdqa YMMWORD PTR \[r15\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 42 3d 0d bf 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r15\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 7d 09 87 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 7d 19 87 99 00 00 00 07 	vextractf128 XMMWORD PTR \[r15\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 3d 06 bf 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r15\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 1d 4b b7 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r15\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 15 99 00 00 00 	vldmxcsr DWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 6f 05 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7f 05 99 00 00 00 	vmovdqa XMMWORD PTR \[rip\+0x99\],xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 79 7e 05 99 00 00 00 	vmovd  DWORD PTR \[rip\+0x99\],xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7b 2d 05 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7e e6 05 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 5a 05 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 39 e0 3d 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 df 05 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 79 14 05 99 00 00 00 07 	vpextrb BYTE PTR \[rip\+0x99\],xmm8,0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 3b 2a 3d 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 44 3d 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 19 4a 35 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rip\+0x99\],xmm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 39 20 3d 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 6f 05 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 7d 7f 05 99 00 00 00 	vmovdqa YMMWORD PTR \[rip\+0x99\],ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3d 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rip\+0x99\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 09 05 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 7d 19 05 99 00 00 00 07 	vextractf128 XMMWORD PTR \[rip\+0x99\],ymm8,0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 3d 06 3d 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rip\+0x99\],0x7        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c4 63 1d 4b 35 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rip\+0x99\],ymm8        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	c5 f8 ae 94 24 99 00 00 00 	vldmxcsr DWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 84 24 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 84 24 99 00 00 00 	vmovdqa XMMWORD PTR \[rsp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 84 24 99 00 00 00 	vmovd  DWORD PTR \[rsp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 84 24 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 84 24 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 84 24 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bc 24 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 84 24 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 84 24 99 00 00 00 07 	vpextrb BYTE PTR \[rsp\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bc 24 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 24 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 24 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rsp\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 24 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 84 24 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 84 24 99 00 00 00 	vmovdqa YMMWORD PTR \[rsp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 24 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rsp\+0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 24 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 24 99 00 00 00 07 	vextractf128 XMMWORD PTR \[rsp\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 24 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rsp\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 24 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rsp\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 c1 78 ae 94 24 99 00 00 00 	vldmxcsr DWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 6f 84 24 99 00 00 00 	vmovdqa xmm8,XMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 79 7f 84 24 99 00 00 00 	vmovdqa XMMWORD PTR \[r12\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7e 84 24 99 00 00 00 	vmovd  DWORD PTR \[r12\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 41 7b 2d 84 24 99 00 00 00 	vcvtsd2si r8d,QWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7e e6 84 24 99 00 00 00 	vcvtdq2pd ymm8,XMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 5a 84 24 99 00 00 00 	vcvtpd2ps xmm8,YMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 39 e0 bc 24 99 00 00 00 	vpavgb xmm15,xmm8,XMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 79 df 84 24 99 00 00 00 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 79 14 84 24 99 00 00 00 07 	vpextrb BYTE PTR \[r12\+0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 3b 2a bc 24 99 00 00 00 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 39 44 bc 24 99 00 00 00 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 19 4a b4 24 99 00 00 00 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r12\+0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 43 39 20 bc 24 99 00 00 00 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 41 7d 6f 84 24 99 00 00 00 	vmovdqa ymm8,YMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 41 7d 7f 84 24 99 00 00 00 	vmovdqa YMMWORD PTR \[r12\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 42 3d 0d bc 24 99 00 00 00 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r12\+0x99\]
[ 	]*[a-f0-9]+:	c4 43 7d 09 84 24 99 00 00 00 07 	vroundpd ymm8,YMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 7d 19 84 24 99 00 00 00 07 	vextractf128 XMMWORD PTR \[r12\+0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 3d 06 bc 24 99 00 00 00 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r12\+0x99\],0x7
[ 	]*[a-f0-9]+:	c4 43 1d 4b b4 24 99 00 00 00 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r12\+0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 14 25 67 ff ff ff 	vldmxcsr DWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 6f 04 25 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 79 7f 04 25 67 ff ff ff 	vmovdqa XMMWORD PTR ds:0xffffffffffffff67,xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 25 67 ff ff ff 	vmovd  DWORD PTR ds:0xffffffffffffff67,xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 25 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7e e6 04 25 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7d 5a 04 25 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 39 e0 3c 25 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 79 df 04 25 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 25 67 ff ff ff 07 	vpextrb BYTE PTR ds:0xffffffffffffff67,xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 25 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 25 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 25 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR ds:0xffffffffffffff67,xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 25 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 25 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c5 7d 7f 04 25 67 ff ff ff 	vmovdqa YMMWORD PTR ds:0xffffffffffffff67,ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 25 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR ds:0xffffffffffffff67
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 25 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 25 67 ff ff ff 07 	vextractf128 XMMWORD PTR ds:0xffffffffffffff67,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 25 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR ds:0xffffffffffffff67,0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 25 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR ds:0xffffffffffffff67,ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 14 65 67 ff ff ff 	vldmxcsr DWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 04 65 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 04 65 67 ff ff ff 	vmovdqa XMMWORD PTR \[riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 04 65 67 ff ff ff 	vmovd  DWORD PTR \[riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 04 65 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 04 65 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 04 65 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 3c 65 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 04 65 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 04 65 67 ff ff ff 07 	vpextrb BYTE PTR \[riz\*2-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a 3c 65 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 3c 65 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a 34 65 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 3c 65 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 04 65 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 04 65 67 ff ff ff 	vmovdqa YMMWORD PTR \[riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d 3c 65 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 04 65 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 04 65 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[riz\*2-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 3c 65 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b 34 65 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 94 23 67 ff ff ff 	vldmxcsr DWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 84 23 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 84 23 67 ff ff ff 	vmovdqa XMMWORD PTR \[rbx\+riz\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 84 23 67 ff ff ff 	vmovd  DWORD PTR \[rbx\+riz\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 84 23 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 84 23 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 84 23 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bc 23 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 84 23 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 84 23 67 ff ff ff 07 	vpextrb BYTE PTR \[rbx\+riz\*1-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bc 23 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 23 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 23 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbx\+riz\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 23 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 84 23 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 84 23 67 ff ff ff 	vmovdqa YMMWORD PTR \[rbx\+riz\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 23 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 23 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 23 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rbx\+riz\*1-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 23 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 23 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbx\+riz\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c5 f8 ae 94 63 67 ff ff ff 	vldmxcsr DWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 6f 84 63 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 79 7f 84 63 67 ff ff ff 	vmovdqa XMMWORD PTR \[rbx\+riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 79 7e 84 63 67 ff ff ff 	vmovd  DWORD PTR \[rbx\+riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c5 7b 2d 84 63 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7e e6 84 63 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 5a 84 63 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 39 e0 bc 63 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 79 df 84 63 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 79 14 84 63 67 ff ff ff 07 	vpextrb BYTE PTR \[rbx\+riz\*2-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c5 3b 2a bc 63 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 39 44 bc 63 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 19 4a b4 63 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbx\+riz\*2-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 63 39 20 bc 63 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c5 7d 6f 84 63 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c5 7d 7f 84 63 67 ff ff ff 	vmovdqa YMMWORD PTR \[rbx\+riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 62 3d 0d bc 63 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\]
[ 	]*[a-f0-9]+:	c4 63 7d 09 84 63 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 7d 19 84 63 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rbx\+riz\*2-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 63 3d 06 bc 63 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbx\+riz\*2-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 63 1d 4b b4 63 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbx\+riz\*2-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 bc 67 ff ff ff 	vldmxcsr DWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 bc 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 bc 67 ff ff ff 	vmovdqa XMMWORD PTR \[r12\+r15\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 bc 67 ff ff ff 	vmovd  DWORD PTR \[r12\+r15\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 bc 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 bc 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 bc 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc bc 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 03 79 df 84 bc 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 79 14 84 bc 67 ff ff ff 07 	vpextrb BYTE PTR \[r12\+r15\*4-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc bc 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 03 39 44 bc bc 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 bc 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r12\+r15\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 03 39 20 bc bc 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 bc 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 bc 67 ff ff ff 	vmovdqa YMMWORD PTR \[r12\+r15\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc bc 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 bc 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 bc 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[r12\+r15\*4-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc bc 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r12\+r15\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 bc 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r12\+r15\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 81 78 ae 94 f8 67 ff ff ff 	vldmxcsr DWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 6f 84 f8 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 79 7f 84 f8 67 ff ff ff 	vmovdqa XMMWORD PTR \[r8\+r15\*8-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 79 7e 84 f8 67 ff ff ff 	vmovd  DWORD PTR \[r8\+r15\*8-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 01 7b 2d 84 f8 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7e e6 84 f8 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 5a 84 f8 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 39 e0 bc f8 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 03 79 df 84 f8 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 79 14 84 f8 67 ff ff ff 07 	vpextrb BYTE PTR \[r8\+r15\*8-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 01 3b 2a bc f8 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 03 39 44 bc f8 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 19 4a b4 f8 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[r8\+r15\*8-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 03 39 20 bc f8 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 01 7d 6f 84 f8 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 01 7d 7f 84 f8 67 ff ff ff 	vmovdqa YMMWORD PTR \[r8\+r15\*8-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 02 3d 0d bc f8 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\]
[ 	]*[a-f0-9]+:	c4 03 7d 09 84 f8 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 7d 19 84 f8 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[r8\+r15\*8-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 03 3d 06 bc f8 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[r8\+r15\*8-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 03 1d 4b b4 f8 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[r8\+r15\*8-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 a5 67 ff ff ff 	vldmxcsr DWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 a5 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 a5 67 ff ff ff 	vmovdqa XMMWORD PTR \[rbp\+r12\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 a5 67 ff ff ff 	vmovd  DWORD PTR \[rbp\+r12\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 a5 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 a5 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 a5 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc a5 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 23 79 df 84 a5 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rbp\+r12\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 79 14 84 a5 67 ff ff ff 07 	vpextrb BYTE PTR \[rbp\+r12\*4-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc a5 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 23 39 44 bc a5 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rbp\+r12\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 a5 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rbp\+r12\*4-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 23 39 20 bc a5 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rbp\+r12\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 a5 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 a5 67 ff ff ff 	vmovdqa YMMWORD PTR \[rbp\+r12\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc a5 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rbp\+r12\*4-0x99\]
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 a5 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rbp\+r12\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 a5 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rbp\+r12\*4-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc a5 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rbp\+r12\*4-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 a5 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rbp\+r12\*4-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 a1 78 ae 94 2c 67 ff ff ff 	vldmxcsr DWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 6f 84 2c 67 ff ff ff 	vmovdqa xmm8,XMMWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 79 7f 84 2c 67 ff ff ff 	vmovdqa XMMWORD PTR \[rsp\+r13\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 79 7e 84 2c 67 ff ff ff 	vmovd  DWORD PTR \[rsp\+r13\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 21 7b 2d 84 2c 67 ff ff ff 	vcvtsd2si r8d,QWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7e e6 84 2c 67 ff ff ff 	vcvtdq2pd ymm8,XMMWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 5a 84 2c 67 ff ff ff 	vcvtpd2ps xmm8,YMMWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 39 e0 bc 2c 67 ff ff ff 	vpavgb xmm15,xmm8,XMMWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 23 79 df 84 2c 67 ff ff ff 07 	vaeskeygenassist xmm8,XMMWORD PTR \[rsp\+r13\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 79 14 84 2c 67 ff ff ff 07 	vpextrb BYTE PTR \[rsp\+r13\*1-0x99\],xmm8,0x7
[ 	]*[a-f0-9]+:	c4 21 3b 2a bc 2c 67 ff ff ff 	vcvtsi2sd xmm15,xmm8,DWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 23 39 44 bc 2c 67 ff ff ff 07 	vpclmulqdq xmm15,xmm8,XMMWORD PTR \[rsp\+r13\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 19 4a b4 2c 67 ff ff ff 80 	vblendvps xmm14,xmm12,XMMWORD PTR \[rsp\+r13\*1-0x99\],xmm8
[ 	]*[a-f0-9]+:	c4 23 39 20 bc 2c 67 ff ff ff 07 	vpinsrb xmm15,xmm8,BYTE PTR \[rsp\+r13\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 21 7d 6f 84 2c 67 ff ff ff 	vmovdqa ymm8,YMMWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 21 7d 7f 84 2c 67 ff ff ff 	vmovdqa YMMWORD PTR \[rsp\+r13\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 22 3d 0d bc 2c 67 ff ff ff 	vpermilpd ymm15,ymm8,YMMWORD PTR \[rsp\+r13\*1-0x99\]
[ 	]*[a-f0-9]+:	c4 23 7d 09 84 2c 67 ff ff ff 07 	vroundpd ymm8,YMMWORD PTR \[rsp\+r13\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 7d 19 84 2c 67 ff ff ff 07 	vextractf128 XMMWORD PTR \[rsp\+r13\*1-0x99\],ymm8,0x7
[ 	]*[a-f0-9]+:	c4 23 3d 06 bc 2c 67 ff ff ff 07 	vperm2f128 ymm15,ymm8,YMMWORD PTR \[rsp\+r13\*1-0x99\],0x7
[ 	]*[a-f0-9]+:	c4 23 1d 4b b4 2c 67 ff ff ff 80 	vblendvpd ymm14,ymm12,YMMWORD PTR \[rsp\+r13\*1-0x99\],ymm8
[ 	]*[a-f0-9]+:	c4 41 79 50 c0       	vmovmskpd r8d,xmm8
[ 	]*[a-f0-9]+:	c4 c1 01 72 f0 07    	vpslld xmm15,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 7c 50 c0       	vmovmskps r8d,ymm8
[ 	]*[a-f0-9]+:	c4 41 79 6f f8       	vmovdqa xmm15,xmm8
[ 	]*[a-f0-9]+:	c4 41 79 7e c0       	vmovd  r8d,xmm8
[ 	]*[a-f0-9]+:	c4 41 7b 2d c0       	vcvtsd2si r8d,xmm8
[ 	]*[a-f0-9]+:	c4 41 7e e6 c0       	vcvtdq2pd ymm8,xmm8
[ 	]*[a-f0-9]+:	c4 41 7d 5a c0       	vcvtpd2ps xmm8,ymm8
[ 	]*[a-f0-9]+:	c4 43 79 df f8 07    	vaeskeygenassist xmm15,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 43 79 14 c0 07    	vpextrb r8d,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 41 3b 2a f8       	vcvtsi2sd xmm15,xmm8,r8d
[ 	]*[a-f0-9]+:	c4 43 01 44 e0 07    	vpclmulqdq xmm12,xmm15,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 43 19 4a f0 80    	vblendvps xmm14,xmm12,xmm8,xmm8
[ 	]*[a-f0-9]+:	c4 43 39 20 f8 07    	vpinsrb xmm15,xmm8,r8d,0x7
[ 	]*[a-f0-9]+:	c4 41 7d 6f f8       	vmovdqa ymm15,ymm8
[ 	]*[a-f0-9]+:	c4 42 05 0d e0       	vpermilpd ymm12,ymm15,ymm8
[ 	]*[a-f0-9]+:	c4 43 7d 09 f8 07    	vroundpd ymm15,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 7d 19 c0 07    	vextractf128 xmm8,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 05 06 e0 07    	vperm2f128 ymm12,ymm15,ymm8,0x7
[ 	]*[a-f0-9]+:	c4 43 1d 4b f7 80    	vblendvpd ymm14,ymm12,ymm15,ymm8
[ 	]*[a-f0-9]+:	c4 43 3d 18 f8 07    	vinsertf128 ymm15,ymm8,xmm8,0x7
[ 	]*[a-f0-9]+:	c4 61 fb 2d 01       	vcvtsd2si r8,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 43 79 17 c0 0a    	vextractps r8d,xmm8,0xa
[ 	]*[a-f0-9]+:	c4 61 fa 2d 01       	vcvtss2si r8,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 41 01 c4 c0 07    	vpinsrw xmm8,xmm15,r8d,0x7
#pass
