#as: -J -march=iamcu -I${srcdir}/$subdir
#objdump: -dw

.*: +file format elf32-iamcu.*

Disassembly of section .text:

0+ <.text>:
 +[a-f0-9]+:	d9 ff                	fcos
 +[a-f0-9]+:	66 0f 58 01          	addpd  \(%ecx\),%xmm0
#pass
