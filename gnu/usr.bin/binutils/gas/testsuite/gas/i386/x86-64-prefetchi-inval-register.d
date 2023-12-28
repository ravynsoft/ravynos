#as: --divide
#objdump: -dw
#name: x86-64 PREFETCHI INVAL REGISTER insns

.*: +file format .*


Disassembly of section .text:

0+ <\.text>:
[ 	]*[a-f0-9]+:[ 	]0f 18 39[ 	]*nopl   \(%rcx\)
[ 	]*[a-f0-9]+:[ 	]0f 18 31[ 	]*nopl   \(%rcx\)
#pass
