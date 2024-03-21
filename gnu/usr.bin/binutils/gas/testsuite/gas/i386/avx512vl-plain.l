.*: Assembler messages:
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.arch generic32
[ 	]*3[ 	]+\.arch \.avx512vl
[ 	]*4[ 	]+_start:
[ 	]*5[ 	]+\{evex\} vaesenc[ 	]+%ymm1, %ymm2, %ymm3
[ 	]*6[ 	]+vgf2p8mulb[ 	]+%ymm1, %ymm2, %ymm3\{%k4\}
[ 	]*7[ 	]+\{evex\} vpclmulqdq[ 	]+\$0, %ymm1, %ymm2, %ymm3
[ 	]*8[ 	]*
[ 	]*9[ 	]+\.arch \.vaes
[ 	]*10[ 	]+\?\?\?\? 62F26D28[ 	]+\{evex\} vaesenc[ 	]+%ymm1, %ymm2, %ymm3
[ 	]*10[ 	]+DCD9
[ 	]*11[ 	]*
[ 	]*12[ 	]+\.arch \.gfni
[ 	]*13[ 	]+\?\?\?\? 62F26D2C[ 	]+vgf2p8mulb[ 	]+%ymm1, %ymm2, %ymm3\{%k4\}
[ 	]*13[ 	]+CFD9
[ 	]*14[ 	]*
[ 	]*15[ 	]+\.arch \.vpclmulqdq
[ 	]*16[ 	]+\?\?\?\? 62F36D28[ 	]+\{evex\} vpclmulqdq[ 	]+\$0, %ymm1, %ymm2, %ymm3
[ 	]*16[ 	]+44D900
[ 	]*17[ 	]+\?\?\?\? 00000000 		\.p2align 4,0
#...
