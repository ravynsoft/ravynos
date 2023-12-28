.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:14: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# 64-bit only invalid MOVSXD with Intel64 ISA
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+movslq	%eax, %cx
[ 	]*5[ 	]+movslq	%eax, %ecx
[ 	]*6[ 	]+movslq	\(%rax\), %ecx
[ 	]*7[ 	]+movsxd	%ax, %ecx
[ 	]*8[ 	]+
[ 	]*9[ 	]+\.intel_syntax noprefix
[ 	]*10[ 	]+movslq	cx, ax
[ 	]*11[ 	]+movslq	ecx, eax
[ 	]*12[ 	]+movslq	ecx, \[rax\]
[ 	]*13[ 	]+movsxd	cx, eax
[ 	]*14[ 	]+movsxd	cx, DWORD PTR \[rax\]
