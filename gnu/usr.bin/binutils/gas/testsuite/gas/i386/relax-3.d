#as: -mshared
#objdump: -dwr

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	eb 24                	jmp    26 <local>
[ 	]*[a-f0-9]+:	eb 1e                	jmp    22 <hidden_def>
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    5 <foo\+0x5>	5: R_386_PC32	global_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    a <foo\+0xa>	a: R_386_PLT32	global_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    f <foo\+0xf>	f: R_386_PC32	weak_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    14 <foo\+0x14>	14: R_386_PC32	weak_hidden_undef
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    19 <foo\+0x19>	19: R_386_PC32	weak_hidden_def
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    1e <foo\+0x1e>	1e: R_386_PC32	hidden_undef

0+22 <hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+23 <weak_hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+24 <global_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+25 <weak_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+26 <local>:
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
