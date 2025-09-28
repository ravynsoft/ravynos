#as: --64
#ld: -shared -Bsymbolic-functions -melf_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[0-9a-f]+ <_start>:
 +[a-f0-9]+:	e8 00 00 00 00       	call   [0-9a-f]+ <foo>

[0-9a-f]+ <foo>:
 +[a-f0-9]+:	c3                   	ret
#pass
