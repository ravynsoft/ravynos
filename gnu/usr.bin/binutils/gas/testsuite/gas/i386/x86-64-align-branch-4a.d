#source: x86-64-align-branch-4.s
#as: -malign-branch-boundary=32 -malign-branch=fused+jcc+jmp
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
   8:	55                   	push   %rbp
   9:	55                   	push   %rbp
   a:	48 89 e5             	mov    %rsp,%rbp
   d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  10:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  13:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  16:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  19:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1c:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1f:	c3                   	ret
  20:	55                   	push   %rbp
  21:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  29:	55                   	push   %rbp
  2a:	55                   	push   %rbp
  2b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  2e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3d:	c2 1e 00             	ret    \$0x1e
  40:	55                   	push   %rbp
#pass
