#source: align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch=fused+jcc
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	65 65 65 a3 01 00 00 00 	gs gs mov %eax,%gs:0x1
   8:	55                   	push   %ebp
   9:	55                   	push   %ebp
   a:	55                   	push   %ebp
   b:	55                   	push   %ebp
   c:	89 e5                	mov    %esp,%ebp
   e:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  11:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  14:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  17:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  1d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  20:	39 c5                	cmp    %eax,%ebp
  22:	74 5b                	je     (0x)?7f( .*)?
  24:	3e 89 73 f4          	mov    %esi,%ds:-0xc\(%ebx\)
  28:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  2b:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  2e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3a:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  3d:	5d                   	pop    %ebp
  3e:	5d                   	pop    %ebp
  3f:	5d                   	pop    %ebp
  40:	74 3d                	je     (0x)?7f( .*)?
  42:	5d                   	pop    %ebp
  43:	74 3a                	je     (0x)?7f( .*)?
  45:	89 44 24 fc          	mov    %eax,-0x4\(%esp\)
  49:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  4c:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  4f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5b:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5e:	5d                   	pop    %ebp
  5f:	eb 24                	jmp    (0x)?85( .*)?
  61:	eb 22                	jmp    (0x)?85( .*)?
  63:	eb 20                	jmp    (0x)?85( .*)?
  65:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
  68:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  6b:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  6e:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  71:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  74:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  77:	5d                   	pop    %ebp
  78:	5d                   	pop    %ebp
  79:	39 c5                	cmp    %eax,%ebp
  7b:	74 02                	je     (0x)?7f( .*)?
  7d:	eb 06                	jmp    (0x)?85( .*)?
#...
  7f:	8b 45 f4             	mov    -0xc\(%ebp\),%eax
  82:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
#...
  85:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  8b:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  91:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  97:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  9d:	89 75 0c             	mov    %esi,0xc\(%ebp\)
  a0:	e9 [0-9a-f ]+       	jmp    .*
  a5:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  ab:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  b1:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  b7:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  bd:	89 75 00             	mov    %esi,0x0\(%ebp\)
  c0:	74 c3                	je     (0x)?85( .*)?
  c2:	74 c1                	je     (0x)?85( .*)?
#pass
