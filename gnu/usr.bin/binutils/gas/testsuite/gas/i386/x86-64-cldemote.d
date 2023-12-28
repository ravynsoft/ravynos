#as:
#objdump: -dw
#name: x86_64 CLDEMOTE insns
#source: x86-64-cldemote.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f 1c 01[ 	]*cldemote \(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*42 0f 1c 84 f0 23 01 00 00[ 	]*cldemote 0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*0f 1c 01[ 	]*cldemote \(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*42 0f 1c 84 f0 34 12 00 00[ 	]*cldemote 0x1234\(%rax,%r14,8\)
#pass
