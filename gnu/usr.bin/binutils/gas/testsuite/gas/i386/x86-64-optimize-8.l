.*: Assembler messages:
.*:6: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check 64bit instructions with optimized encoding
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.allow_index_reg
[ 	]*4[ 	]+\.text
[ 	]*5[ 	]+_start:
[ 	]*6[ 	]+vmovdqa32	%ymm1, %ymm2
#pass
