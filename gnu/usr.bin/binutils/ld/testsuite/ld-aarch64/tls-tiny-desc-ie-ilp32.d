#source: tls-tiny-desc-ie.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 tmpdir/tls-sharedlib-ilp32.so
#notarget: *-*-nto*
#objdump: -dr
#...

Disassembly of section .text:

00010000 \<test\>:
 +10000:	18080020 	ldr	w0, 20004 \<var\>
 +10004:	d503201f 	nop
 +10008:	d503201f 	nop
