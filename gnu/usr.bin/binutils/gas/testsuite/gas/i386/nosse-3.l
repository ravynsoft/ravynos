.*: Assembler messages:
.*:3: Error: .*nosse.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test -march=\+nosse
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+lfence
[ 	]*4[ 	]+\?\?\?\? 0F01C8   		monitor
