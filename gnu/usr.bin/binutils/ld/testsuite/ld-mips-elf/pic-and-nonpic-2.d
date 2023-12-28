#source: pic-and-nonpic-2a.s
#source: pic-and-nonpic-2b.s
#ld: -Tpic-and-nonpic-1.ld
#objdump: -dr

.*

Disassembly of section \.text:

00041000 <__start>:
   41000:	0c010406 	jal	41018 <\.pic\.foo@@V2>
   41004:	00000000 	nop
	\.\.\.

00041018 <\.pic\.foo@@V2>:
   41018:	3c190004 	lui	t9,0x4
   4101c:	27391020 	addiu	t9,t9,4128

00041020 <foo2>:
   41020:	03e00008 	jr	ra
   41024:	00000000 	nop
	\.\.\.
