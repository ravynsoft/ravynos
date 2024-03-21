
.*:     file format elf32-(big|little)arm.*


Disassembly of section .foo:

[0-9a-f]+ <_start>:
 +[0-9a-f]+:	f000 e800 	blx	2001018 <__func_to_branch_to_veneer>

[0-9a-f]+ <__func_to_branch_to_veneer>:
 +[0-9a-f]+:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 200101c <__func_to_branch_to_veneer\+0x4>
 +[0-9a-f]+:	........ 	.word	0x........

Disassembly of section .text:

[0-9a-f]+ <func_to_branch_to>:
 +[0-9a-f]+:	e12fff1e 	bx	lr
