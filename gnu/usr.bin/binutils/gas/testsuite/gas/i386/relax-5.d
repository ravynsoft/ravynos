#objdump: -dwr

.*: +file format .*

Disassembly of section .text:

0+ <printk>:
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .init.text:

0+ <foo>:
 +[a-f0-9]+:	e8 fc ff ff ff       	call   1 <foo\+0x1>	1: R_386_PC32	.text
 +[a-f0-9]+:	e8 fc ff ff ff       	call   6 <foo\+0x6>	6: R_386_PC32	.text
#pass
