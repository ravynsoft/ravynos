#objdump: -dw
#name: i386 TLBSYNC insn

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:	0f 01 ff             	tlbsync
#pass
