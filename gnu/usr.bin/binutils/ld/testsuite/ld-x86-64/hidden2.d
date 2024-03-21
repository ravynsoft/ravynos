#as: --64
#ld: -shared -melf_x86_64
#objdump: -drw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	e8 ([0-9a-f]{2} ){4} *	call   0 .*
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
