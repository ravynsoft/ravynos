#source: lfence-indbr.s
#as: -mlfence-before-indirect-branch=memory
#warning_output: lfence-indbr.e
#objdump: -dw
#name: -mlfence-before-indirect-branch=memory

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	ff d2                	call   \*%edx
 +[a-f0-9]+:	ff e2                	jmp    \*%edx
 +[a-f0-9]+:	ff 12                	call   \*\(%edx\)
 +[a-f0-9]+:	ff 22                	jmp    \*\(%edx\)
 +[a-f0-9]+:	ff 15 00 00 00 00    	call   \*0x0
 +[a-f0-9]+:	ff 25 00 00 00 00    	jmp    \*0x0
#pass
