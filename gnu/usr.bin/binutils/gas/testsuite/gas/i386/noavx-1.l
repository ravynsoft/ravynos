.*: Assembler messages:
.*:4: Error: .*generic.*
.*:8: Error: .*noavx.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \.noavx
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch generic32
[ 	]*4[ 	]+vzeroupper
[ 	]*5[ 	]+\.arch \.avx
[ 	]*6[ 	]+\?\?\?\? C5F877   		vzeroupper
[ 	]*7[ 	]+\.arch \.noavx
[ 	]*8[ 	]+vzeroupper
#pass
