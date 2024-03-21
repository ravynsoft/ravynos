#source: x86-64-align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch=fused+jcc+jmp
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 64 64 64 89 04 25 01 00 00 00 	fs fs fs mov %eax,%fs:0x1
   b:	55                   	push   %rbp
   c:	55                   	push   %rbp
   d:	55                   	push   %rbp
   e:	48 89 e5             	mov    %rsp,%rbp
  11:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  14:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  17:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  20:	48 39 c5             	cmp    %rax,%rbp
  23:	74 5d                	je     (0x)?82( .*)?
  25:	2e 89 75 f4          	cs mov %esi,-0xc\(%rbp\)
  29:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  2c:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  2f:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  32:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  35:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  38:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3e:	5d                   	pop    %rbp
  3f:	5d                   	pop    %rbp
  40:	74 40                	je     (0x)?82( .*)?
  42:	5d                   	pop    %rbp
  43:	74 3d                	je     (0x)?82( .*)?
  45:	2e 89 45 fc          	cs mov %eax,-0x4\(%rbp\)
  49:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  4c:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  4f:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5e:	5d                   	pop    %rbp
  5f:	5d                   	pop    %rbp
  60:	eb 26                	jmp    (0x)?88( .*)?
  62:	eb 24                	jmp    (0x)?88( .*)?
  64:	eb 22                	jmp    (0x)?88( .*)?
  66:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  69:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  6c:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  6f:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  72:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  75:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  78:	5d                   	pop    %rbp
  79:	5d                   	pop    %rbp
  7a:	48 39 c5             	cmp    %rax,%rbp
  7d:	74 03                	je     (0x)?82( .*)?
  7f:	90                   	nop
  80:	eb 06                	jmp    (0x)?88( .*)?
#...
  82:	8b 45 f4             	mov    -0xc\(%rbp\),%eax
  85:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
#...
  88:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  8e:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  94:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  9a:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a0:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a6:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  ac:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b2:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b8:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  be:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  c4:	eb c2                	jmp    (0x)?88( .*)?
  c6:	5d                   	pop    %rbp
  c7:	c3                   	ret
#pass
