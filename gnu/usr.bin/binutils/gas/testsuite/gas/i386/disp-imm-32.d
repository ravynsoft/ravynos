#objdump: -dw
#name: i386 displacements / immediates (32-bit)
#warning_output: disp-imm-32.e

.*: +file format .*

Disassembly of section .text:

0+ <disp_imm>:
[ 	]*[a-f0-9]+:	8b 40 01             	mov    0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:	62 f1 7c 48 28 40 01 	vmovaps 0x40\(%eax\),%zmm0
[ 	]*[a-f0-9]+:	83 c1 01             	add    \$0x1,%ecx
[ 	]*[a-f0-9]+:	8b 00                	mov    \(%eax\),%eax
[ 	]*[a-f0-9]+:	62 f1 7c 48 28 00    	vmovaps \(%eax\),%zmm0
[ 	]*[a-f0-9]+:	83 c1 00             	add    \$0x0,%ecx
[ 	]*[a-f0-9]+:	8b 40 ff             	mov    -0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:	62 f1 7c 48 28 40 ff 	vmovaps -0x40\(%eax\),%zmm0
[ 	]*[a-f0-9]+:	83 c1 ff             	add    \$0xffffffff,%ecx
[ 	]*[a-f0-9]+:	8b (40 01 +|80 01 00 00 00)    	mov    0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:	62 f1 7c 48 28 (40 01|80 40 00 00 00) 	vmovaps 0x40\(%eax\),%zmm0
[ 	]*[a-f0-9]+:	(83 c1 01 +|81 c1 01 00 00 00)    	add    \$0x1,%ecx
#pass
