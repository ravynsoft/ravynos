#objdump: -dw
#name: x86_64 CLZERO insn
#source: clzero.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 fc             	clzero
#pass
