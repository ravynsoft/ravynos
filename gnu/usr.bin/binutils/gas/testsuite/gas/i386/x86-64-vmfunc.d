#source: vmfunc.s
#objdump: -dw
#name: 64bit VMFUNC

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	0f 01 d4             	vmfunc
[ 	]*[a-f0-9]+:	90                   	nop
#pass
