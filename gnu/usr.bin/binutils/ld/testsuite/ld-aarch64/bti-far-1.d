#name: Check linker stubs with indirect calls handle BTI (shared lib).
#source: bti-far.s
#target: [check_shared_lib_support]
#as: -mabi=lp64
#ld: -shared -T bti-far.ld
#objdump: -dr

[^:]*: *file format elf64-.*aarch64


Disassembly of section \.plt:

0000000000018000 <\.plt>:
   18000:	d503245f 	bti	c
   18004:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
   18008:	900000d0 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_>
   1800c:	f9400e11 	ldr	x17, \[x16, #24\]
   18010:	91006210 	add	x16, x16, #0x18
   18014:	d61f0220 	br	x17
   18018:	d503201f 	nop
   1801c:	d503201f 	nop

0000000000018020 <foo@plt>:
   18020:	900000d0 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_>
   18024:	f9401211 	ldr	x17, \[x16, #32\]
   18028:	91008210 	add	x16, x16, #0x20
   1802c:	d61f0220 	br	x17
   18030:	14000004 	b	18040 <__foo_bti_veneer\+0x8>
   18034:	d503201f 	nop

0000000000018038 <__foo_bti_veneer>:
   18038:	d503245f 	bti	c
   1803c:	17fffff9 	b	18020 <foo@plt>

Disassembly of section \.text:

0000000000020000 <_start>:
   20000:	97ffe008 	bl	18020 <foo@plt>
   20004:	94000007 	bl	20020 <___veneer>
   20008:	94000001 	bl	2000c <baz>

000000000002000c <baz>:
   2000c:	d65f03c0 	ret
   20010:	1400000a 	b	20038 <___veneer\+0x18>
   20014:	d503201f 	nop

0000000000020018 <___bti_veneer>:
   20018:	d503245f 	bti	c
   2001c:	17fffffc 	b	2000c <baz>

0000000000020020 <___veneer>:
   20020:	90091910 	adrp	x16, 12340000 <foo>
   20024:	91012210 	add	x16, x16, #0x48
   20028:	d61f0200 	br	x16
	\.\.\.

Disassembly of section \.far:

0000000012340000 <foo>:
    12340000:	9400000c 	bl	12340030 <___veneer>

0000000012340004 <bar>:
    12340004:	9400000b 	bl	12340030 <___veneer>
    12340008:	94000004 	bl	12340018 <__foo_veneer>
    1234000c:	00000000 	udf	#0
    12340010:	14000010 	b	12340050 <___bti_veneer\+0x8>
    12340014:	d503201f 	nop

0000000012340018 <__foo_veneer>:
    12340018:	90f6e6d0 	adrp	x16, 18000 <\.plt>
    1234001c:	9100e210 	add	x16, x16, #0x38
    12340020:	d61f0200 	br	x16
	\.\.\.

0000000012340030 <___veneer>:
    12340030:	90f6e710 	adrp	x16, 20000 <_start>
    12340034:	91006210 	add	x16, x16, #0x18
    12340038:	d61f0200 	br	x16
	\.\.\.

0000000012340048 <___bti_veneer>:
    12340048:	d503245f 	bti	c
    1234004c:	17ffffee 	b	12340004 <bar>
