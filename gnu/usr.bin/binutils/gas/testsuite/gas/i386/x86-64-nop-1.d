#source: nop-1.s
#objdump: -drw
#name: x86-64 .nops 1

.*: +file format .*


Disassembly of section .text:

0+ <single>:
 +[a-f0-9]+:	90                   	nop

0+1 <pseudo_1>:
 +[a-f0-9]+:	90                   	nop

0+2 <pseudo_8>:
 +[a-f0-9]+:	0f 1f 84 00 00 00 00 00 	nopl   0x0\(%rax,%rax,1\)

0+a <pseudo_8_4>:
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)

0+12 <pseudo_20>:
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 0f 1f 84 00 00 00 00 00 	nopw   0x0\(%rax,%rax,1\)

0+26 <pseudo_30>:
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	0f 1f 84 00 00 00 00 00 	nopl   0x0\(%rax,%rax,1\)

0+44 <pseudo_129>:
 +[a-f0-9]+:	eb 7f                	jmp    c5 <end>
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

0+c5 <end>:
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
#pass
