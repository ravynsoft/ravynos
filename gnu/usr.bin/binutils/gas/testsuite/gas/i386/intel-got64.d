#objdump: -dwMintel
#name: x86-64 intel-got

.*: +file format .*

Disassembly of section .text:

0+000 <_start>:
[ 	]*[0-9a-f]+:[ 	]+a1 00 00 00 00 00 00 00 00[ 	]+movabs[ 	]+eax,(ds:)?0x0
[ 	]*[0-9a-f]+:[ 	]+ff 35 00 00 00 00[ 	]+push[ 	]+(QWORD PTR )?\[rip(\+(0x)?0)?\]([ 	]+#.*)?
[ 	]*[0-9a-f]+:[ 	]+c3[ 	]+ret
#pass
