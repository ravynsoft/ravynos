#source: align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch=jcc+jmp
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
  20:	74 5c                	je     (0x)?7e( .*)?
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
  3d:	74 3f                	je     (0x)?7e( .*)?
  3f:	5d                   	pop    %ebp
  40:	74 3c                	je     (0x)?7e( .*)?
  42:	89 44 24 fc          	mov    %eax,-0x4\(%esp\)
  46:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  49:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  4c:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  4f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5b:	5d                   	pop    %ebp
  5c:	eb 27                	jmp    (0x)?85( .*)?
  5e:	66 90                	xchg   %ax,%ax
  60:	eb 23                	jmp    (0x)?85( .*)?
  62:	eb 21                	jmp    (0x)?85( .*)?
  64:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
  67:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  6a:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  6d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  70:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  73:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  76:	5d                   	pop    %ebp
  77:	5d                   	pop    %ebp
  78:	39 c5                	cmp    %eax,%ebp
  7a:	74 02                	je     (0x)?7e( .*)?
  7c:	eb 07                	jmp    (0x)?85( .*)?
#...
  7e:	36 8b 45 f4          	mov    %ss:-0xc\(%ebp\),%eax
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
