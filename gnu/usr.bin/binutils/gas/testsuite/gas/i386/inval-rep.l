.*: Assembler messages:
.*:2: Error: .*
.*:3: Error: .*
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+rep add %ebx, %eax
[ 	]*3[ 	]+repe add %ebx, %eax
[ 	]*4[ 	]+repz add %ebx, %eax
[ 	]*5[ 	]+repne add %ebx, %eax
[ 	]*6[ 	]+repnz add %ebx, %eax
