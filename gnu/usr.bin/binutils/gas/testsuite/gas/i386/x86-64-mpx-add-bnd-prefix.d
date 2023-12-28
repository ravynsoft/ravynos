#as: -madd-bnd-prefix
#warning_output: x86-64-mpx-add-bnd-prefix.e
#objdump: -drw
#name: Check -madd-bnd-prefix (x86-64)

.*: +file format .*


Disassembly of section .text:

0+ <.*>:
[ 	]*[a-f0-9]+:	f2 e8 0e 00 00 00    	bnd call 14 <foo>
[ 	]*[a-f0-9]+:	f2 ff 10             	bnd call \*\(%rax\)
[ 	]*[a-f0-9]+:	f2 74 08             	bnd je 14 <foo>
[ 	]*[a-f0-9]+:	f2 eb 05             	bnd jmp 14 <foo>
[ 	]*[a-f0-9]+:	f2 ff 23             	bnd jmp \*\(%rbx\)
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret

0+14 <foo>:
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 e8 f2 ff ff ff    	bnd call 14 <foo>
[ 	]*[a-f0-9]+:	48 01 c3             	add    %rax,%rbx
[ 	]*[a-f0-9]+:	e2 ed                	loop   14 <foo>
#pass
