#source: pr19636-1.s
#as: --32 -mrelax-relocations=no
#ld: -pie -m elf_i386 --no-dynamic-linker
#objdump: -dw

.*: +file format .*


Disassembly of section .plt:

.* <.plt>:
[ 	]*[a-f0-9]+:	ff b3 04 00 00 00    	push   0x4\(%ebx\)
[ 	]*[a-f0-9]+:	ff a3 08 00 00 00    	jmp    \*0x8\(%ebx\)
[ 	]*[a-f0-9]+:	00 00                	add    %al,\(%eax\)
[ 	]*[a-f0-9]+:	00 00                	add    %al,\(%eax\)
[ 	]*[a-f0-9]+:	ff a3 0c 00 00 00    	jmp    \*0xc\(%ebx\)
[ 	]*[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    [a-f0-9]+ <.*>

Disassembly of section .text:

.* <_start>:
[ 	]*[a-f0-9]+:	3b 80 f8 ff ff ff    	cmp    -0x8\(%eax\),%eax
[ 	]*[a-f0-9]+:	ff a0 fc ff ff ff    	jmp    \*-0x4\(%eax\)
[ 	]*[a-f0-9]+:	e8 df ff ff ff       	call   .* <_start-0x10>
