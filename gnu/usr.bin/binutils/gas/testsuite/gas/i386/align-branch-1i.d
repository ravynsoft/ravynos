#source: align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch-prefix-size=0
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	65 a3 01 00 00 00    	mov    %eax,%gs:0x1
   6:	55                   	push   %ebp
   7:	55                   	push   %ebp
   8:	55                   	push   %ebp
   9:	55                   	push   %ebp
   a:	89 e5                	mov    %esp,%ebp
   c:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
   f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  12:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  15:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  18:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1e:	66 90                	xchg   %ax,%ax
  20:	39 c5                	cmp    %eax,%ebp
  22:	74 5e                	je     (0x)?82( .*)?
  24:	89 73 f4             	mov    %esi,-0xc\(%ebx\)
  27:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  2a:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  2d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  30:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  33:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  36:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  39:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3c:	5d                   	pop    %ebp
  3d:	5d                   	pop    %ebp
  3e:	5d                   	pop    %ebp
  3f:	90                   	nop
  40:	74 40                	je     (0x)?82( .*)?
  42:	5d                   	pop    %ebp
  43:	74 3d                	je     (0x)?82( .*)?
  45:	89 44 24 fc          	mov    %eax,-0x4\(%esp\)
  49:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  4c:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  4f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5e:	5d                   	pop    %ebp
  5f:	90                   	nop
  60:	eb 26                	jmp    (0x)?88( .*)?
  62:	eb 24                	jmp    (0x)?88( .*)?
  64:	eb 22                	jmp    (0x)?88( .*)?
  66:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
  69:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  6c:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  6f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  72:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  75:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  78:	5d                   	pop    %ebp
  79:	5d                   	pop    %ebp
  7a:	39 c5                	cmp    %eax,%ebp
  7c:	74 04                	je     (0x)?82( .*)?
  7e:	66 90                	xchg   %ax,%ax
  80:	eb 06                	jmp    (0x)?88( .*)?
#...
  82:	8b 45 f4             	mov    -0xc\(%ebp\),%eax
  85:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
#...
  88:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  8e:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  94:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  9a:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  a0:	89 75 0c             	mov    %esi,0xc\(%ebp\)
  a3:	e9 [0-9a-f ]+       	jmp    .*
  a8:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  ae:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  b4:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  ba:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  c0:	89 75 00             	mov    %esi,0x0\(%ebp\)
  c3:	74 c3                	je     (0x)?88( .*)?
  c5:	74 c1                	je     (0x)?88( .*)?
#pass
