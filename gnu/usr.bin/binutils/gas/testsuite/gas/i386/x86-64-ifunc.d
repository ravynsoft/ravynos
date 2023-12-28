#source: ifunc.s
#objdump: -drw
#name: x86-64 ifunc

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    5 <ifunc>	1: R_X86_64_PLT32	ifunc(\+0xf+c|-0x4)

0+5 <ifunc>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+6 <bar>:
[ 	]*[a-f0-9]+:	eb 00                	jmp    8 <normal>

0+8 <normal>:
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
