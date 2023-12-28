#source: pr19807-2.s
#as: --64
#ld: -pie -melf_x86_64 -z noreloc-overflow -z notext
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	b8 ([0-9a-f]{2} ){4}      	mov    \$0x[a-f0-9]+,%eax
#pass
