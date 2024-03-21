#objdump: -dw
#name: i386 8087

.*: +file format .*

Disassembly of section .text:

0+ <_8087>:
[ 	]*[0-9a-f]+:	9b db e1[ 	]+fdisi.*
[ 	]*[0-9a-f]+:	9b db e0[ 	]+feni.*
[ 	]*[0-9a-f]+:	db e1[ 	]+fndisi.*
[ 	]*[0-9a-f]+:	db e0[ 	]+fneni.*
#pass
