#source: pr27016a.s
#source: pr27016b.s
#as: --64 -mx86-used-note=no -mrelax-relocations=yes
#ld: -m elf_x86_64 -z max-page-size=0x200000 -z noseparate-code -e main
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+4000e8 <main>:
 +[a-f0-9]+:	55                   	push   %rbp
 +[a-f0-9]+:	48 89 e5             	mov    %rsp,%rbp
 +[a-f0-9]+:	49 c7 c3 2c 40 40 00 	mov    \$0x40402c,%r11
 +[a-f0-9]+:	41 8b 03             	mov    \(%r11\),%eax
 +[a-f0-9]+:	8d 50 01             	lea    0x1\(%rax\),%edx
 +[a-f0-9]+:	49 c7 c3 2c 40 40 00 	mov    \$0x40402c,%r11
 +[a-f0-9]+:	41 89 13             	mov    %edx,\(%r11\)
 +[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax
 +[a-f0-9]+:	5d                   	pop    %rbp
 +[a-f0-9]+:	c3                   	ret
#pass
