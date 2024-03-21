#source: pr27491-1.s
#as: --64
#ld: --gc-sections -melf_x86_64 -z start-stop-gc -shared
#objdump: -dw

.*: +file format elf64-x86-64


Disassembly of section .text:

[a-f0-9]+ <foo>:
 +[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax
 +[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax
#pass
