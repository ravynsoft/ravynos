.*: Assembler messages:
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.arch generic32
[ 	]*3[ 	]+\.arch \.avx512f
[ 	]*4[ 	]+_start:
[ 	]*5[ 	]+vaesenc[ 	]+%zmm1, %zmm2, %zmm3
[ 	]*6[ 	]+vgf2p8mulb[ 	]+%zmm1, %zmm2, %zmm3
[ 	]*7[ 	]+vpclmulqdq[ 	]+\$0, %zmm1, %zmm2, %zmm3
[ 	]*8[ 	]*
[ 	]*9[ 	]+\.arch \.vaes
[ 	]*10[ 	]+\?\?\?\? 62F26D48[ 	]+vaesenc[ 	]+%zmm1, %zmm2, %zmm3
[ 	]*10[ 	]+DCD9
[ 	]*11[ 	]*
[ 	]*12[ 	]+\.arch \.gfni
[ 	]*13[ 	]+\?\?\?\? 62F26D48[ 	]+vgf2p8mulb[ 	]+%zmm1, %zmm2, %zmm3
[ 	]*13[ 	]+CFD9
[ 	]*14[ 	]*
[ 	]*15[ 	]+\.arch \.vpclmulqdq
[ 	]*16[ 	]+\?\?\?\? 62F36D48[ 	]+vpclmulqdq[ 	]+\$0, %zmm1, %zmm2, %zmm3
[ 	]*16[ 	]+44D900
[ 	]*17[ 	]+\?\?\?\? 00000000 		\.p2align 4,0
#...
