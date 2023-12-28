#as: -msse2avx
#objdump: -dw
#name: x86-64 SSE with AVX encoding

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
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
[ 	]*[a-f0-9]+:	c5 c9 58 f4          	vaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 58 f6       	vaddpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 58 31          	vaddpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 f4          	vaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 58 f6       	vaddps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 31          	vaddps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 f4          	vaddsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 31          	vaddsubpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 f4          	vaddsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 31          	vaddsubps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 f4          	vandnpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 55 f6       	vandnpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 31          	vandnpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 f4          	vandnps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 55 f6       	vandnps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 31          	vandnps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 f4          	vandpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 54 f6          	vandpd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 31          	vandpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 f4          	vandps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 54 f6          	vandps %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 31          	vandps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e f4          	vdivpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e 31          	vdivpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e f4          	vdivps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e 31          	vdivps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c f4          	vhaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c 31          	vhaddpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c f4          	vhaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c 31          	vhaddps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d f4          	vhsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d 31          	vhsubpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d f4          	vhsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d 31          	vhsubps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f f4          	vmaxpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 5f f6       	vmaxpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f 31          	vmaxpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f f4          	vmaxps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 5f f6       	vmaxps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f 31          	vmaxps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d f4          	vminpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 5d f6       	vminpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d 31          	vminpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d f4          	vminps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 5d f6       	vminps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d 31          	vminps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 f4          	vmulpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 59 f6       	vmulpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 31          	vmulpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 f4          	vmulps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 59 f6       	vmulps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 31          	vmulps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 f4          	vorpd  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 56 f6          	vorpd  %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 31          	vorpd  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 f4          	vorps  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 56 f6          	vorps  %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 31          	vorps  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 f4          	vpacksswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 31          	vpacksswb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b f4          	vpackssdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b 31          	vpackssdw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 f4          	vpackuswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 31          	vpackuswb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b f4       	vpackusdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b 31       	vpackusdw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc f4          	vpaddb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 fc f6          	vpaddb %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc 31          	vpaddb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd f4          	vpaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 fd f6          	vpaddw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd 31          	vpaddw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe f4          	vpaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 fe f6          	vpaddd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe 31          	vpaddd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 f4          	vpaddq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 d4 f6          	vpaddq %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 31          	vpaddq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec f4          	vpaddsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 ec f6          	vpaddsb %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec 31          	vpaddsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed f4          	vpaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 ed f6          	vpaddsw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed 31          	vpaddsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc f4          	vpaddusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 dc f6          	vpaddusb %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc 31          	vpaddusb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd f4          	vpaddusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 dd f6          	vpaddusw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd 31          	vpaddusw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db f4          	vpand  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 db f6          	vpand  %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db 31          	vpand  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df f4          	vpandn %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 df f6       	vpandn %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df 31          	vpandn \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 f4          	vpavgb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 e0 f6          	vpavgb %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 31          	vpavgb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 f4          	vpavgw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 e3 f6          	vpavgw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 31          	vpavgw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 00    	vpclmullqlqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 01    	vpclmulhqlqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 10    	vpclmullqhqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 11    	vpclmulhqhqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 f4          	vpcmpeqb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 74 f6          	vpcmpeqb %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 31          	vpcmpeqb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 f4          	vpcmpeqw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 75 f6          	vpcmpeqw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 31          	vpcmpeqw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 f4          	vpcmpeqd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 76 f6          	vpcmpeqd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 31          	vpcmpeqd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 f4       	vpcmpeqq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 31       	vpcmpeqq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 f4          	vpcmpgtb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 64 f6       	vpcmpgtb %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 31          	vpcmpgtb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 f4          	vpcmpgtw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 65 f6       	vpcmpgtw %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 31          	vpcmpgtw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 f4          	vpcmpgtd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 66 f6       	vpcmpgtd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 31          	vpcmpgtd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 f4       	vpcmpgtq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 31       	vpcmpgtq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 f4       	vphaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 31       	vphaddw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 f4       	vphaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 31       	vphaddd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 f4       	vphaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 31       	vphaddsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 f4       	vphsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 31       	vphsubw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 f4       	vphsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 31       	vphsubd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 f4       	vphsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 31       	vphsubsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 f4          	vpmaddwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 f5 f6          	vpmaddwd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 31          	vpmaddwd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 f4       	vpmaddubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 31       	vpmaddubsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c f4       	vpmaxsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c 31       	vpmaxsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee f4          	vpmaxsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 ee f6          	vpmaxsw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee 31          	vpmaxsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d f4       	vpmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d 31       	vpmaxsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de f4          	vpmaxub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 de f6          	vpmaxub %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de 31          	vpmaxub \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e f4       	vpmaxuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e 31       	vpmaxuw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f f4       	vpmaxud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f 31       	vpmaxud \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 f4       	vpminsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 31       	vpminsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea f4          	vpminsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 ea f6          	vpminsw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea 31          	vpminsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 f4       	vpminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 31       	vpminsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da f4          	vpminub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 da f6          	vpminub %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da 31          	vpminub \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a f4       	vpminuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a 31       	vpminuw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b f4       	vpminud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b 31       	vpminud \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 f4          	vpmulhuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 e4 f6          	vpmulhuw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 31          	vpmulhuw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b f4       	vpmulhrsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b 31       	vpmulhrsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 f4          	vpmulhw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 e5 f6          	vpmulhw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 31          	vpmulhw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 f4          	vpmullw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 d5 f6          	vpmullw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 31          	vpmullw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 f4       	vpmulld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 31       	vpmulld \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 f4          	vpmuludq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 f4 f6          	vpmuludq %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 31          	vpmuludq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 f4       	vpmuldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 31       	vpmuldq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb f4          	vpor   %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 eb f6          	vpor   %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb 31          	vpor   \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 f4          	vpsadbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 f6 f6          	vpsadbw %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 31          	vpsadbw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 f4       	vpshufb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 31       	vpshufb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 f4       	vpsignb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 31       	vpsignb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 f4       	vpsignw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 31       	vpsignw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a f4       	vpsignd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a 31       	vpsignd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 f4          	vpsllw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 31          	vpsllw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 f4          	vpslld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 31          	vpslld \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 f4          	vpsllq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 31          	vpsllq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 f4          	vpsraw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 31          	vpsraw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 f4          	vpsrad %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 31          	vpsrad \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 f4          	vpsrlw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 31          	vpsrlw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 f4          	vpsrld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 31          	vpsrld \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 f4          	vpsrlq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 31          	vpsrlq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 f4          	vpsubb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 31          	vpsubb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 f4          	vpsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 31          	vpsubw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa f4          	vpsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa 31          	vpsubd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb f4          	vpsubq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb 31          	vpsubq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 f4          	vpsubsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 31          	vpsubsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 f4          	vpsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 31          	vpsubsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 f4          	vpsubusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 31          	vpsubusb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 f4          	vpsubusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 31          	vpsubusw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 f4          	vpunpckhbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 31          	vpunpckhbw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 f4          	vpunpckhwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 31          	vpunpckhwd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a f4          	vpunpckhdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a 31          	vpunpckhdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d f4          	vpunpckhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d 31          	vpunpckhqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 f4          	vpunpcklbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 31          	vpunpcklbw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 f4          	vpunpcklwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 31          	vpunpcklwd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 f4          	vpunpckldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 31          	vpunpckldq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c f4          	vpunpcklqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c 31          	vpunpcklqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef f4          	vpxor  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 ef f6          	vpxor  %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef 31          	vpxor  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c f4          	vsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c 31          	vsubpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c f4          	vsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c 31          	vsubps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 f4          	vunpckhpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 31          	vunpckhpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 f4          	vunpckhps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 31          	vunpckhps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 f4          	vunpcklpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 31          	vunpcklpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 f4          	vunpcklps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 31          	vunpcklps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 f4          	vxorpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 57 f6          	vxorpd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 31          	vxorpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 f4          	vxorps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 57 f6          	vxorps %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 31          	vxorps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc f4       	vaesenc %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc 31       	vaesenc \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd f4       	vaesenclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd 31       	vaesenclast \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de f4       	vaesdec %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de 31       	vaesdec \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df f4       	vaesdeclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df 31       	vaesdeclast \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 00       	vcmpeqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 c2 f6 00       	vcmpeqpd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 00       	vcmpeqpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 00       	vcmpeqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 c2 f6 00       	vcmpeqps %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 00       	vcmpeqps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 01       	vcmpltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 c2 f6 01    	vcmpltpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 01       	vcmpltpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 01       	vcmpltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 c2 f6 01    	vcmpltps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 01       	vcmpltps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 02       	vcmplepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 c2 f6 02    	vcmplepd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 02       	vcmplepd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 02       	vcmpleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 c2 f6 02    	vcmpleps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 02       	vcmpleps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 03       	vcmpunordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 c2 f6 03       	vcmpunordpd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 03       	vcmpunordpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 03       	vcmpunordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 c2 f6 03       	vcmpunordps %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 03       	vcmpunordps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 04       	vcmpneqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 c2 f6 04       	vcmpneqpd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 04       	vcmpneqpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 04       	vcmpneqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 c2 f6 04       	vcmpneqps %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 04       	vcmpneqps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 05       	vcmpnltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 c2 f6 05    	vcmpnltpd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 05       	vcmpnltpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 05       	vcmpnltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 c2 f6 05    	vcmpnltps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 05       	vcmpnltps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 06       	vcmpnlepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 c2 f6 06    	vcmpnlepd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 06       	vcmpnlepd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 06       	vcmpnleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 c2 f6 06    	vcmpnleps %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 06       	vcmpnleps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 07       	vcmpordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 89 c2 f6 07       	vcmpordpd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 07       	vcmpordpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 07       	vcmpordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 88 c2 f6 07       	vcmpordps %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 07       	vcmpordps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 64    	vaeskeygenassist \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 64    	vaeskeygenassist \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 64    	vpcmpestril? \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 64    	vpcmpestril? \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 f9 61 f4 64    	vpcmpestriq \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 64    	vpcmpestril? \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 64    	vpcmpestrml? \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 64    	vpcmpestrml? \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 f9 60 f4 64    	vpcmpestrmq \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 64    	vpcmpestrml? \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 64    	vpcmpistri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 64    	vpcmpistri \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 64    	vpcmpistrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 64    	vpcmpistrm \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 64       	vpshufd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 64       	vpshufd \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 64       	vpshufhw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 64       	vpshufhw \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 64       	vpshuflw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 64       	vpshuflw \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 64    	vroundpd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 64    	vroundpd \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 64    	vroundps \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 64    	vroundps \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d f4 64    	vblendpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d 31 64    	vblendpd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c f4 64    	vblendps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c 31 64    	vblendps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 64       	vcmppd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 49 c2 f6 64    	vcmppd \$0x64,%xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 64       	vcmppd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 64       	vcmpps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 48 c2 f6 64    	vcmpps \$0x64,%xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 64       	vcmpps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 f4 64    	vdppd  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 31 64    	vdppd  \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 f4 64    	vdpps  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 31 64    	vdpps  \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 f4 64    	vmpsadbw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 31 64    	vmpsadbw \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f f4 64    	vpalignr \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f 31 64    	vpalignr \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e f4 64    	vpblendw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e 31 64    	vpblendw \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 64    	vpclmulqdq \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 64    	vpclmulqdq \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 f4 64       	vshufpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 31 64       	vshufpd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 f4 64       	vshufps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 31 64       	vshufps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 79 2f f6       	vcomisd %xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%rcx\),%xmm4
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
[ 	]*[a-f0-9]+:	c4 c1 79 2e f6       	vucomisd %xmm14,%xmm6
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
[ 	]*[a-f0-9]+:	c4 e1 db 2a e1       	vcvtsi2sd %rcx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 db 2a 21       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 da 2a e1       	vcvtsi2ss %rcx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 da 2a 21       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 d9 22 e1 64    	vpinsrq \$0x64,%rcx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 d9 22 21 64    	vpinsrq \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 f9 16 e1 64    	vpextrq \$0x64,%xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 64    	vpextrq \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 12 21          	vmovlpd \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 12 21          	vmovlps \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 16 21          	vmovhpd \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 16 21          	vmovhps \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 f4 64       	vcmpsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b c2 f6 64    	vcmpsd \$0x64,%xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 64       	vcmpsd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b f4 64    	vroundsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b 31 64    	vroundsd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 f4          	vaddsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b 58 f6       	vaddsd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 31          	vaddsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a f4          	vcvtsd2ss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a 31          	vcvtsd2ss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e f4          	vdivsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e 31          	vdivsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f f4          	vmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b 5f f6       	vmaxsd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f 31          	vmaxsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d f4          	vminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b 5d f6       	vminsd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d 31          	vminsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 f4          	vmulsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b 59 f6       	vmulsd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 31          	vmulsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 f4          	vsqrtsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 31          	vsqrtsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c f4          	vsubsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c 31          	vsubsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 00       	vcmpeqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8b c2 f6 00       	vcmpeqsd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 01       	vcmpltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b c2 f6 01    	vcmpltsd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 01       	vcmpltsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 02       	vcmplesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b c2 f6 02    	vcmplesd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 02       	vcmplesd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 03       	vcmpunordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8b c2 f6 03       	vcmpunordsd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 04       	vcmpneqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8b c2 f6 04       	vcmpneqsd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 05       	vcmpnltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b c2 f6 05    	vcmpnltsd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 06       	vcmpnlesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4b c2 f6 06    	vcmpnlesd %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 07       	vcmpordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8b c2 f6 07       	vcmpordsd %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 07       	vcmpordsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 f4          	vaddss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a 58 f6       	vaddss %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 31          	vaddss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a f4          	vcvtss2sd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a 31          	vcvtss2sd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e f4          	vdivss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e 31          	vdivss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f f4          	vmaxss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a 5f f6       	vmaxss %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f 31          	vmaxss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d f4          	vminss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a 5d f6       	vminss %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d 31          	vminss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 f4          	vmulss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a 59 f6       	vmulss %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 31          	vmulss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 f4          	vrcpss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 31          	vrcpss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 f4          	vrsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 31          	vrsqrtss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 f4          	vsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 31          	vsqrtss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c f4          	vsubss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c 31          	vsubss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 00       	vcmpeqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8a c2 f6 00       	vcmpeqss %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 00       	vcmpeqss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 01       	vcmpltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a c2 f6 01    	vcmpltss %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 01       	vcmpltss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 02       	vcmpless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a c2 f6 02    	vcmpless %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 02       	vcmpless \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 03       	vcmpunordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8a c2 f6 03       	vcmpunordss %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 03       	vcmpunordss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 04       	vcmpneqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8a c2 f6 04       	vcmpneqss %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 04       	vcmpneqss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 05       	vcmpnltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a c2 f6 05    	vcmpnltss %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 05       	vcmpnltss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 06       	vcmpnless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a c2 f6 06    	vcmpnless %xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 06       	vcmpnless \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 07       	vcmpordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 8a c2 f6 07       	vcmpordss %xmm6,%xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 07       	vcmpordss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2f f4          	vcomiss %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 78 2f f6       	vcomiss %xmm14,%xmm6
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
[ 	]*[a-f0-9]+:	c4 c1 78 2e f6       	vucomiss %xmm14,%xmm6
[ 	]*[a-f0-9]+:	c5 f8 2e 21          	vucomiss \(%rcx\),%xmm4
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
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 64    	vextractps \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 64    	vpextrd \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 64    	vpextrd \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 64    	vextractps \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 db 2a e1          	vcvtsi2sd %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 db 2a 21          	vcvtsi2sdl \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a e1          	vcvtsi2ss %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a 21          	vcvtsi2ssl \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 f4 64       	vcmpss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 c1 4a c2 f6 64    	vcmpss \$0x64,%xmm14,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 64       	vcmpss \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 f4 64    	vinsertps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 31 64    	vinsertps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a f4 64    	vroundss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a 31 64    	vroundss \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 e1 64       	vpinsrw \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 21 64       	vpinsrw \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 e1 64       	vpinsrw \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 21 64       	vpinsrw \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 64    	vpextrb \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 64    	vpextrb \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%rcx\),%xmm4,%xmm4
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
[ 	]*[a-f0-9]+:	c5 f0 58 c8          	vaddps %xmm0,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c5 f0 58 0c 00       	vaddps \(%rax,%rax(,1)?\),%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c5 f0 58 c8          	vaddps %xmm0,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c5 f0 58 0c 00       	vaddps \(%rax,%rax(,1)?\),%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c5 30 58 c8          	vaddps %xmm0,%xmm9,%xmm9
[ 	]*[a-f0-9]+:	c5 30 58 0c 00       	vaddps \(%rax,%rax(,1)?\),%xmm9,%xmm9
[ 	]*[a-f0-9]+:	c4 a1 70 58 c8       	vaddps %xmm0,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c4 a1 70 58 0c 00    	vaddps \(%rax,%r8(,1)?\),%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c4 c1 70 58 c8       	vaddps %xmm8,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c4 c1 70 58 0c 00    	vaddps \(%r8,%rax(,1)?\),%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c5 32 10 c8          	vmovss %xmm0,%xmm9,%xmm9
[ 	]*[a-f0-9]+:	c4 c1 72 10 c8       	vmovss %xmm8,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c5 72 11 c1          	vmovss %xmm8,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c4 c1 32 11 c1       	vmovss %xmm0,%xmm9,%xmm9
[ 	]*[a-f0-9]+:	c4 c1 39 71 f0 00    	vpsllw \$(0x)?0,%xmm8,%xmm8
[ 	]*[a-f0-9]+:	c5 79 c5 c8 00       	vpextrw \$(0x)?0,%xmm0,%r9d
[ 	]*[a-f0-9]+:	c4 c1 79 c5 c8 00    	vpextrw \$(0x)?0,%xmm8,%ecx
[ 	]*[a-f0-9]+:	c4 63 79 14 c1 00    	vpextrb \$(0x)?0,%xmm8,%ecx
[ 	]*[a-f0-9]+:	c4 c3 79 14 c1 00    	vpextrb \$(0x)?0,%xmm0,%r9d
[ 	]*[a-f0-9]+:	c4 63 31 4a c8 00    	vblendvps %xmm0,%xmm0,%xmm9,%xmm9
[ 	]*[a-f0-9]+:	c4 c3 71 4a c8 00    	vblendvps %xmm0,%xmm8,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c4 63 31 4a c8 00    	vblendvps %xmm0,%xmm0,%xmm9,%xmm9
[ 	]*[a-f0-9]+:	c4 c3 71 4a c8 00    	vblendvps %xmm0,%xmm8,%xmm1,%xmm1
[ 	]*[a-f0-9]+:	c4 e1 fb 2a 00       	vcvtsi2sdq \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e1 fa 2a 00       	vcvtsi2ssq \(%rax\),%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 f9 61 c0 00    	vpcmpestriq \$(0x)?0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c4 e3 f9 60 c0 00    	vpcmpestrmq \$(0x)?0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:	c5 f8 ae 11          	vldmxcsr \(%rcx\)
[ 	]*[a-f0-9]+:	c5 f8 ae 19          	vstmxcsr \(%rcx\)
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
[ 	]*[a-f0-9]+:	c5 c9 58 f4          	vaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 58 31          	vaddpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 f4          	vaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 58 31          	vaddps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 f4          	vaddsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d0 31          	vaddsubpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 f4          	vaddsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb d0 31          	vaddsubps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 f4          	vandnpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 55 31          	vandnpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 f4          	vandnps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 55 31          	vandnps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 f4          	vandpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 54 31          	vandpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 f4          	vandps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 54 31          	vandps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e f4          	vdivpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5e 31          	vdivpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e f4          	vdivps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5e 31          	vdivps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c f4          	vhaddpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7c 31          	vhaddpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c f4          	vhaddps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7c 31          	vhaddps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d f4          	vhsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 7d 31          	vhsubpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d f4          	vhsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 7d 31          	vhsubps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f f4          	vmaxpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5f 31          	vmaxpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f f4          	vmaxps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5f 31          	vmaxps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d f4          	vminpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5d 31          	vminpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d f4          	vminps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5d 31          	vminps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 f4          	vmulpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 59 31          	vmulpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 f4          	vmulps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 59 31          	vmulps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 f4          	vorpd  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 56 31          	vorpd  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 f4          	vorps  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 56 31          	vorps  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 f4          	vpacksswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 63 31          	vpacksswb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b f4          	vpackssdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6b 31          	vpackssdw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 f4          	vpackuswb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 67 31          	vpackuswb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b f4       	vpackusdw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 2b 31       	vpackusdw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc f4          	vpaddb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fc 31          	vpaddb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd f4          	vpaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fd 31          	vpaddw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe f4          	vpaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fe 31          	vpaddd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 f4          	vpaddq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d4 31          	vpaddq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec f4          	vpaddsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ec 31          	vpaddsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed f4          	vpaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ed 31          	vpaddsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc f4          	vpaddusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dc 31          	vpaddusb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd f4          	vpaddusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 dd 31          	vpaddusw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db f4          	vpand  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 db 31          	vpand  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df f4          	vpandn %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 df 31          	vpandn \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 f4          	vpavgb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e0 31          	vpavgb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 f4          	vpavgw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e3 31          	vpavgw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 00    	vpclmullqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 00    	vpclmullqlqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 01    	vpclmulhqlqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 01    	vpclmulhqlqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 10    	vpclmullqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 10    	vpclmullqhqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 11    	vpclmulhqhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 11    	vpclmulhqhqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 f4          	vpcmpeqb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 74 31          	vpcmpeqb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 f4          	vpcmpeqw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 75 31          	vpcmpeqw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 f4          	vpcmpeqd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 76 31          	vpcmpeqd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 f4       	vpcmpeqq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 29 31       	vpcmpeqq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 f4          	vpcmpgtb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 64 31          	vpcmpgtb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 f4          	vpcmpgtw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 65 31          	vpcmpgtw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 f4          	vpcmpgtd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 66 31          	vpcmpgtd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 f4       	vpcmpgtq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 37 31       	vpcmpgtq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 f4       	vphaddw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 01 31       	vphaddw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 f4       	vphaddd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 02 31       	vphaddd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 f4       	vphaddsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 03 31       	vphaddsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 f4       	vphsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 05 31       	vphsubw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 f4       	vphsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 06 31       	vphsubd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 f4       	vphsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 07 31       	vphsubsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 f4          	vpmaddwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f5 31          	vpmaddwd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 f4       	vpmaddubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 04 31       	vpmaddubsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c f4       	vpmaxsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3c 31       	vpmaxsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee f4          	vpmaxsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ee 31          	vpmaxsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d f4       	vpmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3d 31       	vpmaxsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de f4          	vpmaxub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 de 31          	vpmaxub \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e f4       	vpmaxuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3e 31       	vpmaxuw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f f4       	vpmaxud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3f 31       	vpmaxud \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 f4       	vpminsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 38 31       	vpminsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea f4          	vpminsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ea 31          	vpminsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 f4       	vpminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 39 31       	vpminsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da f4          	vpminub %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 da 31          	vpminub \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a f4       	vpminuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3a 31       	vpminuw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b f4       	vpminud %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 3b 31       	vpminud \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 f4          	vpmulhuw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e4 31          	vpmulhuw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b f4       	vpmulhrsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0b 31       	vpmulhrsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 f4          	vpmulhw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e5 31          	vpmulhw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 f4          	vpmullw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d5 31          	vpmullw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 f4       	vpmulld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 40 31       	vpmulld \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 f4          	vpmuludq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f4 31          	vpmuludq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 f4       	vpmuldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 28 31       	vpmuldq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb f4          	vpor   %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 eb 31          	vpor   \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 f4          	vpsadbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f6 31          	vpsadbw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 f4       	vpshufb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 00 31       	vpshufb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 f4       	vpsignb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 08 31       	vpsignb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 f4       	vpsignw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 09 31       	vpsignw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a f4       	vpsignd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 0a 31       	vpsignd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 f4          	vpsllw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f1 31          	vpsllw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 f4          	vpslld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f2 31          	vpslld \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 f4          	vpsllq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f3 31          	vpsllq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 f4          	vpsraw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e1 31          	vpsraw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 f4          	vpsrad %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e2 31          	vpsrad \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 f4          	vpsrlw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d1 31          	vpsrlw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 f4          	vpsrld %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d2 31          	vpsrld \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 f4          	vpsrlq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d3 31          	vpsrlq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 f4          	vpsubb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f8 31          	vpsubb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 f4          	vpsubw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 f9 31          	vpsubw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa f4          	vpsubd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fa 31          	vpsubd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb f4          	vpsubq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 fb 31          	vpsubq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 f4          	vpsubsb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e8 31          	vpsubsb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 f4          	vpsubsw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 e9 31          	vpsubsw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 f4          	vpsubusb %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d8 31          	vpsubusb \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 f4          	vpsubusw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 d9 31          	vpsubusw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 f4          	vpunpckhbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 68 31          	vpunpckhbw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 f4          	vpunpckhwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 69 31          	vpunpckhwd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a f4          	vpunpckhdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6a 31          	vpunpckhdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d f4          	vpunpckhqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6d 31          	vpunpckhqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 f4          	vpunpcklbw %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 60 31          	vpunpcklbw \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 f4          	vpunpcklwd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 61 31          	vpunpcklwd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 f4          	vpunpckldq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 62 31          	vpunpckldq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c f4          	vpunpcklqdq %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 6c 31          	vpunpcklqdq \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef f4          	vpxor  %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 ef 31          	vpxor  \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c f4          	vsubpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 5c 31          	vsubpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c f4          	vsubps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 5c 31          	vsubps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 f4          	vunpckhpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 15 31          	vunpckhpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 f4          	vunpckhps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 15 31          	vunpckhps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 f4          	vunpcklpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 14 31          	vunpcklpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 f4          	vunpcklps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 14 31          	vunpcklps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 f4          	vxorpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 57 31          	vxorpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 f4          	vxorps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 57 31          	vxorps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc f4       	vaesenc %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dc 31       	vaesenc \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd f4       	vaesenclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 dd 31       	vaesenclast \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de f4       	vaesdec %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 de 31       	vaesdec \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df f4       	vaesdeclast %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 49 df 31       	vaesdeclast \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 00       	vcmpeqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 00       	vcmpeqpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 00       	vcmpeqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 00       	vcmpeqps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 01       	vcmpltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 01       	vcmpltpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 01       	vcmpltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 01       	vcmpltps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 02       	vcmplepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 02       	vcmplepd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 02       	vcmpleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 02       	vcmpleps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 03       	vcmpunordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 03       	vcmpunordpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 03       	vcmpunordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 03       	vcmpunordps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 04       	vcmpneqpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 04       	vcmpneqpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 04       	vcmpneqps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 04       	vcmpneqps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 05       	vcmpnltpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 05       	vcmpnltpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 05       	vcmpnltps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 05       	vcmpnltps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 06       	vcmpnlepd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 06       	vcmpnlepd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 06       	vcmpnleps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 06       	vcmpnleps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 07       	vcmpordpd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 07       	vcmpordpd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 07       	vcmpordps %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 07       	vcmpordps \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df f4 64    	vaeskeygenassist \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 df 31 64    	vaeskeygenassist \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 f4 64    	vpcmpestri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 61 31 64    	vpcmpestri \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 f4 64    	vpcmpestrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 60 31 64    	vpcmpestrm \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 f4 64    	vpcmpistri \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 63 31 64    	vpcmpistri \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 f4 64    	vpcmpistrm \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 62 31 64    	vpcmpistrm \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 f4 64       	vpshufd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 70 31 64       	vpshufd \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 f4 64       	vpshufhw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa 70 31 64       	vpshufhw \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 f4 64       	vpshuflw \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fb 70 31 64       	vpshuflw \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 f4 64    	vroundpd \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 09 31 64    	vroundpd \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 f4 64    	vroundps \$0x64,%xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 79 08 31 64    	vroundps \$0x64,\(%rcx\),%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d f4 64    	vblendpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0d 31 64    	vblendpd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c f4 64    	vblendps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0c 31 64    	vblendps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 f4 64       	vcmppd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c2 31 64       	vcmppd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 f4 64       	vcmpps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c2 31 64       	vcmpps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 f4 64    	vdppd  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 41 31 64    	vdppd  \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 f4 64    	vdpps  \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 40 31 64    	vdpps  \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 f4 64    	vmpsadbw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 42 31 64    	vmpsadbw \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f f4 64    	vpalignr \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0f 31 64    	vpalignr \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e f4 64    	vpblendw \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0e 31 64    	vpblendw \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 f4 64    	vpclmulqdq \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 44 31 64    	vpclmulqdq \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 f4 64       	vshufpd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c9 c6 31 64       	vshufpd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 f4 64       	vshufps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 c8 c6 31 64       	vshufps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b f4 00    	vblendvpd %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4b 31 00    	vblendvpd %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a f4 00    	vblendvps %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4a 31 00    	vblendvps %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c f4 00    	vpblendvb %xmm0,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 4c 31 00    	vpblendvb %xmm0,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f f4          	vcomisd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 f9 2f 21          	vcomisd \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 fa e6 f4          	vcvtdq2pd %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c5 fa e6 21          	vcvtdq2pd \(%rcx\),%xmm4
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
[ 	]*[a-f0-9]+:	c4 e1 db 2a e1       	vcvtsi2sd %rcx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 db 2a 21       	vcvtsi2sdq \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 da 2a e1       	vcvtsi2ss %rcx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e1 da 2a 21       	vcvtsi2ssq \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 d9 22 e1 64    	vpinsrq \$0x64,%rcx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 d9 22 21 64    	vpinsrq \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 f9 16 e1 64    	vpextrq \$0x64,%xmm4,%rcx
[ 	]*[a-f0-9]+:	c4 e3 f9 16 21 64    	vpextrq \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 12 21          	vmovlpd \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 12 21          	vmovlps \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 16 21          	vmovhpd \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d8 16 21          	vmovhps \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 cb c2 f4 64       	vcmpsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 64       	vcmpsd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b f4 64    	vroundsd \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0b 31 64    	vroundsd \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 f4          	vaddsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 58 31          	vaddsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a f4          	vcvtsd2ss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5a 31          	vcvtsd2ss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e f4          	vdivsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5e 31          	vdivsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f f4          	vmaxsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5f 31          	vmaxsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d f4          	vminsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5d 31          	vminsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 f4          	vmulsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 59 31          	vmulsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 f4          	vsqrtsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 51 31          	vsqrtsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c f4          	vsubsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb 5c 31          	vsubsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 00       	vcmpeqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 00       	vcmpeqsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 01       	vcmpltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 01       	vcmpltsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 02       	vcmplesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 02       	vcmplesd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 03       	vcmpunordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 03       	vcmpunordsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 04       	vcmpneqsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 04       	vcmpneqsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 05       	vcmpnltsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 05       	vcmpnltsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 06       	vcmpnlesd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 06       	vcmpnlesd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 f4 07       	vcmpordsd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 cb c2 31 07       	vcmpordsd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 f4          	vaddss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 58 31          	vaddss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a f4          	vcvtss2sd %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5a 31          	vcvtss2sd \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e f4          	vdivss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5e 31          	vdivss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f f4          	vmaxss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5f 31          	vmaxss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d f4          	vminss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5d 31          	vminss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 f4          	vmulss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 59 31          	vmulss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 f4          	vrcpss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 53 31          	vrcpss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 f4          	vrsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 52 31          	vrsqrtss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 f4          	vsqrtss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 51 31          	vsqrtss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c f4          	vsubss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca 5c 31          	vsubss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 00       	vcmpeqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 00       	vcmpeqss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 01       	vcmpltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 01       	vcmpltss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 02       	vcmpless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 02       	vcmpless \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 03       	vcmpunordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 03       	vcmpunordss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 04       	vcmpneqss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 04       	vcmpneqss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 05       	vcmpnltss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 05       	vcmpnltss \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 06       	vcmpnless %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 06       	vcmpnless \(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 f4 07       	vcmpordss %xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 07       	vcmpordss \(%rcx\),%xmm6,%xmm6
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
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 64    	vextractps \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 16 e1 64    	vpextrd \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 16 21 64    	vpextrd \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 79 17 e1 64    	vextractps \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 17 21 64    	vextractps \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 db 2a e1          	vcvtsi2sd %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 db 2a 21          	vcvtsi2sdl \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a e1          	vcvtsi2ss %ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 da 2a 21          	vcvtsi2ssl \(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 ca c2 f4 64       	vcmpss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 ca c2 31 64       	vcmpss \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 f4 64    	vinsertps \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 21 31 64    	vinsertps \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a f4 64    	vroundss \$0x64,%xmm4,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e3 49 0a 31 64    	vroundss \$0x64,\(%rcx\),%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 f4       	vpmovsxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 22 21       	vpmovsxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 32 f4       	vpmovzxbq %xmm4,%xmm6
[ 	]*[a-f0-9]+:	c4 e2 79 32 21       	vpmovzxbq \(%rcx\),%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 15 21 64    	vpextrw \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c5 d9 c4 e1 64       	vpinsrw \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 21 64       	vpinsrw \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 e1 64       	vpinsrw \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 d9 c4 21 64       	vpinsrw \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 64    	vpextrb \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%rcx\),%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c5 f9 c5 cc 64       	vpextrw \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 e1 64    	vpextrb \$0x64,%xmm4,%ecx
[ 	]*[a-f0-9]+:	c4 e3 79 14 21 64    	vpextrb \$0x64,%xmm4,\(%rcx\)
[ 	]*[a-f0-9]+:	c4 e3 59 20 e1 64    	vpinsrb \$0x64,%ecx,%xmm4,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 59 20 21 64    	vpinsrb \$0x64,\(%rcx\),%xmm4,%xmm4
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
