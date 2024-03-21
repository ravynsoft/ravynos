
.*:     file format.*


Disassembly of section destsect:

09000000 <[^>]*>:
 9000000:	e7fe      	b.n	9000000 <dest>

Disassembly of section .text:

000080.. <[^>]*>:
    80..:	f000 b802 	b.w	8008 <__dest_veneer>
    80..:	0000      	movs	r0, r0
	...

000080.. <[^>]*>:
    80..:	4778      	bx	pc
    80..:	e7fd      	b.n	.+ <.+>
    80..:	e59fc000 	ldr	ip, \[pc\]	@ 80.. <__dest_veneer\+0xc>
    80..:	e12fff1c 	bx	ip
    80..:	09000001 	.word	0x09000001
