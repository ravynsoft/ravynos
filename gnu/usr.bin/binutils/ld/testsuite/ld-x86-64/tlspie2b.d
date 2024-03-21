#source: tlspie2.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64 -pie
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <__tls_get_addr>:
[ 	]*[a-f0-9]+:	c3                   	ret

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	48 c7 c0 f4 ff ff ff 	mov    \$0xfffffffffffffff4,%rax
[ 	]*[a-f0-9]+:	53                   	push   %rbx
[ 	]*[a-f0-9]+:	64 8b 1c 25 f0 ff ff ff 	mov    %fs:0xfffffffffffffff0,%ebx
[ 	]*[a-f0-9]+:	64 03 1c 25 ec ff ff ff 	add    %fs:0xffffffffffffffec,%ebx
[ 	]*[a-f0-9]+:	64 03 18             	add    %fs:\(%rax\),%ebx
[ 	]*[a-f0-9]+:	66 66 66 66 64 48 8b 04 25 00 00 00 00 	data16 data16 data16 data16 mov %fs:0x0,%rax
[ 	]*[a-f0-9]+:	03 98 f8 ff ff ff    	add    -0x8\(%rax\),%ebx
[ 	]*[a-f0-9]+:	64 48 8b 04 25 00 00 00 00 	mov    %fs:0x0,%rax
[ 	]*[a-f0-9]+:	48 8d 80 fc ff ff ff 	lea    -0x4\(%rax\),%rax
[ 	]*[a-f0-9]+:	03 18                	add    \(%rax\),%ebx
[ 	]*[a-f0-9]+:	89 d8                	mov    %ebx,%eax
[ 	]*[a-f0-9]+:	5b                   	pop    %rbx
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
