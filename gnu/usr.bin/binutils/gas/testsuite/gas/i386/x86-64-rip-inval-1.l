.*: Assembler messages:
.*:3: Error:.* (0x)?ffffffff7fffffff .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+_start:
[ 	]*3[ 	]+\?\?\?\? 488B05FF 		movq test1\(%rip\), %rax
[ 	]*3[ 	]+FFFF7F
[ 	]*4[ 	]+\.set test1, \. - 0x80000001
#pass
