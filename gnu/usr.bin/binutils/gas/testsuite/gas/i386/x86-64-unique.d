#source: unique.s
#objdump: -dw
#name: 64bit unique sections

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
 +[a-f0-9]+:	89 c3                	mov    %eax,%ebx
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .text:

0+ <bar>:
 +[a-f0-9]+:	31 c3                	xor    %eax,%ebx
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .text:

0+ <foo1>:
 +[a-f0-9]+:	89 c3                	mov    %eax,%ebx
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .text:

0+ <bar1>:
 +[a-f0-9]+:	01 c3                	add    %eax,%ebx
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .text:

0+ <bar2>:
 +[a-f0-9]+:	29 c3                	sub    %eax,%ebx
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .text:

0+ <foo2>:
 +[a-f0-9]+:	31 c3                	xor    %eax,%ebx
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	c3                   	ret
#pass
