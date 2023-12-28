.*: Assembler messages:
.*:3: Error:.* (0x)?0*80000000 .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+_start:
[ 	]*3[ 	]+\?\?\?\? 488B0500 		movq test1\(%rip\), %rax
[ 	]*3[ 	]+000080
[ 	]*4[ 	]+\.set test1, \. \+ 0x80000000
#pass
