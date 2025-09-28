#source: x86-64-align-branch-2.s
#as: -malign-branch-boundary=32 -malign-branch=fused+jcc+jmp
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
   8:	55                   	push   %rbp
   9:	55                   	push   %rbp
   a:	55                   	push   %rbp
   b:	55                   	push   %rbp
   c:	48 89 e5             	mov    %rsp,%rbp
   f:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  12:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  15:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  18:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1e:	ff e0                	jmp    \*%rax
  20:	55                   	push   %rbp
  21:	55                   	push   %rbp
  22:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  2a:	48 89 e5             	mov    %rsp,%rbp
  2d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  30:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  33:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  36:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  39:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3c:	ff d0                	call   \*%rax
  3e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  41:	55                   	push   %rbp
  42:	55                   	push   %rbp
  43:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  4b:	48 89 e5             	mov    %rsp,%rbp
  4e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  51:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  54:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  57:	e8 [0-9a-f ]+       	call   .*
  5c:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5f:	55                   	push   %rbp
  60:	55                   	push   %rbp
  61:	55                   	push   %rbp
  62:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  6a:	48 89 e5             	mov    %rsp,%rbp
  6d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  70:	ff 14 25 00 00 00 00 	call   \*0x0
  77:	55                   	push   %rbp
#pass
