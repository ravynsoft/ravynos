#objdump: -dw
#name: i386 movbe

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  \(%ecx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  %bx,\(%ecx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  %ebx,\(%ecx\)
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  \(%ecx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  %bx,\(%ecx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  %ebx,\(%ecx\)
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  \(%ecx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  %bx,\(%ecx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  %ebx,\(%ecx\)
#pass
