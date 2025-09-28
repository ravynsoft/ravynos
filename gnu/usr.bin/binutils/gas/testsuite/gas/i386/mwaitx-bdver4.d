#source: mwaitx.s
#as: -march=bdver4
#objdump: -dw
#name: i386 monitorx and mwaitx insn

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %eax,%ecx,%edx
[       ]*[a-f0-9]+:	67 0f 01 fa          	monitorx %ax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
#pass
