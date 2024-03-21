.*: Assembler messages:
.*:4: Error: .*generic.*
.*:7: Error: .*mmx.*
.*:10: Error: .*3dnow.*
.*:14: Error: .*nommx.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \.nommx
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch generic32
[ 	]*4[ 	]+emms
[ 	]*5[ 	]+\.arch \.mmx
[ 	]*6[ 	]+\?\?\?\? 0F7EC0   		movd	%mm0, %eax
[ 	]*7[ 	]+femms
[ 	]*8[ 	]+\.arch \.3dnow
[ 	]*9[ 	]+\?\?\?\? 0F0E     		femms
[ 	]*10[ 	]+pswapd	%mm1,%mm0
[ 	]*11[ 	]+\.arch \.3dnowa
[ 	]*12[ 	]+\?\?\?\? 0F0FC1BB 		pswapd	%mm1,%mm0
[ 	]*13[ 	]+\.arch \.nommx
[ 	]*14[ 	]+emms
#pass
