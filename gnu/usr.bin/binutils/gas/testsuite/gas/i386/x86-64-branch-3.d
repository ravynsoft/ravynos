#as: -J -mintel64
#objdump: -dwr -Mintel64
#name: x86-64 branch 3
#notarget: *-*-solaris*

.*: +file format .*

Disassembly of section .text:

0+ <bar-0xd>:
[ 	]*[a-f0-9]+:	66 e9 00 00 00 00    	data16 jmp 6 <bar-0x7>	2: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	66 48 e9 00 00 00 00 	data16 rex\.W jmp d <bar>	9: R_X86_64_PLT32	foo-0x4

0+d <bar>:
[ 	]*[a-f0-9]+:	89 c3                	mov    %eax,%ebx
[ 	]*[a-f0-9]+:	66 e8 00 00 00 00    	data16 call 15 <bar\+0x8>	11: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	66 48 e8 00 00 00 00 	data16 rex\.W call 1c <bar\+0xf>	18: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	66 c7 f8 00 00       	xbeginw 21 <bar\+0x14>	1f: R_X86_64_PC16	foo-0x2
[ 	]*[a-f0-9]+:	66 48 c7 f8 00 00 00 00 	data16 rex\.W xbegin 29 <bar\+0x1c>	25: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	48 ff 18             	lcallq \*\(%rax\)
[ 	]*[a-f0-9]+:	48 ff 29             	ljmpq  \*\(%rcx\)
#pass
