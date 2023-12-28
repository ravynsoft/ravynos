.*: Assembler messages:
.*:7: Error: .*
.*:10: Error: .*
.*:14: Error: .*
.*:17: Error: .*
.*:20: Error: .*
.*:23: Error: .*
.*:24: Error: .*
.*:30: Error: .*
.*:33: Error: .*
.*:36: Error: .*
.*:39: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.include "sse-check\.s"
[ 	]*1[ 	]+\# Check SSE instructions
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.text
[ 	]*4[ 	]+_start:
[ 	]*5[ 	]+
[ 	]*6[ 	]+\# SSE instruction
[ 	]*7[ 	]+\?\?\?\? 0F58CA   		addps %xmm2,%xmm1
.*  Error: SSE instruction `addps' is used
[ 	]*8[ 	]+
[ 	]*9[ 	]+\# SSE2 instruction
[ 	]*10[ 	]+\?\?\?\? 660F58CA 		addpd %xmm2,%xmm1
.*  Error: SSE instruction `addpd' is used
[ 	]*11[ 	]+
[ 	]*[0-9]+[ 	]+# special case SSE2 instruction
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F2ACA 		cvtpi2pd %mm2,%xmm1
[ 	]*[0-9]+[ 	]+\?\?\?\? 67660F2A 		cvtpi2pd \(%edx\),%xmm1
.*  Error: SSE instruction `cvtpi2pd' is used
[ 	]*[0-9]+[ 	]+0A
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# SSE3 instruction
[ 	]*[0-9]+[ 	]+\?\?\?\? 660FD0CA 		addsubpd %xmm2,%xmm1
.*  Error: SSE instruction `addsubpd' is used
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# SSSE3 instruction
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F3801 		phaddw %xmm2,%xmm1
.*  Error: SSE instruction `phaddw' is used
[ 	]*[0-9]+[ 	]+CA
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# SSE4 instructions
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F3815 		blendvpd %xmm0,%xmm1,%xmm0
.*  Error: SSE instruction `blendvpd' is used
[ 	]*[0-9]+[ 	]+C1
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F3837 		pcmpgtq %xmm1,%xmm0
.*  Error: SSE instruction `pcmpgtq' is used
[ 	]*[0-9]+[ 	]+C1
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# SSE4a instruction.*
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F78C0 		extrq \$0, \$0, %xmm0
[ 	]*[0-9]+[ 	]+0000
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# PCMUL instruction
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F3A44 		pclmulqdq \$-1,%xmm1,%xmm2
.*  Error: SSE instruction `pclmulqdq' is used
[ 	]*[0-9]+[ 	]+D1FF
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# AES instructions
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F38DE 		aesdec %xmm1,%xmm2
.*  Error: SSE instruction `aesdec' is used
[ 	]*[0-9]+[ 	]+D1
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# SHA instruction
[ 	]*[0-9]+[ 	]+\?\?\?\? 0F38C8C0 		sha1nexte %xmm0, %xmm0
.*  Error: SSE instruction `sha1nexte' is used
[ 	]*[0-9]+[ 	]+
[ 	]*[0-9]+[ 	]+\# GFNI instructions
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F38CF 		gf2p8mulb %xmm1,%xmm2
GAS LISTING .*


.*  Error: SSE instruction `gf2p8mulb' is used
[ 	]*[0-9]+[ 	]+D1
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D09 		vgf2p8mulb %xmm0, %xmm0, %xmm0\{%k1\}
[ 	]*[0-9]+[ 	]+CFC0
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 		vgf2p8mulb %zmm0, %zmm0, %zmm0
[ 	]*[0-9]+[ 	]+CFC0

