#objdump: -dt
#name:    
#source:  dbCC.s


.*:     file format elf32-s12z

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000001 l       .text	00000000 foo



Disassembly of section .text:

00000000 <foo-0x1>:
   0:	01          	nop

00000001 <foo>:
   1:	01          	nop
   2:	0b 84 ff ff 	dbne d0, foo
   6:	0b 88 ff fb 	dbne x, foo
   a:	0b 89 ff f7 	dbne y, foo
   e:	0b 8c f3 ff 	dbne.b \(\+y\), foo
  12:	f3 
