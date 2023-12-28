#as: --64
#ld: -m elf_x86_64 -shared
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+[a-f0-9]+ <printk>:
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .init.text:

0+[a-f0-9]+ <foo>:
 +[a-f0-9]+:	e8 ([0-9a-f]{2} ){4}      	call   [a-f0-9]+ <printk>
 +[a-f0-9]+:	e8 ([0-9a-f]{2} ){4}      	call   [a-f0-9]+ <printk>
#pass
