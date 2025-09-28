#as: --32
#ld: --gc-sections -melf_i386 -z start-stop-gc -shared
#objdump: --syms -dw

.*: +file format elf32-i386


SYMBOL TABLE:
#...
[a-f0-9]+ g       xx	[a-f0-9]+ \.protected __stop_xx
#...
[a-f0-9]+ g       xx	[a-f0-9]+ \.protected __start_xx
#...


Disassembly of section .text:

[a-f0-9]+ <foo>:
 +[a-f0-9]+:	8d 83 ([0-9a-f]{2} ){4}[ \t]+lea +-0x[a-f0-9]+\(%ebx\),%eax
 +[a-f0-9]+:	8d 83 ([0-9a-f]{2} ){4}[ \t]+lea +-0x[a-f0-9]+\(%ebx\),%eax
#pass
