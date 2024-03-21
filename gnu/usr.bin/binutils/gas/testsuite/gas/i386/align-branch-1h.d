#source: align-branch-1.s
#as: -mbranches-within-32B-boundaries -malign-branch-boundary=0
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
  1e:	39 c5                	cmp    %eax,%ebp
  20:	74 5a                	je     (0x)?7c( .*)?
  22:	89 73 f4             	mov    %esi,-0xc\(%ebx\)
  25:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  28:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  2b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  2e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3a:	5d                   	pop    %ebp
  3b:	5d                   	pop    %ebp
  3c:	5d                   	pop    %ebp
  3d:	74 3d                	je     (0x)?7c( .*)?
  3f:	5d                   	pop    %ebp
  40:	74 3a                	je     (0x)?7c( .*)?
  42:	89 44 24 fc          	mov    %eax,-0x4\(%esp\)
  46:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  49:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  4c:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  4f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5b:	5d                   	pop    %ebp
  5c:	eb 24                	jmp    (0x)?82( .*)?
  5e:	eb 22                	jmp    (0x)?82( .*)?
  60:	eb 20                	jmp    (0x)?82( .*)?
  62:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
  65:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  68:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  6b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  6e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  71:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  74:	5d                   	pop    %ebp
  75:	5d                   	pop    %ebp
  76:	39 c5                	cmp    %eax,%ebp
  78:	74 02                	je     (0x)?7c( .*)?
  7a:	eb 06                	jmp    (0x)?82( .*)?
#...
  7c:	8b 45 f4             	mov    -0xc\(%ebp\),%eax
  7f:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
#...
  82:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  88:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  8e:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  94:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  9a:	89 75 0c             	mov    %esi,0xc\(%ebp\)
  9d:	e9 [0-9a-f ]+       	jmp    .*
  a2:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  a8:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  ae:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  b4:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  ba:	89 75 00             	mov    %esi,0x0\(%ebp\)
  bd:	74 c3                	je     (0x)?82( .*)?
  bf:	74 c1                	je     (0x)?82( .*)?
#pass
