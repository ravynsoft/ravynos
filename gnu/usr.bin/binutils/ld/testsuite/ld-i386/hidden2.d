#as: --32
#ld: -shared -melf_i386
#objdump: -drw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	e8 ([0-9a-f]{2} ){4}\s+call   0 .*
[ 	]*[a-f0-9]+:	c3\s+ret *
#pass
