#source: x86-64-align-branch-2.s
#as: -malign-branch-boundary=32 -malign-branch=indirect+call
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 64 64 89 04 25 01 00 00 00 	fs fs mov %eax,%fs:0x1
   a:	55                   	push   %rbp
   b:	55                   	push   %rbp
   c:	55                   	push   %rbp
   d:	55                   	push   %rbp
   e:	48 89 e5             	mov    %rsp,%rbp
  11:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  14:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  17:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  20:	ff e0                	jmp    \*%rax
  22:	2e 2e 55             	cs cs push %rbp
  25:	55                   	push   %rbp
  26:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  2e:	48 89 e5             	mov    %rsp,%rbp
  31:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  40:	ff d0                	call   \*%rax
  42:	2e 2e 2e 2e 2e 89 75 f4 	cs cs cs cs cs mov %esi,-0xc\(%rbp\)
  4a:	55                   	push   %rbp
  4b:	55                   	push   %rbp
  4c:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  54:	48 89 e5             	mov    %rsp,%rbp
  57:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  60:	e8 [0-9a-f ]+       	call   .*
  65:	2e 2e 2e 2e 2e 89 75 f4 	cs cs cs cs cs mov %esi,-0xc\(%rbp\)
  6d:	2e 2e 55             	cs cs push %rbp
  70:	55                   	push   %rbp
  71:	55                   	push   %rbp
  72:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  7a:	48 89 e5             	mov    %rsp,%rbp
  7d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  80:	ff 14 25 00 00 00 00 	call   \*0x0
  87:	55                   	push   %rbp
#pass
