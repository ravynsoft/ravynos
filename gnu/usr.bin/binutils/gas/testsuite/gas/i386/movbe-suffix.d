#objdump: -dwMsuffix
#name: i386 movbe w/ suffix
#source: movbe.s

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbew \(%ecx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbel \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbew %bx,\(%ecx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbel %ebx,\(%ecx\)
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbew \(%ecx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbel \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbew %bx,\(%ecx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbel %ebx,\(%ecx\)
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbew \(%ecx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbel \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbew %bx,\(%ecx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbel %ebx,\(%ecx\)
#pass
