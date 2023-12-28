#as: -msse2avx
#objdump: -dw
#name: i386 SSE with AVX encoding

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	66 0f ae 11          	data16 ldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	66 0f ae 19          	data16 stmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2psx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 c9 58 f4          	vaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 58 31          	vaddpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 f4          	vaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 31          	vaddps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 f4          	vaddsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 31          	vaddsubpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 f4          	vaddsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 31          	vaddsubps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 f4          	vandnpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 31          	vandnpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 f4          	vandnps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 31          	vandnps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 f4          	vandpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 31          	vandpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 f4          	vandps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 31          	vandps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e f4          	vdivpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e 31          	vdivpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e f4          	vdivps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e 31          	vdivps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c f4          	vhaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c 31          	vhaddpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c f4          	vhaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c 31          	vhaddps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d f4          	vhsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d 31          	vhsubpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d f4          	vhsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d 31          	vhsubps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f f4          	vmaxpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f 31          	vmaxpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f f4          	vmaxps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f 31          	vmaxps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d f4          	vminpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d 31          	vminpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d f4          	vminps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d 31          	vminps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 f4          	vmulpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 31          	vmulpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 f4          	vmulps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 31          	vmulps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 f4          	vorpd  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 31          	vorpd  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 f4          	vorps  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 31          	vorps  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 f4          	vpacksswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 31          	vpacksswb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b f4          	vpackssdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b 31          	vpackssdw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 f4          	vpackuswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 31          	vpackuswb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b f4       	vpackusdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b 31       	vpackusdw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc f4          	vpaddb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc 31          	vpaddb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd f4          	vpaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd 31          	vpaddw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe f4          	vpaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe 31          	vpaddd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 f4          	vpaddq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 31          	vpaddq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec f4          	vpaddsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec 31          	vpaddsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed f4          	vpaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed 31          	vpaddsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc f4          	vpaddusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc 31          	vpaddusb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd f4          	vpaddusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd 31          	vpaddusw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db f4          	vpand  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db 31          	vpand  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df f4          	vpandn %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df 31          	vpandn \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 f4          	vpavgb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 31          	vpavgb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 f4          	vpavgw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 31          	vpavgw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 00    	vpclmullqlqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 01    	vpclmulhqlqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 10    	vpclmullqhqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 11    	vpclmulhqhqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 f4          	vpcmpeqb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 31          	vpcmpeqb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 f4          	vpcmpeqw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 31          	vpcmpeqw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 f4          	vpcmpeqd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 31          	vpcmpeqd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 f4       	vpcmpeqq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 31       	vpcmpeqq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 f4          	vpcmpgtb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 31          	vpcmpgtb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 f4          	vpcmpgtw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 31          	vpcmpgtw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 f4          	vpcmpgtd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 31          	vpcmpgtd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 f4       	vpcmpgtq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 31       	vpcmpgtq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 f4       	vphaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 31       	vphaddw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 f4       	vphaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 31       	vphaddd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 f4       	vphaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 31       	vphaddsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 f4       	vphsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 31       	vphsubw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 f4       	vphsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 31       	vphsubd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 f4       	vphsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 31       	vphsubsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 f4          	vpmaddwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 31          	vpmaddwd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 f4       	vpmaddubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 31       	vpmaddubsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c f4       	vpmaxsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c 31       	vpmaxsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee f4          	vpmaxsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee 31          	vpmaxsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d f4       	vpmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d 31       	vpmaxsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de f4          	vpmaxub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de 31          	vpmaxub \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e f4       	vpmaxuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e 31       	vpmaxuw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f f4       	vpmaxud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f 31       	vpmaxud \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 f4       	vpminsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 31       	vpminsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea f4          	vpminsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea 31          	vpminsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 f4       	vpminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 31       	vpminsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da f4          	vpminub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da 31          	vpminub \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a f4       	vpminuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a 31       	vpminuw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b f4       	vpminud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b 31       	vpminud \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 f4          	vpmulhuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 31          	vpmulhuw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b f4       	vpmulhrsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b 31       	vpmulhrsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 f4          	vpmulhw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 31          	vpmulhw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 f4          	vpmullw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 31          	vpmullw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 f4       	vpmulld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 31       	vpmulld \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 f4          	vpmuludq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 31          	vpmuludq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 f4       	vpmuldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 31       	vpmuldq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb f4          	vpor   %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb 31          	vpor   \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 f4          	vpsadbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 31          	vpsadbw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 f4       	vpshufb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 31       	vpshufb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 f4       	vpsignb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 31       	vpsignb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 f4       	vpsignw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 31       	vpsignw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a f4       	vpsignd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a 31       	vpsignd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 f4          	vpsllw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 31          	vpsllw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 f4          	vpslld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 31          	vpslld \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 f4          	vpsllq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 31          	vpsllq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 f4          	vpsraw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 31          	vpsraw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 f4          	vpsrad %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 31          	vpsrad \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 f4          	vpsrlw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 31          	vpsrlw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 f4          	vpsrld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 31          	vpsrld \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 f4          	vpsrlq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 31          	vpsrlq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 f4          	vpsubb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 31          	vpsubb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 f4          	vpsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 31          	vpsubw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa f4          	vpsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa 31          	vpsubd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb f4          	vpsubq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb 31          	vpsubq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 f4          	vpsubsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 31          	vpsubsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 f4          	vpsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 31          	vpsubsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 f4          	vpsubusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 31          	vpsubusb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 f4          	vpsubusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 31          	vpsubusw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 f4          	vpunpckhbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 31          	vpunpckhbw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 f4          	vpunpckhwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 31          	vpunpckhwd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a f4          	vpunpckhdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a 31          	vpunpckhdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d f4          	vpunpckhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d 31          	vpunpckhqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 f4          	vpunpcklbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 31          	vpunpcklbw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 f4          	vpunpcklwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 31          	vpunpcklwd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 f4          	vpunpckldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 31          	vpunpckldq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c f4          	vpunpcklqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c 31          	vpunpcklqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef f4          	vpxor  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef 31          	vpxor  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c f4          	vsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c 31          	vsubpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c f4          	vsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c 31          	vsubps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 f4          	vunpckhpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 31          	vunpckhpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 f4          	vunpckhps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 31          	vunpckhps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 f4          	vunpcklpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 31          	vunpcklpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 f4          	vunpcklps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 31          	vunpcklps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 f4          	vxorpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 31          	vxorpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 f4          	vxorps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 31          	vxorps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc f4       	vaesenc %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc 31       	vaesenc \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd f4       	vaesenclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd 31       	vaesenclast \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de f4       	vaesdec %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de 31       	vaesdec \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df f4       	vaesdeclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df 31       	vaesdeclast \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 00       	vcmpeqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 00       	vcmpeqpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 00       	vcmpeqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 00       	vcmpeqps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 01       	vcmpltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 01       	vcmpltpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 01       	vcmpltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 01       	vcmpltps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 02       	vcmplepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 02       	vcmplepd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 02       	vcmpleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 02       	vcmpleps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 03       	vcmpunordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 03       	vcmpunordpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 03       	vcmpunordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 03       	vcmpunordps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 04       	vcmpneqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 04       	vcmpneqpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 04       	vcmpneqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 04       	vcmpneqps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 05       	vcmpnltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 05       	vcmpnltpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 05       	vcmpnltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 05       	vcmpnltps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 06       	vcmpnlepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 06       	vcmpnlepd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 06       	vcmpnleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 06       	vcmpnleps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 07       	vcmpordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 07       	vcmpordpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 07       	vcmpordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 07       	vcmpordps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 64    	vaeskeygenassist \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 64    	vaeskeygenassist \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 64    	vpcmpestri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 64    	vpcmpestri \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 64    	vpcmpestrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 64    	vpcmpestrm \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 64    	vpcmpistri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 64    	vpcmpistri \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 64    	vpcmpistrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 64    	vpcmpistrm \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 64       	vpshufd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 64       	vpshufd \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 64       	vpshufhw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 64       	vpshufhw \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 64       	vpshuflw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 64       	vpshuflw \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 64    	vroundpd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 64    	vroundpd \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 64    	vroundps \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 64    	vroundps \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d f4 64    	vblendpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d 31 64    	vblendpd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c f4 64    	vblendps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c 31 64    	vblendps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 64       	vcmppd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 64       	vcmppd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 64       	vcmpps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 64       	vcmpps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 f4 64    	vdppd  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 31 64    	vdppd  \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 f4 64    	vdpps  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 31 64    	vdpps  \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 f4 64    	vmpsadbw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 31 64    	vmpsadbw \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f f4 64    	vpalignr \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f 31 64    	vpalignr \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e f4 64    	vpblendw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e 31 64    	vpblendw \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 64    	vpclmulqdq \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 64    	vpclmulqdq \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 f4 64       	vshufpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 31 64       	vshufpd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 f4 64       	vshufps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 31 64       	vshufps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 d9 12 21          	vmovlpd \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 12 21          	vmovlps \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 16 21          	vmovhpd \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 16 21          	vmovhps \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 f4 64       	vcmpsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 64       	vcmpsd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b f4 64    	vroundsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b 31 64    	vroundsd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 f4          	vaddsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 31          	vaddsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a f4          	vcvtsd2ss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a 31          	vcvtsd2ss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e f4          	vdivsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e 31          	vdivsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f f4          	vmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f 31          	vmaxsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d f4          	vminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d 31          	vminsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 f4          	vmulsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 31          	vmulsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 f4          	vsqrtsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 31          	vsqrtsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c f4          	vsubsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c 31          	vsubsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 00       	vcmpeqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 00       	vcmpeqsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 01       	vcmpltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 01       	vcmpltsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 02       	vcmplesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 02       	vcmplesd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 03       	vcmpunordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 03       	vcmpunordsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 04       	vcmpneqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 04       	vcmpneqsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 05       	vcmpnltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 05       	vcmpnltsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 06       	vcmpnlesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 06       	vcmpnlesd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 07       	vcmpordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 07       	vcmpordsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 f4          	vaddss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 31          	vaddss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a f4          	vcvtss2sd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a 31          	vcvtss2sd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e f4          	vdivss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e 31          	vdivss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f f4          	vmaxss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f 31          	vmaxss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d f4          	vminss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d 31          	vminss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 f4          	vmulss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 31          	vmulss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 f4          	vrcpss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 31          	vrcpss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 f4          	vrsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 31          	vrsqrtss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 f4          	vsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 31          	vsqrtss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c f4          	vsubss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c 31          	vsubss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 00       	vcmpeqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 00       	vcmpeqss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 01       	vcmpltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 01       	vcmpltss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 02       	vcmpless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 02       	vcmpless \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 03       	vcmpunordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 03       	vcmpunordss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 04       	vcmpneqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 04       	vcmpneqss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 05       	vcmpnltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 05       	vcmpnltss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 06       	vcmpnless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 06       	vcmpnless \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 07       	vcmpordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 07       	vcmpordss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 64    	vpextrd \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 64    	vpextrd \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 64    	vextractps \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 db 2a e1          	vcvtsi2sd %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 db 2a 21          	vcvtsi2sd \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a e1          	vcvtsi2ss %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a 21          	vcvtsi2ss \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 f4 64       	vcmpss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 64       	vcmpss \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 f4 64    	vinsertps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 31 64    	vinsertps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a f4 64    	vroundss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a 31 64    	vroundss \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 e1 64       	vpinsrw \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 21 64       	vpinsrw \$0x64,\(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 64    	vpextrb \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 f7 f4          	vmaskmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 c8 12 f4          	vmovhlps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 16 f4          	vmovlhps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 10 f4          	vmovsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 10 f4          	vmovss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 72 f4 64       	vpslld \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 fc 64       	vpslldq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 f4 64       	vpsllq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 71 f4 64       	vpsllw \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 72 e4 64       	vpsrad \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 71 e4 64       	vpsraw \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 72 d4 64       	vpsrld \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 dc 64       	vpsrldq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 d4 64       	vpsrlq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 71 d4 64       	vpsrlw \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 5b f4          	vcvtdq2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5b 21          	vcvtdq2ps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb e6 f4          	vcvtpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb e6 21          	vcvtpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5a f4          	vcvtpd2ps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5a 21          	vcvtpd2psx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 5b f4          	vcvtps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 5b 21          	vcvtps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e6 f4          	vcvttpd2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 e6 21          	vcvttpd2dqx \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 5b f4          	vcvttps2dq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 5b 21          	vcvttps2dq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 28 21          	vmovapd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 28 21          	vmovaps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 6f 21          	vmovdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 6f 21          	vmovdqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 16 f4          	vmovshdup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 16 21          	vmovshdup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 12 f4          	vmovsldup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 12 21          	vmovsldup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 10 21          	vmovupd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 10 21          	vmovups \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1c f4       	vpabsb %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1c 21       	vpabsb \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1d f4       	vpabsw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1d 21       	vpabsw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 1e f4       	vpabsd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 1e 21       	vpabsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 41 f4       	vphminposuw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 41 21       	vphminposuw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 17 f4       	vptest %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 17 21       	vptest \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 53 f4          	vrcpps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 53 21          	vrcpps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 52 f4          	vrsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 52 21          	vrsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 51 f4          	vsqrtpd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 51 21          	vsqrtpd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 51 f4          	vsqrtps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 51 21          	vsqrtps \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 db f4       	vaesimc %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 db 21       	vaesimc \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 28 f4          	vmovapd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 29 21          	vmovapd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 28 f4          	vmovaps %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 29 21          	vmovaps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6f f4          	vmovdqa %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 7f 21          	vmovdqa %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fa 6f f4          	vmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7f 21          	vmovdqu %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 10 f4          	vmovupd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 11 21          	vmovupd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 10 f4          	vmovups %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 11 21          	vmovups %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fb f0 21          	vlddqu \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 2a 21       	vmovntdqa \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 e7 21          	vmovntdq %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 2b 21          	vmovntpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 2b 21          	vmovntps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 c9 58 f4          	vaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 58 31          	vaddpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 f4          	vaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 31          	vaddps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 f4          	vaddsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 31          	vaddsubpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 f4          	vaddsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 31          	vaddsubps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 f4          	vandnpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 31          	vandnpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 f4          	vandnps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 31          	vandnps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 f4          	vandpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 31          	vandpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 f4          	vandps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 31          	vandps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e f4          	vdivpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e 31          	vdivpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e f4          	vdivps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e 31          	vdivps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c f4          	vhaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c 31          	vhaddpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c f4          	vhaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c 31          	vhaddps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d f4          	vhsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d 31          	vhsubpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d f4          	vhsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d 31          	vhsubps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f f4          	vmaxpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f 31          	vmaxpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f f4          	vmaxps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f 31          	vmaxps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d f4          	vminpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d 31          	vminpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d f4          	vminps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d 31          	vminps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 f4          	vmulpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 31          	vmulpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 f4          	vmulps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 31          	vmulps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 f4          	vorpd  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 31          	vorpd  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 f4          	vorps  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 31          	vorps  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 f4          	vpacksswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 31          	vpacksswb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b f4          	vpackssdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b 31          	vpackssdw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 f4          	vpackuswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 31          	vpackuswb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b f4       	vpackusdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b 31       	vpackusdw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc f4          	vpaddb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc 31          	vpaddb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd f4          	vpaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd 31          	vpaddw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe f4          	vpaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe 31          	vpaddd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 f4          	vpaddq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 31          	vpaddq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec f4          	vpaddsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec 31          	vpaddsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed f4          	vpaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed 31          	vpaddsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc f4          	vpaddusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc 31          	vpaddusb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd f4          	vpaddusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd 31          	vpaddusw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db f4          	vpand  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db 31          	vpand  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df f4          	vpandn %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df 31          	vpandn \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 f4          	vpavgb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 31          	vpavgb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 f4          	vpavgw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 31          	vpavgw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 00    	vpclmullqlqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 01    	vpclmulhqlqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 10    	vpclmullqhqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 11    	vpclmulhqhqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 f4          	vpcmpeqb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 31          	vpcmpeqb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 f4          	vpcmpeqw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 31          	vpcmpeqw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 f4          	vpcmpeqd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 31          	vpcmpeqd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 f4       	vpcmpeqq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 31       	vpcmpeqq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 f4          	vpcmpgtb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 31          	vpcmpgtb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 f4          	vpcmpgtw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 31          	vpcmpgtw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 f4          	vpcmpgtd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 31          	vpcmpgtd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 f4       	vpcmpgtq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 31       	vpcmpgtq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 f4       	vphaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 31       	vphaddw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 f4       	vphaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 31       	vphaddd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 f4       	vphaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 31       	vphaddsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 f4       	vphsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 31       	vphsubw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 f4       	vphsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 31       	vphsubd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 f4       	vphsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 31       	vphsubsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 f4          	vpmaddwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 31          	vpmaddwd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 f4       	vpmaddubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 31       	vpmaddubsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c f4       	vpmaxsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c 31       	vpmaxsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee f4          	vpmaxsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee 31          	vpmaxsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d f4       	vpmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d 31       	vpmaxsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de f4          	vpmaxub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de 31          	vpmaxub \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e f4       	vpmaxuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e 31       	vpmaxuw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f f4       	vpmaxud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f 31       	vpmaxud \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 f4       	vpminsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 31       	vpminsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea f4          	vpminsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea 31          	vpminsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 f4       	vpminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 31       	vpminsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da f4          	vpminub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da 31          	vpminub \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a f4       	vpminuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a 31       	vpminuw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b f4       	vpminud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b 31       	vpminud \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 f4          	vpmulhuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 31          	vpmulhuw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b f4       	vpmulhrsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b 31       	vpmulhrsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 f4          	vpmulhw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 31          	vpmulhw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 f4          	vpmullw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 31          	vpmullw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 f4       	vpmulld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 31       	vpmulld \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 f4          	vpmuludq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 31          	vpmuludq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 f4       	vpmuldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 31       	vpmuldq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb f4          	vpor   %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb 31          	vpor   \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 f4          	vpsadbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 31          	vpsadbw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 f4       	vpshufb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 31       	vpshufb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 f4       	vpsignb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 31       	vpsignb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 f4       	vpsignw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 31       	vpsignw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a f4       	vpsignd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a 31       	vpsignd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 f4          	vpsllw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 31          	vpsllw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 f4          	vpslld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 31          	vpslld \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 f4          	vpsllq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 31          	vpsllq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 f4          	vpsraw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 31          	vpsraw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 f4          	vpsrad %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 31          	vpsrad \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 f4          	vpsrlw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 31          	vpsrlw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 f4          	vpsrld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 31          	vpsrld \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 f4          	vpsrlq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 31          	vpsrlq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 f4          	vpsubb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 31          	vpsubb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 f4          	vpsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 31          	vpsubw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa f4          	vpsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa 31          	vpsubd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb f4          	vpsubq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb 31          	vpsubq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 f4          	vpsubsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 31          	vpsubsb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 f4          	vpsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 31          	vpsubsw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 f4          	vpsubusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 31          	vpsubusb \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 f4          	vpsubusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 31          	vpsubusw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 f4          	vpunpckhbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 31          	vpunpckhbw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 f4          	vpunpckhwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 31          	vpunpckhwd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a f4          	vpunpckhdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a 31          	vpunpckhdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d f4          	vpunpckhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d 31          	vpunpckhqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 f4          	vpunpcklbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 31          	vpunpcklbw \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 f4          	vpunpcklwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 31          	vpunpcklwd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 f4          	vpunpckldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 31          	vpunpckldq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c f4          	vpunpcklqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c 31          	vpunpcklqdq \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef f4          	vpxor  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef 31          	vpxor  \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c f4          	vsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c 31          	vsubpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c f4          	vsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c 31          	vsubps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 f4          	vunpckhpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 31          	vunpckhpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 f4          	vunpckhps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 31          	vunpckhps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 f4          	vunpcklpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 31          	vunpcklpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 f4          	vunpcklps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 31          	vunpcklps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 f4          	vxorpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 31          	vxorpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 f4          	vxorps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 31          	vxorps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc f4       	vaesenc %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc 31       	vaesenc \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd f4       	vaesenclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd 31       	vaesenclast \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de f4       	vaesdec %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de 31       	vaesdec \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df f4       	vaesdeclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df 31       	vaesdeclast \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 00       	vcmpeqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 00       	vcmpeqpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 00       	vcmpeqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 00       	vcmpeqps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 01       	vcmpltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 01       	vcmpltpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 01       	vcmpltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 01       	vcmpltps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 02       	vcmplepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 02       	vcmplepd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 02       	vcmpleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 02       	vcmpleps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 03       	vcmpunordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 03       	vcmpunordpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 03       	vcmpunordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 03       	vcmpunordps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 04       	vcmpneqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 04       	vcmpneqpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 04       	vcmpneqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 04       	vcmpneqps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 05       	vcmpnltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 05       	vcmpnltpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 05       	vcmpnltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 05       	vcmpnltps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 06       	vcmpnlepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 06       	vcmpnlepd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 06       	vcmpnleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 06       	vcmpnleps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 07       	vcmpordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 07       	vcmpordpd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 07       	vcmpordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 07       	vcmpordps \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 64    	vaeskeygenassist \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 64    	vaeskeygenassist \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 64    	vpcmpestri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 64    	vpcmpestri \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 64    	vpcmpestrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 64    	vpcmpestrm \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 64    	vpcmpistri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 64    	vpcmpistri \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 64    	vpcmpistrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 64    	vpcmpistrm \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 64       	vpshufd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 64       	vpshufd \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 64       	vpshufhw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 64       	vpshufhw \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 64       	vpshuflw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 64       	vpshuflw \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 64    	vroundpd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 64    	vroundpd \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 64    	vroundps \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 64    	vroundps \$0x64,\(%ecx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d f4 64    	vblendpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d 31 64    	vblendpd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c f4 64    	vblendps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c 31 64    	vblendps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 64       	vcmppd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 64       	vcmppd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 64       	vcmpps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 64       	vcmpps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 f4 64    	vdppd  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 31 64    	vdppd  \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 f4 64    	vdpps  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 31 64    	vdpps  \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 f4 64    	vmpsadbw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 31 64    	vmpsadbw \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f f4 64    	vpalignr \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f 31 64    	vpalignr \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e f4 64    	vpblendw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e 31 64    	vpblendw \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 64    	vpclmulqdq \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 64    	vpclmulqdq \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 f4 64       	vshufpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 31 64       	vshufpd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 f4 64       	vshufps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 31 64       	vshufps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 5a f4          	vcvtps2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 5a 21          	vcvtps2pd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 12 f4          	vmovddup %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 12 21          	vmovddup \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 20 f4       	vpmovsxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 20 21       	vpmovsxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 23 f4       	vpmovsxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 23 21       	vpmovsxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 25 f4       	vpmovsxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 25 21       	vpmovsxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 30 f4       	vpmovzxbw %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 30 21       	vpmovzxbw \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 33 f4       	vpmovzxwd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 33 21       	vpmovzxwd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 35 f4       	vpmovzxdq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 35 21       	vpmovzxdq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 2e f4          	vucomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2e 21          	vucomisd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 10 21          	vmovsd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 13 21          	vmovlpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 13 21          	vmovlps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 17 21          	vmovhpd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f8 17 21          	vmovhps %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fb 11 21          	vmovsd %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 d6 21          	vmovq  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 fa 7e 21          	vmovq  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fb 2d cc          	vcvtsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2d 09          	vcvtsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c cc          	vcvttsd2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fb 2c 09          	vcvttsd2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 d9 12 21          	vmovlpd \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 12 21          	vmovlps \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 16 21          	vmovhpd \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 16 21          	vmovhps \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 f4 64       	vcmpsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 64       	vcmpsd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b f4 64    	vroundsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b 31 64    	vroundsd \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 f4          	vaddsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 31          	vaddsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a f4          	vcvtsd2ss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a 31          	vcvtsd2ss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e f4          	vdivsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e 31          	vdivsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f f4          	vmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f 31          	vmaxsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d f4          	vminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d 31          	vminsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 f4          	vmulsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 31          	vmulsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 f4          	vsqrtsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 31          	vsqrtsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c f4          	vsubsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c 31          	vsubsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 00       	vcmpeqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 00       	vcmpeqsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 01       	vcmpltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 01       	vcmpltsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 02       	vcmplesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 02       	vcmplesd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 03       	vcmpunordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 03       	vcmpunordsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 04       	vcmpneqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 04       	vcmpneqsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 05       	vcmpnltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 05       	vcmpnltsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 06       	vcmpnlesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 06       	vcmpnlesd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 07       	vcmpordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 07       	vcmpordsd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 f4          	vaddss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 31          	vaddss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a f4          	vcvtss2sd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a 31          	vcvtss2sd \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e f4          	vdivss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e 31          	vdivss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f f4          	vmaxss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f 31          	vmaxss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d f4          	vminss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d 31          	vminss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 f4          	vmulss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 31          	vmulss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 f4          	vrcpss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 31          	vrcpss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 f4          	vrsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 31          	vrsqrtss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 f4          	vsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 31          	vsqrtss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c f4          	vsubss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c 31          	vsubss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 00       	vcmpeqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 00       	vcmpeqss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 01       	vcmpltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 01       	vcmpltss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 02       	vcmpless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 02       	vcmpless \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 03       	vcmpunordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 03       	vcmpunordss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 04       	vcmpneqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 04       	vcmpneqss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 05       	vcmpnltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 05       	vcmpnltss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 06       	vcmpnless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 06       	vcmpnless \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 07       	vcmpordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 07       	vcmpordss \(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f 21          	vcomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 21 f4       	vpmovsxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 21 21       	vpmovsxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 24 f4       	vpmovsxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 24 21       	vpmovsxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 31 f4       	vpmovzxbd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 31 21       	vpmovzxbd \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 34 f4       	vpmovzxwq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 34 21       	vpmovzxwq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f8 2e f4          	vucomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 10 21          	vmovss \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 11 21          	vmovss %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 7e e1          	vmovd  %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 7e 21          	vmovd  %xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 f9 6e e1          	vmovd  %ecx,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 6e 21          	vmovd  \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa 2d cc          	vcvtss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2d 09          	vcvtss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c cc          	vcvttss2si %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 fa 2c 09          	vcvttss2si \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 64    	vpextrd \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 64    	vpextrd \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 64    	vextractps \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 db 2a e1          	vcvtsi2sd %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 db 2a 21          	vcvtsi2sd \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a e1          	vcvtsi2ss %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a 21          	vcvtsi2ss \(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 f4 64       	vcmpss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 64       	vcmpss \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 f4 64    	vinsertps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 31 64    	vinsertps \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a f4 64    	vroundss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a 31 64    	vroundss \$0x64,\(%ecx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%ecx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 e1 64       	vpinsrw \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 21 64       	vpinsrw \$0x64,\(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 64    	vpextrb \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%ecx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%ecx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 f7 f4          	vmaskmovdqu %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 7e f4          	vmovq  %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 50 cc          	vmovmskpd %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f8 50 cc          	vmovmskps %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 f9 d7 cc          	vpmovmskb %xmm4,%ecx
[ 	]*[a-f0-9]+:	c5 c8 12 f4          	vmovhlps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 16 f4          	vmovlhps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 10 f4          	vmovsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 10 f4          	vmovss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 d9 72 f4 64       	vpslld \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 fc 64       	vpslldq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 f4 64       	vpsllq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 71 f4 64       	vpsllw \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 72 e4 64       	vpsrad \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 71 e4 64       	vpsraw \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 72 d4 64       	vpsrld \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 dc 64       	vpsrldq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 73 d4 64       	vpsrlq \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 71 d4 64       	vpsrlw \$0x64,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
#pass
