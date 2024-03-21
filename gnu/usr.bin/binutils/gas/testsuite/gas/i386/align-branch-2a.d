#source: align-branch-2.s
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
   a:	89 e5                	mov    %esp,%ebp
   c:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
   f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  12:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  15:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  18:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1e:	ff e0                	jmp    \*%eax
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
  3c:	ff d0                	call   \*%eax
  3e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  41:	55                   	push   %ebp
  42:	55                   	push   %ebp
  43:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  49:	89 e5                	mov    %esp,%ebp
  4b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  4e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  51:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  54:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  57:	e8 [0-9a-f ]+       	call   .*
  5c:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5f:	55                   	push   %ebp
  60:	55                   	push   %ebp
  61:	55                   	push   %ebp
  62:	55                   	push   %ebp
  63:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  69:	89 e5                	mov    %esp,%ebp
  6b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  6e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  71:	ff 15 00 00 00 00    	call   \*0x0
  77:	55                   	push   %ebp
#pass
