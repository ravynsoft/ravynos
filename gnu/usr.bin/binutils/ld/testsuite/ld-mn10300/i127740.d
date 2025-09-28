
tmpdir/i127740.x:     file format elf32-.*

Disassembly of section .text:

0+0100 <_main>:
 100:	2d 00 03[ 	]+mov	768,d1
 103:	cb[ 	]+nop	
 104:	cb[ 	]+nop	
 105:	cb[ 	]+nop	
	...

0+0200 <_dummy>:
 200:	00[ 	]+clr	d0
 201:	02 00 00[ 	]+movbu	d0,\(0 <_main-0x100>\)
 204:	df 00 00[ 	]+ret	\[\],0
	...
