#as: -malign-branch-boundary=32 -malign-branch=indirect+call
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
  1e:	e8 fc ff ff ff       	call   1f <foo\+0x1f>
  23:	55                   	push   %ebp
  24:	55                   	push   %ebp
  25:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  2b:	89 e5                	mov    %esp,%ebp
  2d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  30:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  33:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  36:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  39:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3c:	ff 91 00 00 00 00    	call   \*0x0\(%ecx\)
  42:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
#pass
