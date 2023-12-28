.*: Assembler messages:
.*:6: Error: .*
.*:8: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.include "arch-avx-1\.s"
[ 	]*1[ 	]+\# Test -march=
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\# AES \+ AVX
[ 	]*4[ 	]+\?\?\?\? C4E279DC 	vaesenc  \(%ecx\),%xmm0,%xmm2
[ 	]*4[ 	]+11
[ 	]*5[ 	]+\# PCLMUL \+ AVX
[ 	]*6[ 	]+vpclmulqdq \$8,%xmm4,%xmm6,%xmm2
[ 	]*7[ 	]+# GFNI \+ AVX
[ 	]*8[ 	]+vgf2p8mulb %xmm1,%xmm2,%xmm3
