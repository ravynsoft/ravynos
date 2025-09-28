#source: x86-64-lfence-indbr.s
#as: -mlfence-before-indirect-branch=all
#warning_output: x86-64-lfence-indbr.e
#objdump: -dw
#name: x86-64 -mlfence-before-indirect-branch=all

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff d2                	call   \*%rdx
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	ff e2                	jmp    \*%rdx
 +[a-f0-9]+:	ff 12                	call   \*\(%rdx\)
 +[a-f0-9]+:	ff 22                	jmp    \*\(%rdx\)
 +[a-f0-9]+:	ff 14 25 00 00 00 00 	call   \*0x0
 +[a-f0-9]+:	ff 24 25 00 00 00 00 	jmp    \*0x0
#pass
