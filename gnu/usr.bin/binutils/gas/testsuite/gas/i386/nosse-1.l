.*: Assembler messages:
.*:4: Error: .*generic.*
.*:8: Error: .*nosse.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \.nosse
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch generic32
[ 	]*4[ 	]+lfence
[ 	]*5[ 	]+\.arch \.sse2
[ 	]*6[ 	]+\?\?\?\? 0FAEE8   		lfence
[ 	]*7[ 	]+\.arch \.nosse
[ 	]*8[ 	]+lfence
#pass
