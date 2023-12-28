.*: Assembler messages:
.*:4: Error: .*
.*:6: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.include "arch-avx-1\.s"
[ 	]*1[ 	]+\# Test -march=
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\# AES \+ AVX
[ 	]*4[ 	]+vaesenc  \(%ecx\),%xmm0,%xmm2
[ 	]*5[ 	]+\# PCLMUL \+ AVX
[ 	]*6[ 	]+vpclmulqdq \$8,%xmm4,%xmm6,%xmm2
[ 	]*7[ 	]+# GFNI \+ AVX
[ 	]*8[ 	]+\?\?\?\? C4E269CF 	vgf2p8mulb %xmm1,%xmm2,%xmm3
[ 	]*8[ 	]+D9
