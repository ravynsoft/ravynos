#as: -mlfence-before-indirect-branch=all -mlfence-before-ret=or
#warning_output: lfence-byte.e
#objdump: -dw
#name: -mlfence-before-indirect-branch=all -mlfence-before-ret=or

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	f3 aa                	rep stos %al,%es:\(%edi\)
 +[a-f0-9]+:	83 0c 24 00          	orl    \$0x0,\(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff d0                	call   \*%eax
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	66 66 c3             	data16 retw
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	9b                   	fwait
 +[a-f0-9]+:	83 0c 24 00          	orl    \$0x0,\(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	f3 c3                	repz ret
 +[a-f0-9]+:	c3                   	ret
 +[a-f0-9]+:	f3 ff d0             	repz call \*%eax
#pass
