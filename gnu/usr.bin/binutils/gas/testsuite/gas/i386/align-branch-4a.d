#source: align-branch-4.s
#as: -malign-branch-boundary=32 -malign-branch=fused+jcc+jmp
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
   6:	55                   	push   %ebp
   7:	55                   	push   %ebp
   8:	55                   	push   %ebp
   9:	55                   	push   %ebp
   a:	55                   	push   %ebp
   b:	89 e5                	mov    %esp,%ebp
   d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  10:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  13:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  16:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  19:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1c:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1f:	c3                   	ret
  20:	55                   	push   %ebp
  21:	55                   	push   %ebp
  22:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  28:	89 e5                	mov    %esp,%ebp
  2a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  2d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  30:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  33:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  36:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  39:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3c:	c2 1e 00             	ret    \$0x1e
  3f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
#pass
