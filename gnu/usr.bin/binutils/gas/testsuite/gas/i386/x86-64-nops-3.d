#source: nops-3.s
#as: -mtune=generic64
#objdump: -drw
#name: x86-64 nops 3

.*: +file format .*


Disassembly of section .text:

0+ <nop>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	66 0f 1f 84 00 00 00 00 00 	nopw   0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	89 c3                	mov    %eax,%ebx
[ 	]*[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
#pass
