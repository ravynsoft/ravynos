#as: -mevexlig=256
#objdump: -dw
#name: x86-64 EVEX non-LIG insns with -mevexlig=256

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 f1 7d 08 7e 21    	\{evex\} vmovd %xmm4,\(%rcx\)
 +[a-f0-9]+:	62 f1 7d 08 7e e1    	\{evex\} vmovd %xmm4,%ecx
 +[a-f0-9]+:	62 f1 7d 08 6e 21    	\{evex\} vmovd \(%rcx\),%xmm4
 +[a-f0-9]+:	62 f1 7d 08 6e e1    	\{evex\} vmovd %ecx,%xmm4
 +[a-f0-9]+:	62 f1 fd 08 7e 21    	\{evex\} vmovq %xmm4,\(%rcx\)
 +[a-f0-9]+:	62 f1 fd 08 7e e1    	\{evex\} vmovq %xmm4,%rcx
 +[a-f0-9]+:	62 f1 fd 08 6e 21    	\{evex\} vmovq \(%rcx\),%xmm4
 +[a-f0-9]+:	62 f1 fd 08 6e e1    	\{evex\} vmovq %rcx,%xmm4
 +[a-f0-9]+:	62 f1 fe 08 7e f4    	\{evex\} vmovq %xmm4,%xmm6
 +[a-f0-9]+:	62 f3 7d 08 17 c0 00 	\{evex\} vextractps \$0x0,%xmm0,%eax
 +[a-f0-9]+:	62 f3 7d 08 17 00 00 	\{evex\} vextractps \$0x0,%xmm0,\(%rax\)
 +[a-f0-9]+:	62 f3 7d 08 14 c0 00 	\{evex\} vpextrb \$0x0,%xmm0,%eax
 +[a-f0-9]+:	62 f3 7d 08 14 00 00 	\{evex\} vpextrb \$0x0,%xmm0,\(%rax\)
 +[a-f0-9]+:	62 f1 7d 08 c5 c0 00 	\{evex\} vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	62 f3 7d 08 15 c0 00 	\{evex\} vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	62 f3 7d 08 15 00 00 	\{evex\} vpextrw \$0x0,%xmm0,\(%rax\)
 +[a-f0-9]+:	62 f3 7d 08 16 c0 00 	\{evex\} vpextrd \$0x0,%xmm0,%eax
 +[a-f0-9]+:	62 f3 7d 08 16 00 00 	\{evex\} vpextrd \$0x0,%xmm0,\(%rax\)
 +[a-f0-9]+:	62 f3 fd 08 16 c0 00 	\{evex\} vpextrq \$0x0,%xmm0,%rax
 +[a-f0-9]+:	62 f3 fd 08 16 00 00 	\{evex\} vpextrq \$0x0,%xmm0,\(%rax\)
 +[a-f0-9]+:	62 f3 7d 08 21 c0 00 	\{evex\} vinsertps \$0x0,%xmm0,%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 7d 08 21 00 00 	\{evex\} vinsertps \$0x0,\(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 7d 08 20 c0 00 	\{evex\} vpinsrb \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 7d 08 20 00 00 	\{evex\} vpinsrb \$0x0,\(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 f1 7d 08 c4 c0 00 	\{evex\} vpinsrw \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	62 f1 7d 08 c4 00 00 	\{evex\} vpinsrw \$0x0,\(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 7d 08 22 c0 00 	\{evex\} vpinsrd \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 7d 08 22 00 00 	\{evex\} vpinsrd \$0x0,\(%rax\),%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 fd 08 22 c0 00 	\{evex\} vpinsrq \$0x0,%rax,%xmm0,%xmm0
 +[a-f0-9]+:	62 f3 fd 08 22 00 00 	\{evex\} vpinsrq \$0x0,\(%rax\),%xmm0,%xmm0
#pass
