#source: align-branch-2.s
#as: -malign-branch-boundary=32 -malign-branch=indirect+call
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
  42:	36 36 36 36 36 89 75 f4 	ss ss ss ss mov %esi,%ss:-0xc\(%ebp\)
  4a:	55                   	push   %ebp
  4b:	55                   	push   %ebp
  4c:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  52:	89 e5                	mov    %esp,%ebp
  54:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  57:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  60:	e8 [0-9a-f ]+       	call   .*
  65:	36 36 36 36 36 89 75 f4 	ss ss ss ss mov %esi,%ss:-0xc\(%ebp\)
  6d:	3e 55                	ds push %ebp
  6f:	55                   	push   %ebp
  70:	55                   	push   %ebp
  71:	55                   	push   %ebp
  72:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  78:	89 e5                	mov    %esp,%ebp
  7a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  7d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  80:	ff 15 00 00 00 00    	call   \*0x0
  86:	55                   	push   %ebp
#pass
