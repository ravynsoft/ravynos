#source: x86-64-align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch=fused+jcc
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
  23:	74 5b                	je     (0x)?80( .*)?
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
  40:	74 3e                	je     (0x)?80( .*)?
  42:	5d                   	pop    %rbp
  43:	74 3b                	je     (0x)?80( .*)?
  45:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  48:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  4b:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  4e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  51:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  54:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  57:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5d:	5d                   	pop    %rbp
  5e:	5d                   	pop    %rbp
  5f:	eb 25                	jmp    (0x)?86( .*)?
  61:	eb 23                	jmp    (0x)?86( .*)?
  63:	eb 21                	jmp    (0x)?86( .*)?
  65:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  68:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  6b:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  6e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  71:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  74:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  77:	5d                   	pop    %rbp
  78:	5d                   	pop    %rbp
  79:	48 39 c5             	cmp    %rax,%rbp
  7c:	74 02                	je     (0x)?80( .*)?
  7e:	eb 06                	jmp    (0x)?86( .*)?
#...
  80:	8b 45 f4             	mov    -0xc\(%rbp\),%eax
  83:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
#...
  86:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  8c:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  92:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  98:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  9e:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a4:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  aa:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b0:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b6:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  bc:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  c2:	eb c2                	jmp    (0x)?86( .*)?
  c4:	5d                   	pop    %rbp
  c5:	c3                   	ret
#pass
