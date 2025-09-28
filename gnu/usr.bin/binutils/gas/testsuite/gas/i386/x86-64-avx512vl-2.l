.*: Assembler messages:
.*:5: Error: .*corei7.*
.*:6: Error: .*corei7.*
.*:7: Error: .*corei7.*
.*:9: Error: .*corei7\.avx\.avx512vl.*
.*:10: Error: .*corei7\.avx\.avx512vl.*
.*:11: Error: .*corei7\.avx\.avx512vl.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.arch corei7
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+\.arch \.avx
[ 	]*5[ 	]+vpconflictd	%xmm0, %xmm25 
[ 	]*6[ 	]+vpconflictd	%ymm0, %ymm25 
[ 	]*7[ 	]+vpconflictd	%ymm0, %zmm25 
[ 	]*8[ 	]+\.arch \.avx512vl
[ 	]*9[ 	]+vpconflictd	%xmm0, %xmm25 
[ 	]*10[ 	]+vpconflictd	%ymm0, %ymm25 
[ 	]*11[ 	]+vpconflictd	%zmm0, %zmm25 
[ 	]*12[ 	]+\.arch \.avx512cd
[ 	]*13[ 	]+\?\?\?\? 62627D08 		vpconflictd	%xmm0, %xmm25 
[ 	]*13[ 	]+C4C8
[ 	]*14[ 	]+\?\?\?\? 62627D28 		vpconflictd	%ymm0, %ymm25 
[ 	]*14[ 	]+C4C8
[ 	]*15[ 	]+\?\?\?\? 62627D48 		vpconflictd	%zmm0, %zmm25 
[ 	]*15[ 	]+C4C8
#pass
