#source: align-branch-6.s
#as: -mbranches-within-32B-boundaries -D
#objdump: -dw
#warning_output: align-branch-6.e

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	0f 1f 80 00 00 00 00 	nopl   0x0\(%rax\)
 +[a-f0-9]+:	f2 73 bf             	bnd jae 0 <_start>
 +[a-f0-9]+:	c3                   	ret
#pass
