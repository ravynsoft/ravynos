#source: tlsle-symbol-offset.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: -dr
#...
Disassembly of section .text:

0000000000010000 <.text>:
 +10000:	d53bd040 	mrs	x0, tpidr_el0
 +10004:	91400400 	add	x0, x0, #0x1, lsl #12
 +10008:	91010000 	add	x0, x0, #0x40
 +1000c:	d65f03c0 	ret
