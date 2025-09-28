#as:
#objdump: -dw
#name: x86-64 INVPCID insns

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid \(%rax\),%rdx
#pass
