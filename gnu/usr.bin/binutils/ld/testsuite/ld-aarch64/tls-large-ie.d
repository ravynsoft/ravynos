#source: tls-large-ie.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#notarget: aarch64_be-*-*
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
   10000:	58000121 	ldr	x1, 10024 \<test\+0x24\>
   10004:	10000102 	adr	x2, 10024 \<test\+0x24\>
   10008:	8b020021 	add	x1, x1, x2
   1000c:	d53bd042 	mrs	x2, tpidr_el0
   10010:	d2a00000 	movz	x0, #0x0, lsl #16
   10014:	f2800100 	movk	x0, #0x8
   10018:	f8606820 	ldr	x0, \[x1, x0\]
   1001c:	8b020000 	add	x0, x0, x2
   10020:	d503201f 	nop
   10024:	0000ffdc 	.word	0x0000ffdc
   10028:	00000000 	.word	0x00000000
