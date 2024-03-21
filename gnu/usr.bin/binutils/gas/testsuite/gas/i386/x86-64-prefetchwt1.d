#as:
#objdump: -dw
#name: x86_64 PREFETCHWT1 insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 0d 11             	prefetchwt1 \(%rcx\)
[ 	]*[a-f0-9]+:	42 0f 0d 94 f0 23 01 00 00 	prefetchwt1 0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:	0f 0d 11             	prefetchwt1 \(%rcx\)
[ 	]*[a-f0-9]+:	42 0f 0d 94 f0 34 12 00 00 	prefetchwt1 0x1234\(%rax,%r14,8\)
#pass
