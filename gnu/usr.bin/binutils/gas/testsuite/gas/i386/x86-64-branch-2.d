#as: -J
#objdump: -dwr
#name: x86-64 branch 2
#notarget: *-*-solaris*

.*: +file format .*

Disassembly of section .text:

0+ <bar-0xb>:
[ 	]*[a-f0-9]+:	66 e9 00 00          	jmpw   4 <bar-0x7>	2: R_X86_64_PC16	foo-0x2
[ 	]*[a-f0-9]+:	66 48 e9 00 00 00 00 	data16 rex\.W jmp b <bar>	7: R_X86_64_PLT32	foo-0x4

0+b <bar>:
[ 	]*[a-f0-9]+:	89 c3                	mov    %eax,%ebx
[ 	]*[a-f0-9]+:	66 e8 00 00          	callw  11 <bar\+0x6>	f: R_X86_64_PC16	foo-0x2
[ 	]*[a-f0-9]+:	66 48 e8 00 00 00 00 	data16 rex\.W call 18 <bar\+0xd>	14: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	66 c3                	retw
[ 	]*[a-f0-9]+:	66 c2 08 00          	retw   \$0x8
#pass
