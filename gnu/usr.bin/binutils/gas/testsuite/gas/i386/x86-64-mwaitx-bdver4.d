#source: x86-64-mwaitx.s
#as: -march=bdver4
#objdump: -dw
#name: x86_64 monitorx and mwaitx insn

.*: +file format .*


Disassembly of section \.text:

0000000000000000 <_start>:
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	67 0f 01 fa          	monitorx %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
#pass
