#as:
#objdump: -dw
#name: x86_64 CLWB insns
#source: x86-64-clwb.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   \(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*66 42 0f ae b4 f0 23 01 00 00[ 	]*clwb   0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   \(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*66 42 0f ae b4 f0 34 12 00 00[ 	]*clwb   0x1234\(%rax,%r14,8\)
#pass
