#source: pr20244-4.s
#as: --32
#ld: -m elf_i386 -z noseparate-code
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+804807c <_start>:
 +[a-f0-9]+:	8b 05 8c 90 04 08    	mov    0x804908c,%eax
 +[a-f0-9]+:	c3                   	ret

0+8048083 <ifunc>:
 +[a-f0-9]+:	b8 ef be ad 0b       	mov    \$0xbadbeef,%eax
 +[a-f0-9]+:	c3                   	ret
#pass
