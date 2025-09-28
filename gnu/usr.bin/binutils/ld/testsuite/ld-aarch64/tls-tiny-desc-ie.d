#source: tls-tiny-desc-ie.s
#ld: -T relocs.ld -e0 tmpdir/tls-sharedlib.so
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
 +10000:	58080040 	ldr	x0, 20008 \<var\>
 +10004:	d503201f 	nop
 +10008:	d503201f 	nop
