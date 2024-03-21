#objdump: -dw
#name: x86-64 RdRnd

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f c7 f3          	rdrand %bx
[ 	]*[a-f0-9]+:	0f c7 f3             	rdrand %ebx
[ 	]*[a-f0-9]+:	48 0f c7 f3          	rdrand %rbx
[ 	]*[a-f0-9]+:	66 41 0f c7 f0       	rdrand %r8w
[ 	]*[a-f0-9]+:	41 0f c7 f0          	rdrand %r8d
[ 	]*[a-f0-9]+:	49 0f c7 f0          	rdrand %r8
[ 	]*[a-f0-9]+:	66 0f c7 f3          	rdrand %bx
[ 	]*[a-f0-9]+:	0f c7 f3             	rdrand %ebx
[ 	]*[a-f0-9]+:	48 0f c7 f3          	rdrand %rbx
[ 	]*[a-f0-9]+:	66 41 0f c7 f0       	rdrand %r8w
[ 	]*[a-f0-9]+:	41 0f c7 f0          	rdrand %r8d
[ 	]*[a-f0-9]+:	49 0f c7 f0          	rdrand %r8
#pass
