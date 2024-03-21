#name: aarch64-farcall-bl-defsym
#source: farcall-bl-defsym.s
#as:
#ld: -Ttext 0x1000 --defsym=bar=0x8001000
#objdump: -dr
#...

Disassembly of section .text:

0+1000 <_start>:
[ \t]+1000:[ \t]+94000004[ \t]+bl[ \t]+1010 <__bar_veneer>
[ \t]+1004:[ \t]+d65f03c0[ \t]+ret
[ \t]+1008:[ \t]+14000008[ \t]+b[ \t]+1028 <__bar_veneer\+0x18>
[ \t]+100c:[ \t]+d503201f[ \t]+nop
0+1010 <__bar_veneer>:
[ \t]+1010:[ \t]+90040010[ \t]+adrp[ \t]+x16, 8001000 <bar>
[ \t]+1014:[ \t]+91000210[ \t]+add[ \t]+x16, x16, #0x0
[ \t]+1018:[ \t]+d61f0200[ \t]+br[ \t]+x16
	...
