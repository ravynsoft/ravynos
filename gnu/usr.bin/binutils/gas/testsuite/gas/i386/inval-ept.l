.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:14: Error: .*
.*:15: Error: .*
.*:16: Error: .*
.*:17: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal EPT instructions
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+invept	\(%ecx\), %bx
[ 	]*5[ 	]+invept	%ebx, \(%ecx\)
[ 	]*6[ 	]+invept	%ebx, %ecx
[ 	]*7[ 	]+invvpid	\(%ecx\), %bx
[ 	]*8[ 	]+invvpid %ebx, \(%ecx\)
[ 	]*9[ 	]+invvpid	%ebx, %ecx
[ 	]*10[ 	]+
[ 	]*11[ 	]+\.intel_syntax noprefix
[ 	]*12[ 	]+invept bx, oword ptr \[ecx\]
[ 	]*13[ 	]+invept oword ptr \[ecx\], ebx
[ 	]*14[ 	]+invept ecx, ebx
[ 	]*15[ 	]+invvpid bx, oword ptr \[ecx\]
[ 	]*16[ 	]+invvpid oword ptr \[ecx\], ebx
[ 	]*17[ 	]+invvpid ecx, ebx
