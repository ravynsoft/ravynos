#source: x86-64-align-branch-1.s
#as: -malign-branch-boundary=32 -malign-branch=jcc
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
  20:	74 5b                	je     (0x)?7d( .*)?
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
  3c:	74 3f                	je     (0x)?7d( .*)?
  3e:	2e 5d                	cs pop %rbp
  40:	74 3b                	je     (0x)?7d( .*)?
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
  5c:	eb 25                	jmp    (0x)?83( .*)?
  5e:	eb 23                	jmp    (0x)?83( .*)?
  60:	eb 21                	jmp    (0x)?83( .*)?
  62:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
  65:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  68:	89 7d f8             	mov    %edi,-0x8\(%rbp\)
  6b:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  6e:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  71:	89 75 f4             	mov    %esi,-0xc\(%rbp\)
  74:	5d                   	pop    %rbp
  75:	5d                   	pop    %rbp
  76:	48 39 c5             	cmp    %rax,%rbp
  79:	74 02                	je     (0x)?7d( .*)?
  7b:	eb 06                	jmp    (0x)?83( .*)?
#...
  7d:	8b 45 f4             	mov    -0xc\(%rbp\),%eax
  80:	89 45 fc             	mov    %eax,-0x4\(%rbp\)
#...
  83:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  89:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  8f:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  95:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  9b:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a1:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  a7:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  ad:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b3:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  b9:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%rbp\)
  bf:	eb c2                	jmp    (0x)?83( .*)?
  c1:	5d                   	pop    %rbp
  c2:	c3                   	ret
#pass
