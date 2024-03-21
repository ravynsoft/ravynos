#source: tls-tiny-desc-le.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0
#notarget: *-*-nto*
#objdump: -dr
#...

Disassembly of section .text:

00010000 \<test\>:
 +10000:	52a00000 	movz	w0, #0x0, lsl #16
 +10004:	72800100 	movk	w0, #0x8
 +10008:	d503201f 	nop
