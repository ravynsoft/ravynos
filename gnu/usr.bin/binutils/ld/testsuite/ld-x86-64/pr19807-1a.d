#source: pr19807-1.s
#as: --64
#ld: -pie -melf_x86_64 -z noreloc-overflow -z notext
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	48 c7 c0 ([0-9a-f]{2} ){4}	mov    \$0x[a-f0-9]+,%rax
#pass
