#source: tls-tiny-gd-le.s
#ld: -T relocs.ld -e0
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
 +10000:	d53bd041 	mrs	x1, tpidr_el0
 +10004:	91400020 	add	x0, x1, #0x0, lsl #12
 +10008:	91004000 	add	x0, x0, #0x10
