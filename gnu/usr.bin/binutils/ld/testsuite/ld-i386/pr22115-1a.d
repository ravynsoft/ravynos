#source: pr22115-1.s
#as: --32 -mrelax-relocations=yes
#ld: -pie -z text -m elf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	8d 80 ([0-9a-f]{2} ){4} *	lea    -?0x[a-f0-9]+\(%eax\),%eax
#pass
