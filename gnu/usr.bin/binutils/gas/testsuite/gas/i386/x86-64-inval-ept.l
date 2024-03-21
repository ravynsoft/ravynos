.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:14: Error: .*
.*:15: Error: .*
.*:16: Error: .*
.*:17: Error: .*
.*:18: Error: .*
.*:19: Error: .*
.*:20: Error: .*
.*:21: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal EPT instructions in 64bit mode\.
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+invept	\(%rcx\), %bx
[ 	]*5[ 	]+invept	\(%rcx\), %ebx
[ 	]*6[ 	]+invept	%rbx, \(%rcx\)
[ 	]*7[ 	]+invept	%rbx, %rcx
[ 	]*8[ 	]+invvpid	\(%rcx\), %bx
[ 	]*9[ 	]+invvpid	\(%rcx\), %ebx
[ 	]*10[ 	]+invvpid %rbx, \(%rcx\)
[ 	]*11[ 	]+invvpid	%rbx, %rcx
[ 	]*12[ 	]+
[ 	]*13[ 	]+\.intel_syntax noprefix
[ 	]*14[ 	]+invept bx, oword ptr \[rcx\]
[ 	]*15[ 	]+invept ebx, oword ptr \[rcx\]
[ 	]*16[ 	]+invept oword ptr \[rcx\], rbx
[ 	]*17[ 	]+invept rcx, rbx
[ 	]*18[ 	]+invvpid bx, oword ptr \[rcx\]
[ 	]*19[ 	]+invvpid ebx, oword ptr \[rcx\]
[ 	]*20[ 	]+invvpid oword ptr \[rcx\], rbx
[ 	]*21[ 	]+invvpid rcx, rbx
