#source: call1.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -z call-nop=suffix-0x90
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	e8 ([0-9a-f]{2} ){4} *	call +[a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	90                   	nop
#pass
