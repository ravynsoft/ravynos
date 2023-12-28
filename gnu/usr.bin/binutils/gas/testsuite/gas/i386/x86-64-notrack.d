#objdump: -dw
#name: x86-64 NOTRACK prefix

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	3e ff d0             	notrack call \*%rax
[ 	]*[a-f0-9]+:	3e 41 ff d0          	notrack call \*%r8
[ 	]*[a-f0-9]+:	3e ff e0             	notrack jmp \*%rax
[ 	]*[a-f0-9]+:	3e 41 ff e0          	notrack jmp \*%r8
[ 	]*[a-f0-9]+:	3e ff 10             	notrack call \*\(%rax\)
[ 	]*[a-f0-9]+:	3e 41 ff 10          	notrack call \*\(%r8\)
[ 	]*[a-f0-9]+:	3e ff 20             	notrack jmp \*\(%rax\)
[ 	]*[a-f0-9]+:	3e 41 ff 20          	notrack jmp \*\(%r8\)
[ 	]*[a-f0-9]+:	3e 67 ff 10          	notrack call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 41 ff 10       	notrack call \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e 67 ff 20          	notrack jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 41 ff 20       	notrack jmp \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%rax
[ 	]*[a-f0-9]+:	3e f2 41 ff d0       	notrack bnd call \*%r8
[ 	]*[a-f0-9]+:	3e f2 ff e0          	notrack bnd jmp \*%rax
[ 	]*[a-f0-9]+:	3e f2 41 ff e0       	notrack bnd jmp \*%r8
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%rax\)
[ 	]*[a-f0-9]+:	3e f2 41 ff 10       	notrack bnd call \*\(%r8\)
[ 	]*[a-f0-9]+:	3e f2 ff 20          	notrack bnd jmp \*\(%rax\)
[ 	]*[a-f0-9]+:	3e f2 41 ff 20       	notrack bnd jmp \*\(%r8\)
[ 	]*[a-f0-9]+:	3e 67 f2 ff 10       	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 f2 41 ff 10    	notrack bnd call \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e 67 f2 ff 20       	notrack bnd jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 f2 41 ff 20    	notrack bnd jmp \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%rax
[ 	]*[a-f0-9]+:	3e f2 41 ff d0       	notrack bnd call \*%r8
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%rax\)
[ 	]*[a-f0-9]+:	3e f2 41 ff 10       	notrack bnd call \*\(%r8\)
[ 	]*[a-f0-9]+:	3e 67 f2 ff 10       	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 f2 41 ff 10    	notrack bnd call \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e ff d0             	notrack call \*%rax
[ 	]*[a-f0-9]+:	3e 41 ff d0          	notrack call \*%r8
[ 	]*[a-f0-9]+:	3e ff e0             	notrack jmp \*%rax
[ 	]*[a-f0-9]+:	3e 41 ff e0          	notrack jmp \*%r8
[ 	]*[a-f0-9]+:	3e ff 10             	notrack call \*\(%rax\)
[ 	]*[a-f0-9]+:	3e 41 ff 10          	notrack call \*\(%r8\)
[ 	]*[a-f0-9]+:	3e ff 20             	notrack jmp \*\(%rax\)
[ 	]*[a-f0-9]+:	3e 41 ff 20          	notrack jmp \*\(%r8\)
[ 	]*[a-f0-9]+:	3e 67 ff 10          	notrack call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 41 ff 10       	notrack call \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e 67 ff 20          	notrack jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 41 ff 20       	notrack jmp \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%rax
[ 	]*[a-f0-9]+:	3e f2 41 ff d0       	notrack bnd call \*%r8
[ 	]*[a-f0-9]+:	3e f2 ff e0          	notrack bnd jmp \*%rax
[ 	]*[a-f0-9]+:	3e f2 41 ff e0       	notrack bnd jmp \*%r8
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%rax\)
[ 	]*[a-f0-9]+:	3e f2 41 ff 10       	notrack bnd call \*\(%r8\)
[ 	]*[a-f0-9]+:	3e f2 ff 20          	notrack bnd jmp \*\(%rax\)
[ 	]*[a-f0-9]+:	3e f2 41 ff 20       	notrack bnd jmp \*\(%r8\)
[ 	]*[a-f0-9]+:	3e 67 f2 ff 10       	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 f2 41 ff 10    	notrack bnd call \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e 67 f2 ff 20       	notrack bnd jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 f2 41 ff 20    	notrack bnd jmp \*\(%r8d\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%rax
[ 	]*[a-f0-9]+:	3e f2 41 ff d0       	notrack bnd call \*%r8
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%rax\)
[ 	]*[a-f0-9]+:	3e f2 41 ff 10       	notrack bnd call \*\(%r8\)
[ 	]*[a-f0-9]+:	3e 67 f2 ff 10       	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 67 f2 41 ff 10    	notrack bnd call \*\(%r8d\)
[ 	]*[a-f0-9]+:	f2 3e ff d0          	bnd notrack call \*%rax
[ 	]*[a-f0-9]+:	3e 66 ff d0          	ds call \*%ax
[ 	]*[a-f0-9]+:	66 3e ff d0          	ds call \*%ax
#pass
