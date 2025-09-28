.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:11: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal INVPCID instructions
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+foo:
[ 	]*4[ 	]+invpcid	\(%ecx\), %bx
[ 	]*5[ 	]+invpcid %ebx, \(%ecx\)
[ 	]*6[ 	]+invpcid	%ebx, %ecx
[ 	]*7[ 	]+
[ 	]*8[ 	]+\.intel_syntax noprefix
[ 	]*9[ 	]+invpcid bx, \[ecx\]
[ 	]*10[ 	]+invpcid \[ecx\], ebx
[ 	]*11[ 	]+invpcid ecx, ebx
