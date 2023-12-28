#source: pr27491-4a.s
#source: pr27491-4b.s
#as: --64
#ld: --gc-sections -melf_x86_64 -z start-stop-gc -shared
#objdump: --syms -dw

.*: +file format elf64-x86-64


SYMBOL TABLE:
#...
[a-f0-9]+ g       xx	[a-f0-9]+ \.protected __stop_xx
#...
[a-f0-9]+ g       xx	[a-f0-9]+ \.protected __start_xx
#...


Disassembly of section .text:

[a-f0-9]+ <foo>:
 +[a-f0-9]+:	48 8d 05 ([0-9a-f]{2} ){4}[ \t]+lea +0x[a-f0-9]+\(%rip\),%rax[ \t]+# [a-f0-9]+ <__start_xx>
 +[a-f0-9]+:	48 8d 05 ([0-9a-f]{2} ){4}[ \t]+lea +0x[a-f0-9]+\(%rip\),%rax[ \t]+# [a-f0-9]+ <__stop_xx>
#pass
