#source: pr20244-2.s
#as: --32
#ld: -m elf_i386 -z noseparate-code
#objdump: --sym -dw

.*: +file format .*

SYMBOL TABLE:
#...
0+8048085 l   i   .text	00000000 bar
#...
0+8048086 g     F .text	00000000 _start
#...
0+8048084 g   i   .text	00000000 foo
#...

Disassembly of section .text:

0+8048084 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+8048085 <bar>:
 +[a-f0-9]+:	c3                   	ret

0+8048086 <_start>:
 +[a-f0-9]+:	ff 15 a8 90 04 08    	call   \*0x80490a8
 +[a-f0-9]+:	ff 25 ac 90 04 08    	jmp    \*0x80490ac
 +[a-f0-9]+:	c7 05 ac 90 04 08 00 00 00 00 	movl   \$0x0,0x80490ac
 +[a-f0-9]+:	83 3d a8 90 04 08 00 	cmpl   \$0x0,0x80490a8
 +[a-f0-9]+:	b9 fc ff ff ff       	mov    \$0xfffffffc,%ecx
#pass
