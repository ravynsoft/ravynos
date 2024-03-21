.*: Assembler messages:
.*:6: Error: .*junk.*
.*:7: Error: .*junk.*
.*:8: Error: .*junk.*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.intel_syntax noprefix
[ 	]*3[ 	]+\.arch \.no87
[ 	]*4[ 	]+stN:
[ 	]*5[ 	]+\?\?\?\? A1000000[ 	]+mov	eax, st
[ 	]*5[ 	]+00
[ 	]*6[ 	]+mov	eax, st\(7\)
[ 	]*7[ 	]+mov	eax, st \( 7 \)
[ 	]*8[ 	]+mov	eax, x\(7\)
[ 	]*9[ 	]+\?\?\?\? 00000000 		\.p2align 4,0
#...
