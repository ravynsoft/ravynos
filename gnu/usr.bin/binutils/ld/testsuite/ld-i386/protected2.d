#as: --32
#ld: -shared -melf_i386
#objdump: -drw

.*: +file format .*


Disassembly of section .text:

0+[a-f0-9]+ <foo>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	e8 fa ff ff ff       	call   [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
