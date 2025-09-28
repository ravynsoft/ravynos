#as: --64 -mbranches-within-32B-boundaries
#ld: -melf_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	85 d2                	test   %edx,%edx
 +[a-f0-9]+:	74 21                	je     [a-f0-9]+ <_start\+0x25>
 +[a-f0-9]+:	48 85 ff             	test   %rdi,%rdi
 +[a-f0-9]+:	74 1c                	je     [a-f0-9]+ <_start\+0x25>
 +[a-f0-9]+:	66 66 66 64 48 8b 04 25 00 00 00 00 	data16 data16 data16 mov %fs:0x0,%rax
 +[a-f0-9]+:	2e 2e 2e 2e 48 8b 98 fc ff ff ff 	cs cs cs cs mov -0x4\(%rax\),%rbx
 +[a-f0-9]+:	48 85 db             	test   %rbx,%rbx
 +[a-f0-9]+:	74 00                	je     [a-f0-9]+ <_start\+0x25>
 +[a-f0-9]+:	c3                   	ret
#pass
