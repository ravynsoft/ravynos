#source: align-branch-4.s
#as: -malign-branch-boundary=32 -malign-branch=ret
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 64 a3 01 00 00 00 	fs mov %eax,%fs:0x1
   7:	55                   	push   %ebp
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
  20:	c3                   	ret
  21:	3e 3e 3e 55          	ds ds ds push %ebp
  25:	55                   	push   %ebp
  26:	64 a3 01 00 00 00    	mov    %eax,%fs:0x1
  2c:	89 e5                	mov    %esp,%ebp
  2e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  40:	c2 1e 00             	ret    \$0x1e
  43:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
#pass
