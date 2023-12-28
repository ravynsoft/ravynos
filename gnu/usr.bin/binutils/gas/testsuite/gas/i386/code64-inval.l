.*: Assembler messages:
.*:3: Error: unknown pseudo-op: `.code64'
.*:4: Error: `movabs' is only supported in 64-bit mode
.*:5: Error: `movabs' is only supported in 64-bit mode
GAS LISTING .*


[ 	]*1[ 	]+\.include "code64\.s"
[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+_start:
[ 	]*3[ 	]+\.code64
[ 	]*4[ 	]+movabs  0x87654321, %al
[ 	]*5[ 	]+movabs  \$0x87654321, %rax
