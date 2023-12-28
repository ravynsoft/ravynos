#as: -madd-bnd-prefix
#warning_output: mpx-add-bnd-prefix.e
#objdump: -drw
#name: Check -madd-bnd-prefix

.*: +file format .*


Disassembly of section .text:

0+ <.*>:
[ 	]*[a-f0-9]+:	f2 e8 0e 00 00 00    	bnd call 14 <foo>
[ 	]*[a-f0-9]+:	f2 ff 10             	bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	f2 74 08             	bnd je 14 <foo>
[ 	]*[a-f0-9]+:	f2 eb 05             	bnd jmp 14 <foo>
[ 	]*[a-f0-9]+:	f2 ff 23             	bnd jmp \*\(%ebx\)
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret

0+14 <foo>:
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 c3                	bnd ret
[ 	]*[a-f0-9]+:	f2 e8 f2 ff ff ff    	bnd call 14 <foo>
[ 	]*[a-f0-9]+:	01 c3                	add    %eax,%ebx
[ 	]*[a-f0-9]+:	e2 ee                	loop   14 <foo>
#pass
