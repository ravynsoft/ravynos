#source: pr27491-1.s
#as: --32
#ld: --gc-sections -melf_i386 -z start-stop-gc -shared
#objdump: -dw

.*: +file format elf32-i386


Disassembly of section .text:

[a-f0-9]+ <foo>:
 +[a-f0-9]+:	c7 c0 00 00 00 00    	mov    \$0x0,%eax
 +[a-f0-9]+:	c7 c0 00 00 00 00    	mov    \$0x0,%eax
#pass
