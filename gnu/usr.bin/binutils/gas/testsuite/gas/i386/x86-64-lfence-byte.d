#as: -mlfence-before-indirect-branch=all -mlfence-before-ret=or
#warning_output: x86-64-lfence-byte.e
#objdump: -dw
#name: x86-64 -mlfence-before-indirect-branch=all -mlfence-before-ret=or

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	f3 aa                	rep stos %al,%es:\(%rdi\)
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff d0                	call   \*%rax
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	66 66 c3             	data16 retw
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	9b                   	fwait
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	c3                   	ret
 +[a-f0-9]+:	f3 ff d0             	repz call \*%rax
#pass
