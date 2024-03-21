#name: aarch64-emit-relocs-309-up
#source: emit-relocs-309.s
#as:
#ld: -Ttext 0x0 --section-start .got=0xffff8
#objdump: -dr
#...

Disassembly of section \.text:

0000000000000000 <_start>:
   0:	d503201f 	nop
   4:	587fffe0 	ldr	x0, 100000 .*
