#source: tls-tiny-gd-ie.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 tmpdir/tls-sharedlib-ilp32.so
#notarget: *-*-nto*
#objdump: -dr
#...

Disassembly of section .text:

00010000 \<test\>:
 +10000:	18080020 	ldr	w0, 20004 \<var\>
 +10004:	d53bd041 	mrs	x1, tpidr_el0
 +10008:	0b000020 	add	w0, w1, w0
