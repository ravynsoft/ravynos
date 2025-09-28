#objdump: -dwMintel
#name: i386 RdRnd (Intel disassembly)
#source: rdrnd.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	66 0f c7 f3          	rdrand bx
[ 	]*[a-f0-9]+:	0f c7 f3             	rdrand ebx
[ 	]*[a-f0-9]+:	66 0f c7 f3          	rdrand bx
[ 	]*[a-f0-9]+:	0f c7 f3             	rdrand ebx
#pass
