#objdump: -dw
#name: x86-64 movbe

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 45 0f 38 f0 29    	movbe  \(%r9\),%r13w
[ 	]*[a-f0-9]+:	45 0f 38 f0 29       	movbe  \(%r9\),%r13d
[ 	]*[a-f0-9]+:	4d 0f 38 f0 29       	movbe  \(%r9\),%r13
[ 	]*[a-f0-9]+:	66 45 0f 38 f1 29    	movbe  %r13w,\(%r9\)
[ 	]*[a-f0-9]+:	45 0f 38 f1 29       	movbe  %r13d,\(%r9\)
[ 	]*[a-f0-9]+:	4d 0f 38 f1 29       	movbe  %r13,\(%r9\)
[ 	]*[a-f0-9]+:	66 45 0f 38 f0 29    	movbe  \(%r9\),%r13w
[ 	]*[a-f0-9]+:	45 0f 38 f0 29       	movbe  \(%r9\),%r13d
[ 	]*[a-f0-9]+:	4d 0f 38 f0 29       	movbe  \(%r9\),%r13
[ 	]*[a-f0-9]+:	66 45 0f 38 f1 29    	movbe  %r13w,\(%r9\)
[ 	]*[a-f0-9]+:	45 0f 38 f1 29       	movbe  %r13d,\(%r9\)
[ 	]*[a-f0-9]+:	4d 0f 38 f1 29       	movbe  %r13,\(%r9\)
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbe  \(%rcx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbe  \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	48 0f 38 f0 19       	movbe  \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbe  %bx,\(%rcx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbe  %ebx,\(%rcx\)
[ 	]*[a-f0-9]+:	48 0f 38 f1 19       	movbe  %rbx,\(%rcx\)
#pass
