.*: Assembler messages:
.*:6: Error: .*`bndmov'.*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.intel_syntax noprefix
[ 	]*3[ 	]+\.arch generic32
[ 	]*4[ 	]+bnd:
[ 	]*5[ 	]+\?\?\?\? A1000000[ 	]+mov	eax, bnd0
[ 	]*5[ 	]+00
[ 	]*6[ 	]+bndmov	bnd0, bnd0
[ 	]*7[ 	]+\?\?\?\? 00000000 		\.p2align 4,0
#...
