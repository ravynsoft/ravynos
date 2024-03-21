#source: x86-64-align-branch-4.s
#as: -malign-branch-boundary=32 -malign-branch=ret
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 64 89 04 25 01 00 00 00 	fs mov %eax,%fs:0x1
   9:	55                   	push   %rbp
   a:	55                   	push   %rbp
   b:	48 89 e5             	mov    %rsp,%rbp
   e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  11:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  14:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  17:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  20:	c3                   	ret
  21:	2e 2e 55             	cs cs push %rbp
  24:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  2c:	55                   	push   %rbp
  2d:	55                   	push   %rbp
  2e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  40:	c2 1e 00             	ret    \$0x1e
  43:	55                   	push   %rbp
#pass
