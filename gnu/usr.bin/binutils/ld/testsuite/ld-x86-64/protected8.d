#as: --64
#ld: -shared -melf_x86_64
#objdump: -drw

.*: +file format .*


Disassembly of section .text:

0+[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	8b 05 ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%eax        # [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
