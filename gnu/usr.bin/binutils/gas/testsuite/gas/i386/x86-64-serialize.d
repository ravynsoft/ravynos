#as:
#objdump: -dw
#name: x86_64 SERIALIZE insns
#source: serialize.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 e8 +	serialize
#pass
