#objdump: -dwMintel
#name: i386 intel-got

.*: +file format .*

Disassembly of section .text:

0+000 <_start>:
[ 	]*[0-9a-f]+:[ 	]+8b 15 04 00 00 00[ 	]+mov[ 	]+edx,(DWORD PTR )?(ds:)?0x4
[ 	]*[0-9a-f]+:[ 	]+c3[ 	]+ret
#pass
