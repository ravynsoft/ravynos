
.*:     file format.*


Disassembly of section destsect:

00008002 <[^>]*>:
    8002:	f7ff fffe 	bl	8002 <dest>

Disassembly of section .text:

001080.. <[^>]*>:
  1080..:	f040 8002 	bne.w	108008 <__dest_veneer>
  1080..:	0000      	movs	r0, r0
	...

001080.. <[^>]*>:
  1080..:	f85f f000 	ldr.w	pc, \[pc\]	@ 10800c <__dest_veneer\+0x4>
  1080..:	00008003 	.word	0x00008003
