.*: Assembler messages:
.*:2: Error: .*
.*:3: Error: .*
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+rep add %rbx, %rax
[ 	]*3[ 	]+repe add %rbx, %rax
[ 	]*4[ 	]+repz add %rbx, %rax
[ 	]*5[ 	]+repne add %rbx, %rax
[ 	]*6[ 	]+repnz add %rbx, %rax
