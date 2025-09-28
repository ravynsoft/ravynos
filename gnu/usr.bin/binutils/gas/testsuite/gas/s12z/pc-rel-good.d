#objdump: -d -r -t
#name:    PC relative branches (close to the limit)
#source:  pc-rel-good.s


.*:     file format elf32-s12z

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00003fff l       .text	00000000 .label



Disassembly of section .text:

00000000 <.label-0x3fff>:
       0:	26 bf ff    	bne .label
	...

00003fff <.label>:
	...
    7fff:	26 c0 00    	bne .label
