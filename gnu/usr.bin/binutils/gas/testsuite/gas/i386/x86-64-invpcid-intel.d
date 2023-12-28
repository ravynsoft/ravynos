#as:
#objdump: -dwMintel
#name: x86-64 INVPCID insns (Intel disassembly)
#source: x86-64-invpcid.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid rdx,\[rax\]
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid rdx,\[rax\]
[ 	]*[a-f0-9]+:	66 0f 38 82 10       	invpcid rdx,\[rax\]
#pass
