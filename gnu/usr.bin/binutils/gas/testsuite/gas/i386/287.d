#objdump: -dw
#name: i386 287

.*: +file format .*

Disassembly of section .text:

0+ <_8087>:
[ 	]*[0-9a-f]+:	db e4[ 	]+fnsetpm.*
[ 	]*[0-9a-f]+:	db e5[ 	]+frstpm.*
[ 	]*[0-9a-f]+:	9b db e4[ 	]+fsetpm.*
#pass
