#ld: -m elf_i386 -z noseparate-code
#as: --32 -mrelax-relocations=yes
#objdump: -dw
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

.*: +file format .*


Disassembly of section .text:

0+804807c <__start>:
 +[a-f0-9]+:	ff 93 fc ff ff ff    	call   \*-0x4\(%ebx\)
 +[a-f0-9]+:	ff a3 fc ff ff ff    	jmp    \*-0x4\(%ebx\)
 +[a-f0-9]+:	03 83 fc ff ff ff    	add    -0x4\(%ebx\),%eax
 +[a-f0-9]+:	8b 83 fc ff ff ff    	mov    -0x4\(%ebx\),%eax
 +[a-f0-9]+:	85 83 fc ff ff ff    	test   %eax,-0x4\(%ebx\)
 +[a-f0-9]+:	c7 c0 a1 80 04 08    	mov    \$0x80480a1,%eax

0+80480a0 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+80480a1 <bar>:
 +[a-f0-9]+:	c3                   	ret
#pass
