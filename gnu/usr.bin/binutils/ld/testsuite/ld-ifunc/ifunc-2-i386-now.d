#source: ifunc-2-i386.s
#ld: -z now -m elf_i386 -shared --hash-style=sysv -z noseparate-code $NO_DT_RELR_LDFLAGS
#as: --32
#objdump: -dw
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

.*: +file format .*


Disassembly of section .plt:

0+f0 <\*ABS\*@plt-0x10>:
 +[a-f0-9]+:	ff b3 04 00 00 00    	push   0x4\(%ebx\)
 +[a-f0-9]+:	ff a3 08 00 00 00    	jmp    \*0x8\(%ebx\)
 +[a-f0-9]+:	00 00                	add    %al,\(%eax\)
	...

0+100 <\*ABS\*@plt>:
 +[a-f0-9]+:	ff a3 0c 00 00 00    	jmp    \*0xc\(%ebx\)
 +[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
 +[a-f0-9]+:	e9 e0 ff ff ff       	jmp    f0 <\*ABS\*@plt-0x10>

Disassembly of section .text:

0+110 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+111 <bar>:
 +[a-f0-9]+:	e8 00 00 00 00       	call   116 <bar\+0x5>
 +[a-f0-9]+:	5b                   	pop    %ebx
 +[a-f0-9]+:	81 c3 9e 10 00 00    	add    \$0x109e,%ebx
 +[a-f0-9]+:	e8 de ff ff ff       	call   100 <\*ABS\*@plt>
 +[a-f0-9]+:	8b 83 0c 00 00 00    	mov    0xc\(%ebx\),%eax
 +[a-f0-9]+:	c3                   	ret
#pass
