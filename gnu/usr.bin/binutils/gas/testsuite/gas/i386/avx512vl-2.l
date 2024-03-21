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
[ 	]*5[ 	]+vpconflictd	%xmm0, %xmm5 
[ 	]*6[ 	]+vpconflictd	%ymm0, %ymm5 
[ 	]*7[ 	]+vpconflictd	%ymm0, %zmm5 
[ 	]*8[ 	]+\.arch \.avx512vl
[ 	]*9[ 	]+vpconflictd	%xmm0, %xmm5 
[ 	]*10[ 	]+vpconflictd	%ymm0, %ymm5 
[ 	]*11[ 	]+vpconflictd	%zmm0, %zmm5 
[ 	]*12[ 	]+\.arch \.avx512cd
[ 	]*13[ 	]+\?\?\?\? 62F27D08 		vpconflictd	%xmm0, %xmm5 
[ 	]*13[ 	]+C4E8
[ 	]*14[ 	]+\?\?\?\? 62F27D28 		vpconflictd	%ymm0, %ymm5 
[ 	]*14[ 	]+C4E8
[ 	]*15[ 	]+\?\?\?\? 62F27D48 		vpconflictd	%zmm0, %zmm5 
[ 	]*15[ 	]+C4E8
#pass
