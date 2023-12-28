#source: pr19609-2.s
#as: --32 -mrelax-relocations=yes
#ld: -pie -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	ff 92 fc ff ff ff    	call   \*-0x4\(%edx\)
