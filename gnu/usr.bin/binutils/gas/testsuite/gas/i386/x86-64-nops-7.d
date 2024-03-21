#source: nops-7.s
#objdump: -drw
#name: x86-64 nops 7

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	e9 f9 01 00 00       	jmp    200 <func1>
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
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 2e 0f 1f 84 00 00 00 00 00 	cs nopw 0x0\(%rax,%rax,1\)

0+200 <func1>:
 +[a-f0-9]+:	31 db                	xor    %ebx,%ebx
 +[a-f0-9]+:	e9 f9 00 00 00       	jmp    300 <func2>
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
 +[a-f0-9]+:	0f 1f 80 00 00 00 00 	nopl   0x0\(%rax\)

0+300 <func2>:
 +[a-f0-9]+:	31 db                	xor    %ebx,%ebx
 +[a-f0-9]+:	eb 7c                	jmp    380 <func3>
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
 +[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)

0+380 <func3>:
 +[a-f0-9]+:	31 c9                	xor    %ecx,%ecx
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	0f 1f 80 00 00 00 00 	nopl   0x0\(%rax\)

0+3c0 <func4>:
 +[a-f0-9]+:	31 d2                	xor    %edx,%edx
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nopw 0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	0f 1f 84 00 00 00 00 00 	nopl   0x0\(%rax,%rax,1\)

0+3e0 <func5>:
 +[a-f0-9]+:	31 ff                	xor    %edi,%edi
#pass
