#objdump: -dw
#name: x86-64 long insns

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f2 f0 f0 f0 f2 f2 f2 f2 f2 f2 f0 f0 f3 0f 10 	repnz lock lock lock repnz repnz repnz repnz repnz repnz lock lock \(bad\)
[ 	]*[a-f0-9]+:	00 f2                	add    %dh,%dl
[ 	]*[a-f0-9]+:	f0 f0 f0 f2 f2 f2 f2 f0 f0 f0 f0 f3 0f 10 00 	lock lock lock repnz repnz repnz repnz lock lock lock lock movss \(%rax\),%xmm0
#pass
