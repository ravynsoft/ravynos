#source: relax-3.s
#objdump: -dwr

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	eb 21                	jmp    23 <local>
[ 	]*[a-f0-9]+:	eb 1b                	jmp    1f <hidden_def>
[ 	]*[a-f0-9]+:	eb 1b                	jmp    21 <global_def>
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    7 <foo\+0x7>	7: R_386_PLT32	global_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    c <foo\+0xc>	c: R_386_PC32	weak_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    11 <foo\+0x11>	11: R_386_PC32	weak_hidden_undef
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    16 <foo\+0x16>	16: R_386_PC32	weak_hidden_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    1b <foo\+0x1b>	1b: R_386_PC32	hidden_undef

0+1f <hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+20 <weak_hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+21 <global_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+22 <weak_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+23 <local>:
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
