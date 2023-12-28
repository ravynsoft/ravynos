#as: -malign-branch-boundary=32 -malign-branch=indirect+call
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
  1e:	e8 00 00 00 00       	call   23 <foo\+0x23>
  23:	55                   	push   %rbp
  24:	55                   	push   %rbp
  25:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
  2d:	48 89 e5             	mov    %rsp,%rbp
  30:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  33:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  36:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  39:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3c:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3f:	ff 15 00 00 00 00    	call   \*0x0\(%rip\)        # 45 <foo\+0x45>
  45:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
#pass
