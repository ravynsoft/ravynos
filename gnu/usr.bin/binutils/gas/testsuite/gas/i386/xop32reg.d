#source: xop32reg.s
#objdump: -dw
#name: i386 ignore rex_b in case of 32 bit decoding

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f e9 78 e1 4d c2[ 	]+vphsubbw -0x3e\(%ebp\),%xmm1
[ 	]*[a-f0-9]+:	8f c9 78 e1 4d c2[ 	]+vphsubbw -0x3e\(%ebp\),%xmm1
[       ]*[a-f0-9]+:	8f e8 40 cd 04 08 07[ 	]+vpcomtruew \(%eax,%ecx,1\),%xmm7,%xmm0
[       ]*[a-f0-9]+:	8f c8 40 cd 04 08 07[ 	]+vpcomtruew \(%eax,%ecx,1\),%xmm7,%xmm0
#pass
