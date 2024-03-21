#objdump: -dwMsuffix
#name: x86-64 movbe w/ suffix
#source: x86-64-movbe.s

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 45 0f 38 f0 29    	movbew \(%r9\),%r13w
[ 	]*[a-f0-9]+:	45 0f 38 f0 29       	movbel \(%r9\),%r13d
[ 	]*[a-f0-9]+:	4d 0f 38 f0 29       	movbeq \(%r9\),%r13
[ 	]*[a-f0-9]+:	66 45 0f 38 f1 29    	movbew %r13w,\(%r9\)
[ 	]*[a-f0-9]+:	45 0f 38 f1 29       	movbel %r13d,\(%r9\)
[ 	]*[a-f0-9]+:	4d 0f 38 f1 29       	movbeq %r13,\(%r9\)
[ 	]*[a-f0-9]+:	66 45 0f 38 f0 29    	movbew \(%r9\),%r13w
[ 	]*[a-f0-9]+:	45 0f 38 f0 29       	movbel \(%r9\),%r13d
[ 	]*[a-f0-9]+:	4d 0f 38 f0 29       	movbeq \(%r9\),%r13
[ 	]*[a-f0-9]+:	66 45 0f 38 f1 29    	movbew %r13w,\(%r9\)
[ 	]*[a-f0-9]+:	45 0f 38 f1 29       	movbel %r13d,\(%r9\)
[ 	]*[a-f0-9]+:	4d 0f 38 f1 29       	movbeq %r13,\(%r9\)
[ 	]*[a-f0-9]+:	66 0f 38 f0 19       	movbew \(%rcx\),%bx
[ 	]*[a-f0-9]+:	0f 38 f0 19          	movbel \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	48 0f 38 f0 19       	movbeq \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	66 0f 38 f1 19       	movbew %bx,\(%rcx\)
[ 	]*[a-f0-9]+:	0f 38 f1 19          	movbel %ebx,\(%rcx\)
[ 	]*[a-f0-9]+:	48 0f 38 f1 19       	movbeq %rbx,\(%rcx\)
#pass
