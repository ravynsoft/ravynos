#source: tlspie3.s
#as: --32 -mrelax-relocations=yes
#ld: -melf_i386 -pie
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

[0-9a-f]+ <___tls_get_addr>:
[ 	]*[a-f0-9]+:	c3                   	ret

[0-9a-f]+ <_start>:
[ 	]*[a-f0-9]+:	55                   	push   %ebp
[ 	]*[a-f0-9]+:	89 e5                	mov    %esp,%ebp
[ 	]*[a-f0-9]+:	56                   	push   %esi
[ 	]*[a-f0-9]+:	53                   	push   %ebx
[ 	]*[a-f0-9]+:	e8 00 00 00 00       	call   [0-9a-f]+ .*
[ 	]*[a-f0-9]+:	5b                   	pop    %ebx
[ 	]*[a-f0-9]+:	81 c3 ([0-9a-f]{2} ){4}[ 	]+add    \$0x[0-9a-f]+,%ebx
[ 	]*[a-f0-9]+:	65 8b 35 f0 ff ff ff 	mov    %gs:0xfffffff0,%esi
[ 	]*[a-f0-9]+:	65 03 35 ec ff ff ff 	add    %gs:0xffffffec,%esi
[ 	]*[a-f0-9]+:	c7 c0 f4 ff ff ff    	mov    \$0xfffffff4,%eax
[ 	]*[a-f0-9]+:	65 03 30             	add    %gs:\(%eax\),%esi
[ 	]*[a-f0-9]+:	65 a1 00 00 00 00    	mov    %gs:0x0,%eax
[ 	]*[a-f0-9]+:	8d b6 00 00 00 00    	lea    0x0\(%esi\),%esi
[ 	]*[a-f0-9]+:	03 30                	add    \(%eax\),%esi
[ 	]*[a-f0-9]+:	65 a1 00 00 00 00    	mov    %gs:0x0,%eax
[ 	]*[a-f0-9]+:	81 e8 04 00 00 00    	sub    \$0x4,%eax
[ 	]*[a-f0-9]+:	03 30                	add    \(%eax\),%esi
[ 	]*[a-f0-9]+:	89 f0                	mov    %esi,%eax
[ 	]*[a-f0-9]+:	5b                   	pop    %ebx
[ 	]*[a-f0-9]+:	5e                   	pop    %esi
[ 	]*[a-f0-9]+:	c9                   	leave
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
