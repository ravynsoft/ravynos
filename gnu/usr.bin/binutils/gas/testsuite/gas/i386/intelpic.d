#as: -J
#objdump: -dw
#name: i386 intelpic

.*: +file format .*

Disassembly of section .text:

0+ <gs_foo>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+1 <bar>:
[ 	]*[a-f0-9]+:	8d 83 14 00 00 00    	lea    0x14\(%ebx\),%eax
[ 	]*[a-f0-9]+:	8b 83 00 00 00 00    	mov    0x0\(%ebx\),%eax
[ 	]*[a-f0-9]+:	ff 24 85 0d 00 00 00 	jmp    \*0xd\(,%eax,4\)
[ 	]*[a-f0-9]+:	8d 83 14 00 00 00    	lea    0x14\(%ebx\),%eax
[ 	]*[a-f0-9]+:	ff 24 85 0d 10 00 00 	jmp    \*0x100d\(,%eax,4\)
[ 	]*[a-f0-9]+:	ff 24 85 28 10 00 00 	jmp    \*0x1028\(,%eax,4\)
[ 	]*[a-f0-9]+:	90                   	nop

0+29 <L11>:
[ 	]*[a-f0-9]+:	ff 24 85 29 10 00 00 	jmp    \*0x1029\(,%eax,4\)
[ 	]*[a-f0-9]+:	ff 24 85 37 10 00 00 	jmp    \*0x1037\(,%eax,4\)

0+37 <L12>:
[ 	]*[a-f0-9]+:	90                   	nop
#pass
