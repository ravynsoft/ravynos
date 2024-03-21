#as: -mbranches-within-32B-boundaries
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
  20:	70 62                	jo     84 <.*>
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
  3d:	74 45                	je     84 <.*>
  3f:	5d                   	pop    %ebp
  40:	74 42                	je     84 <.*>
  42:	89 44 24 fc          	mov    %eax,-0x4\(%esp\)
  46:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  49:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  4c:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  4f:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  52:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  55:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  58:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  5b:	5d                   	pop    %ebp
  5c:	eb 2c                	jmp    8a <.*>
  5e:	66 90                	xchg   %ax,%ax
  60:	eb 28                	jmp    8a <.*>
  62:	eb 26                	jmp    8a <.*>
  64:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
  67:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  6a:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  6d:	5d                   	pop    %ebp
  6e:	5d                   	pop    %ebp
  6f:	40                   	inc    %eax
  70:	72 12                	jb     84 <.*>
  72:	36 36 89 45 fc       	ss mov %eax,%ss:-0x4\(%ebp\)
  77:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  7a:	89 7d f8             	mov    %edi,-0x8\(%ebp\)
  7d:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
  80:	21 c3                	and    %eax,%ebx
  82:	7c 06                	jl     8a <.*>
00000084 <label2>:
  84:	8b 45 f4             	mov    -0xc\(%ebp\),%eax
  87:	89 45 fc             	mov    %eax,-0x4\(%ebp\)
0000008a <label3>:
  8a:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  90:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  96:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  9c:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  a2:	89 75 0c             	mov    %esi,0xc\(%ebp\)
  a5:	e9 [0-9a-f ]+       	jmp    .*
  aa:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  b0:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  b6:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  bc:	89 b5 50 fb ff ff    	mov    %esi,-0x4b0\(%ebp\)
  c2:	89 75 00             	mov    %esi,0x0\(%ebp\)
  c5:	74 c3                	je     8a <.*>
  c7:	74 c1                	je     8a <.*>
#pass
