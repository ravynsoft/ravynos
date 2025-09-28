#objdump: -dw
#name: i386 SE1 insns
#source: se1.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 cf             	encls
[ 	]*[a-f0-9]+:	0f 01 d7             	enclu
[ 	]*[a-f0-9]+:	0f 01 c0             	enclv
#pass
