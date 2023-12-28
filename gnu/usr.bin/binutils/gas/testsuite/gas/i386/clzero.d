#objdump: -dw
#name: i386 CLZERO insn

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:	0f 01 fc             	clzero
#pass
