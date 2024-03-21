#objdump: -dw -Mx86-64
#name: .code64 directive in 32-bit mode.

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	a0 21 43 65 87 00 00 00 00 	movabs 0x87654321,%al
 +[a-f0-9]+:	48 b8 21 43 65 87 00 00 00 00 	movabs \$0x87654321,%rax
#pass
