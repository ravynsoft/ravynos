#objdump: -dw
#name: i386 SMX

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	0f 37                	getsec
#pass
