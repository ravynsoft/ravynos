#source: x86-64-lfence-ret.s
#as: -mlfence-before-ret=or
#objdump: -dw -Mintel64
#name: x86-64 -mlfence-before-ret=or

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 c3                	data16 ret
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 c2 14 00          	data16 ret \$0x14
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	c3                   	ret
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	c2 1e 00             	ret    \$0x1e
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 48 c3             	data16 rex\.W ret
 +[a-f0-9]+:	48 83 0c 24 00       	orq    \$0x0,\(%rsp\)
 +[a-f0-9]+:	0f ae e8             	lfence
 +[a-f0-9]+:	66 48 c2 28 00       	data16 rex\.W ret \$0x28
#pass
