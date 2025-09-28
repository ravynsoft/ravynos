#objdump: -drw
#name: x86-64 (ILP32) pcrel

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	b0 00                	mov    \$0x0,%al	1: R_X86_64_PC8	xtrn\+0x1
[ 	]*[a-f0-9]+:	66 b8 00 00          	mov    \$0x0,%ax	4: R_X86_64_PC16	xtrn\+0x2
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax	7: R_X86_64_PC32	xtrn\+0x1
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	e: R_X86_64_PC32	xtrn\+0x3
[ 	]*[a-f0-9]+:	b0 00                	mov    \$0x0,%al	13: R_X86_64_8	xtrn
[ 	]*[a-f0-9]+:	66 b8 00 00          	mov    \$0x0,%ax	16: R_X86_64_16	xtrn
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax	19: R_X86_64_32	xtrn
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	20: R_X86_64_32S	xtrn
[ 	]*[a-f0-9]+:	48 b8 00 00 00 00 00 00 00 00 	movabs \$0x0,%rax	26: R_X86_64_64	xtrn
[ 	]*[a-f0-9]+:	48 a1 00 00 00 00 00 00 00 00 	movabs 0x0,%rax	30: R_X86_64_64	xtrn
#pass
