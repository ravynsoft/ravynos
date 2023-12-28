#objdump: -dwMintel
#name: i386 long insns (Intel disassembly)
#source: long-1.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f2 f0 f0 f0 f2 f2 f2 f2 f2 f2 f0 f0 f3 0f 10 	repnz lock lock lock repnz repnz repnz repnz repnz repnz lock lock \(bad\)
[ 	]*[a-f0-9]+:	00 f2                	add    dl,dh
[ 	]*[a-f0-9]+:	f0 f0 f0 f2 f2 f2 f2 f0 f0 f0 f0 f3 0f 10 00 	lock lock lock repnz repnz repnz repnz lock lock lock lock movss xmm0,DWORD PTR \[eax\]
#pass
