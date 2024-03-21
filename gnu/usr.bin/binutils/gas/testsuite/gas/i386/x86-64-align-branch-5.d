#source: align-branch-5.s
#as: -malign-branch-boundary=32 -malign-branch=jcc+fused+jmp
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	c1 e9 02             	shr    \$0x2,%ecx
#...
   3:	c1 e9 02             	shr    \$0x2,%ecx
   6:	c1 e9 02             	shr    \$0x2,%ecx
   9:	89 d1                	mov    %edx,%ecx
   b:	31 c0                	xor    %eax,%eax
   d:	c1 e9 02             	shr    \$0x2,%ecx
  10:	c1 e9 02             	shr    \$0x2,%ecx
  13:	c1 e9 02             	shr    \$0x2,%ecx
  16:	c1 e9 02             	shr    \$0x2,%ecx
  19:	c1 e9 02             	shr    \$0x2,%ecx
  1c:	c1 e9 02             	shr    \$0x2,%ecx
  1f:	f6 c2 02             	test   \$0x2,%dl
  22:	f3 ab                	rep stos %eax,%es:\(%rdi\)
  24:	75 dd                	jne    (0x)?3( .*)?
  26:	31 c0                	xor    %eax,%eax
  28:	c1 e9 02             	shr    \$0x2,%ecx
#...
  2b:	c1 e9 02             	shr    \$0x2,%ecx
  2e:	c1 e9 02             	shr    \$0x2,%ecx
  31:	89 d1                	mov    %edx,%ecx
  33:	31 c0                	xor    %eax,%eax
  35:	c1 e9 02             	shr    \$0x2,%ecx
  38:	c1 e9 02             	shr    \$0x2,%ecx
  3b:	c1 e9 02             	shr    \$0x2,%ecx
  3e:	f6 c2 02             	test   \$0x2,%dl
  41:	e8 00 00 00 00       	call   (0x)?46( .*)?
  46:	75 e3                	jne    (0x)?2b( .*)?
  48:	31 c0                	xor    %eax,%eax
#pass
