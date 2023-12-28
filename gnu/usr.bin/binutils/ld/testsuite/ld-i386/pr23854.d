#as: --32 -mrelax-relocations=yes
#ld: -melf_i386
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	66 13 81 f8 ff ff ff 	adc    -0x8\(%ecx\),%ax
 +[a-f0-9]+:	66 03 99 f8 ff ff ff 	add    -0x8\(%ecx\),%bx
 +[a-f0-9]+:	66 23 89 f8 ff ff ff 	and    -0x8\(%ecx\),%cx
 +[a-f0-9]+:	66 3b 91 f8 ff ff ff 	cmp    -0x8\(%ecx\),%dx
 +[a-f0-9]+:	66 0b b9 f8 ff ff ff 	or     -0x8\(%ecx\),%di
 +[a-f0-9]+:	66 1b b1 f8 ff ff ff 	sbb    -0x8\(%ecx\),%si
 +[a-f0-9]+:	66 2b a9 f8 ff ff ff 	sub    -0x8\(%ecx\),%bp
 +[a-f0-9]+:	66 33 a1 f8 ff ff ff 	xor    -0x8\(%ecx\),%sp
 +[a-f0-9]+:	66 85 89 f8 ff ff ff 	test   %cx,-0x8\(%ecx\)
 +[a-f0-9]+:	66 13 81 fc ff ff ff 	adc    -0x4\(%ecx\),%ax
 +[a-f0-9]+:	66 03 99 fc ff ff ff 	add    -0x4\(%ecx\),%bx
 +[a-f0-9]+:	66 23 89 fc ff ff ff 	and    -0x4\(%ecx\),%cx
 +[a-f0-9]+:	66 3b 91 fc ff ff ff 	cmp    -0x4\(%ecx\),%dx
 +[a-f0-9]+:	66 0b b9 fc ff ff ff 	or     -0x4\(%ecx\),%di
 +[a-f0-9]+:	66 1b b1 fc ff ff ff 	sbb    -0x4\(%ecx\),%si
 +[a-f0-9]+:	66 2b a9 fc ff ff ff 	sub    -0x4\(%ecx\),%bp
 +[a-f0-9]+:	66 33 a1 fc ff ff ff 	xor    -0x4\(%ecx\),%sp
 +[a-f0-9]+:	66 85 89 fc ff ff ff 	test   %cx,-0x4\(%ecx\)
#pass
