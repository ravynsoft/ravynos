#source: ospke.s
#name: x86-64 OSPKE insns
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 ee             	rdpkru
[ 	]*[a-f0-9]+:	0f 01 ef             	wrpkru
#pass
