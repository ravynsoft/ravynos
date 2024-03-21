#objdump: -dw
#name: x86-64 AVX2 insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 5d 8c 31       	vpmaskmovd \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 8e 21       	vpmaskmovd %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 dd 8c 31       	vpmaskmovq \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 cd 8e 21       	vpmaskmovq %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 fd 01 d6 07    	vpermpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 fd 01 31 07    	vpermpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 fd 00 d6 07    	vpermq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 fd 00 31 07    	vpermq \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 36 d4       	vpermd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 36 11       	vpermd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 16 d4       	vpermps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 16 11       	vpermps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 47 d4       	vpsllvd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 47 11       	vpsllvd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 47 d4       	vpsllvq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 47 11       	vpsllvq \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 46 d4       	vpsravd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 46 11       	vpsravd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 45 d4       	vpsrlvd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 45 11       	vpsrlvd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 45 d4       	vpsrlvq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 45 11       	vpsrlvq \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 7d 2a 21       	vmovntdqa \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 f4       	vbroadcastsd %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 18 f4       	vbroadcastss %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 4d 02 d4 07    	vpblendd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 02 11 07    	vpblendd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 46 d4 07    	vperm2i128 \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 46 11 07    	vperm2i128 \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 5d 38 f4 07    	vinserti128 \$0x7,%xmm4,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 5d 38 31 07    	vinserti128 \$0x7,\(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 5a 21       	vbroadcasti128 \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 49 47 d4       	vpsllvd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 47 39       	vpsllvd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 c9 47 d4       	vpsllvq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 c9 47 39       	vpsllvq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 46 d4       	vpsravd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 46 39       	vpsravd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 45 d4       	vpsrlvd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 45 39       	vpsrlvd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 c9 45 d4       	vpsrlvq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 c9 45 39       	vpsrlvq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 59 8c 31       	vpmaskmovd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 d9 8c 31       	vpmaskmovq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 7d 39 e6 07    	vextracti128 \$0x7,%ymm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 7d 39 21 07    	vextracti128 \$0x7,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 8e 21       	vpmaskmovd %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 c9 8e 21       	vpmaskmovq %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 49 02 d4 07    	vpblendd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 02 11 07    	vpblendd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 79 59 f4       	vpbroadcastq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 59 21       	vpbroadcastq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 59 f4       	vpbroadcastq %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 59 21       	vpbroadcastq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 e4       	vpbroadcastd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 21       	vpbroadcastd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 58 f4       	vpbroadcastd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 58 21       	vpbroadcastd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 79 f4       	vpbroadcastw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 79 21       	vpbroadcastw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 79 f4       	vpbroadcastw %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 79 21       	vpbroadcastw \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 78 f4       	vpbroadcastb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 78 21       	vpbroadcastb \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 78 f4       	vpbroadcastb %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 78 21       	vpbroadcastb \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 18 f4       	vbroadcastss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 5d 8c 31       	vpmaskmovd \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 8e 21       	vpmaskmovd %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 5d 8c 31       	vpmaskmovd \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 8e 21       	vpmaskmovd %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 dd 8c 31       	vpmaskmovq \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 cd 8e 21       	vpmaskmovq %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 dd 8c 31       	vpmaskmovq \(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 cd 8e 21       	vpmaskmovq %ymm4,%ymm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 fd 01 d6 07    	vpermpd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 fd 01 31 07    	vpermpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 fd 01 31 07    	vpermpd \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 fd 00 d6 07    	vpermq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 fd 00 31 07    	vpermq \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e3 fd 00 31 07    	vpermq \$0x7,\(%rcx\),%ymm6
[ 	]*[a-f0-9]+:	c4 e2 4d 36 d4       	vpermd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 36 11       	vpermd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 36 11       	vpermd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 16 d4       	vpermps %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 16 11       	vpermps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 16 11       	vpermps \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 47 d4       	vpsllvd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 47 11       	vpsllvd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 47 11       	vpsllvd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 47 d4       	vpsllvq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 47 11       	vpsllvq \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 47 11       	vpsllvq \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 46 d4       	vpsravd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 46 11       	vpsravd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 46 11       	vpsravd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 45 d4       	vpsrlvd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 45 11       	vpsrlvd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 45 11       	vpsrlvd \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 45 d4       	vpsrlvq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 45 11       	vpsrlvq \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 cd 45 11       	vpsrlvq \(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 7d 2a 21       	vmovntdqa \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 2a 21       	vmovntdqa \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 19 f4       	vbroadcastsd %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 18 f4       	vbroadcastss %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 4d 02 d4 07    	vpblendd \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 02 11 07    	vpblendd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 02 11 07    	vpblendd \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 46 d4 07    	vperm2i128 \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 46 11 07    	vperm2i128 \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 46 11 07    	vperm2i128 \$0x7,\(%rcx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 5d 38 f4 07    	vinserti128 \$0x7,%xmm4,%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 5d 38 31 07    	vinserti128 \$0x7,\(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e3 5d 38 31 07    	vinserti128 \$0x7,\(%rcx\),%ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 5a 21       	vbroadcasti128 \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 5a 21       	vbroadcasti128 \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 49 47 d4       	vpsllvd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 47 39       	vpsllvd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 47 39       	vpsllvd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 c9 47 d4       	vpsllvq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 c9 47 39       	vpsllvq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 c9 47 39       	vpsllvq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 46 d4       	vpsravd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 46 39       	vpsravd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 46 39       	vpsravd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 45 d4       	vpsrlvd %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 49 45 39       	vpsrlvd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 49 45 39       	vpsrlvd \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 c9 45 d4       	vpsrlvq %xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 c9 45 39       	vpsrlvq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 c9 45 39       	vpsrlvq \(%rcx\),%xmm6,%xmm7
[ 	]*[a-f0-9]+:	c4 e2 59 8c 31       	vpmaskmovd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 59 8c 31       	vpmaskmovd \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 d9 8c 31       	vpmaskmovq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 d9 8c 31       	vpmaskmovq \(%rcx\),%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 7d 39 e6 07    	vextracti128 \$0x7,%ymm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 7d 39 21 07    	vextracti128 \$0x7,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 7d 39 21 07    	vextracti128 \$0x7,%ymm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 8e 21       	vpmaskmovd %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 49 8e 21       	vpmaskmovd %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 c9 8e 21       	vpmaskmovq %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e2 c9 8e 21       	vpmaskmovq %xmm4,%xmm6,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 49 02 d4 07    	vpblendd \$0x7,%xmm4,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 02 11 07    	vpblendd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 49 02 11 07    	vpblendd \$0x7,\(%rcx\),%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c4 e2 79 59 f4       	vpbroadcastq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 59 21       	vpbroadcastq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 59 21       	vpbroadcastq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 59 f4       	vpbroadcastq %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 59 21       	vpbroadcastq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 59 21       	vpbroadcastq \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 e4       	vpbroadcastd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 21       	vpbroadcastd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 58 21       	vpbroadcastd \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 58 f4       	vpbroadcastd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 58 21       	vpbroadcastd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 58 21       	vpbroadcastd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 79 f4       	vpbroadcastw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 79 21       	vpbroadcastw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 79 21       	vpbroadcastw \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 79 f4       	vpbroadcastw %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 79 21       	vpbroadcastw \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 79 21       	vpbroadcastw \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 78 f4       	vpbroadcastb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 78 21       	vpbroadcastb \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 78 21       	vpbroadcastb \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 78 f4       	vpbroadcastb %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 78 21       	vpbroadcastb \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 78 21       	vpbroadcastb \(%rcx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 79 18 f4       	vbroadcastss %xmm4,%xmm6
#pass
