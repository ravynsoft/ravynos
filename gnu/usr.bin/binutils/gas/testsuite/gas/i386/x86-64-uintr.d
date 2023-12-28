#as:
#objdump: -dw
#name: x86_64 UINTR insns
#source: x86-64-uintr.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f3 0f 01 ec          	uiret
 +[a-f0-9]+:	f3 0f 01 ed          	testui
 +[a-f0-9]+:	f3 0f 01 ee          	clui
 +[a-f0-9]+:	f3 0f 01 ef          	stui
 +[a-f0-9]+:	f3 0f c7 f0          	senduipi %rax
 +[a-f0-9]+:	f3 41 0f c7 f2       	senduipi %r10
#pass
