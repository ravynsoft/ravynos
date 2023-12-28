#as: --64
#ld: --gc-sections -melf_x86_64 -T pr18160.t
#objdump: -dw

.*: +file format elf32-i386


Disassembly of section .text:

0+ <start>:
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    5 <foo>

0+5 <foo>:
[ 	]*[a-f0-9]+:	c3                   	ret
