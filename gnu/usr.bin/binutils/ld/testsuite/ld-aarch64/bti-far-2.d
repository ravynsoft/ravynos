#name: Check linker stubs with indirect calls handle BTI (exe).
#source: bti-far.s
#as: -mabi=lp64
#ld: -T bti-far.ld
#objdump: -dr

[^:]*: *file format elf64-.*aarch64


Disassembly of section \.text:

0000000000020000 <_start>:
   20000:	9400000e 	bl	20038 <__foo_veneer>
   20004:	94000007 	bl	20020 <___veneer>
   20008:	94000001 	bl	2000c <baz>

000000000002000c <baz>:
   2000c:	d65f03c0 	ret
   20010:	14000010 	b	20050 <__foo_veneer\+0x18>
   20014:	d503201f 	nop

0000000000020018 <___bti_veneer>:
   20018:	d503245f 	bti	c
   2001c:	17fffffc 	b	2000c <baz>

0000000000020020 <___veneer>:
   20020:	90091910 	adrp	x16, 12340000 <foo>
   20024:	9100e210 	add	x16, x16, #0x38
   20028:	d61f0200 	br	x16
	\.\.\.

0000000000020038 <__foo_veneer>:
   20038:	90091910 	adrp	x16, 12340000 <foo>
   2003c:	91006210 	add	x16, x16, #0x18
   20040:	d61f0200 	br	x16
	\.\.\.

Disassembly of section \.far:

0000000012340000 <foo>:
    12340000:	94000008 	bl	12340020 <___veneer>

0000000012340004 <bar>:
    12340004:	94000007 	bl	12340020 <___veneer>
    12340008:	97fffffe 	bl	12340000 <foo>
    1234000c:	00000000 	udf	#0
    12340010:	1400000c 	b	12340040 <___bti_veneer\+0x8>
    12340014:	d503201f 	nop

0000000012340018 <__foo_bti_veneer>:
    12340018:	d503245f 	bti	c
    1234001c:	17fffff9 	b	12340000 <foo>

0000000012340020 <___veneer>:
    12340020:	90f6e710 	adrp	x16, 20000 <_start>
    12340024:	91006210 	add	x16, x16, #0x18
    12340028:	d61f0200 	br	x16
	\.\.\.

0000000012340038 <___bti_veneer>:
    12340038:	d503245f 	bti	c
    1234003c:	17fffff2 	b	12340004 <bar>
