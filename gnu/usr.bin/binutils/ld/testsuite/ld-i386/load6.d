#as: --32
#ld: -shared -melf_i386
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	13 81 f8 ff ff ff    	adc    -0x8\(%ecx\),%eax
[ 	]*[a-f0-9]+:	03 99 f8 ff ff ff    	add    -0x8\(%ecx\),%ebx
[ 	]*[a-f0-9]+:	23 89 f8 ff ff ff    	and    -0x8\(%ecx\),%ecx
[ 	]*[a-f0-9]+:	3b 91 f8 ff ff ff    	cmp    -0x8\(%ecx\),%edx
[ 	]*[a-f0-9]+:	0b b9 f8 ff ff ff    	or     -0x8\(%ecx\),%edi
[ 	]*[a-f0-9]+:	1b b1 f8 ff ff ff    	sbb    -0x8\(%ecx\),%esi
[ 	]*[a-f0-9]+:	2b a9 f8 ff ff ff    	sub    -0x8\(%ecx\),%ebp
[ 	]*[a-f0-9]+:	33 a1 f8 ff ff ff    	xor    -0x8\(%ecx\),%esp
[ 	]*[a-f0-9]+:	85 89 f8 ff ff ff    	test   %ecx,-0x8\(%ecx\)
[ 	]*[a-f0-9]+:	13 81 fc ff ff ff    	adc    -0x4\(%ecx\),%eax
[ 	]*[a-f0-9]+:	03 99 fc ff ff ff    	add    -0x4\(%ecx\),%ebx
[ 	]*[a-f0-9]+:	23 89 fc ff ff ff    	and    -0x4\(%ecx\),%ecx
[ 	]*[a-f0-9]+:	3b 91 fc ff ff ff    	cmp    -0x4\(%ecx\),%edx
[ 	]*[a-f0-9]+:	0b b9 fc ff ff ff    	or     -0x4\(%ecx\),%edi
[ 	]*[a-f0-9]+:	1b b1 fc ff ff ff    	sbb    -0x4\(%ecx\),%esi
[ 	]*[a-f0-9]+:	2b a9 fc ff ff ff    	sub    -0x4\(%ecx\),%ebp
[ 	]*[a-f0-9]+:	33 a1 fc ff ff ff    	xor    -0x4\(%ecx\),%esp
[ 	]*[a-f0-9]+:	85 89 fc ff ff ff    	test   %ecx,-0x4\(%ecx\)
#pass
