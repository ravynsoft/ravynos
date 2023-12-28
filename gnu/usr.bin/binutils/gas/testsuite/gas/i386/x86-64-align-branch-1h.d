#source: x86-64-align-branch-1.s
#as: -mbranches-within-32B-boundaries -malign-branch-boundary=0
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	64 89 04 25 01 00 00 00 	mov    %eax,%fs:0x1
   8:	55                   	push   %rbp
   9:	55                   	push   %rbp
   a:	55                   	push   %rbp
   b:	48 89 e5             	mov    %rsp,%rbp
   e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  11:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  14:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  17:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  1d:	48 39 c5             	cmp    %rax,%rbp
  20:	74 5a                	je     (0x)?7c( .*)?
  22:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  25:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  28:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  2b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  2e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  31:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  34:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  37:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  3a:	5d                   	pop    %rbp
  3b:	5d                   	pop    %rbp
  3c:	74 3e                	je     (0x)?7c( .*)?
  3e:	5d                   	pop    %rbp
  3f:	74 3b                	je     (0x)?7c( .*)?
  41:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  44:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  47:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  4a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  4d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  50:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  53:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  56:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  59:	5d                   	pop    %rbp
  5a:	5d                   	pop    %rbp
  5b:	eb 25                	jmp    (0x)?82( .*)?
  5d:	eb 23                	jmp    (0x)?82( .*)?
  5f:	eb 21                	jmp    (0x)?82( .*)?
  61:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  64:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  67:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  6a:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  6d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  70:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  73:	5d                   	pop    %rbp
  74:	5d                   	pop    %rbp
  75:	48 39 c5             	cmp    %rax,%rbp
  78:	74 02                	je     (0x)?7c( .*)?
  7a:	eb 06                	jmp    (0x)?82( .*)?
#...
  7c:	8b 45 f4             	mov    -0xc\(%rbp\),%eax
  7f:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
#...
  82:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  88:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  8e:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  94:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  9a:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a0:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a6:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  ac:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b2:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b8:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  be:	eb c2                	jmp    (0x)?82( .*)?
  c0:	5d                   	pop    %rbp
  c1:	c3                   	ret
#pass
