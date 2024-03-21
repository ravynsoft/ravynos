#objdump: -dw
#name: x86 conditional operators in insn operands

.*: +file format .*

Disassembly of section .text:

0+ <cond>:
 +[a-f0-9]+:	b8 01 00 00 00 +	mov +\$(0x)?1,%eax
 +[a-f0-9]+:	b9 ff ff ff ff +	mov +\$0xffffffff,%ecx
 +[a-f0-9]+:	ba ff ff ff ff +	mov +\$0xffffffff,%edx
 +[a-f0-9]+:	bb ff ff ff ff +	mov +\$0xffffffff,%ebx
 +[a-f0-9]+:	bc ff ff ff ff +	mov +\$0xffffffff,%esp
 +[a-f0-9]+:	bd ff ff ff ff +	mov +\$0xffffffff,%ebp
 +[a-f0-9]+:	be ff ff ff ff +	mov +\$0xffffffff,%esi
 +[a-f0-9]+:	bf ff ff ff ff +	mov +\$0xffffffff,%edi
#pass
