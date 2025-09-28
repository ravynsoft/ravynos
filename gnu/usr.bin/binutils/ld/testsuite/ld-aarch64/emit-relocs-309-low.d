#name: aarch64-emit-relocs-309-low
#source: emit-relocs-309.s
#as:
#ld: -Ttext 0x100004 --section-start .got=0x0
#objdump: -dr
#...

Disassembly of section \.text:

0000000000100004 <_start>:
  100004:	d503201f 	nop
  100008:	58800000 	ldr	x0, 8 .*
