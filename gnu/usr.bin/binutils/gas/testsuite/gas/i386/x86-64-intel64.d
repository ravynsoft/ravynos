#as: -mintel64
#objdump: -dw
#name: x86-64 Intel64

.*: +file format .*

Disassembly of section .text:
0+ <_start>:
[ 	]*[a-f0-9]+:	48 0f b4 08          	lfs    \(%rax\),%rcx
[ 	]*[a-f0-9]+:	48 0f b4 08          	lfs    \(%rax\),%rcx
[ 	]*[a-f0-9]+:	48 0f b5 11          	lgs    \(%rcx\),%rdx
[ 	]*[a-f0-9]+:	48 0f b5 11          	lgs    \(%rcx\),%rdx
[ 	]*[a-f0-9]+:	48 0f b2 1a          	lss    \(%rdx\),%rbx
[ 	]*[a-f0-9]+:	48 0f b2 1a          	lss    \(%rdx\),%rbx
[ 	]*[a-f0-9]+:	48 ff 18             	rex\.W lcall \*\(%rax\)
[ 	]*[a-f0-9]+:	48 ff 29             	rex\.W ljmp \*\(%rcx\)
[ 	]*[a-f0-9]+:	0f 05                	syscall
[ 	]*[a-f0-9]+:	0f 07                	sysretl
[ 	]*[a-f0-9]+:	48 0f 07             	sysretq
[ 	]*[a-f0-9]+:	48 0f b4 01          	lfs    \(%rcx\),%rax
[ 	]*[a-f0-9]+:	48 0f b4 01          	lfs    \(%rcx\),%rax
[ 	]*[a-f0-9]+:	48 0f b5 0a          	lgs    \(%rdx\),%rcx
[ 	]*[a-f0-9]+:	48 0f b5 0a          	lgs    \(%rdx\),%rcx
[ 	]*[a-f0-9]+:	48 0f b2 13          	lss    \(%rbx\),%rdx
[ 	]*[a-f0-9]+:	48 0f b2 13          	lss    \(%rbx\),%rdx
[ 	]*[a-f0-9]+:	48 ff 19             	rex\.W lcall \*\(%rcx\)
[ 	]*[a-f0-9]+:	48 ff 2a             	rex\.W ljmp \*\(%rdx\)
#pass
