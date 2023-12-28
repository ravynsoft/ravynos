#source: ../ld-x86-64/pr26018.s
#as: --32
#ld: -shared -Bsymbolic-functions -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[0-9a-f]+ <_start>:
 +[a-f0-9]+:	e8 00 00 00 00       	call   [0-9a-f]+ <foo>

[0-9a-f]+ <foo>:
 +[a-f0-9]+:	c3                   	ret
#pass
