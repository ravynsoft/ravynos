#source: call1.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64 -z call-nop=prefix-0x90
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	e8 ([0-9a-f]{2} ){4} *	call +[a-f0-9]+ <foo>
#pass
