#objdump: -drw
#name: i386 ifunc

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    1 <foo\+0x1>	1: R_386_PLT32	ifunc

0+5 <ifunc>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+6 <bar>:
[ 	]*[a-f0-9]+:	eb 00                	jmp    8 <normal>

0+8 <normal>:
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
