#source: align-branch-2.s
#as: -malign-branch-boundary=32 -malign-branch=indirect
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 64 64 a3 01 00 00 00 	fs fs mov %eax,%fs:0x1
   8:	55                   	push   %ebp
   9:	55                   	push   %ebp
   a:	55                   	push   %ebp
   b:	55                   	push   %ebp
   c:	89 e5                	mov    %esp,%ebp
   e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  11:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  14:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  17:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  20:	ff e0                	jmp    \*%eax
  22:	3e 3e 55             	ds ds push %ebp
  25:	55                   	push   %ebp
  26:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  2c:	89 e5                	mov    %esp,%ebp
  2e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  40:	ff d0                	call   \*%eax
  42:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  45:	55                   	push   %ebp
  46:	55                   	push   %ebp
  47:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  4d:	89 e5                	mov    %esp,%ebp
  4f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5b:	e8 [0-9a-f ]+       	call   .*
  60:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  63:	55                   	push   %ebp
  64:	55                   	push   %ebp
  65:	55                   	push   %ebp
  66:	55                   	push   %ebp
  67:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  6d:	89 e5                	mov    %esp,%ebp
  6f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  72:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  75:	ff 15 00 00 00 00    	call   \*0x0
  7b:	55                   	push   %ebp
#pass
