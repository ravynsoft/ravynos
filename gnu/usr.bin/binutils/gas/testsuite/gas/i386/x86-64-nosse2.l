.*: Assembler messages:
.*:6: Error: .*paddb.*
.*:7: Error: .*movq.*
.*:8: Error: .*movq.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \.nosse2
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch generic64
[ 	]*4[ 	]+\.arch \.nosse2
[ 	]*5[ 	]+\?\?\?\? 0F58C0   		addps %xmm0, %xmm0
[ 	]*6[ 	]+paddb %xmm0, %xmm0
[ 	]*7[ 	]+movq %xmm0, %rax
[ 	]*8[ 	]+movq %rax, %xmm0
#pass
