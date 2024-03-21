#as: -J
#objdump: -dw -Mintel64
#name: x86-64 branch

.*: +file format .*

Disassembly of section .text:

[0-9a-f]+ <.*>:
[ 	]*[a-f0-9]+:	ff d0                	call   \*%rax
[ 	]*[a-f0-9]+:	ff d0                	call   \*%rax
[ 	]*[a-f0-9]+:	66 ff d0             	data16 call \*%rax
[ 	]*[a-f0-9]+:	66 ff d0             	data16 call \*%rax
[ 	]*[a-f0-9]+:	66 ff 10             	data16 call \*\(%rax\)
[ 	]*[a-f0-9]+:	ff e0                	jmp    \*%rax
[ 	]*[a-f0-9]+:	ff e0                	jmp    \*%rax
[ 	]*[a-f0-9]+:	66 ff e0             	data16 jmp \*%rax
[ 	]*[a-f0-9]+:	66 ff e0             	data16 jmp \*%rax
[ 	]*[a-f0-9]+:	66 ff 20             	data16 jmp \*\(%rax\)
[ 	]*[a-f0-9]+:	e8 (00|5b) 00 (00|10) 00       	call   ((0x)?1f|10007a) <.*>
[ 	]*[a-f0-9]+:	e9 (00|60) 00 (00|10) 00       	jmp    ((0x)?24|100084) <.*>
[ 	]*[a-f0-9]+:	66 e8 00 00 00 00    	data16 call (0x2a|2a <.*>)
[ 	]*[a-f0-9]+:	66 e9 00 00 00 00    	data16 jmp (0x30|30 <.*>)
[ 	]*[a-f0-9]+:	66 0f 82 00 00 00 00 	data16 jb (0x37|37 <.*>)
[ 	]*[a-f0-9]+:	66 c3                	data16 ret
[ 	]*[a-f0-9]+:	66 c2 08 00          	data16 ret \$0x8
[ 	]*[a-f0-9]+:	3e 74 03[ 	]+je,pt  +[0-9a-fx]+ <.*>
[ 	]*[a-f0-9]+:	2e 74 00[ 	]+je,pn  +[0-9a-fx]+ <.*>
[0-9a-f]+ <label>:
[ 	]*[a-f0-9]+:	ff d0                	call   \*%rax
[ 	]*[a-f0-9]+:	ff d0                	call   \*%rax
[ 	]*[a-f0-9]+:	66 ff d0             	data16 call \*%rax
[ 	]*[a-f0-9]+:	66 ff d0             	data16 call \*%rax
[ 	]*[a-f0-9]+:	66 ff 10             	data16 call \*\(%rax\)
[ 	]*[a-f0-9]+:	ff e0                	jmp    \*%rax
[ 	]*[a-f0-9]+:	ff e0                	jmp    \*%rax
[ 	]*[a-f0-9]+:	66 ff e0             	data16 jmp \*%rax
[ 	]*[a-f0-9]+:	66 ff e0             	data16 jmp \*%rax
[ 	]*[a-f0-9]+:	66 ff 20             	data16 jmp \*\(%rax\)
[ 	]*[a-f0-9]+:	e8 .. 00 (00|10) 00       	call   [0-9a-fx]* <.*>
[ 	]*[a-f0-9]+:	e9 .. 00 (00|10) 00       	jmp    [0-9a-fx]* <.*>
[ 	]*[a-f0-9]+:	66 c3                	data16 ret
[ 	]*[a-f0-9]+:	66 c2 08 00          	data16 ret \$0x8
#pass
