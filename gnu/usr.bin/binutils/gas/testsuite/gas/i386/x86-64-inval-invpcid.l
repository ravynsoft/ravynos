.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:13: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal 64bit INVPCID instructions
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+invvpid	\(%rcx\), %bx
[ 	]*5[ 	]+invvpid	\(%rcx\), %ebx
[ 	]*6[ 	]+invvpid %rbx, \(%rcx\)
[ 	]*7[ 	]+invvpid	%rbx, %rcx
[ 	]*8[ 	]+
[ 	]*9[ 	]+\.intel_syntax noprefix
[ 	]*10[ 	]+invvpid bx, \[rcx\]
[ 	]*11[ 	]+invvpid ebx, \[rcx\]
[ 	]*12[ 	]+invvpid \[rcx\], rbx
[ 	]*13[ 	]+invvpid rcx, rbx
