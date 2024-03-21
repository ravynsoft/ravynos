#as: -mbranches-within-32B-boundaries
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	c1 e9 02             	shr    \$0x2,%ecx
[0-9a-f]+ <.*>:
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
  1f:	80 fa 02             	cmp    \$0x2,%dl
  22:	70 df                	jo     [0-9a-fx]+ <.*>
  24:	2e 2e 2e 2e 31 c0    	cs cs cs cs xor %eax,%eax
  2a:	c1 e9 02             	shr    \$0x2,%ecx

[0-9a-f]+ <.*>:
  2d:	c1 e9 02             	shr    \$0x2,%ecx
  30:	c1 e9 02             	shr    \$0x2,%ecx
  33:	89 d1                	mov    %edx,%ecx
  35:	31 c0                	xor    %eax,%eax
  37:	c1 e9 02             	shr    \$0x2,%ecx
  3a:	c1 e9 02             	shr    \$0x2,%ecx
  3d:	c1 e9 02             	shr    \$0x2,%ecx
  40:	f6 c2 02             	test   \$0x2,%dl
  43:	75 e8                	jne    [0-9a-fx]+ <.*>
  45:	31 c0                	xor    %eax,%eax

[0-9a-f]+ <.*>:
  47:	c1 e9 02             	shr    \$0x2,%ecx
  4a:	c1 e9 02             	shr    \$0x2,%ecx
  4d:	89 d1                	mov    %edx,%ecx
  4f:	c1 e9 02             	shr    \$0x2,%ecx
  52:	c1 e9 02             	shr    \$0x2,%ecx
  55:	89 d1                	mov    %edx,%ecx
  57:	c1 e9 02             	shr    \$0x2,%ecx
  5a:	89 d1                	mov    %edx,%ecx
  5c:	31 c0                	xor    %eax,%eax
  5e:	ff c0                	inc    %eax
  60:	76 cb                	jbe    [0-9a-fx]+ <.*>
  62:	31 c0                	xor    %eax,%eax
#pass
