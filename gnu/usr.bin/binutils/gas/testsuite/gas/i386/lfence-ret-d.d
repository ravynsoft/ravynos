#source: lfence-ret.s
#as: -mlfence-before-ret=shl
#objdump: -dw
#name: -mlfence-before-ret=shl

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	66 c1 24 24 00       	shlw   \$0x0,\(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 c3                	retw
 +[a-f0-9]+:	66 c1 24 24 00       	shlw   \$0x0,\(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 c2 14 00          	retw   \$0x14
 +[a-f0-9]+:	c1 24 24 00          	shll   \$0x0,\(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	c3                   	ret
 +[a-f0-9]+:	c1 24 24 00          	shll   \$0x0,\(%esp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	c2 1e 00             	ret    \$0x1e
#pass
