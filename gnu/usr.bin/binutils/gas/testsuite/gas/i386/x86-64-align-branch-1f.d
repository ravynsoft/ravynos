#source: x86-64-align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch=jcc+jmp
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
  20:	74 5d                	je     (0x)?7f( .*)?
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
  3c:	74 41                	je     (0x)?7f( .*)?
  3e:	2e 5d                	cs pop %rbp
  40:	74 3d                	je     (0x)?7f( .*)?
  42:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  45:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  48:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  4b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  4e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  51:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  54:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  57:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  5a:	5d                   	pop    %rbp
  5b:	5d                   	pop    %rbp
  5c:	eb 27                	jmp    (0x)?85( .*)?
  5e:	66 90                	xchg   %ax,%ax
  60:	eb 23                	jmp    (0x)?85( .*)?
  62:	eb 21                	jmp    (0x)?85( .*)?
  64:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  67:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  6a:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  6d:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  70:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  73:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  76:	5d                   	pop    %rbp
  77:	5d                   	pop    %rbp
  78:	48 39 c5             	cmp    %rax,%rbp
  7b:	74 02                	je     (0x)?7f( .*)?
  7d:	eb 06                	jmp    (0x)?85( .*)?
#...
  7f:	8b 45 f4             	mov    -0xc\(%rbp\),%eax
  82:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
#...
  85:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  8b:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  91:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  97:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  9d:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a3:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a9:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  af:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b5:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  bb:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  c1:	eb c2                	jmp    (0x)?85( .*)?
  c3:	5d                   	pop    %rbp
  c4:	c3                   	ret
#pass
