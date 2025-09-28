#as: -a32
#source: xcoff-toc-1.s
#objdump: -dr
#name: XCOFF TOC reloc test 1

.*
Disassembly of section \.text:

00000000 <\.foo>:
   0:	80 22 00 00 	l       r1,0\(r2\)
			2: R_TOC	data-0x10010
   4:	4e 80 00 20 	br
