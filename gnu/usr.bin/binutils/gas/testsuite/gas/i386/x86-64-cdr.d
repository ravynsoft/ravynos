#objdump: -dw
#name: x86-64 control/debug register with ignored MOD field
#source: cdr.s

.*: +file format .*

Disassembly of section .text:

0+ <start>:
[ 	]*[a-f0-9]+:	0f 22 1f             	mov    %rdi,%cr3
[ 	]*[a-f0-9]+:	0f 20 1f             	mov    %cr3,%rdi
[ 	]*[a-f0-9]+:	0f 21 1f             	mov    %db3,%rdi
[ 	]*[a-f0-9]+:	0f 23 1f             	mov    %rdi,%db3
#pass
