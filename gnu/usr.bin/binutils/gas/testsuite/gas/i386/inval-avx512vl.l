.*: Assembler messages:
.*:5: Error: .*unsupported instruction.*
.*:6: Error: .*unsupported instruction.*
GAS LISTING .*


[ 	]*1[ 	]+# Check illegal AVX512VL instructions
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch \.noavx512vl
[ 	]*4[ 	]+_start:
[ 	]*5[ 	]+vp2intersectd %ymm1, %ymm2, %k3
[ 	]*6[ 	]+vp2intersectq %ymm1, %ymm2, %k3
